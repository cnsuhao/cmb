//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include <rtvl/rtvl_tensor.hxx>
#include <rtvl/rtvl_tensor_d.hxx>
#include <rtvl/rtvl_tokens.hxx>
#include <rtvl/rtvl_vote.hxx>
#include <rtvl/rtvl_voter.hxx>
#include <rtvl/rtvl_votee.hxx>
#include <rtvl/rtvl_votee_d.hxx>
#include <rtvl/rtvl_weight_smooth.hxx>
#include <rgtl/rgtl_serialize_istream.hxx>

#include <rgtl/rgtl_object_array_points.hxx>
#include <rgtl/rgtl_octree_cell_bounds.hxx>
#include <rgtl/rgtl_octree_objects.hxx>

#include <vnl/vnl_matrix_fixed.h>
#include <vnl/vnl_vector_fixed.h>

#include <vtksys/RegularExpression.hxx>

#include <vcl_cmath.h>
#include <vcl_cstdlib.h>
#include <vcl_exception.h>
#include <vcl_iostream.h>
#include <vcl_fstream.h>
#include <vcl_map.h>
#include <vcl_memory.h>
#include <vcl_string.h>
#include <vcl_vector.h>

#include <vtkCellArray.h>
#include <vtkDoubleArray.h>
#include <vtkIntArray.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkTimerLog.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkXMLPolyDataWriter.h>

#include <vtkLIDARReader.h>

class TerrainPoint
{
public:
  bool known;
  int level;
  double scale;
  vtkIdType id;
  double z;
  vnl_vector_fixed<double, 3> normal;
  TerrainPoint()
    {
    this->known = false;
    this->level = 0;
    this->scale = 0;
    this->id = -1;
    this->z = 0;
    this->normal.fill(0.0);
    }
};

class TerrainLevel
{
public:
  unsigned int Ni;
  unsigned int Nj;
  double Origin[2];
  double Spacing[2];
  vcl_vector<TerrainPoint> Terrain;
  TerrainPoint& GetPoint(unsigned int i, unsigned int j)
    {
    return this->Terrain[j*this->Ni + i];
    }
  TerrainPoint& GetPoint(unsigned int index[2])
    {
    return this->Terrain[index[1]*this->Ni + index[0]];
    }
  void Contribute(int i, int j, double w, double x, double y,
                  double& z, vnl_vector_fixed<double, 3>& n,
                  double& tw, int& level, double& scale)
    {
    if(i >= 0 && i < int(this->Ni) && j >= 0 && j < int(this->Nj))
      {
      TerrainPoint& tp = this->GetPoint(i, j);
      if(tp.known)
        {
        if(tp.normal[2] < 0.01)
          {
          vcl_cerr << "bad normal!!!" << vcl_endl
                   << "  n = " << tp.normal << vcl_endl;
          return;
          }
        double px = this->Origin[0] + this->Spacing[0]*i;
        double py = this->Origin[1] + this->Spacing[1]*j;
        double pz = tp.z;
        double nx = tp.normal[0];
        double ny = tp.normal[1];
        double nz = tp.normal[2];
        // Contribute the height of this plane at the given (x,y) location.
        z += w*(pz + ((px-x)*nx + (py-y)*ny)/nz);
        n += w*tp.normal;
        scale += w*tp.scale;
        tw += w;
        if(tp.level > level)
          {
          level = tp.level;
          }
        }
      }
    }
};

class TerrainExtract
{
public:
  TerrainExtract(const char* inname, const char* outname);
  void Extract2D();
  void Save();
  void SaveSegments();
  void NextLevel(const char* fname);
private:

  struct Location
  {
    double z;
    double saliency;
    double constraint;
  };

  void LevelInit(const char* fname);
  void SegmentInit();
  void SegmentVisualize();
  bool SegmentVote(Location& loc);
  void SegmentSearch();
  bool SegmentRefine(Location a, Location c);
  bool SegmentLocalMax(Location a, Location c);

  vcl_string OutName;

  rtvl_tokens<3> Tokens;
  vcl_auto_ptr< rtvl_weight<3> > TVW;

  double InOrigin[2];
  double InSize[2];
  rgtl_octree_cell_bounds<2> Bounds2D;
  rgtl_object_array_points<2> Points2D;
  vcl_auto_ptr< rgtl_octree_objects<2> > Objects2D;

  vtkSmartPointer<vtkPoints> OutPoints;
  vtkSmartPointer<vtkDoubleArray> OutNormals;
  vtkSmartPointer<vtkIntArray> OutLevels;
  vtkSmartPointer<vtkDoubleArray> OutScales;

  vtkSmartPointer<vtkTimerLog> Timer;

  vcl_auto_ptr<TerrainLevel> Level;

  vtkSmartPointer<vtkPoints> SegmentPoints;
  vtkSmartPointer<vtkCellArray> SegmentLines;

  typedef vcl_multimap<double, int> SegmentVotersType;
  SegmentVotersType SegmentVoters;
  unsigned int SegmentIJ[2];
  double SegmentXY[2];
  double SegmentRange[2];

  vnl_vector_fixed<double, 3> LastNormal;

  bool Verbose;
  unsigned int LevelIndex;
  unsigned int IntervalSearchCount;
  unsigned int IntervalSuccessCount;
  unsigned int VoteCount;
};

TerrainExtract::TerrainExtract(const char* inname, const char* outname)
{
  this->OutName = outname;
  this->Timer = vtkSmartPointer<vtkTimerLog>::New();
  this->Verbose = false;
  double bounds[6];
  std::string fileNameStr = inname;
  if (fileNameStr.find(".pts") != std::string::npos ||
    fileNameStr.find(".bin") != std::string::npos)
    {
    vtkSmartPointer<vtkLIDARReader> reader =
      vtkSmartPointer<vtkLIDARReader>::New();
    reader->SetFileName(inname);
    reader->Update();
    reader->GetOutput()->GetPoints()->GetBounds(bounds);
    }
  else
    {
    vtkSmartPointer<vtkXMLPolyDataReader> reader =
      vtkSmartPointer<vtkXMLPolyDataReader>::New();
    reader->SetFileName(inname);
    reader->Update();
    reader->GetOutput()->GetPoints()->GetBounds(bounds);
    }
  this->InOrigin[0] = bounds[0];
  this->InOrigin[1] = bounds[2];
  this->InSize[0] = bounds[1]-bounds[0];
  this->InSize[1] = bounds[3]-bounds[2];
}

void TerrainExtract::LevelInit(const char* fname)
{
  // Load serialized tokens from the file.
  vcl_ifstream fin(fname, vcl_ios::in | vcl_ios::binary);
  rgtl_serialize_istream loader(fin);
  loader >> this->Tokens;

  // Create the weight profile for this scale.
  this->TVW.reset(new rtvl_weight_smooth<3>(this->Tokens.scale));
  vcl_cout << "  voting at scale " << this->Tokens.scale << vcl_endl;

  // Copy the point x-y coordinates.
  unsigned int n = this->Tokens.points.get_number_of_points();
  this->Points2D.set_number_of_points(n);
  for(unsigned int i=0; i < n; ++i)
    {
    double p[3];
    this->Tokens.points.get_point(i, p);
    this->Points2D.set_point(i, p);
    }

  // Compute spatial data structure bounds.
  double bds[2][2];
  this->Points2D.compute_bounds(bds);
  this->Bounds2D.compute_bounds(bds, 1.01);

  // Create the 2-D spatial data structure.
  this->Objects2D.reset(
    new rgtl_octree_objects<2>(this->Points2D, this->Bounds2D, 8)
    );

  // Allocate the terrain representation for this level.
  double const factor = 1;
  this->Level.reset(new TerrainLevel);
  this->Level->Origin[0] = this->InOrigin[0];
  this->Level->Origin[1] = this->InOrigin[1];
  this->Level->Spacing[0] = this->Tokens.scale * factor;
  this->Level->Spacing[1] = this->Tokens.scale * factor;
  this->Level->Ni = 1 + int(vcl_ceil(this->InSize[0] /
                                     this->Level->Spacing[0]));
  this->Level->Nj = 1 + int(vcl_ceil(this->InSize[1] /
                                     this->Level->Spacing[1]));
  this->Level->Terrain.resize(this->Level->Ni * this->Level->Nj);

  // Initialize visualization of this level.
  this->OutPoints = vtkSmartPointer<vtkPoints>::New();
  this->SegmentPoints = vtkSmartPointer<vtkPoints>::New();
  this->SegmentLines = vtkSmartPointer<vtkCellArray>::New();
  this->OutNormals = vtkSmartPointer<vtkDoubleArray>::New();
  this->OutNormals->SetName("Normal");
  this->OutNormals->SetNumberOfComponents(3);
  this->OutLevels = vtkSmartPointer<vtkIntArray>::New();
  this->OutLevels->SetName("Level");
  this->OutScales = vtkSmartPointer<vtkDoubleArray>::New();
  this->OutScales->SetName("Scale");
}

void TerrainExtract::Save()
{
  vtkSmartPointer<vtkCellArray> verts = vtkSmartPointer<vtkCellArray>::New();
  vtkSmartPointer<vtkCellArray> polys = vtkSmartPointer<vtkCellArray>::New();
  vcl_vector<bool> pt_done(this->OutPoints->GetNumberOfPoints(), false);
  for(unsigned int j=0; j < this->Level->Nj; ++j)
    {
    for(unsigned int i=0; i < this->Level->Ni; ++i)
      {
      vtkIdType ids[4];
      vtkIdType nids = 0;
      {
      TerrainPoint& tp = this->Level->GetPoint(i, j);
      if(tp.known) { ids[nids++] = tp.id; }
      }
      if((i+1) < this->Level->Ni)
        {
        TerrainPoint& tp = this->Level->GetPoint(i+1, j);
        if(tp.known) { ids[nids++] = tp.id; }
        }
      if((i+1) < this->Level->Ni && (j+1) < this->Level->Nj)
        {
        TerrainPoint& tp = this->Level->GetPoint(i+1, j+1);
        if(tp.known) { ids[nids++] = tp.id; }
        }
      if((j+1) < this->Level->Nj)
        {
        TerrainPoint& tp = this->Level->GetPoint(i, j+1);
        if(tp.known) { ids[nids++] = tp.id; }
        }
      if(nids > 2)
        {
        polys->InsertNextCell(nids, ids);
        for(vtkIdType k=0; k < nids; ++k)
          {
          pt_done[ids[k]] = true;
          }
        }
      }
    }
  for(vtkIdType id = 0; id < this->OutPoints->GetNumberOfPoints(); ++id)
    {
    if(!pt_done[id])
      {
      verts->InsertNextCell(1, &id);
      }
    }
  vtkSmartPointer<vtkPolyData> pd = vtkSmartPointer<vtkPolyData>::New();
  pd->SetPoints(this->OutPoints);
  pd->SetVerts(verts);
  pd->SetPolys(polys);
  pd->GetPointData()->SetScalars(this->OutScales);
  pd->GetPointData()->AddArray(this->OutLevels);
  pd->GetPointData()->SetNormals(this->OutNormals);
  vcl_string fname = this->OutName;
  char buf[64];
  sprintf(buf, "_terrain_%02u.vtp", this->LevelIndex);
  fname += buf;
  vtkSmartPointer<vtkXMLPolyDataWriter> writer =
    vtkSmartPointer<vtkXMLPolyDataWriter>::New();
  writer->SetFileName(fname.c_str());
  writer->SetInput(pd);
  writer->Write();
}

void TerrainExtract::Extract2D()
{
  this->IntervalSearchCount = 0;
  this->IntervalSuccessCount = 0;
  this->VoteCount = 0;
  this->Timer->StartTimer();
  for(unsigned int j=0; j < this->Level->Nj; ++j)
    {
    for(unsigned int i=0; i < this->Level->Ni; ++i)
      {
      this->SegmentIJ[0] = i;
      this->SegmentIJ[1] = j;
      this->SegmentXY[0] = this->Level->Origin[0] + this->Level->Spacing[0]*i;
      this->SegmentXY[1] = this->Level->Origin[1] + this->Level->Spacing[1]*j;
      this->SegmentInit();
      this->SegmentVisualize();
      this->SegmentSearch();
      }
    }
  this->Timer->StopTimer();
  vcl_cout << "  extraction time " << this->Timer->GetElapsedTime()
           << vcl_endl;
  vcl_cout << "  intervals searched " << this->IntervalSearchCount
           << vcl_endl;
  vcl_cout << "  intervals succeeded " << this->IntervalSuccessCount
           << vcl_endl;
    vcl_cout << "  votes cast " << this->VoteCount << vcl_endl;
}

void TerrainExtract::SaveSegments()
{
  vtkSmartPointer<vtkPolyData> pd = vtkSmartPointer<vtkPolyData>::New();
  pd->SetPoints(this->SegmentPoints);
  pd->SetLines(this->SegmentLines);
  vtkSmartPointer<vtkXMLPolyDataWriter> writer =
    vtkSmartPointer<vtkXMLPolyDataWriter>::New();
  vcl_string fname = this->OutName;
  char buf[64];
  sprintf(buf, "_terrain_segments_%02u.vtp", this->LevelIndex);
  fname += buf;
  writer->SetFileName(fname.c_str());
  writer->SetInput(pd);
  writer->Write();
}

void TerrainExtract::SegmentInit()
{
  this->SegmentVoters.clear();

  // Select an initial search range.
  TerrainPoint& tp = this->Level->GetPoint(this->SegmentIJ);
  if(tp.known)
    {
    this->SegmentRange[0] = tp.z - this->Tokens.scale / 2;
    this->SegmentRange[1] = tp.z + this->Tokens.scale / 2;
    }
  else
    {
    this->SegmentRange[0] = -1000;
    this->SegmentRange[1] = +1000;
    }

  // Lookup voters that contribute to points on this line segment.
  vcl_vector<int> voter_ids;
  int num_voters =
    this->Objects2D->query_sphere(this->SegmentXY, 3*this->Tokens.scale,
                                  voter_ids);
  for(int i=0; i < num_voters; ++i)
    {
    int id = voter_ids[i];
    double p[3];
    this->Tokens.points.get_point(id, p);
    rtvl_tensor<3> const& tensor = this->Tokens.tokens[id];
    double flatness = tensor.saliency(0)/tensor.lambda(0);
    double const flatness_threshold = 0;
    if(flatness >= flatness_threshold &&
       p[2] + 3*this->Tokens.scale > this->SegmentRange[0] &&
       p[2] - 3*this->Tokens.scale < this->SegmentRange[1])
      {
      SegmentVotersType::value_type entry(p[2], id);
      this->SegmentVoters.insert(entry);
      }
    }

  // Shrink the range to within reach of the voters.  This is an
  // optimization, and should not be propagated to another level.
  if(!this->SegmentVoters.empty())
    {
    // Compute the search range along the line within reach of the
    // voters.
    double range[2] =
      {this->SegmentVoters.begin()->first - 3*this->Tokens.scale,
       this->SegmentVoters.rbegin()->first + 3*this->Tokens.scale};

    // Shrink the line segment if possible.
    if(range[0] > this->SegmentRange[0])
      {
      this->SegmentRange[0] = range[0];
      }
    if(range[1] < this->SegmentRange[1])
      {
      this->SegmentRange[1] = range[1];
      }
    }
}

void TerrainExtract::SegmentVisualize()
{
  if(this->SegmentVoters.empty())
    {
    return;
    }

  // Visualize the segment.
  double p[2][3];
  p[0][0] = p[1][0] = this->SegmentXY[0];
  p[0][1] = p[1][1] = this->SegmentXY[1];
  p[0][2] = this->SegmentRange[0];
  p[1][2] = this->SegmentRange[1];

  vtkIdType ids[2];
  ids[0] = this->SegmentPoints->InsertNextPoint(p[0]);
  ids[1] = this->SegmentPoints->InsertNextPoint(p[1]);
  this->SegmentLines->InsertNextCell(2, ids);
}

bool TerrainExtract::SegmentVote(Location& loc)
{
  // Find the voters in reach.
  double sigma = this->Tokens.scale;
  SegmentVotersType::iterator first =
    this->SegmentVoters.lower_bound(loc.z - 3*sigma);
  SegmentVotersType::iterator last =
    this->SegmentVoters.upper_bound(loc.z + 3*sigma);
  if(first == last)
    {
    loc.saliency = 0;
    loc.constraint = 0;
    return false;
    }

  // Cast a vote with every voter.
  vnl_vector_fixed<double, 3> votee_location;
  votee_location(0) = this->SegmentXY[0];
  votee_location(1) = this->SegmentXY[1];
  votee_location(2) = loc.z;
  vnl_matrix_fixed<double, 3, 3> votee_tensor(0.0);
  vnl_matrix_fixed<double, 3, 3> votee_tensor_d[3];
  votee_tensor_d[0].fill(0.0);
  votee_tensor_d[1].fill(0.0);
  votee_tensor_d[2].fill(0.0);
  rtvl_votee_d<3> votee(votee_location, votee_tensor, votee_tensor_d);

  rtvl_weight<3>& tvw = *this->TVW;

  vnl_vector_fixed<double, 3> voter_location;
  for(SegmentVotersType::iterator vi = first; vi != last; ++vi)
    {
    int j = vi->second;
    this->Tokens.points.get_point(j, voter_location.data_block());
    rtvl_voter<3> voter(voter_location, this->Tokens.tokens[j]);
    rtvl_vote(voter, votee, tvw, false);
    ++this->VoteCount;
    }

  rtvl_tensor_d<3> tensor(votee_tensor, votee_tensor_d);
  vnl_vector_fixed<double, 3> dsal;
  tensor.saliency_d(0, dsal);
  vnl_vector_fixed<double, 3>& normal = this->LastNormal;
  normal = tensor.basis(0);
  if(normal[2] < 0)
    {
    normal = -normal;
    }

  loc.saliency = tensor.saliency(0);
  loc.constraint = dot_product(normal, dsal);
  return true;
}

#if 0
  if(this->Verbose)
    {
    fprintf(stderr, "normal = %g %g %g\n", normal[0], normal[1], normal[2]);
    fprintf(stderr, "dsal   = %g %g %g\n", dsal[0], dsal[1], dsal[2]);
    vnl_vector_fixed<double, 3> const& ev0 = tensor.basis(0);
    vnl_vector_fixed<double, 3> const& ev1 = tensor.basis(1);
    fprintf(stderr, "ev0 = %g %g %g\n", ev0[0], ev0[1], ev0[2]);
    fprintf(stderr, "ev1 = %g %g %g\n", ev1[0], ev1[1], ev1[2]);
    fflush(stderr);
    vcl_cerr << "tdz =\n" << votee_tensor_d[2] << vcl_endl;
    }
#endif

void TerrainExtract::SegmentSearch()
{
  double step = this->Tokens.scale / 2;

  // Shrink the step size if the range is small.
  double size = this->SegmentRange[1] - this->SegmentRange[0];
  if(4*step > size)
    {
    step = size/4;
    }

  int prev_locs_count = 0;
  Location locs[3];
  double z = this->SegmentRange[0];
  for(;;)
    {
    locs[prev_locs_count].z = z;
    if(this->SegmentVote(locs[prev_locs_count]))
      {
      if(prev_locs_count == 2)
        {
        if(locs[0].saliency < locs[1].saliency &&
           locs[2].saliency < locs[1].saliency)
          {
          if(this->SegmentRefine(locs[0], locs[1]))
            {
            return;
            }
          if(this->SegmentRefine(locs[1], locs[2]))
            {
            return;
            }
          }
        locs[0] = locs[1];
        locs[1] = locs[2];
        }
      else
        {
        ++prev_locs_count;
        }
      }
    else
      {
      prev_locs_count = 0;
      }

    if(z >= this->SegmentRange[1])
      {
      break;
      }
    z += step;
    if(z >= this->SegmentRange[1])
      {
      z = this->SegmentRange[1];
      }
    }

  // No terrain was found.  Use information from previous scale.
  TerrainPoint& tp = this->Level->GetPoint(this->SegmentIJ);
  if(tp.known)
    {
    vnl_vector_fixed<double, 3> p;
    p(0) = this->SegmentXY[0];
    p(1) = this->SegmentXY[1];
    p(2) = tp.z;
    tp.id = this->OutPoints->InsertNextPoint(p.data_block());
    this->OutNormals->InsertNextTupleValue(tp.normal.data_block());
    this->OutLevels->InsertNextTupleValue(&tp.level);
    this->OutScales->InsertNextTupleValue(&tp.scale);
    }
}

bool TerrainExtract::SegmentRefine(Location a, Location c)
{
  if(a.constraint > 0 && c.constraint < 0)
    {
    return this->SegmentLocalMax(a, c);
    }
  else
    {
    return false;
    }
}

bool TerrainExtract::SegmentLocalMax(Location a, Location c)
{
  if(this->Verbose)
    {
    fprintf(stderr, "Finding local max between %g and %g\n", a.z, c.z);
    }
  vnl_vector_fixed<double, 3> p;
  p(0) = this->SegmentXY[0];
  p(1) = this->SegmentXY[1];

  ++this->IntervalSearchCount;

  // Given a Lipshitz constant (bound on derivative) for the saliency
  // function, we can stop the search when the bracket width is small
  // enough and the current saliency low enough that the saliency
  // maximum could not possibly be high enough.
  //
  // TODO: Actually get the constant.  This value is too conservative.
  double const max_width = this->Tokens.scale/8;
  double const min_saliency = 10;

  int count = 0;
  double saliency = 0;
  double constraint = 10000;
  double const accuracy = this->Tokens.scale/100;
  while((c.z-a.z > accuracy) && vcl_fabs(constraint) > 1e-8)
    {
    // When the saliency is not high enough, we want to shrink the
    // window as quickly as possible to end early.
    double wa = 0.5;
    double wc = 0.5;
    if(count < 10 && saliency > min_saliency)
      {
      // When the saliency is high enough, we use linear interpolation
      // to predict the location of the zero for the first few steps.
      // This reduces the number of iterations to converge.
      wa = -c.constraint;
      wc = a.constraint;
      double wt = wa+wc;
      wa /= wt;
      wc /= wt;

      // Make sure the new point is not too close to either side.
      double const clip = 1.0/256;
      if(wa < clip)
        {
        wa = clip;
        wc = 1.0-clip;
        }
      else if(wc < clip)
        {
        wa = 1.0-clip;
        wc = clip;
        }
      }
    Location b = {wa*a.z + wc*c.z, 0, 0};
    p(2) = b.z;
    if(!this->SegmentVote(b))
      {
      fprintf(stderr, "Vote continuity failure!!!!!\n");
      return false;
      }
    if(this->Verbose)
      {
      fprintf(stderr, "Vote at %1.15g gave sal=%g, cons=%g\n",
              b.z, b.saliency, b.constraint);
      }
    saliency = b.saliency;
    constraint = b.constraint;
    if(b.constraint > 0)
      {
      a = b;
      if(this->Verbose)
        {
        fprintf(stderr, "  updating 'a'\n");
        }
      }
    else
      {
      c = b;
      if(this->Verbose)
        {
        fprintf(stderr, "  updating 'c'\n");
        }
      }
    if(++count >= 50)
      {
      if(b.saliency > 200)
        {
        fprintf(stderr,
                "Failed to converge after %d iterations i=%d j=%d\n"
                "  bracket = %g %g, width %g\n"
                "  a.saliency = %g, a.constraint = %g\n"
                "  c.saliency = %g, c.constraint = %g\n",
                count, this->SegmentIJ[0], this->SegmentIJ[1],
                a.z, c.z, c.z-a.z,
                a.saliency, a.constraint, c.saliency, c.constraint);
        }
      break;
      }
    if(b.saliency < min_saliency && (c.z-a.z < max_width))
      {
      if(this->Verbose)
        {
        fprintf(stderr, "Dropping early after %d iters %g %g, width=%g\n",
                count, a.z, c.z, c.z-a.z);
        }
      break;
      }
    }
  if(saliency > 200)
    {
    // Make sure the normal direction is acceptable.
    TerrainPoint& tp = this->Level->GetPoint(this->SegmentIJ);
    if(tp.known)
      {
      if(dot_product(tp.normal, this->LastNormal) < 0.866)
        {
        if(this->Verbose)
          {
          vcl_cerr << "  normal jumped:\n"
                   << "    old = " << tp.normal << "\n"
                   << "    new = " << this->LastNormal << vcl_endl;
          }
        // TODO: Should abort the whole segment?
        return false;
        }
      }
    if(this->LastNormal[2] < 0.1)
      {
      vcl_cerr << "  terrain too steep, n = "
               << this->LastNormal << vcl_endl;
      return false;
      }

    ++this->IntervalSuccessCount;
    tp.id = this->OutPoints->InsertNextPoint(p.data_block());
    this->OutNormals->InsertNextTupleValue(this->LastNormal.data_block());
    int level = this->LevelIndex;
    double scale = this->Tokens.scale;
    this->OutLevels->InsertNextTupleValue(&level);
    this->OutScales->InsertNextTupleValue(&scale);
    tp.known = true;
    tp.level = this->LevelIndex;
    tp.scale = this->Tokens.scale;
    tp.z = p(2);
    tp.normal = this->LastNormal;
    if(this->Verbose)
      {
      fprintf(stderr, "Found at %g in %d iterations\n", p(2), count);
      }
    return true;
    }
  else
    {
    if(this->Verbose)
      {
      fprintf(stderr, "Did not find here!\n");
      }
    return false;
    }
}

void TerrainExtract::NextLevel(const char* fname)
{
  vcl_auto_ptr<TerrainLevel> prevLevel = this->Level;
  this->LevelInit(fname);

  {
  vtksys::RegularExpression lvl(".*level_([0-9]+).tvl");
  unsigned int index = 0;
  if(lvl.find(fname))
    {
    sscanf(lvl.match(1).c_str(), "%u", &index);
    }
  this->LevelIndex = index;
  }

  // Short-circuit for the first level.
  if(!prevLevel.get())
    {
    return;
    }

  // Initialize the next level with bilinear interpolation of this
  // level.
  for(unsigned int j=0; j < this->Level->Nj; ++j)
    {
    for(unsigned int i=0; i < this->Level->Ni; ++i)
      {
      double p[2] = {this->Level->Origin[0] + this->Level->Spacing[0]*i,
                     this->Level->Origin[1] + this->Level->Spacing[1]*j};
      double prev_index[2] =
        {(p[0] - prevLevel->Origin[0]) / prevLevel->Spacing[0],
         (p[1] - prevLevel->Origin[1]) / prevLevel->Spacing[1]};
      int prev_i = int(vcl_floor(prev_index[0]));
      int prev_j = int(vcl_floor(prev_index[1]));
      double dx = prev_index[0] - prev_i;
      double dy = prev_index[1] - prev_j;
      double z = 0;
      double w = 0;
      vnl_vector_fixed<double, 3> n;
      n.fill(0.0);
      int level = -1;
      double scale = 0;
      prevLevel->Contribute(prev_i, prev_j, (1-dx)*(1-dy), p[0], p[1],
                            z, n, w, level, scale);
      prevLevel->Contribute(prev_i+1, prev_j, (dx)*(1-dy), p[0], p[1],
                            z, n, w, level, scale);
      prevLevel->Contribute(prev_i, prev_j+1, (1-dx)*(dy), p[0], p[1],
                            z, n, w, level, scale);
      prevLevel->Contribute(prev_i+1, prev_j+1, (dx)*(dy), p[0], p[1],
                            z, n, w, level, scale);
      if(w > 0)
        {
        z /= w;
        scale /= w;
        n.normalize();
        TerrainPoint& tp = this->Level->GetPoint(i, j);
        tp.known = true;
        tp.level = level;
        tp.scale = scale;
        tp.z = z;
        tp.normal = n;
        }
      }
    }
}

int main(int argc, const char* argv[])
{
  int idxMax = 0;
  int idxMin = 0;
  if(!(argc == 5 &&
       sscanf(argv[3], "%d", &idxMax) == 1 &&
       sscanf(argv[4], "%d", &idxMin) == 1))
    {
    fprintf(stderr, "Specify: orig.vtp name max min\n");
    return 1;
    }
  try
    {
    char buf[64];
    TerrainExtract t(argv[1], argv[2]);
    for(int a = idxMax; a >= idxMin; --a)
      {
      vcl_string fname = argv[2];
      sprintf(buf, "_level_%02u.tvl", a);
      fname += buf;
      vcl_cout << vcl_endl << "Processing " << fname << vcl_endl;
      t.NextLevel(fname.c_str());
      t.Extract2D();
      t.SaveSegments();
      t.Save();
      }
    }
  catch(vcl_exception& e)
    {
    vcl_cerr << "caught exception: " << e.what() << vcl_endl;
    return 1;
    }
  catch(...)
    {
    vcl_cerr << "caught unknown exception!" << vcl_endl;
    return 1;
    }
  return 0;
}

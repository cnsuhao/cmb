//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkArcDepressFilter.h"

#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkExecutive.h"
#include "vtkFloatArray.h"
#include "vtkGenericCell.h"
#include "vtkImplicitFunction.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkLine.h"
#include "vtkMergePoints.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkTransform.h"
#include "vtkTriangle.h"
#include "vtkIncrementalPointLocator.h"
#include "vtkPiecewiseFunction.h"
#include <vtkPolyDataNormals.h>
#include "vtkSmartPointer.h"
#include "vtkKochanekSpline.h"

#include <algorithm>
#include <math.h>

vtkStandardNewMacro(vtkArcDepressFilter);

class ArcDepressFunction
{
public:
  ArcDepressFunction() {}
  virtual ~ArcDepressFunction(){}
  virtual double evaluate(double d, double minF, double maxF) = 0;
  virtual double evaluate(double d) = 0;
  virtual void addPoint(double w, double v, double s, double m) = 0;
  virtual void clearPoints() = 0;
};

class ArcDepressPiecewiseFun: public ArcDepressFunction
{
public:
  ArcDepressPiecewiseFun()
    {
      fun = vtkPiecewiseFunction::New();
    }
  virtual ~ArcDepressPiecewiseFun()
  {
    fun->Delete();
  }
  virtual double evaluate(double d, double minF, double maxF)
  {
    double wd = fun->GetValue(d);
    return (1-wd)*minF+(wd)*maxF;
  }
  virtual double evaluate(double d)
  {
    return fun->GetValue(d);
  }
  void addPoint(double w, double v, double s, double m)
  {
    fun->AddPoint(w, v, s, m);
  }
  void clearPoints()
  {
    fun->RemoveAllPoints();
  }

  vtkPiecewiseFunction * fun;
};

class ArcDepressSplineFun: public ArcDepressFunction
{
public:
  ArcDepressSplineFun()
  {
    fun = vtkKochanekSpline::New();
    computed= false;
  }
  virtual ~ArcDepressSplineFun()
  {
    fun->Delete();
  }
  virtual double evaluate(double d, double minF, double maxF)
  {
    if(!computed)
    {
      fun->Compute();
      computed = true;
    }

    double wd = fun->Evaluate(d);
    double dist = maxF-minF;
    return minF+(wd)*dist;
  }

  virtual double evaluate(double d)
  {
    if(!computed)
    {
      fun->Compute();
      computed = true;
    }
    double tmp = fun->Evaluate(d);
    if(tmp < 0) tmp = 0;
    else if( tmp > 1) tmp = 1;
    return tmp;
  }


  void addPoint(double w, double v, double /*s*/, double /*m*/ )
  {
    if(fun->GetNumberOfPoints()!=0 && prev == w)
    {
      w += 0.000001;
    }
    fun->AddPoint(w,v);
    prev = w;
  }

  void clearPoints()
  {
    fun->RemoveAllPoints();
    computed = false;
  }
  double prev;
  bool computed;
  vtkKochanekSpline * fun;
};

class DepArcData
{
public:
  enum range{MIN = 0,MAX = 1};
  enum FunctionEnum{WeightFun = 0, DispFun = 1};
  DepArcData()
  :IsEnabled(true),
   IsRelative(false),
   IsSymmetric(true)
  {
    MinMaxDespDepth[DepArcData::MIN] = -8;
    MinMaxDespDepth[DepArcData::MAX] = -3;
    MinMaxDist[DepArcData::MIN] = -1;
    MinMaxDist[DepArcData::MAX] = 1;
    PiecewiseFun[0] = new ArcDepressPiecewiseFun();
    PiecewiseFun[1] = new ArcDepressPiecewiseFun();
    SelectedFunction[0] = PiecewiseFun[0];
    SelectedFunction[1] = PiecewiseFun[1];
    SplineFun[0] = new ArcDepressSplineFun();
    SplineFun[1] = new ArcDepressSplineFun();
  }

  ~DepArcData()
  {
    delete PiecewiseFun[0];
    delete PiecewiseFun[1];
    delete SplineFun[0];
    delete SplineFun[1];
  }

  struct line_seg
  {
    double pt1[2];
    double pt2[2];
    double line[3];
    bool valid;
  };

  void addPoint(FunctionEnum e, double w, double v, double s, double m)
  {
    PiecewiseFun[e]->addPoint(w,v,s,m);
    SplineFun[e]->addPoint(w,v,s,m);
  }

  void clearPoints()
  {
    PiecewiseFun[0]->clearPoints();
    SplineFun[0]->clearPoints();
    PiecewiseFun[1]->clearPoints();
    SplineFun[1]->clearPoints();
  }

  bool select(bool weight, bool disp)
  {
    return select(WeightFun, weight) || select(DispFun, disp);
  }
  bool select(FunctionEnum e, bool b)
  {
    bool result;
    if(b)
    {
      result = SelectedFunction[e] != SplineFun[e];
      SelectedFunction[e] = SplineFun[e];
    }
    else
    {
      result = SelectedFunction[e] != PiecewiseFun[e];
      SelectedFunction[e] = PiecewiseFun[e];
    }
    return result;
  }

  bool IsEnabled;
  bool IsRelative;
  bool IsSymmetric;
  std::vector<line_seg> points;
  double MinMaxDespDepth[2];
  double MinMaxDist[2];


  static double dist(double const i1[2], double const i2[2])
  {
    return std::sqrt((i1[0] - i2[0])*(i1[0] - i2[0])+
                     (i1[1] - i2[1])*(i1[1] - i2[1]));
  }

  static double dot(double const i1[2], double const i2[2])
  {
    return i1[0]*i2[0]+i1[1]*i2[1];
  }

  bool getDistance(double pt[2], double & result) const
  {
    if(points.empty())
      {
      return false;
      }
    result = std::max(std::abs(MinMaxDist[MAX]), std::abs(MinMaxDist[MAX]))+1;
    double closePt[2];
    line_seg const* cls = NULL;
    for (unsigned int i = 0; i<points.size(); ++i)
      {
      line_seg const& ls = points[i];
      double l2 = (ls.pt1[0] - ls.pt2[0])*(ls.pt1[0] - ls.pt2[0])+
                  (ls.pt1[1] - ls.pt2[1])*(ls.pt1[1] - ls.pt2[1]);
      if(l2 == 0)
        {
        double tmp = dist(ls.pt1, pt);
        if(tmp < result)
          {
          result = tmp;
          closePt[0] = ls.pt1[0];
          closePt[1] = ls.pt1[1];
          cls = &ls;
          }
        continue;
        }
      double tpt1[] = {pt[0]-ls.pt1[0], pt[1]-ls.pt1[1]};
      double tpt2[] = {ls.pt2[0]-ls.pt1[0], ls.pt2[1]-ls.pt1[1]};
      double t = dot(tpt1,tpt2)/l2;
      if(t <0.0)
        {
        double tmp = dist(pt, ls.pt1);
        if(tmp < result)
          {
          result = tmp;
          closePt[0] = ls.pt1[0];
          closePt[1] = ls.pt1[1];
          cls = &ls;
          }
        }
      else if(t > 1.0)
        {
        double tmp = dist(pt, ls.pt2);
        if(tmp < result)
          {
          result = tmp;
          closePt[0] = ls.pt2[0];
          closePt[1] = ls.pt2[1];
          cls = &ls;
          }
        }
      else
        {
        double v[2] = {ls.pt1[0]+t*(ls.pt2[0]-ls.pt1[0]), ls.pt1[1]+t*(ls.pt2[1]-ls.pt1[1])};
        double tmp = dist(pt, v);
        if(tmp < result)
          {
          result = tmp;
          closePt[0] = v[0];
          closePt[1] = v[1];
          cls = &ls;
          }
        }
      }
    if(IsSymmetric || cls == NULL)
      {
      return result <= MinMaxDist[MAX];
      }

    //calculate sign
    double cpDir[] = {pt[0]-closePt[0],pt[1]-closePt[1]};
    double lineDir[] = {cls->pt2[0] - cls->pt1[0], cls->pt2[1] - cls->pt1[1]};
    double sign = lineDir[1]*cpDir[0] - lineDir[0]*cpDir[1];
    if(sign<0)
      {
      result = -result;
      }

    return MinMaxDist[MIN] <= result && result <= MinMaxDist[MAX];
  }


  double apply(double d, double pt) const
  {
    if(d < MinMaxDist[MIN] || d > MinMaxDist[MAX])
      {
      return pt;
      }
    if(d < 0)
      {
      d = d/-MinMaxDist[MIN];
      }
    else if( d >= 0)
      {
      d = d/MinMaxDist[MAX];
      }
    double w = SelectedFunction[WeightFun]->evaluate(d);
    double tmp = SelectedFunction[DispFun]->evaluate(d, MinMaxDespDepth[MIN], MinMaxDespDepth[MAX]);
    if(IsRelative)
      {
      return pt + w*tmp;
      }
    else
      {
      return w*tmp +(1-w)*pt;
      }
  }

  void apply(double d, double *pt, double *n) const
  {
    if(d < MinMaxDist[MIN] || d > MinMaxDist[MAX])
    {
      return;
    }
    if(d < 0)
    {
      d = d/-MinMaxDist[MIN];
    }
    else if( d >= 0)
    {
      d = d/MinMaxDist[MAX];
    }
    double w = SelectedFunction[WeightFun]->evaluate(d);
    double tmp = SelectedFunction[DispFun]->evaluate(d, MinMaxDespDepth[MIN], MinMaxDespDepth[MAX]);
    if(IsRelative)
    {
      pt[0] = pt[0] + w*tmp*n[0];
      pt[1] = pt[1] + w*tmp*n[1];
      pt[2] = pt[2] + w*tmp*n[2];
    }
    else
    {
      pt[0] = w*tmp*n[0] + (1-w)*pt[0];
      pt[1] = w*tmp*n[1] + (1-w)*pt[1];
      pt[2] = w*tmp*n[2] + (1-w)*pt[2];
    }
  }
private:
  ArcDepressFunction * SelectedFunction[2];
  ArcDepressPiecewiseFun * PiecewiseFun[2];
  ArcDepressSplineFun * SplineFun[2];
};

namespace
{

void cross(double const i1[2], double const i2[2], double r[3])
{
  r[0] = i1[1] - i2[1];
  r[1] = i2[0] - i1[0];
  r[2] = i1[0]*i2[1] - i1[1]*i2[0];
}

double dot(double const i1[2], double const i2[2])
{
  return i1[0]*i2[0]+i1[1]*i2[1];
}

}

void vtkArcDepressFilter::PrintSelf(ostream& /*os*/, vtkIndent /*indent*/)
{
}

void vtkArcDepressFilter::ClearActiveArcPoints(int arc_ind)
{
  if(arc_ind < 0 || static_cast<size_t>(arc_ind) >= Arcs.size() || Arcs[arc_ind] == NULL) return;
  Arcs[arc_ind]->points.clear();
  this->Modified();
}

void vtkArcDepressFilter::SetArcAsClosed(int arc_ind)
{
  if(arc_ind < 0 || static_cast<size_t>(arc_ind) >= Arcs.size() || Arcs[arc_ind] == NULL) return;
  if(Arcs[arc_ind]->points.empty()) return;
  double pt1 = Arcs[arc_ind]->points[0].pt1[0];
  double pt2 = Arcs[arc_ind]->points[0].pt1[1];
  AddPointToArc(arc_ind,pt1, pt2);
}

void vtkArcDepressFilter::SetAxis(int axis)
{
  Axis = axis;
}

void vtkArcDepressFilter::AddPointToArc(double in_ind, double v1, double v2)
{
  int ind = static_cast<int>(in_ind);
  if(ind < 0) return;
  if(static_cast<size_t>(ind) >= Arcs.size() || Arcs[ind] == NULL) return;
  if(Arcs[ind]->points.empty())
    {
    DepArcData::line_seg l;
    l.pt1[0] = v1;
    l.pt1[1] = v2;
    l.valid = false;
    Arcs[ind]->points.push_back(l);
    }
  else if(Arcs[ind]->points.size() == 1 && !Arcs[ind]->points[0].valid)
    {
    DepArcData::line_seg & p = Arcs[ind]->points[0];
    p.pt2[0] = v1;
    p.pt2[1] = v2;
    cross(p.pt1, p.pt2, p.line);
    p.valid = true;
    }
  else
    {
    DepArcData::line_seg & p = Arcs[ind]->points[Arcs[ind]->points.size()-1];
    DepArcData::line_seg l;

    l.pt1[0] = p.pt2[0];
    l.pt1[1] = p.pt2[1];

    l.pt2[0] = v1;
    l.pt2[1] = v2;
    cross(l.pt1, l.pt2, l.line);
    p.valid = true;
    Arcs[ind]->points.push_back(l);
    }
  this->Modified();
}

void vtkArcDepressFilter
::SetControlRanges( double arc_ind,
                    double minDispDepth, double maxDispDepth,
                    double minDist, double maxDist)
{
  int ind = static_cast<int>(arc_ind);
  if(ind < 0) return;
  if(static_cast<size_t>(ind) >= Arcs.size() || Arcs[ind] == NULL) return;
  DepArcData * data = Arcs[ind];
  data->MinMaxDespDepth[DepArcData::MIN] = minDispDepth;
  data->MinMaxDespDepth[DepArcData::MAX] = maxDispDepth;
  data->MinMaxDist[DepArcData::MIN] = (minDist!=0)?minDist:-1e-23;
  data->MinMaxDist[DepArcData::MAX] = (maxDist!=0)?maxDist:1e-23;
  this->Modified();
}

void vtkArcDepressFilter::AddArc(int arc_ind)
{
  if(arc_ind < 0) return;
  size_t st_arc_ind = static_cast<size_t>(arc_ind);
  if(st_arc_ind >= this->Arcs.size())
  {
    this->Arcs.resize(st_arc_ind+1, NULL);
  }
  if(st_arc_ind < this->Arcs.size() && this->Arcs[st_arc_ind] == NULL)
  {
    this->Arcs[st_arc_ind] = new DepArcData();
    this->Modified();
  }
}

void vtkArcDepressFilter::RemoveArc(int arc_ind)
{
  if(static_cast<size_t>(arc_ind) >= Arcs.size() || Arcs[arc_ind] == NULL ) return;
  delete this->Arcs[arc_ind];
  this->Arcs[arc_ind] = NULL;
  this->Modified();
}

void vtkArcDepressFilter::SetArcEnable(int arc_ind, int isEnabled)
{
  if(static_cast<size_t>(arc_ind) >= Arcs.size() || arc_ind < 0 || Arcs[arc_ind] == NULL) return;
  Arcs[arc_ind]->IsEnabled = isEnabled != 0;
  this->Modified();
}

void vtkArcDepressFilter::SetFunctionModes(int arc_ind, int isRelative, int isSymmetric)
{
  if(static_cast<size_t>(arc_ind) >= Arcs.size() || arc_ind < 0 || Arcs[arc_ind] == NULL) return;
  Arcs[arc_ind]->IsRelative = isRelative != 0;
  Arcs[arc_ind]->IsSymmetric = isSymmetric != 0;
  this->Modified();
}

void vtkArcDepressFilter::ClearFunctions( int arc_ind )
{
  if(arc_ind < 0 || static_cast<size_t>(arc_ind) >= Arcs.size() || Arcs[arc_ind] == NULL) return;
  Arcs[arc_ind]->clearPoints();
  this->Modified();
}

void vtkArcDepressFilter::SelectFunctionType( int arc_ind, int weight_spline_type, int disp_spline_type)
{
  if(arc_ind < 0 || static_cast<size_t>(arc_ind) >= Arcs.size() || Arcs[arc_ind] == NULL) return;
  if(Arcs[arc_ind]->select(weight_spline_type != 0, disp_spline_type != 0))
  {
    this->Modified();
  }
}

void vtkArcDepressFilter::AddWeightingFunPoint( double arc_ind, double x, double y, double m, double s)
{
  int ind = static_cast<int>(arc_ind);
  if(ind < 0) return;
  if(static_cast<size_t>(ind) >= Arcs.size() || Arcs[ind] == NULL) return;
  Arcs[ind]->addPoint(DepArcData::WeightFun, x,y,m,s);
}

void vtkArcDepressFilter::AddDispFunPoint( double arc_ind, double x, double y, double m, double s)
{
  int ind = static_cast<int>(arc_ind);
  if(ind < 0) return;
  if(static_cast<size_t>(ind) >= Arcs.size() || Arcs[ind] == NULL) return;
  Arcs[ind]->addPoint(DepArcData::DispFun, x,y,m,s);
}

vtkArcDepressFilter::vtkArcDepressFilter()
:Axis(2)
{
  this->UseNormalDirection = false;
}

vtkArcDepressFilter::~vtkArcDepressFilter()
{
}

void vtkArcDepressFilter::ResizeOrder(int size)
{
  if(size < 0) return;
  ApplyOrder.resize(size, -1);
  this->Modified();
}

void vtkArcDepressFilter::SetOrderValue(int loc, int arc_ind)
{
  if(static_cast<size_t>(loc) >= ApplyOrder.size() || loc < 0 ||
     (arc_ind != -1 && static_cast<size_t>(arc_ind) >= Arcs.size()))
    return;
  ApplyOrder[loc] = arc_ind;
  this->Modified();
}

int vtkArcDepressFilter::RequestData(vtkInformation *vtkNotUsed(request),
                                     vtkInformationVector **inputVector,
                                     vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and output
  vtkPolyData *input = vtkPolyData::SafeDownCast(
                                                 inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData *output = vtkPolyData::SafeDownCast(
                                                  outInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkSmartPointer<vtkPolyDataNormals> normalGenerator;
  vtkDataArray* normals = NULL;
  if(UseNormalDirection)
  {
    normals = input->GetPointData()->GetNormals();
    if(normals == NULL)
    {
      normalGenerator = vtkSmartPointer<vtkPolyDataNormals>::New();
      normalGenerator->SetInputData(input);
      normalGenerator->ComputePointNormalsOn();
      normalGenerator->ComputeCellNormalsOff();
      normalGenerator->SplittingOff();
      normalGenerator->Update();
      input = normalGenerator->GetOutput();
      normals = input->GetPointData()->GetNormals();
    }
  }

  vtkIdType i;
  vtkCellArray *newVerts, *newLines, *newPolys;
  vtkPoints *newPoints;
  vtkIdType estimatedSize, numCells=input->GetNumberOfCells();
  vtkIdType numPts=input->GetNumberOfPoints();
  vtkPoints *inPts=input->GetPoints();
  vtkDataArray* normalsGeneric = input->GetPointData()->GetNormals();
  if(normalsGeneric)
  {
  }

  // Initialize self; create output objects
  //
  if ( numPts < 1 || inPts == NULL)
    {
    vtkWarningMacro(<<"No data to clip");
    return 1;
    }

  this->IsProcessing = true;
  bool enabled = !ApplyOrder.empty();

  if(!enabled)
    {
    vtkPolyData *t = vtkPolyData::SafeDownCast(
                       inInfo->Get(vtkDataObject::DATA_OBJECT()));
    //Do nothing for now.
    output->ShallowCopy(t);
    this->IsProcessing = false;
    return 1;
    }

  estimatedSize = numCells;
  estimatedSize = estimatedSize / 1024 * 1024; //multiple of 1024
  if (estimatedSize < 1024)
    {
    estimatedSize = 1024;
    }

  newPoints = vtkPoints::New();
  newPoints->Allocate(numPts,numPts/2);
  newVerts = vtkCellArray::New();
  newVerts->Allocate(estimatedSize,estimatedSize/2);
  newLines = vtkCellArray::New();
  newLines->Allocate(estimatedSize,estimatedSize/2);
  newPolys = vtkCellArray::New();
  newPolys->Allocate(estimatedSize,estimatedSize/2);

  //Transform points
  double point[3];
  double normal[3];
  for ( i=0; i < numPts; i++ )
    {
    inPts->GetPoint(i, point);
    if(UseNormalDirection && normals != NULL )
      normals->GetTuple(i, normal);
    double pt2d[] = {point[0], point[1]};
    for(unsigned int j = 0; j < ApplyOrder.size(); ++j)
      {
      int id = ApplyOrder[j];
      if(id < 0 || Arcs[id] == NULL) continue;
      DepArcData const& dad = *Arcs[id];
      double d;
      if(dad.getDistance(pt2d, d))
        {
        if(UseNormalDirection && normals != NULL )
          dad.apply(d,point,normal);
        else
          point[2] = dad.apply(d, point[2]);
        }
      }
    newPoints->InsertNextPoint(point);
    }
  newVerts->DeepCopy(input->GetVerts());
  newLines->DeepCopy(input->GetLines());
  newPolys->DeepCopy(input->GetPolys());

  if (newVerts->GetNumberOfCells())
    {
    output->SetVerts(newVerts);
    }
  newVerts->Delete();

  if (newLines->GetNumberOfCells())
    {
    output->SetLines(newLines);
    }
  newLines->Delete();

  if (newPolys->GetNumberOfCells())
    {
    output->SetPolys(newPolys);
    }
  newPolys->Delete();

  output->SetPoints(newPoints);

  if(UseNormalDirection && normals != NULL)
  {
    normalGenerator = vtkSmartPointer<vtkPolyDataNormals>::New();
    normalGenerator->SetInputData(output);
    normalGenerator->ComputePointNormalsOn();
    normalGenerator->ComputeCellNormalsOff();
    normalGenerator->SplittingOff();
    normalGenerator->Update();
    output->GetPointData()->SetNormals(normalGenerator->GetOutput()->GetPointData()->GetNormals());
  }

  newPoints->Delete();

  output->Squeeze();

  //copy reset of the data
  this->IsProcessing = false;
  return 1;
}

void vtkArcDepressFilter::setUseNormalDirection(int in)
{
  if( in == -1 ) return;
  if( in == 1 )
  {
    this->UseNormalDirection = true;
  }
  if( in == 0)
  {
    this->UseNormalDirection = false;
  }
}

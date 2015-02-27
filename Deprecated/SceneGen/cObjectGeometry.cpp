// cObjectGeometry.cpp: implementation of the cObjectGeometry class.
//
//////////////////////////////////////////////////////////////////////

#include "cObjectGeometry.h"
#include "PathFix.h"
#include "triangle.h"
#include <stdlib.h> // for atoi()

//#define MAKESURF      0
#define RESOLVE_AT    0.1
#define NUM_RANGES    10.0

//***********************
//**** TriType  Class ***
//***********************
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TriType::TriType()
{
  idx[0] = 0;  idx[1] = 0;  idx[2] = 0;
}

TriType::~TriType()
{
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
void cObjectGeometry::Initialize()
{
  baseoffset.assign(0.0, 0.0, 0.0);

  nodelist.clear();
  nodeclr.clear();
  trilist.clear();
  aabb.Reset();

  pt_color = false;

  if (dl_ptid>-1)
    if (glIsList(dl_ptid)) glDeleteLists(dl_ptid,1);
  if (dl_wfid>-1)
    if (glIsList(dl_wfid)) glDeleteLists(dl_wfid,1);
  if (dl_sfid>-1)
    if (glIsList(dl_sfid)) glDeleteLists(dl_sfid,1);

  dl_ptid = -1;
  dl_wfid = -1;
  dl_sfid = -1;
}

void cObjectGeometry::ClearDisplayLists()
{
  if (dl_ptid>-1)
    if (glIsList(dl_ptid)) glDeleteLists(dl_ptid,1);
  if (dl_wfid>-1)
    if (glIsList(dl_wfid)) glDeleteLists(dl_wfid,1);
  if (dl_sfid>-1)
    if (glIsList(dl_sfid)) glDeleteLists(dl_sfid,1);

  dl_ptid = -1;
  dl_wfid = -1;
  dl_sfid = -1;
}

cObjectGeometry::cObjectGeometry()
{
  objid = "";
  filekeep = "";

  Initialize();
}

cObjectGeometry::~cObjectGeometry()
{
  objid = "";
  filekeep = "";

  Initialize();
}

//////////////////////////////////////////////////////////////////////
// Input Functions
//////////////////////////////////////////////////////////////////////

long cObjectGeometry::AddNode( gmVector3 innode, gmVector3 inclr )
{
  pt_color = true;
  aabb.Adjust(innode);

  nodelist.push_back(innode);
  nodeclr.push_back(inclr);
  return ((long)nodelist.size()-1);
}

long cObjectGeometry::AddNode( gmVector3 innode, gmVector3 inclr, long idx )
{
  pt_color = true;
  aabb.Adjust(innode);

  nodelist[idx] = innode;
  nodeclr[idx] = inclr;
  return (idx);
}

long cObjectGeometry::AddNode( gmVector3 innode )
{
  aabb.Adjust(innode);

  nodelist.push_back(innode);
  return ((long)nodelist.size()-1);
}

long cObjectGeometry::AddNode( gmVector3 innode, long idx )
{
  aabb.Adjust(innode);

  nodelist[idx] = innode;
  return (idx);
}

long cObjectGeometry::AddTri( TriType intri )
{
  trilist.push_back(intri);
  return ((long)trilist.size()-1);
}

long cObjectGeometry::AddTri( TriType intri, long idx )
{
  trilist[idx] = intri;
  return (idx);
}

//////////////////////////////////////////////////////////////////////
// Plotting Functions
//////////////////////////////////////////////////////////////////////

void cObjectGeometry::PlotPoints( long baselist )
{
//Attempted optimization based on arrays
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_DOUBLE, 0, &nodelist[0] );
  if (pt_color)
  {
    glEnableClientState(GL_COLOR_ARRAY);
    glColorPointer(3, GL_DOUBLE, 0, &nodeclr[0] );
  }
  glDrawArrays(GL_POINTS, 0, nodelist.size());
  glDisableClientState(GL_VERTEX_ARRAY);
  if (pt_color)
  {
    glDisableClientState(GL_COLOR_ARRAY);
  }
/*
  //the display list contains ONLY the drawing info
  if (dl_ptid == -1)
  {
    dl_ptid = baselist;
    while ( glIsList(dl_ptid) ) dl_ptid++;

    glNewList(dl_ptid, GL_COMPILE_AND_EXECUTE);
    //make a display list
    long ii;

    //Plot the verts
    glBegin(GL_POINTS);
    for (ii=0; ii<(long)nodelist.size(); ii++)
    {
      if (pt_color) glColor3d(nodeclr[ii][0],nodeclr[ii][1],nodeclr[ii][2]);
      glVertex3d(nodelist[ii][0],nodelist[ii][1],nodelist[ii][2]);
    }
    glEnd();

    glEndList();

  }
  else glCallList(dl_ptid);
*/
}

void cObjectGeometry::PlotWireframe( long baselist )
{
  if (!HasSurface()) return;
  //the display list contains ONLY the drawing info
  if (dl_wfid == -1)
  {
    dl_wfid = baselist;
    while ( glIsList(dl_wfid) ) dl_wfid++;

    glNewList(dl_wfid, GL_COMPILE_AND_EXECUTE);
    //make a display list
    long ii;

    //Plot the edges
    for (ii=0; ii<(long)trilist.size(); ii++)
    {
      glBegin(GL_LINE_LOOP);
      if (pt_color) glColor3d(nodeclr[trilist[ii].idx[0]][0],nodeclr[trilist[ii].idx[0]][1],nodeclr[trilist[ii].idx[0]][2]);
      glVertex3d(nodelist[trilist[ii].idx[0]][0],nodelist[trilist[ii].idx[0]][1],nodelist[trilist[ii].idx[0]][2]);
      if (pt_color) glColor3d(nodeclr[trilist[ii].idx[1]][0],nodeclr[trilist[ii].idx[1]][1],nodeclr[trilist[ii].idx[1]][2]);
      glVertex3d(nodelist[trilist[ii].idx[1]][0],nodelist[trilist[ii].idx[1]][1],nodelist[trilist[ii].idx[1]][2]);
      if (pt_color) glColor3d(nodeclr[trilist[ii].idx[2]][0],nodeclr[trilist[ii].idx[2]][1],nodeclr[trilist[ii].idx[2]][2]);
      glVertex3d(nodelist[trilist[ii].idx[2]][0],nodelist[trilist[ii].idx[2]][1],nodelist[trilist[ii].idx[2]][2]);
      glEnd();
    }

    glEndList();

  }
  else glCallList(dl_wfid);
}

void cObjectGeometry::PlotSurface( long baselist )
{
  if (!HasSurface()) return;
  //the display list contains ONLY the drawing info
  if (dl_sfid == -1)
  {
    dl_sfid = baselist;
    while ( glIsList(dl_sfid) ) dl_sfid++;

    glNewList(dl_sfid, GL_COMPILE_AND_EXECUTE);
    //make a display list
    long ii;

    //Plot the surface
    glBegin(GL_TRIANGLES);
    for (ii=0; ii<(long)trilist.size(); ii++)
    {
      glNormal3d(trilist[ii].nrm[0], trilist[ii].nrm[1], trilist[ii].nrm[2]);
      if (pt_color) glColor3d(nodeclr[trilist[ii].idx[0]][0],nodeclr[trilist[ii].idx[0]][1],nodeclr[trilist[ii].idx[0]][2]);
      glVertex3d(nodelist[trilist[ii].idx[0]][0],nodelist[trilist[ii].idx[0]][1],nodelist[trilist[ii].idx[0]][2]);
      if (pt_color) glColor3d(nodeclr[trilist[ii].idx[1]][0],nodeclr[trilist[ii].idx[1]][1],nodeclr[trilist[ii].idx[1]][2]);
      glVertex3d(nodelist[trilist[ii].idx[1]][0],nodelist[trilist[ii].idx[1]][1],nodelist[trilist[ii].idx[1]][2]);
      if (pt_color) glColor3d(nodeclr[trilist[ii].idx[2]][0],nodeclr[trilist[ii].idx[2]][1],nodeclr[trilist[ii].idx[2]][2]);
      glVertex3d(nodelist[trilist[ii].idx[2]][0],nodelist[trilist[ii].idx[2]][1],nodelist[trilist[ii].idx[2]][2]);
    }
    glEnd();

    glEndList();

  }
  else glCallList(dl_sfid);
}

//object reading method
long cObjectGeometry::Read( ifstream& inpf )
{
  string tag, fname;
  bool fileread = false, fndkey=false;

  inpf >> tag;
  while ( !inpf.eof() && (tag!="}") )
  {
    if (tag[0]=='%') fndkey = true;

    if (tag=="objid") inpf >> objid;

    if (tag=="file")
    {
      inpf >> filekeep;
      if (objid=="") objid = filekeep;
      ReadData( filekeep.c_str() );
      fileread = true;
    }

  }

  if ( !fileread )
  {
    cout << "tObj: File not found!\n";
    return 0;
  }

  if ( fndkey )
  {
    cout << "Object Geometry: key values not allowed in definition!\n";
    return 0;
  }

  return 1;
}

//////////////////////////////////////////////////////////////////////
// File Input Functions
//////////////////////////////////////////////////////////////////////
void cObjectGeometry::FixNormals()
{
  long ii;
  //compute the normals for the tris
  for (ii = 0; ii<(long)trilist.size(); ii++)
  {
    gmVector3 d01, d02, nrml;

    d01 = nodelist[ trilist[ii].idx[1] ] - nodelist[ trilist[ii].idx[0] ];
    d02 = nodelist[ trilist[ii].idx[2] ] - nodelist[ trilist[ii].idx[0] ];
    nrml = cross(d01,d02);
    nrml.normalize();
    trilist[ii].nrm = nrml;
  }
}

void cObjectGeometry::RemoveDupTris()
{
  bool found;
  long ii = 1, ij = 0;

  while (ii<(long)trilist.size())
  {
    found = false;
    for (ij=0; ij<ii; ij++)
    {
       if ( ( (trilist[ii].idx[0] == trilist[ij].idx[0]) || (trilist[ii].idx[0] == trilist[ij].idx[1]) || (trilist[ii].idx[0] == trilist[ij].idx[2]) ) &&
            ( (trilist[ii].idx[1] == trilist[ij].idx[0]) || (trilist[ii].idx[1] == trilist[ij].idx[1]) || (trilist[ii].idx[1] == trilist[ij].idx[2]) ) &&
            ( (trilist[ii].idx[2] == trilist[ij].idx[0]) || (trilist[ii].idx[2] == trilist[ij].idx[1]) || (trilist[ii].idx[2] == trilist[ij].idx[2]) ) )
       {
         found = true;
         break;
       }
    }
    if (found)
    {
      trilist.erase(trilist.begin()+ii);
    }
    else
    {
      ii++;
    }
  }
}

struct subList
{
  cAABB bndr;
  vector<long> idx;
};

struct SLG
{
  vector<subList> sl;
} slg;

double cObjectGeometry::avgRadius( gmVector3 center, double radius )
{
  long ii, ij;
  vector<gmVector3> hits;  //inside radius point list
  gmVector3 test;
  double dx, dy, dist, minx, maxx, miny, maxy;
  double r2, avg;

  r2 = radius * radius;
  avg = 0.0;
  hits.clear();
  minx = center[0]-radius;
  maxx = center[0]+radius;
  miny = center[1]-radius;
  maxy = center[1]+radius;

  for (ii=0; ii<(long)slg.sl.size(); ii++)
  {
    if (slg.sl[ii].bndr.Inside(center))
    {
      for( ij=0; ij<(long)slg.sl[ii].idx.size(); ij++)
      {
        //quick bounding check
        test = nodelist[slg.sl[ii].idx[ij]];
        if (test[0]>minx)
        {
          if (test[0]<maxx)
          {
            if (test[1]>miny)
            {
              if (test[1]<maxy)
              {
                dx = test[0]-center[0];
                dy = test[1]-center[1];
                dist = (dx*dx)+(dy*dy);
                if (dist<r2) hits.push_back(test);
              }
            }
          }
        }
      }
    }
  }

  if (hits.empty()) return (gmGOOGOL);

  for (ii=0; ii<(long)hits.size(); ii++)
  {
    avg += hits[ii][2];
  }
  avg /= (double)hits.size();
  hits.clear();
  return (avg);
}

double cObjectGeometry::DistErr( double z1, double z2, double ze )
{
  gmVector2 d21, d1e;

  d21.assign(2.0, z2-z1);
  d1e.assign(-1.0, z1-ze);

  return fabs( ((d21[0]*d1e[1])-(d1e[0]*d21[1])) / d21.length() );
}

/*
double cObjectGeometry::DistErr( double z1, double z2, double ze )
{
  return fabs( ((z1+z2)*0.5) - ze );
}
*/

void cObjectGeometry::MakeSurface()
{
  //only works for heightfield data (with Z being up)
  long ii, ij, cnt;

  double *zpts;   //semi-isometric height-field data
  double *zdelt;  //error values for each point above
  double maxdelt, mindelt;

  long numx, numy, numzpts;
  long iix, iiy, idx;

  vector<TriType> ntl;    //new triangle list
  double offsetx, offsety, rangex, rangey, addoffset;
  double stepsize, stepsizey, ix, iy;
  gmVector3 inode;
  subList nsl;
  char labelText[1000];

  offsetx = aabb.GetMin()[0];
  offsety = aabb.GetMin()[1];
  rangex = aabb.GetRange()[0];
  rangey = aabb.GetRange()[1];

  stepsize = RESOLVE_AT;  //centimeter for most surface data
  stepsizey = RESOLVE_AT*0.866025404;  //centimeter * sin(60d)

  sprintf(labelText," Redefining Surface - Build Range Data ");
  emit startProgress(labelText);

  //create slg ranges
  double ssx = rangex/NUM_RANGES;
  double ssy = rangey/NUM_RANGES;

  double datamin, datamax;
  datamin = aabb.GetMin()[2];
  datamax = aabb.GetMax()[2];

  nsl.bndr.Reset();
  nsl.idx.clear();
  slg.sl.resize(121,nsl);  //it would be 100 but there might be overlap
  cnt=0;
  for (ix=offsetx; ix<(offsetx+rangex); ix+=ssx)
  {
    for (iy=offsety; iy<(offsety+rangey); iy+=ssy)
    {
      inode[0] = ix; inode[1] = iy; inode[2] = datamin;
      slg.sl[cnt].bndr.Adjust(inode);
      inode[0] = ix+ssx; inode[1] = iy+ssy; inode[2] = datamax;
      slg.sl[cnt].bndr.Adjust(inode);
      cnt++;
    }
  }

  //apportion points to slg ranges
  emit setProgressMax( (long)nodelist.size() );

  gmVector3 bmin, bmax;
  for (ii=0; ii<(long)nodelist.size(); ii++)
  {
    if (ii%1000==0)
    {
      //qApp->processEvents();
      emit setProgressVal( ii );
    }

    inode = nodelist[ii];
    for (ij=0; ij<(long)slg.sl.size(); ij++)
    {
      bmin = slg.sl[ij].bndr.GetMin();
      bmax = slg.sl[ij].bndr.GetMax();
      if (inode[0] > (bmin[0] - stepsize))
      {
        if (inode[0] < (bmax[0] + stepsize))
        {
          if (inode[1] > (bmin[1] - stepsize))
          {
            if (inode[1] < (bmax[1] + stepsize))
            {
              slg.sl[ij].idx.push_back(ii);  //EVIL
            }
          }
        }
      }
    }
  }

  emit finishProgress();

  //assign data to grid
  numx = ((long)(rangex/stepsize)+1);
  numy = ((long)(rangey/stepsizey)+1);
  numzpts = numx * numy;
  zpts = new double[numzpts];
  zdelt = new double[numzpts];

  sprintf(labelText," Redefining Surface - Assign New Grid ");
  emit startProgress(labelText);
  emit setProgressMax( numzpts );

  idx=0;
  for (iiy=0; iiy<numy; iiy++)
  {
    addoffset=0;
    if (iiy%2==0) addoffset = stepsize * 0.5;
    for (iix=0; iix<numx; iix++)
    {
      inode[0] = offsetx+(iix*stepsize)+addoffset;
      inode[1] = offsety+(iiy*stepsizey);
      inode[2] = aabb.GetMid()[2];
      zpts[idx] = avgRadius( inode, stepsize );
      idx++;
      if (idx%1000==0)
      {
        //qApp->processEvents();
        emit setProgressVal( idx );
      }
    }
  }


  //node removal?
  double z1,z2,z3,z4,z5,z6;
  double zd2, zd3;
  long offx, addx, addy;
  long acnt=0;
  double asum=0.0;

  maxdelt=0.0;
  mindelt=gmGOOGOL;

  idx=0;
  for (iiy=0; iiy<numy; iiy++)
  {

    if ((iiy%2)==0) offx=0;
    else offx = -1;

    for (iix=0; iix<numx; iix++)
    {
      if (zpts[idx]==gmGOOGOL) zdelt[idx] = 0.0;
      else //zdelt[idx] = gmGOOGOL;
      {
        //get the 6 neighbors
        addx = iix+offx;
        if (addx<0) { zdelt[idx] = gmGOOGOL; idx++; continue; }
        addy = iiy-1;
        if (addy<0) { zdelt[idx] = gmGOOGOL; idx++; continue; }
        z1 = zpts[ addx+(addy*numx) ];
        if (z1==gmGOOGOL) { zdelt[idx] = gmGOOGOL; idx++; continue; }

        addx = iix+offx+1;
        if (addx>=numx) { zdelt[idx] = gmGOOGOL; idx++; continue; }
        z2 = zpts[ addx+(addy*numx) ];
        if (z2==gmGOOGOL) { zdelt[idx] = gmGOOGOL; idx++; continue; }

        addx = iix-1;
        if (addx<0) { zdelt[idx] = gmGOOGOL; idx++; continue; }
        addy = iiy;
        z3 = zpts[ addx+(addy*numx) ];
        if (z3==gmGOOGOL) { zdelt[idx] = gmGOOGOL; idx++; continue; }

        addx = iix+1;
        if (addx>=numx) { zdelt[idx] = gmGOOGOL; idx++; continue; }
        z4 = zpts[ addx+(addy*numx) ];
        if (z4==gmGOOGOL) { zdelt[idx] = gmGOOGOL; idx++; continue; }

        addx = iix+offx;
        addy = iiy+1;
        if (addy>=numy) { zdelt[idx] = gmGOOGOL; idx++; continue; }
        z5 = zpts[ addx+(addy*numx) ];
        if (z5==gmGOOGOL) { zdelt[idx] = gmGOOGOL; idx++; continue; }

        addx = iix+offx+1;
        z6 = zpts[ addx+(addy*numx) ];
        if (z6==gmGOOGOL) { zdelt[idx] = gmGOOGOL; idx++; continue; }

        //find max delta
        zdelt[idx] = DistErr(z1,z6,zpts[idx]);
        zd2 = DistErr(z2,z5,zpts[idx]);
        zd3 = DistErr(z3,z4,zpts[idx]);
        if (zdelt[idx]<zd2) zdelt[idx] = zd2;
        if (zdelt[idx]<zd3) zdelt[idx] = zd3;
      }

      asum+=zdelt[idx];
      acnt++;
      if (zdelt[idx]<mindelt) mindelt=zdelt[idx];
      if (zdelt[idx]>maxdelt) maxdelt=zdelt[idx];

      idx++;
    }
  }

  //replace the nodelist with the surface node list
  nodelist.clear();

  double testsize = stepsize * 0.08715574274766;  //sin of 5 degrees
//  double testsize = stepsize * 0.05233595624294;  //sin of 3 degrees

  idx=0;
  for (iiy=0; iiy<numy; iiy++)
  {
    addoffset=0;
    if (iiy%2==0) addoffset = stepsize * 0.5;
    for (iix=0; iix<numx; iix++)
    {
      //if (zdelt[idx]>gmEPSILON)
      if (zdelt[idx]>testsize)
      {
        inode[0] = offsetx+(iix*stepsize)+addoffset;
        inode[1] = offsety+(iiy*stepsizey);
        inode[2] = zpts[idx];
        nodelist.push_back(inode);
      }
      idx++;
    }
  }
  emit finishProgress();

  //triangulate
  struct triangulateio in, out;
  newTriangulateIO( &in );
  newTriangulateIO( &out );

  in.numberofpoints = nodelist.size();
  in.pointlist = new double[in.numberofpoints*2];
  for (ii=0; ii<(long)nodelist.size(); ii++)
  {
    in.pointlist[ii*2]=nodelist[ii][0];
    in.pointlist[(ii*2)+1]=nodelist[ii][1];
  }

  triangulate("zQ", &in, &out, NULL);
  TriType temp;
  //Pull information from triangulation
  for (ii=0; ii<out.numberoftriangles; ii++) //???
  {
    temp.idx[0]=out.trianglelist[(ii*3)];
    temp.idx[1]=out.trianglelist[(ii*3)+1];
    temp.idx[2]=out.trianglelist[(ii*3)+2];
    trilist.push_back(temp);
  }

  deleteTriangulateIO( &in );
  deleteTriangulateIO( &out );

  //clean-up
  delete [] zpts;
  delete [] zdelt;
  for (ii=0; ii<(long)slg.sl.size(); ii++)
  {
    slg.sl[ii].idx.clear();
  }
  slg.sl.clear();
  FixNormals();
}

void cObjectGeometry::ReadData( const char *fname )
{
  char labelText[1000];
  string infname(fname);
  string s2dm = ".2dm";
  string s3dm = ".3dm";
  string ssth = ".sth";
  string spoly = ".poly";
  string sbinpts = "bin.pts";
  string spts = ".pts";
  string stxtpts = ".txt";
  string mcpoly = ".mcpoly";
  string sobj = ".obj";

  if (filekeep=="") filekeep = pathfix(fname);
  if (objid=="") objid = pathfix(fname);

  nodelist.clear();
  trilist.clear();

  printf("    Reading Geometry File: %s\n",fname);
  sprintf(labelText," Reading %s ",fname);
  //QString lT(labelText);

  emit startProgress(labelText);
  //emit setProgressLabel(&lT);

  if (infname.find(s2dm) != string::npos)
  {
    Read2DM(fname);
  } else
  if (infname.find(s3dm) != string::npos)
  {
    Read3DM(fname);
  } else
  if (infname.find(ssth) != string::npos)
  {
    ReadSTH(fname);
  } else
  if (infname.find(spoly) != string::npos)
  {
    ReadPoly(fname);
  } else
  if (infname.find(sbinpts) != string::npos)
  {
    ReadBinaryPTS(fname);
  } else
  if (infname.find(spts) != string::npos)
  {
    ReadPTS(fname);
  } else
  if (infname.find(stxtpts) != string::npos)
  {
    ReadTextPTS(fname);
  } else
  if (infname.find(mcpoly) != string::npos)
  {
    ReadMCPoly(fname);
  } else
  if (infname.find(sobj) != string::npos)
  {
    ReadOBJ(fname);
  }
  emit finishProgress();
#ifdef MAKESURF
  if (!HasSurface())
  {
    printf("      number of nodes (presurface):%ld\n",(long)nodelist.size());
    MakeSurface();
  }
#endif
  printf("      number of nodes:%ld   number of tris:%ld\n",(long)nodelist.size(),(long)trilist.size());
  gmVector3 mn, mx;
  mn = aabb.GetMin();
  mx = aabb.GetMax();
  printf("      Bounding Box: (%lf, %lf, %lf)-(%lf, %lf, %lf)\n\n", mn[0], mn[1], mn[2], mx[0], mx[1], mx[2]);
}

void cObjectGeometry::Read2DM( const char *fname )
{
  string tag;
  long ind, outd, nd[4];
  gmVector3 innode;
  TriType intri;

  long numtris, numverts, tricnt, vertcnt;

  ifstream fin(pathfix(fname));  //our input file stream

  if(fin.is_open()==0)
  {
    cout << "Problem opening 2dm file.\n";
    return;
  }

  Initialize();

  // 2-pass read for efficient allocation
  // Pass 1 - get the counts
  fin >> tag;

  numtris = 0;
  numverts = 0;

  while (!fin.eof())
  {
    fin >> tag;
    if (tag == "E3T") //triangle structure
    {
      fin >> ind >> nd[0] >> nd[1] >> nd[2] >> outd;
      numtris++;
    }
    else if (tag == "E4Q")
    {
      fin >> ind >> nd[0] >> nd[1] >> nd[2] >> nd[3] >> outd;
      numtris += 2;
    }
    else if (tag == "ND")
    {
      fin >> ind >> innode[0] >> innode[1] >> innode[2];
      numverts++;
    }
  }

  fin.close();

  //allocate memory
  innode[0] = innode[1] = innode[2] = 0.0;
  nodelist.resize(numverts,innode);
  trilist.resize(numtris,intri);

  tricnt = 0;
  vertcnt = 0;

  emit setProgressMax( numverts + numtris );

  //Pass 2 - Now read the data
  fin.clear();
  fin.open(pathfix(fname));

  fin >> tag;

  while (!fin.eof())
  {
    fin >> tag;
    if (tag == "E3T") //triangle structure
    {
      fin >> ind >> nd[0] >> nd[1] >> nd[2] >> outd;
      intri.idx[0] = nd[0] - 1; intri.idx[1] = nd[1] - 1; intri.idx[2] = nd[2] - 1;
      AddTri( intri, tricnt );
      tricnt++;
    }
    else if (tag == "E4Q") //triangle structure
    {
      fin >> ind >> nd[0] >> nd[1] >> nd[2] >> nd[3] >> outd;
      intri.idx[0] = nd[0] - 1; intri.idx[1] = nd[1] - 1; intri.idx[2] = nd[2] - 1;
      AddTri( intri, tricnt );
      tricnt++;
      intri.idx[0] = nd[0] - 1; intri.idx[2] = nd[2] - 1; intri.idx[3] = nd[3] - 1;
      AddTri( intri, tricnt );
      tricnt++;
    }
    else if (tag == "ND")
    {
      fin >> ind >> innode[0] >> innode[1] >> innode[2];
      AddNode( innode, vertcnt );
      vertcnt++;
    }
    if ((tricnt+vertcnt)%100 == 0)
    {
      //qApp->processEvents();
      emit setProgressVal( tricnt+vertcnt );
    }
  }

  fin.close();

  //compute the normals for the tris
  FixNormals();
}

void cObjectGeometry::Read3DM( const char *fname )
{
  string tag;
  long ind, outd, nd[4];
  gmVector3 innode;
  TriType intri;

  long numtris, numverts, tricnt, vertcnt;

  ifstream fin(pathfix(fname));  //our input file stream

  if(fin.is_open()==0)
  {
    cout << "Problem opening 3dm file.\n";
    return;
  }

  Initialize();

  // 2-pass read for efficient allocation
  // Pass 1 - get the counts
  fin >> tag;

  numtris = 0;
  numverts = 0;

  while (!fin.eof())
  {
    fin >> tag;

    if (tag == "E4T")
    {
      fin >> ind >> nd[0] >> nd[1] >> nd[2] >> nd[3] >> outd;
      numtris += 4;
    }
    else if (tag == "ND")
    {
      fin >> ind >> innode[0] >> innode[1] >> innode[2];
      numverts++;
    }
  }

  fin.close();

  //allocate memory
  innode[0] = innode[1] = innode[2] = 0.0;
  nodelist.assign(numverts,innode);
  trilist.assign(numtris,intri);

  tricnt = 0;
  vertcnt = 0;

  emit setProgressMax( numverts + numtris );

  //Pass 2 - Now read the data
  fin.clear();
  fin.open(pathfix(fname));

  fin >> tag;

  while (!fin.eof())
  {
    fin >> tag;

    if (tag == "E4T") //tetrahedral structure
    {
      fin >> ind >> nd[0] >> nd[1] >> nd[2] >> nd[3] >> outd;

      nd[0]--; nd[1]--; nd[2]--; nd[3]--;  //zero-based

      intri.idx[0] = nd[0]; intri.idx[1] = nd[1]; intri.idx[2] = nd[2];
      AddTri( intri, tricnt );
      tricnt++;

      intri.idx[0] = nd[0]; intri.idx[1] = nd[2]; intri.idx[2] = nd[3];
      AddTri( intri, tricnt );
      tricnt++;

      intri.idx[0] = nd[1]; intri.idx[1] = nd[3]; intri.idx[2] = nd[2];
      AddTri( intri, tricnt );
      tricnt++;

      intri.idx[0] = nd[0]; intri.idx[1] = nd[3]; intri.idx[2] = nd[1];
      AddTri( intri, tricnt );
      tricnt++;
    }
    else if (tag == "ND")
    {
      fin >> ind >> innode[0] >> innode[1] >> innode[2];
      AddNode( innode, vertcnt );
      vertcnt++;
    }
    if ((tricnt+vertcnt)%100 == 0)
    {
      //qApp->processEvents();
      emit setProgressVal( tricnt+vertcnt );
    }
  }

  fin.close();

  //compute the normals for the tris
  FixNormals();
}

void cObjectGeometry::ReadSTH( const char *fname )
{
  string tag;
  long ii, numverts, numfacets, nd[3], mat1, mat2;
  gmVector3 innode;
  TriType intri;

  ifstream fin(pathfix(fname));  //our input file stream


  if(fin.is_open()==0)
  {
    cout << "Problem opening sth file.\n";
    return;
  }

  Initialize();

  fin >> tag;
// get vertexes
  fin >> numverts;
  nodelist.assign(numverts,innode);
  emit setProgressMax( numverts );
  for ( ii = 0;  ii < numverts; ii++)
  {
    fin >> innode[0] >> innode[1] >> innode[2];
    AddNode( innode, ii );
    if ((ii%100) == 0)
    {
      //qApp->processEvents();
      emit setProgressVal( ii );
    }
  }
// Get connectivity
  fin >> numfacets;
  trilist.assign(numfacets,intri);
  emit setProgressMax( numfacets );
  for ( ii = 0;  ii < numfacets; ii++)
  {
    fin >> nd[0] >> nd[1] >> nd[2] >> mat1 >> mat2;
    intri.idx[0] = nd[0]; intri.idx[1] = nd[1]; intri.idx[2] = nd[2];
    AddTri( intri, ii );
    if ((ii%100) == 0)
    {
      //qApp->processEvents();
      emit setProgressVal( ii );
    }
  }

  fin.close();

  //compute the normals for the tris
  FixNormals();
}

/*  Mark Cowan's objects */
void cObjectGeometry::ReadMCPoly( const char *fname )
{
  string tag;
  long ii, ij, numverts, tmp[3], numfacets, numfacnod, nd[100];
  gmVector3 innode;
  TriType intri;

  ifstream fin(pathfix(fname));  //our input file stream

  if(fin.is_open()==0)
  {
    cout << "Problem opening mcpoly file.\n";
    return;
  }

  Initialize();

// get vertexes
  fin >> numverts >> tmp[0] >> tmp[1] >> tmp[2];
  nodelist.assign(numverts, innode);
  emit setProgressMax( numverts );
  for (ii = 0; ii < numverts;  ii++)
  {
    fin >> tmp[0] >> innode[0] >> innode[1] >> innode[2];
    AddNode( innode, ii );
    if ((ii%100) == 0)
    {
      //qApp->processEvents();
      emit setProgressVal( ii );
    }
  }

// Get connectivity
  fin >> numfacets >> tmp[0];
  emit setProgressMax( numfacets );
  for ( ii = 0;  ii < numfacets; ii++)
  {
    fin >> tmp[0] >> numfacnod;
    for (ij = 0; ij < numfacnod; ij++) fin >> nd[ij];
    //assume convexity (triangle fan - ignorant)
    intri.idx[0] = nd[0];
    for (ij=2; ij<numfacnod; ij++)
    {
      intri.idx[1] = nd[ij-1]; intri.idx[2] = nd[ij];
      AddTri( intri );
    }
    if ((ii%100) == 0)
    {
      //qApp->processEvents();
      emit setProgressVal( ii );
    }
  }

  fin.close();

  //compute the normals for the tris
  FixNormals();
}

void cObjectGeometry::ReadPoly( const char *fname )
{
  string tag;
  long ii, ij, numverts, tmp[3], numfacets, numfacnod, nd[100];
  gmVector3 innode;
  TriType intri;

  ifstream fin(pathfix(fname));  //our input file stream

  if(fin.is_open()==0)
  {
    cout << "Problem opening poly file.\n";
    return;
  }

  Initialize();

// get vertexes
  fin >> numverts >> tmp[0] >> tmp[1] >> tmp[2];
  nodelist.assign(numverts, innode);
  emit setProgressMax( numverts );
  for (ii = 0; ii < numverts;  ii++)
  {
    fin >> tmp[0] >> innode[0] >> innode[1] >> innode[2];
    AddNode( innode, ii );
    if ((ii%100) == 0)
    {
      //qApp->processEvents();
      emit setProgressVal( ii );
    }
  }

// Get connectivity
  fin >> numfacets >> tmp[0];
  emit setProgressMax( numfacets );
  for ( ii = 0;  ii < numfacets; ii++)
  {
    fin >> tmp[0] >> tmp[1] >> numfacnod;
    for (ij = 0; ij < numfacnod; ij++) fin >> nd[ij];
    //assume convexity (triangle fan - ignorant)
    intri.idx[0] = nd[0] - 1;
    for (ij=2; ij<numfacnod; ij++)
    {
      intri.idx[1] = nd[ij-1] - 1; intri.idx[2] = nd[ij] - 1;
      AddTri( intri);
    }
    if ((ii%100) == 0)
    {
      //qApp->processEvents();
      emit setProgressVal( ii );
    }
  }

  fin.close();

  //compute the normals for the tris
  FixNormals();
}

void cObjectGeometry::ReadPTS( const char *fname )
{
  long ii, numverts;
  gmVector3 innode, vrgb;
  int rgb[3];
  int nuts;
  char inln[200];

  ifstream fin(pathfix(fname));  //our input file stream

  if(fin.is_open()==0)
  {
    cout << "Problem opening pts file.\n";
    return;
  }

//clean up
  Initialize();

// get vertexes
  fin >> numverts;
  emit setProgressMax(numverts);
  nodelist.assign(numverts,innode);

// determine if there is color information
  fin.getline(inln, 199);  //finish the numverts line
  fin.getline(inln, 199);  //Get the first vertex line
  int numin = sscanf(inln, "%lf %lf %lf %d %d %d %d",
                        &innode[0], &innode[1], &innode[2], &nuts, &rgb[0], &rgb[1], &rgb[2]);
  if (numin<7)
  {
    AddNode( innode, 0 );
  }
  else
  {
    nodeclr.assign(numverts,innode);
    vrgb[0] = (double)rgb[0]/255.0;
    vrgb[1] = (double)rgb[1]/255.0;
    vrgb[2] = (double)rgb[2]/255.0;
    AddNode( innode, vrgb, 0 );
  }

  for ( ii = 1;  ii < numverts; ii++)
  {
    if (pt_color)
    {
      fin >> innode[0] >> innode[1] >> innode[2] >> nuts >> rgb[0] >> rgb[1] >> rgb[2];
      vrgb[0] = (double)rgb[0]/255.0;
      vrgb[1] = (double)rgb[1]/255.0;
      vrgb[2] = (double)rgb[2]/255.0;
      fin.getline(inln, 199);  //finish the line
      AddNode( innode, vrgb, ii );
    }
    else
    {
      fin >> innode[0] >> innode[1] >> innode[2];
      fin.getline(inln, 199);  //finish the line
      AddNode( innode, ii );
    }

    if ((ii%1000)==0)
    {
      //qApp->processEvents();
      emit setProgressVal(ii);
    }
  }
  fin.close();
}

void cObjectGeometry::ReadBinaryPTS( const char *fname )
{
  long ii, numverts;
  gmVector3 innode;

  ifstream fin(pathfix(fname), ios::binary);  //our input file stream

  if(fin.is_open()==0)
  {
    cout << "Problem opening pts file.\n";
    return;
  }

//clean up
  Initialize();

// get vertexes
  fin.read((char *)&numverts, sizeof(long));
  nodelist.assign(numverts,innode);
  emit setProgressMax( numverts );
  for ( ii = 0;  ii < numverts; ii++)
  {
    fin.read((char *)&innode[0],sizeof(double)*3);
    AddNode( innode, ii );
    if ((ii%1000)==0)
    {
      //qApp->processEvents();
      emit setProgressVal(ii);
    }
  }
  fin.close();
}

void cObjectGeometry::ReadTextPTS( const char *fname )
{
  gmVector3 innode;

  ifstream fin(pathfix(fname));  //our input file stream

  if(fin.is_open()==0)
  {
    cout << "Problem opening txt file.\n";
    return;
  }

//clean up
  Initialize();

// get vertexes
  while (!fin.eof())
  {
    fin >> innode[0] >> innode[1] >> innode[2];
    AddNode( innode );
  }

  fin.close();
  printf("Num points = %ld\n", (long)nodelist.size());

}

void cObjectGeometry::ReadOBJ( const char *fname )
{
  // Read a Wavefront Object file
  string tag, s1, s2, s3;
  long numverts, numtris, tricnt, vertcnt;
  gmVector3 innode;
  TriType intri;
  char inln[255];

  ifstream fin(pathfix(fname));  //our input file stream

  if(fin.is_open()==0)
  {
    cout << "Problem opening poly file.\n";
    return;
  }

  Initialize();

  // 2-pass read for efficient allocation
  // Pass 1 - get the counts
  fin >> tag;

  numtris = 0;
  numverts = 0;

  while (!fin.eof())
  {

    if (tag[0] == '#') fin.getline(inln,254);

    if (tag == "v")  //only read verts
    {
      fin >> innode[0] >> innode[1] >> innode[2];
      numverts ++;
    }
    else if (tag == "f")  //and facets
    {
      fin >> s1 >> s2 >> s3;   //hard coded for triangular facets (EVIL)
      numtris++;
    }
    fin >> tag;
  }

  fin.close();

  //allocate memory
  innode[0] = innode[1] = innode[2] = 0.0;
  nodelist.assign(numverts,innode);
  trilist.assign(numtris,intri);

  tricnt = 0;
  vertcnt = 0;
  emit setProgressMax( numverts+numtris );

  //Pass 2 - Now read the data
  fin.clear();
  fin.open(pathfix(fname));

  fin >> tag;

  while (!fin.eof())
  {
    // do something here
    if (tag[0] == '#') fin.getline(inln,254);

    if (tag == "v")  //only read verts
    {
      fin >> innode[0] >> innode[1] >> innode[2];
      AddNode( innode, vertcnt );
      //cout << "v <"<< innode[0] <<" , "<< innode[1] <<" , "<< innode[2] <<" >\n";
      vertcnt++;
    }
    else if (tag == "f")  //and facets
    {
      fin >> s1 >> s2 >> s3;   //hard coded for triangular facets (EVIL)
      intri.idx[0] = atoi(s1.c_str())-1;
      intri.idx[1] = atoi(s2.c_str())-1;
      intri.idx[2] = atoi(s3.c_str())-1;
      //cout << "f <"<< intri.idx[0] <<" , "<< intri.idx[1] <<" , "<< intri.idx[2] <<" >\n";
      AddTri( intri, tricnt);
      tricnt++;
    }
    fin >> tag;
    if ((tricnt+vertcnt)%1000==0)
    {
      //qApp->processEvents();
      emit setProgressVal(tricnt+vertcnt);
    }
  }

  fin.close();

  //compute the normals for the tris
  FixNormals();
}

//////////////////////////////////////////////////////////////////////
// File Output Functions
//////////////////////////////////////////////////////////////////////
void cObjectGeometry::WriteData( const char *fname )
{
  string outfname(fname);
  string s2dm = ".2dm";
  string ssth = ".sth";
  string spoly = ".poly";
  string spts = ".pts";
  string stxtpts = ".txt";
  string sobj = ".obj";
  string sstl = ".stl";

  if (outfname.find(s2dm) != string::npos)
  {
    Write2DM(fname);
    return;
  }
  if (outfname.find(spoly) != string::npos)
  {
    WritePoly(fname);
    return;
  }
  if (outfname.find(spts) != string::npos)
  {
    WritePTS(fname);
    return;
  }
  if (outfname.find(stxtpts) != string::npos)
  {
    WriteTextPTS(fname);
    return;
  }
  if (outfname.find(sobj) != string::npos)
  {
    WriteOBJ(fname);
    return;
  }
  if (outfname.find(sstl) != string::npos)
  {
    WriteSTL(fname);
    return;
  }

  cout << "The output file -" << outfname <<"- does not have a recognized extension\n";

}

void cObjectGeometry::Write2DM( const char *fname )
{
  long ii;
  ofstream fout(fname);  //our input file stream

  if(fout.is_open()==0)
  {
    cout << "Problem opening 2dm file.\n";
    return;
  }

  fout << "MESH2D\n";

  for (ii=0; ii<(long)trilist.size(); ii++)
  {
    fout << "E3T " << (ii+1) << " " << (trilist[ii].idx[0]+1) << " " << (trilist[ii].idx[1]+1) << " " << (trilist[ii].idx[2]+1) << " 0\n";
  }

  for (ii=0; ii<(long)nodelist.size(); ii++)
  {
    fout << "ND " << (ii+1) << " " << nodelist[ii][0] << " " << nodelist[ii][1] << " " << nodelist[ii][2] << " 0\n";
  }

  fout << "END\n";

  fout.close();

}

void cObjectGeometry::WritePoly( const char *fname )
{
  long ii;
  ofstream fout(fname);  //our input file stream

  if(fout.is_open()==0)
  {
    cout << "Problem opening poly file.\n";
    return;
  }

  fout << nodelist.size() << " 3 0 0\n";
  for (ii=0; ii<(long)nodelist.size(); ii++)
  {
    fout << (ii+1) << " " << nodelist[ii][0] << " " << nodelist[ii][1] << " " << nodelist[ii][2] << " 0\n";
  }

  fout << trilist.size() << " 0\n";
  for (ii=0; ii<(long)trilist.size(); ii++)
  {
    fout << "1 0\n";
    fout << "3 " << (trilist[ii].idx[0]+1) << " " << (trilist[ii].idx[1]+1) << " " << (trilist[ii].idx[2]+1) << "\n";
  }

  fout << "0\n0\n";

  fout.close();
}

void cObjectGeometry::WritePTS( const char *fname )
{
  long ii;
  ofstream fout(fname);  //our output file stream

  if(fout.is_open()==0)
  {
    cout << "Problem opening pts file.\n";
    return;
  }

  fout << nodelist.size() << "\n";

  for (ii=0; ii<(long)nodelist.size(); ii++)
  {
    fout << (nodelist[ii][0]) << " " << (nodelist[ii][1]) << " " << (nodelist[ii][2]) << " 0\n";
  }

  fout.close();
}


void cObjectGeometry::WriteBinaryPTS( const char *fname )
{
  long ii, numverts;

  ofstream fout(pathfix(fname), ios::binary);  //our input file stream

  if(fout.is_open()==0)
  {
    cout << "Problem opening binary pts file.\n";
    return;
  }

  numverts = nodelist.size();
  fout.write((char *)&numverts, sizeof(long));
  for ( ii = 0;  ii < numverts; ii++)
  {
    fout.write((char *)&nodelist[ii][0],sizeof(double)*3);
  }

  fout.close();
}

void cObjectGeometry::WriteTextPTS( const char *fname )
{
  long ii;
  ofstream fout(fname);  //our output file stream

  if(fout.is_open()==0)
  {
    cout << "Problem opening text pts file.\n";
    return;
  }

  for (ii=0; ii<(long)nodelist.size(); ii++)
  {
    fout << (nodelist[ii][0]) << " " << (nodelist[ii][1]) << " " << (nodelist[ii][2]) << "\n";
  }

  fout.close();
}

void cObjectGeometry::WriteOBJ( const char *fname )
{
  // Write a Wavefront Object file
  long ii;
  ofstream fout(fname);  //our output file stream

  if(fout.is_open()==0)
  {
    cout << "Problem opening obj file.\n";
    return;
  }

  fout << "# Simplified Obj export function from Convert3D by Barry C White\n\n";

  for (ii=0; ii<(long)nodelist.size(); ii++)
  {
    fout << "v " << (nodelist[ii][0]) << " " << (nodelist[ii][1]) << " " << (nodelist[ii][2]) << "\n";
  }

  for (ii=0; ii<(long)trilist.size(); ii++)
  {
    fout << "f " << (trilist[ii].idx[0]+1) << " " << (trilist[ii].idx[1]+1) << " " << (trilist[ii].idx[2]+1) << "\n";
  }

  fout.close();
}

void cObjectGeometry::WriteSTL( const char *fname )
{
  long ii;
  ofstream fout(fname);  //our input file stream

  if(fout.is_open()==0)
  {
    cout << "Problem opening stl file.\n";
    return;
  }

  fout << "solid\n\n";

  for (ii=0; ii<(long)trilist.size(); ii++)
  {
    fout << "facet normal " << trilist[ii].nrm[0] << " " << trilist[ii].nrm[1] << " " << trilist[ii].nrm[2] << "\n";
    fout << "  outer loop\n";
    fout << "    vertex " << nodelist[trilist[ii].idx[0]][0] << " " << nodelist[trilist[ii].idx[0]][1] << " " << nodelist[trilist[ii].idx[0]][2] << "\n";
    fout << "    vertex " << nodelist[trilist[ii].idx[1]][0] << " " << nodelist[trilist[ii].idx[1]][1] << " " << nodelist[trilist[ii].idx[1]][2] << "\n";
    fout << "    vertex " << nodelist[trilist[ii].idx[2]][0] << " " << nodelist[trilist[ii].idx[2]][1] << " " << nodelist[trilist[ii].idx[2]][2] << "\n";
    fout << "  endloop\n";
    fout << " endfacet\n";
  }

  fout << "endsolid\n";

  fout.close();

}

void cObjectGeometry::WriteXML( ofstream &fout )
{
  fout << "  <ObjectGeometry>\n";
  fout << "    <filename> " << filekeep << " </filename>\n";
  fout << "    <index> " << objid << " </index>\n";
  fout << "    <base_offset>\n";
  fout << "      <X> " << baseoffset[0] << " </X>\n";
  fout << "      <Y> " << baseoffset[1] << " </Y>\n";
  fout << "      <Z> " << baseoffset[2] << " </Z>\n";
  fout << "    </base_offset>\n";
  fout << "  </ObjectGeometry>\n\n";
}

//////////////////////////////////////////////////////////////////////
// Fix the base point for small numbers
//////////////////////////////////////////////////////////////////////
void cObjectGeometry::FixBase( double dx, double dy, double dz )
{
  aabb.Reset();
  baseoffset[0] = dx;
  baseoffset[1] = dy;
  baseoffset[2] = dz;
  for (long ii=0; ii<(long)nodelist.size(); ii++)
  {
    nodelist[ii] += baseoffset;
    aabb.Adjust(nodelist[ii]);
  }
}

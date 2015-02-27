// cSurfaceObjectDefinition.cpp: implementation of the cSurfaceObjectDefinition class.
//
//////////////////////////////////////////////////////////////////////

#include "cSurfaceObjectDefinition.h"
#include "EulerAngles.h"

#include <iostream>
#include <fstream>
#include <string>

using namespace std;

//#define HIGHLIGHT_WIREFRAME 1

//***************************
//**** cSurfaceObjectDefinition  Class ***
//***************************
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

void cSurfaceObjectDefinition::Initialize()
{
  name = "";
  geomid = "";
  matid = 0;

  selected = false;
  highlighted = false;

  surfitem = false;

  mater.SetFace(GL_FRONT_AND_BACK);
  mater.SetAmbient(0.03, 0.04, 0.03);
  mater.SetDiffuse(0.3, 0.4, 0.3);

  himater = mater;
  himater.SetAmbient(0.06, 0.08, 0.06);
  himater.SetDiffuse(0.6, 0.8, 0.6);

  scl.assign(1.0, 1.0, 1.0);
  rot = rot.identity();
  pos.assign(0.0, 0.0, 0.0);

  //objgeom.Initialize();
  objgeomp = NULL;
}

cSurfaceObjectDefinition::cSurfaceObjectDefinition()
{
  Initialize();
}

cSurfaceObjectDefinition::~cSurfaceObjectDefinition()
{
  Initialize();
}

//////////////////////////////////////////////////////////////////////
// Object Reading Method
//////////////////////////////////////////////////////////////////////
long cSurfaceObjectDefinition::Read( ifstream& inpf )
{
  string tag;
  bool fileread = false, fndkey=false;
  float cr, cg, cb;
  gmVector3 zaxis(0.0, 0.0, 1.0), yaxis( 0.0, 1.0, 0.0), xaxis(1.0, 0.0, 0.0);
  gmMatrix4 rotmat;
  double rotval;

  inpf >> tag;
  while ( !inpf.eof() && (tag!="}") )
  {
    if (tag[0]=='%') fndkey = true;

    if (tag=="name") inpf >> name;

    if (tag=="index") inpf >> geomid;

    if (tag=="material") inpf >> matid;

    if (tag=="pos") inpf >> pos[0] >> pos[1] >> pos[2];

    if (tag=="pos_x") inpf >> pos[0];

    if (tag=="pos_y") inpf >> pos[1];

    if (tag=="pos_z") inpf >> pos[2];

    if (tag=="uniform_scale")
    {
      double uscl;
      inpf >> uscl;
      scl[0] = uscl;  scl[1] = uscl;  scl[2] = uscl;
    }

    if (tag=="rotate_x")
    {
      inpf >> rotval;
      //Apply rotation instantly
      rotmat = rotmat.identity();
      rotmat = rotmat.rotate(-rotval, xaxis);        //angles are negative because our Matrix is left-handed
      rot *= rotmat;
    }

    if (tag=="rotate_y")
    {
      inpf >> rotval;
      //Apply rotation instantly
      rotmat = rotmat.identity();
      rotmat = rotmat.rotate(-rotval, yaxis);        //angles are negative because our Matrix is left-handed
      rot *= rotmat;
    }

    if (tag=="rotate_z")
    {
      inpf >> rotval;
      //Apply rotation instantly
      rotmat = rotmat.identity();
      rotmat = rotmat.rotate(-rotval, zaxis);        //angles are negative because our Matrix is left-handed
      rot *= rotmat;
    }

    if (tag=="scale") inpf >> scl[0] >> scl[1] >> scl[2];

    if (tag=="scale_x") inpf >> scl[0];

    if (tag=="scale_y") inpf >> scl[1];

    if (tag=="scale_z") inpf >> scl[2];

    if (tag=="color")
    {
      inpf >> cr >> cg >> cb;
      SetColor(cr,cg,cb);
    }

    if (tag=="hicolor")
    {
      inpf >> cr >> cg >> cb;
      SetHighLightColor(cr,cg,cb);
    }

  }

  if ( !fileread )
  {
    cout << "tObj: File not found!\n";
    return 0;
  }

  if ( fndkey )
  {
    cout << "Object Gemetry: key values not allowed in definition!\n";
    return 0;
  }

  return 1;
}

//////////////////////////////////////////////////////////////////////
// Direct Object Manipulation Methods
//////////////////////////////////////////////////////////////////////
cAABB cSurfaceObjectDefinition::GetTransformedAABB()
{
  cAABB taabb;
  gmVector3 mn, mx, pt;

  mn = objgeomp->GetAABB().GetMin();
  mx = objgeomp->GetAABB().GetMax();

  mn[0] *= scl[0]; mn[1] *= scl[1]; mn[2] *= scl[2];
  mx[0] *= scl[0]; mx[1] *= scl[1]; mx[2] *= scl[2];

  pt = mn;  //point 1
  pt = rot.transform(pt) + pos;
  taabb.Adjust(pt);

  pt = mn;  //point 2
  pt[0] = mx[0];
  pt = rot.transform(pt) + pos;
  taabb.Adjust(pt);

  pt = mn;  //point 3
  pt[1] = mx[1];
  pt = rot.transform(pt) + pos;
  taabb.Adjust(pt);

  pt = mn;  //point 4
  pt[0] = mx[0];
  pt[1] = mx[1];
  pt = rot.transform(pt) + pos;
  taabb.Adjust(pt);

  pt = mx;  //point 5
  pt = rot.transform(pt) + pos;
  taabb.Adjust(pt);

  pt = mx;  //point 6
  pt[0] = mn[0];
  pt = rot.transform(pt) + pos;
  taabb.Adjust(pt);

  pt = mx;  //point 7
  pt[1] = mn[1];
  pt = rot.transform(pt) + pos;
  taabb.Adjust(pt);

  pt = mx;  //point 8
  pt[0] = mn[0];
  pt[1] = mn[1];
  pt = rot.transform(pt) + pos;
  taabb.Adjust(pt);

  return (taabb);

}

const char * cSurfaceObjectDefinition::GetName()
{
  return name.c_str();
}

void cSurfaceObjectDefinition::ChangePosition( double dx, double dy, double dz )
{
  pos[0] += dx;
  pos[1] += dy;
  pos[2] += dz;
}

void cSurfaceObjectDefinition::ChangeRotation( gmMatrix4& pre )
{
  rot *= pre;
}

void cSurfaceObjectDefinition::FixPosition( double dx, double dy, double dz )
{
  objgeomp->FixBase(dx,dy,dz);
}

void cSurfaceObjectDefinition::GetOffset( double& dx, double& dy, double& dz )
{
  gmVector3 offset;

  offset = objgeomp->GetBaseOffset();
  dx = offset[0];
  dy = offset[1];
  dz = offset[2];
}

void cSurfaceObjectDefinition::RotationEuler( double yaw, double pitch, double roll)
{
  gmVector3 zaxis(0.0, 0.0, 1.0), yaxis( 0.0, 1.0, 0.0), xaxis(1.0, 0.0, 0.0);
  gmMatrix4 rotz, rotx, roty;

  rot = rot.identity();
  rotz = rotz.identity();
  roty = roty.identity();
  rotx = rotx.identity();

  rotz = rotz.rotate(-yaw, zaxis);        //angles are negative because our Matrix is left-handed
  roty = roty.rotate(-pitch, yaxis);
  rotx = rotx.rotate(-roll, xaxis);

  rot *= rotz;
  rot *= roty;
  rot *= rotx;
}

void cSurfaceObjectDefinition::RotationPhiTheta( double phi, double theta)
{
  gmVector3 zaxis(0.0, 0.0, 1.0), xaxis(1.0, 0.0, 0.0);
  gmMatrix4 rotz, rotx;

  rot = rot.identity();
  rotz = rotz.identity();
  rotx = rotx.identity();

  rotz = rotz.rotate(-phi, zaxis);        //angles are negative because our Matrix is left-handed
  rotx = rotx.rotate(-theta, xaxis);

  rot *= rotz;
  rot *= rotx;
}

void cSurfaceObjectDefinition::MakeEuler( double& yaw, double& pitch, double& roll )
{
// Coordinate system: Right-Hand Rule, yaw about Z-axis, pitch about Y, roll about X
  EulerAngles outAngs;
  HMatrix R;
  int ii, ij;

  for (ii=0; ii<4; ii++)
    for (ij=0; ij<4; ij++)
    {
      R[ii][ij] = rot[ij][ii];  //transpose for rhr
    }

  outAngs = Eul_FromHMatrix(R,EulOrdZYXs);
  yaw = outAngs.x * gmRADTODEG;
  pitch = outAngs.y * gmRADTODEG;
  roll = outAngs.z * gmRADTODEG;
}

double cSurfaceObjectDefinition::GetRotZ( )
{
  gmVector3 base(1.0, 0.0, 0.0), roted;
  double ret;

  roted = rot.transform(base);
  ret = atan2(roted[1], roted[0])  * gmRADTODEG;
  if (ret<0) ret+=360.0;

  return (ret);
}


//////////////////////////////////////////////////////////////////////
// Utility Functions
//////////////////////////////////////////////////////////////////////
void cSurfaceObjectDefinition::SetColor( float r, float g, float b )
{
  mater.SetAmbient( r*0.1, g*0.1, b*0.1);
  mater.SetDiffuse( r, g, b);
}

void cSurfaceObjectDefinition::SetHighLightColor( float r, float g, float b )
{
  himater.SetAmbient( r*0.5, g*0.5, b*0.5);
  himater.SetDiffuse( r, g, b);
}

//////////////////////////////////////////////////////////////////////
// Plotting Functions
//////////////////////////////////////////////////////////////////////
void cSurfaceObjectDefinition::Display( long baselist)  //for unselected ObjectDefinitions
{
  float ar, ag, ab, aa;

//ASSUMED: we are modifying the GL_MODELVIEW matrix
//  glMatrixMode(GL_MODELVIEW);
  double mogl[16];
  glPushMatrix();
  rot.MakeOpenGL(mogl);
  glTranslated(pos[0], pos[1], pos[2]);
  glMultMatrixd(mogl);
  glScaled(scl[0],scl[1],scl[2]);

  if (objgeomp->HasSurface())
  {
    if (selected)
    {
      himater.Fix();
      himater.GetDiffuse(ar,ag,ab,aa);
    }
    else
    {
      mater.Fix();
      mater.GetDiffuse(ar,ag,ab,aa);
    }

    objgeomp->PlotSurface( baselist );

#ifdef HIGHLIGHT_WIREFRAME
    if (selected)
    {
      glDisable(GL_LIGHTING);
      glColor3f(ar,ag,ab);
      objgeomp->PlotWireframe( baselist );
      glEnable(GL_LIGHTING);
    }
#endif

  }
  else
  {
    if (selected)
    {
      himater.GetDiffuse(ar,ag,ab,aa);
    }
    else
    {
      mater.GetDiffuse(ar,ag,ab,aa);
    }
    glDisable(GL_LIGHTING);
    glColor3f(ar,ag,ab);
    objgeomp->PlotPoints( baselist );
    glEnable(GL_LIGHTING);
  }

  glPopMatrix();
}

void cSurfaceObjectDefinition::Display( long baselist, gmMatrix4 rotd, gmVector3 poso)  //for selected ObjectDefinitions
{
  float ar, ag, ab, aa;

//ASSUMED: we are modifying the GL_MODELVIEW matrix
//  glMatrixMode(GL_MODELVIEW);
  double mogl[16], moff[16];
  glPushMatrix();
  rot.MakeOpenGL(mogl);
  rotd.MakeOpenGL(moff);

  glTranslated(poso[0], poso[1], poso[2]);
  glTranslated(pos[0], pos[1], pos[2]);
  glMultMatrixd(moff);
  glMultMatrixd(mogl);
  glScaled(scl[0],scl[1],scl[2]);

  if (objgeomp->HasSurface())
  {
    if (selected)
    {
      himater.Fix();
      himater.GetDiffuse(ar,ag,ab,aa);
    }
    else
    {
      mater.Fix();
      mater.GetDiffuse(ar,ag,ab,aa);
    }

    objgeomp->PlotSurface( baselist );

#ifdef HIGHLIGHT_WIREFRAME
    if (selected)
    {
      glDisable(GL_LIGHTING);
      glColor3f(ar,ag,ab);
      objgeomp->PlotWireframe( baselist );
      glEnable(GL_LIGHTING);
    }
#endif

  }
  else
  {
    if (selected)
    {
      himater.GetDiffuse(ar,ag,ab,aa);
    }
    else
    {
      mater.GetDiffuse(ar,ag,ab,aa);
    }
    glDisable(GL_LIGHTING);
    glColor3f(ar,ag,ab);
    objgeomp->PlotPoints( baselist );
    glEnable(GL_LIGHTING);
  }

  glPopMatrix();
}

//////////////////////////////////////////////////////////////////////
// Placement method
//////////////////////////////////////////////////////////////////////
gmVector3 cSurfaceObjectDefinition::FindClosest(gmVector3 closeto)
{
  long ii, numpts;
  gmVector3 pt, newpt, ctxy, npxy;
  double dist, newdist;

  numpts = objgeomp->NumNodes();

  dist = gmGOOGOL;
  pt.assign(0.0,0.0,0.0);

  ctxy.assign(closeto[0],closeto[1],0.0);

  for (ii=0; ii<numpts; ii++)
  {
    newpt = objgeomp->GetNode( ii );
    newpt = rot.transform(newpt) + pos;

    npxy.assign(newpt[0],newpt[1],0.0);

    newdist = (npxy-ctxy).lengthSquared();
    if (newdist < dist)
    {
      pt = newpt;
      dist = newdist;
    }
  }

  return (pt);
}

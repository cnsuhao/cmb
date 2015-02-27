// cAOI.cpp: implementation of the cAOI class.
//
//////////////////////////////////////////////////////////////////////

#include "cAOI.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

cAOI::cAOI()
{
  Init();
}

cAOI::~cAOI()
{
  Init();
}

void cAOI::Init()
{
  name = "base";
  numpts = 0;
  bndry.clear();
  base = 0.0;
  height = 0.0;
  matid = 0;
}

//////////////////////////////////////////////////////////////////////
// Read
//////////////////////////////////////////////////////////////////////
long cAOI::Read( ifstream& inpf )
{
  string tag;
  int ptsread=0;
  bool fndbase = false, fndhght = false, fndkey = false, fndnpts = false;
  gmVector2 v2;

  inpf >> tag;
  while ( !inpf.eof() && (tag!="}") )
  {
    if (tag[0]=='%') fndkey = true;

    if (tag=="num_points")
    {
      if (!fndnpts)
      {
        fndnpts = true;
        inpf >> numpts;
        v2.assign(0.0, 0.0);
        bndry.resize(numpts,v2);
      }
      else
      {
        cout << "Area of Interest: Num_Points assigned already!\n";
      }
    }

    if (tag=="pt2")
    {
      if (ptsread<numpts)
      {
        inpf >> bndry[ptsread][0] >> bndry[ptsread][1];
      }
      ptsread++;
    }

    if (tag=="base")
    {
      inpf >> base;
      fndbase = true;
    }

    if (tag=="height")
    {
      inpf >> height;
      fndhght = true;
    }

    if (tag=="default_material")
    {
      inpf >> matid;
    }

    inpf >> tag;
  }

  if ( !fndhght )
  {
    cout << "Area of Interest: Height not found!\n";
    return 0;
  }

  if ( !fndbase )
  {
    cout << "Area of Interest: Base not found!\n";
    return 0;
  }

  if ( fndkey )
  {
    cout << "Area of Interest: Key values not allowed in definition!\n";
    return 0;
  }

  if (numpts<3)
  {
    cout << "Area of Interest: The number of points is not enough to form a convex polyhedron!\n";
    return 0;
  }

  if ( ptsread!=numpts)
  {
    cout << "Area of Interest: The number of points read is inconsistent with the number of poinst specified!\n";
    return 0;
  }

  //Things we do not check for (at present) - convex polygon definition

  return 1;
}

void cAOI::Plot( )
{
  int ii;
// plot boundary region
  glDisable(GL_LIGHTING);
  glColor3f(0.8, 0.8, 0.8);
  glBegin(GL_LINE_LOOP); //base
    for (ii=0; ii<numpts; ii++)
    {
      glVertex3d( bndry[ii][0],bndry[ii][1],base);
    }
  glEnd();
  glBegin(GL_LINE_LOOP); //top
    for (ii=0; ii<numpts; ii++)
    {
      glVertex3d( bndry[ii][0],bndry[ii][1],base+height);
    }
  glEnd();
  glBegin(GL_LINES); //connect top and bottom
    for (ii=0; ii<numpts; ii++)
    {
      glVertex3d( bndry[ii][0],bndry[ii][1],base);
      glVertex3d( bndry[ii][0],bndry[ii][1],base+height);
    }
  glEnd();
  glEnable(GL_LIGHTING);
// finished

}

cAABB cAOI::GetBoundingBox()
{
  cAABB naabb;
  gmVector3  node;
  int ii;

  naabb.Reset();

  for (ii=0; ii<numpts; ii++)
  {
    node.assign( bndry[ii][0], bndry[ii][1], base );
    naabb.Adjust(node);
  }

  for (ii=0; ii<numpts; ii++)
  {
    node.assign( bndry[ii][0], bndry[ii][1], base+height );
    naabb.Adjust(node);
  }

  return (naabb);
}

void cAOI::AddPoint(double x, double y)
{
  gmVector2 pt2( x, y );

  bndry.push_back( pt2 );
  numpts = bndry.size();

}

void cAOI::SetBase(double bs)
{
  height = height + ( bs - base );
  if (height<0.0) height = 0.0;
  base = bs;
}

void cAOI::SetTop(double tp)
{
  if (tp<base) tp = base;
  height = tp - base;
}

void cAOI::SetHeights(double bs, double ht)
{
  SetBase(bs);
  SetTop(bs+ht);
}

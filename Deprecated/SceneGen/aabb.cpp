// AABB.cpp: implementation of the cAABB class (axis aligned bounding box)
//
//////////////////////////////////////////////////////////////////////

#include "aabb.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

cAABB::cAABB()
{
  Reset();
}

cAABB::~cAABB()
{

}

void cAABB::Reset()
{
  mn[0] = gmGOOGOL; mn[1] = gmGOOGOL; mn[2] = gmGOOGOL;
  mx[0] = -gmGOOGOL; mx[1] = -gmGOOGOL; mx[2] = -gmGOOGOL;
}

void cAABB::Adjust( gmVector3 vtx )
{
  if (vtx[0] < mn[0]) mn[0] = vtx[0];
  if (vtx[1] < mn[1]) mn[1] = vtx[1];
  if (vtx[2] < mn[2]) mn[2] = vtx[2];

  if (vtx[0] > mx[0]) mx[0] = vtx[0];
  if (vtx[1] > mx[1]) mx[1] = vtx[1];
  if (vtx[2] > mx[2]) mx[2] = vtx[2];
}

bool cAABB::Inside( gmVector3 vtx )
{
  if (vtx[0] < mn[0]) return false;
  if (vtx[1] < mn[1]) return false;
  if (vtx[2] < mn[2]) return false;

  if (vtx[0] > mx[0]) return false;
  if (vtx[1] > mx[1]) return false;
  if (vtx[2] > mx[2]) return false;

  return true;
}

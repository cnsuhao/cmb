/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkOrientedGlyphContourRepresentation2.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkOrientedGlyphContourRepresentation2.h"
#include "vtkCleanPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkObjectFactory.h"
#include "vtkProperty.h"
#include "vtkAssemblyPath.h"
#include "vtkMath.h"
#include "vtkInteractorObserver.h"
#include "vtkLine.h"
#include "vtkCoordinate.h"
#include "vtkGlyph3D.h"
#include "vtkCursor2D.h"
#include "vtkCylinderSource.h"
#include "vtkPolyData.h"
#include "vtkPoints.h"
#include "vtkDoubleArray.h"
#include "vtkPointData.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkTransform.h"
#include "vtkCamera.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"
#include "vtkFocalPlanePointPlacer.h"
#include "vtkBezierContourLineInterpolator.h"
#include "vtkSphereSource.h"
#include "vtkIncrementalOctreePointLocator.h"

#include <map>

vtkStandardNewMacro(vtkOrientedGlyphContourRepresentation2);

namespace
{
  typedef std::map<int,int> vtkInternalMapBase;
  typedef std::map<int,int>::iterator vtkInternalMapIterator;
  enum ModifiedPointFlags
    {
    Point_Modified = 1 << 1,
    Point_Deleted = 1 << 2,
    Point_Inserted = 1 << 4
    };
}
class vtkOrientedGlyphContourRepresentation2::vtkInternalMap : public vtkInternalMapBase {};


//----------------------------------------------------------------------
vtkOrientedGlyphContourRepresentation2::vtkOrientedGlyphContourRepresentation2()
{
  this->LoggingEnabled = false;
  this->ModifiedPointMap = new vtkOrientedGlyphContourRepresentation2::vtkInternalMap();
}

//----------------------------------------------------------------------
vtkOrientedGlyphContourRepresentation2::~vtkOrientedGlyphContourRepresentation2()
{
  if ( this->ModifiedPointMap )
    {
    delete this->ModifiedPointMap;
    }
}

//----------------------------------------------------------------------
int vtkOrientedGlyphContourRepresentation2::DeleteNthNode(int n)
{
  int good = this->Superclass::DeleteNthNode(n);
  if ( good )
    {
    this->UpdatePropertyMap(n,Point_Deleted);
    }
  return good;
}

//----------------------------------------------------------------------
int vtkOrientedGlyphContourRepresentation2::SetNthNodeSelected(int n)
{
  if ( n < 0 ||
    static_cast<unsigned int>(n) >= this->Internal->Nodes.size() )
    {
    // Failed.
    return 0;
    }

  if(this->Internal->Nodes[n]->Selected != 1)
    {
    this->Internal->Nodes[n]->Selected = 1;
    this->NeedToRender = 1;
    this->Modified();
    }
  return 1;
}

//-----------------------------------------------------------------------------
void vtkOrientedGlyphContourRepresentation2::UpdateLines(int index)
{
  this->UpdatePropertyMap(index,Point_Modified);
  this->Superclass::UpdateLines(index);
}

//----------------------------------------------------------------------
int vtkOrientedGlyphContourRepresentation2::AddNodeOnContour(int X, int Y)
{
  int idx;

  double worldPos[3];
  double worldOrient[9] = {1.0,0.0,0.0,
                           0.0,1.0,0.0,
                           0.0,0.0,1.0};

  // Compute the world position from the display position
  // based on the concrete representation's constraints
  // If this is not a valid display location return 0
  double displayPos[2];
  displayPos[0] = X;
  displayPos[1] = Y;
  if ( !this->PointPlacer->ComputeWorldPosition( this->Renderer,
                                                 displayPos, worldPos,
                                                 worldOrient) )
    {
    return 0;
    }

  double pos[3];
  if ( !this->FindClosestPointOnContour( X, Y, pos, &idx ) )
    {
    return 0;
    }

  if ( !this->PointPlacer->ComputeWorldPosition( this->Renderer,
                                                 displayPos,
                                                 pos,
                                                 worldPos,
                                                 worldOrient) )
    {
    return 0;
    }

  // Add a new point at this position
  vtkContourRepresentationNode *node = new vtkContourRepresentationNode;
  node->WorldPosition[0] = worldPos[0];
  node->WorldPosition[1] = worldPos[1];
  node->WorldPosition[2] = worldPos[2];
  node->Selected = 0;

  this->GetRendererComputedDisplayPositionFromWorldPosition(
          worldPos, worldOrient, node->NormalizedDisplayPosition );
  this->Renderer->DisplayToNormalizedDisplay(
         node->NormalizedDisplayPosition[0],
         node->NormalizedDisplayPosition[1] );

  memcpy(node->WorldOrientation, worldOrient, 9*sizeof(double) );

  this->Internal->Nodes.insert(this->Internal->Nodes.begin() + idx, node);

  this->UpdatePropertyMap(idx,Point_Inserted);
  this->Superclass::UpdateLines( idx );
  this->NeedToRender = 1;

  return 1;
}
//----------------------------------------------------------------------
void vtkOrientedGlyphContourRepresentation2::UpdatePropertyMap(int index, int flags)
{
  if ( this->GetLoggingEnabled() == 1 )
    {
    vtkInternalMap::iterator it;
    it = this->ModifiedPointMap->find(index);
    if ( it == this->ModifiedPointMap->end() )
      {
      int value = 0;
      value |= flags;
      this->ModifiedPointMap->insert(it,
        std::pair<int,int>(index,value));
      }
    else if ( flags & Point_Inserted )
      {
      //special use case, we have to insert a value into the map
      //this means we have to recurse a bit
      int oldFlags = it->second;
      it->second = Point_Inserted;
      this->UpdatePropertyMap(index+1,oldFlags);
      }
    else
      {
      it->second |= flags;
      }
    }
  }

vtkPolyData* vtkOrientedGlyphContourRepresentation2::GetContourRepresentationAsPolyData()
{
  // Make sure we are up to date with any changes made in the placer
  this->UpdateContour();
  this->BuildLines();

  return Lines;
 }


//-----------------------------------------------------------------------------
int vtkOrientedGlyphContourRepresentation2::GetNodeModifiedFlags(int n)
{
  int flag = 0;
  vtkInternalMap::iterator it;
  it = this->ModifiedPointMap->find(n);
  if ( it != this->ModifiedPointMap->end() )
    {
    flag = it->second;
    }
   return flag;
}


//----------------------------------------------------------------------
void vtkOrientedGlyphContourRepresentation2::Initialize( vtkPolyData * pd )
{
  vtkPoints *points   = pd->GetPoints();
  vtkIdType nPoints = points->GetNumberOfPoints();
  if (nPoints <= 0)
    {
    return; // Yeah right.. build from nothing !
    }

  // Clear all existing nodes.
  for(unsigned int i=0;i<this->Internal->Nodes.size();i++)
    {
    for (unsigned int j=0;j<this->Internal->Nodes[i]->Points.size();j++)
      {
      delete this->Internal->Nodes[i]->Points[j];
      }
    this->Internal->Nodes[i]->Points.clear();
    delete this->Internal->Nodes[i];
    }
  this->Internal->Nodes.clear();

  vtkPolyData *tmpPoints = vtkPolyData::New();
  tmpPoints->DeepCopy(pd);
  this->Locator->SetDataSet(tmpPoints);
  tmpPoints->Delete();

  //reserver space in memory to speed up vector push_back
  this->Internal->Nodes.reserve(nPoints);

  //account for the offset if the input has vert cells
  vtkIdList *pointIds = pd->GetCell(pd->GetNumberOfVerts())->GetPointIds();
  vtkIdType numPointsInLineCells = pointIds->GetNumberOfIds();

  // Get the worldOrient from the point placer
  double ref[3], displayPos[2], worldPos[3];
  double worldOrient[9] = {1.0,0.0,0.0,
                           0.0,1.0,0.0,
                           0.0,0.0,1.0};
  ref[0] = 0.0; ref[1] = 0.0; ref[2] = 0.0;
  displayPos[0] = 0.0; displayPos[1] = 0.0;
  this->PointPlacer->ComputeWorldPosition(this->Renderer,
                                 displayPos, ref, worldPos, worldOrient );

  // Add nodes without calling rebuild lines
  // to improve performance dramatically(~15x) on large datasets

  double *pos;
  //we use nPoints so we don't add the last point
  //if it is a closed loop, since that is covered as an exception
  //after the for loop
  for ( vtkIdType i=0; i < nPoints; i++ )
    {
    pos = points->GetPoint( pointIds->GetId(i) );
    this->GetRendererComputedDisplayPositionFromWorldPosition(
                          pos, worldOrient, displayPos );

    // Add a new point at this position
    vtkContourRepresentationNode *node = new vtkContourRepresentationNode;
    node->WorldPosition[0] = pos[0];
    node->WorldPosition[1] = pos[1];
    node->WorldPosition[2] = pos[2];
    node->Selected = 0;

    node->NormalizedDisplayPosition[0] = displayPos[0];
    node->NormalizedDisplayPosition[1] = displayPos[1];

    this->Renderer->DisplayToNormalizedDisplay(
      node->NormalizedDisplayPosition[0],
      node->NormalizedDisplayPosition[1] );

    memcpy(node->WorldOrientation, worldOrient, 9*sizeof(double) );

    this->Internal->Nodes.push_back(node);

    if ( this->LineInterpolator && this->GetNumberOfNodes() > 1 )
      {
      // Give the line interpolator a chance to update the node.
      int didNodeChange = this->LineInterpolator->UpdateNode(
        this->Renderer, this, node->WorldPosition, this->GetNumberOfNodes()-1 );

      // Give the point placer a chance to validate the updated node. If its not
      // valid, discard the LineInterpolator's change.
      if ( didNodeChange && !this->PointPlacer->ValidateWorldPosition(
                node->WorldPosition, worldOrient ) )
        {
        node->WorldPosition[0] = worldPos[0];
        node->WorldPosition[1] = worldPos[1];
        node->WorldPosition[2] = worldPos[2];
        }
      }
    }

  if (pointIds->GetId(0) == pointIds->GetId(numPointsInLineCells-1))
    {
    this->SetClosedLoop(1);
    }

  // Update the contour representation from the nodes using the line interpolator
  for (vtkIdType i=1; i <= nPoints; ++i)
    {
    this->UpdateLines(i);
    }
  this->BuildRepresentation();

  // Show the contour.
  this->VisibilityOn();
}


//-----------------------------------------------------------------------------
int vtkOrientedGlyphContourRepresentation2::RenderOpaqueGeometry(
  vtkViewport *viewport)
{
  // Since we know RenderOpaqueGeometry gets called first, will do the
  // build here
  this->BuildRepresentation();

  int count=0;
  count += this->LinesActor->RenderOpaqueGeometry(viewport);
  if ( this->Actor->GetVisibility() )
    {
    count += this->Actor->RenderOpaqueGeometry(viewport);
    }
  if ( this->ActiveActor->GetVisibility() )
    {
    count += this->ActiveActor->RenderOpaqueGeometry(viewport);
    }
  if(this->ShowSelectedNodes && this->SelectedNodesActor &&
      this->SelectedNodesActor->GetVisibility())
    {
    count += this->SelectedNodesActor->RenderOpaqueGeometry(viewport);
    }

  return count;
}


//-----------------------------------------------------------------------------
void vtkOrientedGlyphContourRepresentation2::PrintSelf(ostream& os,
                                                      vtkIndent indent)
{
  //Superclass typedef defined in vtkTypeMacro() found in vtkSetGet.h
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Logging Enabled: "
     << (this->LoggingEnabled ? "On\n" : "Off\n");
}

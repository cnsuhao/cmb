//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkClipPolygons.h"

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

#include <math.h>

vtkStandardNewMacro(vtkClipPolygons);
vtkCxxSetObjectMacro(vtkClipPolygons, Transform, vtkTransform);

//----------------------------------------------------------------------------
// Construct with user-specified implicit function; InsideOut turned off; value
// set to 0.0; and generate clip scalars turned off.
vtkClipPolygons::vtkClipPolygons()
{
  this->Locator = NULL;
  this->Value = 0.0;
  this->Transform = 0;
  this->ActiveGroupIdx = -1;
  this->IsProcessing = false;

  this->SetNumberOfOutputPorts(2);

  this->GenerateClippedOutput = 0;
  vtkPolyData *output2 = vtkPolyData::New();
  this->GetExecutive()->SetOutputData(1, output2);
  output2->Delete();
}

//----------------------------------------------------------------------------
vtkClipPolygons::~vtkClipPolygons()
{
  if ( this->Locator )
    {
    this->Locator->UnRegister(this);
    this->Locator = NULL;
    }
  this->SetTransform(static_cast<vtkTransform*>(0));
  this->RemoveAllClipPolygons();
}

//----------------------------------------------------------------------------
int vtkClipPolygons::IsActiveGroupValid()
{
  if ( this->ActiveGroupIdx < 0)
    {
    return 0;
    }
  return 1;
}

//----------------------------------------------------------------------------
void vtkClipPolygons::SetGroupInvert(int val)
{
  if(this->IsActiveGroupValid() &&
    (this->GroupInvert.find(this->ActiveGroupIdx) == this->GroupInvert.end() ||
    this->GroupInvert[this->ActiveGroupIdx] != val))
    {
    this->GroupInvert[this->ActiveGroupIdx] = val;
    this->Modified();
    }
}

//----------------------------------------------------------------------------
void vtkClipPolygons::AddClipPolygon(vtkImplicitFunction* cf)
{
  if(this->IsActiveGroupValid())
    {
    PolygonInfo* cfInfo = new PolygonInfo();
    cfInfo->Polygon = cf;
    this->Polygons[this->ActiveGroupIdx].push_back(cfInfo);
    if (this->Polygons[this->ActiveGroupIdx].size() == 1)
      {
      this->GroupInvert[this->ActiveGroupIdx] = 0;
      }
    this->Modified();
    }
}

//----------------------------------------------------------------------------
void vtkClipPolygons::RemoveClipPolygon(vtkImplicitFunction* cf)
{
  if(cf && this->IsActiveGroupValid())
    {
    bool removed = false;
    for(std::vector<PolygonInfo*>::iterator itRemove =
      this->Polygons[this->ActiveGroupIdx].begin();
      itRemove !=this->Polygons[this->ActiveGroupIdx].end(); itRemove++)
      {
      if((*itRemove)->Polygon == cf)
        {
        delete *itRemove;
        this->Polygons[this->ActiveGroupIdx].erase(itRemove);
        removed = true;
        break;
        }
      }
    if(removed)
      {
      this->Modified();
      }
    }
}

//----------------------------------------------------------------------------
void vtkClipPolygons::RemoveAllClipPolygons()
{
  for(std::map<int, std::vector<PolygonInfo*> >::iterator it=this->Polygons.begin();
    it != this->Polygons.end(); it++)
    {
    for(std::vector<PolygonInfo*>::iterator itRemove =
      it->second.begin(); itRemove !=it->second.end(); itRemove++)
      {
      delete *itRemove;
      }
    }
  this->Polygons.clear();
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkClipPolygons::SetClipApplyPolygon(int idx, int val)
{
  if(this->IsActiveGroupValid())
    {
    if(idx>=0 && idx<static_cast<int>(this->Polygons[this->ActiveGroupIdx].size()))
      {
      if(this->Polygons[this->ActiveGroupIdx][idx]->ApplyPolygon != val)
        {
        this->Polygons[this->ActiveGroupIdx][idx]->ApplyPolygon = val;
        }
      this->Modified();
      }
    }
}

//----------------------------------------------------------------------------
void vtkClipPolygons::SetClipInsideOut(int idx, int val)
{
  if(this->IsActiveGroupValid())
    {
    if(idx>=0 && idx<static_cast<int>(this->Polygons[this->ActiveGroupIdx].size()))
      {
      if(this->Polygons[this->ActiveGroupIdx][idx]->InsideOut != val)
        {
        this->Polygons[this->ActiveGroupIdx][idx]->InsideOut = val;
        }
      this->Modified();
      }
    }
}

//----------------------------------------------------------------------------
void vtkClipPolygons::SetClipAsROI(int idx, int val)
{
  if(this->IsActiveGroupValid())
    {
    if(idx>=0 && idx<static_cast<int>(this->Polygons[this->ActiveGroupIdx].size()))
      {
      if(this->Polygons[this->ActiveGroupIdx][idx]->AsROI != val)
        {
        this->Polygons[this->ActiveGroupIdx][idx]->AsROI = val;
        if(val)
          {
          this->Polygons[this->ActiveGroupIdx][idx]->InsideOut = 0;
          }
        }
      this->Modified();
      }
    }
}

//-----------------------------------------------------------------------------
void vtkClipPolygons::SetTransform(double elements[16])
{
  vtkTransform *tmpTransform = vtkTransform::New();
  tmpTransform->SetMatrix(elements);
  this->SetTransform(tmpTransform);
  tmpTransform->Delete();
}

//----------------------------------------------------------------------------
// Overload standard modified time function. If Clip functions is modified,
// then this object is modified as well.
unsigned long vtkClipPolygons::GetMTime()
{
  unsigned long mTime=this->Superclass::GetMTime();
  unsigned long time;

  for(std::map<int, std::vector<PolygonInfo*> >::iterator itMap=this->Polygons.begin();
    itMap != this->Polygons.end(); itMap++)
    {
    for(std::vector<PolygonInfo*>::iterator it =
      itMap->second.begin(); it !=itMap->second.end(); it++)
      {
      if((*it)->Polygon)
        {
        time = (*it)->Polygon->GetMTime();
        mTime = ( time > mTime ? time : mTime );
        }
      }
    }
  if ( this->Locator != NULL )
    {
    time = this->Locator->GetMTime();
    mTime = ( time > mTime ? time : mTime );
    }

  return mTime;
}

//----------------------------------------------------------------------------
vtkPolyData *vtkClipPolygons::GetClippedOutput()
{
  return vtkPolyData::SafeDownCast(
    this->GetExecutive()->GetOutputData(1));
}

//----------------------------------------------------------------------------
vtkAlgorithmOutput* vtkClipPolygons::GetClippedOutputPort()
{
  return this->GetOutputPort(1);
}

//----------------------------------------------------------------------------
//
// Clip through data generating surface.
//
int vtkClipPolygons::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  if(this->IsProcessing)
    {
    vtkErrorMacro(<< "vtkClipPolygons::RequestData is called again while it is still processing");
    return 0;
    }

  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and output
  vtkPolyData *input = vtkPolyData::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkIdType cellId, i, updateTime;
  vtkPoints *cellPts;
  vtkDataArray *clipScalars;
  vtkFloatArray *cellScalars;
  vtkGenericCell *cell;
  vtkCellArray *newVerts, *newLines, *newPolys, *connList=NULL;
  vtkCellArray *clippedVerts=NULL, *clippedLines=NULL;
  vtkCellArray *clippedPolys=NULL, *clippedList=NULL;
  vtkPoints *newPoints;
  vtkIdList *cellIds;
  double s;
  vtkIdType estimatedSize, numCells=input->GetNumberOfCells();
  vtkIdType numPts=input->GetNumberOfPoints();
  vtkPoints *inPts=input->GetPoints();
  int numberOfPoints;
  vtkPointData *inPD=input->GetPointData(), *outPD = output->GetPointData();
  vtkCellData *inCD=input->GetCellData(), *outCD = output->GetCellData();
  vtkCellData *outClippedCD = NULL;

  vtkDebugMacro(<< "Clipping polygonal data");

  // Initialize self; create output objects
  //
  if ( numPts < 1 || inPts == NULL)
    {
    vtkWarningMacro(<<"No data to clip");
    return 1;
    }

  this->IsProcessing = true;

  bool HasClipFunc = false;
  std::map<int, std::vector<PolygonInfo*> >ClipFuncs;
  //std::vector<PolygonInfo*> ClipROIs;
  //get all acitve polygons
  for(std::map<int, std::vector<PolygonInfo*> >::iterator itMap=this->Polygons.begin();
    itMap != this->Polygons.end(); itMap++)
    {
    for(std::vector<PolygonInfo*>::iterator it =
      itMap->second.begin(); it !=itMap->second.end(); it++)
      {
      if((*it)->Polygon && (*it)->ApplyPolygon)
        {
        HasClipFunc = true;
        ClipFuncs[itMap->first].push_back(*it);
        }
/*
      else
        {
        HasClipROI = true;
        ClipROIs.push_back(*it);
        }
*/
      }
    }

  if (!HasClipFunc)
    {
    output->ShallowCopy(input);
    this->IsProcessing = false;
    return 1;
    }
  this->UpdateProgress( 0.0 );

  vtkFloatArray *tmpScalars = vtkFloatArray::New();
  tmpScalars->SetNumberOfTuples(numPts);
  inPD = vtkPointData::New();
  inPD->ShallowCopy(input->GetPointData());//copies original
//  bool passAllClip;
//  bool insideROI;
  bool shouldClip = false;
  double transformedPoint[3], *pointPtr;
  double point[3];
  for ( i=0; i < numPts; i++ )
    {
    inPts->GetPoint(i, point);
    if (this->Transform)
      {
      this->Transform->TransformPoint(point, transformedPoint);
      pointPtr = transformedPoint;
      }
    else
      {
      pointPtr = point;
      }
    for(std::map<int, std::vector<PolygonInfo*> >::iterator itMap=this->Polygons.begin();
        itMap != this->Polygons.end(); itMap++)

      for(std::map<int, std::vector<PolygonInfo*> >::iterator itFuncs=ClipFuncs.begin();
        itFuncs != ClipFuncs.end(); itFuncs++)
      {
      shouldClip = true;
      for(std::vector<PolygonInfo*>::iterator itF =
        itFuncs->second.begin(); itF !=itFuncs->second.end(); itF++)
        {
        s = (*itF)->Polygon->FunctionValue(pointPtr);
        if (( !((*itF)->InsideOut) && s <= 0) || (((*itF)->InsideOut) && s > 0))
          {
          shouldClip = false;
          break;
          }
        }
      if (this->GroupInvert[itFuncs->first])
        {
        shouldClip = !shouldClip;
        }
      if (shouldClip)
        {
        break;
        }
      }
    s = shouldClip ? (this->Value - 1.0) : (this->Value + 1.0) ;
    tmpScalars->SetComponent(i,0,s);
    }
  clipScalars = tmpScalars;

  // Create objects to hold output of clip operation
  //
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

  // locator used to merge potentially duplicate points
  if ( this->Locator == NULL )
    {
    this->CreateDefaultLocator();
    }
  this->Locator->InitPointInsertion (newPoints, input->GetBounds());

  if ( !input->GetPointData()->GetScalars())
    {
    outPD->CopyScalarsOff();
    }
  else
    {
    outPD->CopyScalarsOn();
    }
  outPD->InterpolateAllocate(inPD,estimatedSize,estimatedSize/2);
  outCD->CopyAllocate(inCD,estimatedSize,estimatedSize/2);

  // If generating second output, setup clipped output
  if ( this->GenerateClippedOutput )
    {
    this->GetClippedOutput()->Initialize();
    outClippedCD = this->GetClippedOutput()->GetCellData();
    outClippedCD->CopyAllocate(inCD,estimatedSize,estimatedSize/2);
    clippedVerts = vtkCellArray::New();
    clippedVerts->Allocate(estimatedSize,estimatedSize/2);
    clippedLines = vtkCellArray::New();
    clippedLines->Allocate(estimatedSize,estimatedSize/2);
    clippedPolys = vtkCellArray::New();
    clippedPolys->Allocate(estimatedSize,estimatedSize/2);
    }

  cellScalars = vtkFloatArray::New();
  cellScalars->Allocate(VTK_CELL_SIZE);

  // perform clipping on cells
  int abort=0;
  updateTime = numCells/20 + 1;  // update roughly every 5%
  cell = vtkGenericCell::New();
  for (cellId=0; cellId < numCells && !abort; cellId++)
    {
    input->GetCell(cellId,cell);
    cellPts = cell->GetPoints();
    cellIds = cell->GetPointIds();
    numberOfPoints = cellPts->GetNumberOfPoints();

    // evaluate implicit cutting function
    for ( i=0; i < numberOfPoints; i++ )
      {
      s = clipScalars->GetComponent(cellIds->GetId(i),0);
      cellScalars->InsertTuple(i, &s);
      }

    switch ( cell->GetCellDimension() )
      {
      case 0: //points are generated-------------------------------
        connList = newVerts;
        clippedList = clippedVerts;
        break;

      case 1: //lines are generated----------------------------------
        connList = newLines;
        clippedList = clippedLines;
        break;

      case 2: //triangles are generated------------------------------
        connList = newPolys;
        clippedList = clippedPolys;
        break;

      } //switch

    cell->Clip(this->Value, cellScalars, this->Locator, connList,
               inPD, outPD, inCD, cellId, outCD, 0);

    if ( this->GenerateClippedOutput )
      {
      cell->Clip(this->Value, cellScalars, this->Locator, clippedList,
                 inPD, outPD, inCD, cellId, outClippedCD, 0);
      }

    // The UpdateProgress is somehow(?) causing the outPD's Arrays got cleared out!!
    if ( !(cellId % updateTime) )
      {
      this->UpdateProgress(static_cast<double>(cellId) / numCells);
      abort = this->GetAbortExecute();
      }
    } //for each cell
  cell->Delete();

  vtkDebugMacro(<<"Created: "
               << newPoints->GetNumberOfPoints() << " points, "
               << newVerts->GetNumberOfCells() << " verts, "
               << newLines->GetNumberOfCells() << " lines, "
               << newPolys->GetNumberOfCells() << " polys");

  if ( this->GenerateClippedOutput )
    {
    vtkDebugMacro(<<"Created (clipped output): "
                 << clippedVerts->GetNumberOfCells() << " verts, "
                 << clippedLines->GetNumberOfCells() << " lines, "
                 << clippedPolys->GetNumberOfCells() << " triangles");
    }

  // Update ourselves.  Because we don't know upfront how many verts, lines,
  // polys we've created, take care to reclaim memory.
  //

  if ( this->Polygons.size() )
    {
    clipScalars->Delete();
    inPD->Delete();
    }

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

  if ( this->GenerateClippedOutput )
    {
    this->GetClippedOutput()->SetPoints(newPoints);

    if (clippedVerts->GetNumberOfCells())
      {
      this->GetClippedOutput()->SetVerts(clippedVerts);
      }
    clippedVerts->Delete();

    if (clippedLines->GetNumberOfCells())
      {
      this->GetClippedOutput()->SetLines(clippedLines);
      }
    clippedLines->Delete();

    if (clippedPolys->GetNumberOfCells())
      {
      this->GetClippedOutput()->SetPolys(clippedPolys);
      }
    clippedPolys->Delete();

    this->GetClippedOutput()->GetPointData()->PassData(outPD);
    this->GetClippedOutput()->Squeeze();
    }

  output->SetPoints(newPoints);
  newPoints->Delete();
  cellScalars->Delete();

  this->Locator->Initialize();//release any extra memory
  output->Squeeze();
  this->IsProcessing = false;
  this->UpdateProgress( 1.0 );

  return 1;
}


//----------------------------------------------------------------------------
// Specify a spatial locator for merging points. By default,
// an instance of vtkMergePoints is used.
void vtkClipPolygons::SetLocator(vtkIncrementalPointLocator *locator)
{
  if ( this->Locator == locator)
    {
    return;
    }

  if ( this->Locator )
    {
    this->Locator->UnRegister(this);
    this->Locator = NULL;
    }

  if ( locator )
    {
    locator->Register(this);
    }

  this->Locator = locator;
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkClipPolygons::CreateDefaultLocator()
{
  if ( this->Locator == NULL )
    {
    this->Locator = vtkMergePoints::New();
    }
}

//----------------------------------------------------------------------------
int vtkClipPolygons::GetNumberOfActivePolygons()
{
  int ret = 0;
  for(std::map<int, std::vector<PolygonInfo*> >::iterator itMap=this->Polygons.begin();
    itMap != this->Polygons.end(); itMap++)
    {
    for(std::vector<PolygonInfo*>::iterator it =
      itMap->second.begin(); it !=itMap->second.end(); it++)
      {
      if((*it)->Polygon && (*it)->ApplyPolygon)
        {
        ret++;
        }
      }
    }
  return ret;
}

//----------------------------------------------------------------------------
void vtkClipPolygons::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
/*
  if ( this->ClipFunction )
    {
    os << indent << "Clip Function: " << this->ClipFunction << "\n";
    }
  else
    {
    os << indent << "Clip Function: (none)\n";
    }
  os << indent << "InsideOut: " << (this->InsideOut ? "On\n" : "Off\n");
  os << indent << "Value: " << this->Value << "\n";
*/
  if ( this->Locator )
    {
    os << indent << "Locator: " << this->Locator << "\n";
    }
  else
    {
    os << indent << "Locator: (none)\n";
    }

  os << indent << "Generate Clipped Output: " << (this->GenerateClippedOutput ? "On\n" : "Off\n");
}

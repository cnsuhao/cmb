//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkSceneContourSource.h"

#include "vtkCellArray.h"
#include "vtkContourPointCollection.h"
#include "vtkDoubleArray.h"
#include "vtkFloatArray.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMergePoints.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"

#include <set>

//for data dumping in debug
//#define DUMP_DEBUG_DATA
#ifdef DUMP_DEBUG_DATA
#define DUMP_DEBUG_DATA_DIR "E:/Work/"
#include "vtkNew.h"
#include "vtkXMLPolyDataWriter.h"
#include <sstream>
#endif

namespace
{
enum ModifiedPointFlags
{
  Point_Modified = 1 << 1,
  Point_Deleted = 1 << 2,
  Point_Inserted = 1 << 4
};
}

vtkIdType vtkSceneContourSource::NextId = 0;

vtkStandardNewMacro(vtkSceneContourSource);

vtkSceneContourSource::vtkSceneContourSource()
  : Id(NextId++)
{
  this->Source = vtkPolyData::New();
  this->Source->SetVerts(NULL);
  this->Source->SetLines(NULL);
  this->Source->SetPolys(NULL);

  this->EndNodes = vtkIdTypeArray::New();
  this->Collection = vtkContourPointCollection::GetInstance();

  //Register is needed since if a contour is on the undo stack it still exists,
  //but it has no end nodes in the collection mapper, but shouldn't be deleted
  this->Collection->RegisterContour(this->Id);

  this->ClosedLoop = false;
  this->SetNumberOfInputPorts(0);
}

vtkSceneContourSource::~vtkSceneContourSource()
{
  if (this->Source)
  {
    this->Source->Delete();
  }

  if (this->EndNodes)
  {
    //we have to keep the collection end node counts correct
    for (vtkIdType i = 0; i < this->EndNodes->GetNumberOfTuples(); ++i)
    {
      this->Collection->RemoveAsEndNode(this->EndNodes->GetValue(i), this->Id);
    }
    this->EndNodes->Delete();
  }

  //We unregister so that when the collection hits zero registered
  //contours it cleanups the point locator. this fixes bug #9548 and #9575
  //remember unregister is called on delete, while RemoveAsEndNode is called
  //at delete and UNDO / REDO time
  this->Collection->UnRegisterContour(this->Id);

  this->Collection = NULL;
}

vtkPolyData* vtkSceneContourSource::GetWidgetOutput()
{

  if (!this->Source)
  {
    return NULL;
  }

  // get the ouptut
  vtkPolyData* output = vtkPolyData::New();
  vtkPoints* points = vtkPoints::New();
  vtkCellArray* lines = vtkCellArray::New();
  vtkIdList* newIds = vtkIdList::New();

  //setup an array to show that a point is selected
  vtkIdTypeArray* selected = vtkIdTypeArray::New();
  selected->SetName("selected");

  double pos[3];
  vtkIdType npts, *pts;
  this->Source->GetLines()->GetCell(0, npts, pts);
  npts -= this->GetClosedLoop();
  selected->SetNumberOfValues(npts);
  points->SetNumberOfPoints(npts);
  for (vtkIdType i = 0; i < npts; ++i)
  {
    this->Source->GetPoint(pts[i], pos);
    points->SetPoint(i, pos);
    selected->SetValue(i, this->Collection->IsEndNode(pts[i]));
    newIds->InsertNextId(i);
  }

  lines->InsertNextCell(newIds);
  newIds->Delete();
  output->SetPoints(points);
  output->GetPointData()->AddArray(selected);
  output->SetLines(lines);
  selected->FastDelete();
  points->FastDelete();
  lines->FastDelete();
  return output;
}

void vtkSceneContourSource::CopyData(vtkPolyData* source)
{
  if (!source)
  {
    vtkErrorMacro("Unable to copy data as input is NULL");
    return;
  }

#ifdef DUMP_DEBUG_DATA
  {
    vtkNew<vtkXMLPolyDataWriter> writer;
    writer->SetInput(source);
    std::stringstream buffer;
    buffer << DUMP_DEBUG_DATA_DIR << "contourInput_" << this->Id << ".vtp";
    writer->SetFileName(buffer.str().c_str());
    writer->Write();
    writer->Update();
  }
#endif

  //should this be a class ivar?
  if (this->Source->GetNumberOfLines() == 0)
  {
    this->InitSourceData(source);
  }
  else
  {
    this->EditSourceData(source);
  }

  //update the selected nodes
  this->UpdateSelectedNodes(source);

#ifdef DUMP_DEBUG_DATA
  {
    vtkNew<vtkXMLPolyDataWriter> writer;
    writer->SetInput(this->Source);
    std::stringstream buffer;
    buffer << DUMP_DEBUG_DATA_DIR << "contourOutput_" << this->Id << ".vtp";
    writer->SetFileName(buffer.str().c_str());
    writer->Write();
    writer->Update();
  }
#endif
}

void vtkSceneContourSource::InitSourceData(vtkPolyData* source)
{

  vtkMergePoints* pointLocator = this->Collection->GetPointLocator();

  //now we need to go through all the lines in the source
  //construct a duplicate cell structure that uses the global id collection
  vtkCellArray* lines = vtkCellArray::New();
  vtkCellArray* oldLines = source->GetLines();
  if (!oldLines)
  {
    vtkErrorMacro("Unable to copy contour since input had no lines.");
    return;
  }

  vtkIdList* oldIds = vtkIdList::New();
  vtkIdList* newIds = vtkIdList::New();
  vtkIdType newId = -1;
  vtkIdType oldId = -1;
  vtkIdType previousId = -1;
  double pos[3];
  oldLines->InitTraversal();

  while (oldLines->GetNextCell(oldIds))
  {
    //for each cell we convert the ids from the old set, to the new id's
    for (vtkIdType i = 0; i < oldIds->GetNumberOfIds(); ++i)
    {
      oldId = oldIds->GetId(i);
      source->GetPoint(oldId, pos);
      pointLocator->InsertUniquePoint(pos, newId);
      if (previousId != newId)
      {
        //we don't want degenerate cells that connect to the same point
        //ie have a cell array like 6 0 1 1 2 2 3 instead we want
        // 4 0 1 2 3
        newIds->InsertNextId(newId);
      }
      previousId = newId;
    }
    //clear the id structures
    oldIds->Reset();
  }
  newIds->Squeeze();

  //now time to detect if we have a closed loop
  if (newIds->GetNumberOfIds() > 1)
  {
    this->SetClosedLoop(newIds->GetId(0) == newIds->GetId(newIds->GetNumberOfIds() - 1));
  }

  //now that we have made a single cell with all the points, lets insert it
  lines->InsertNextCell(newIds);

  oldIds->Delete();
  newIds->Delete();

  this->Source->SetPoints(this->Collection->GetPoints());
  this->Source->SetLines(lines);

  lines->FastDelete();
  this->Modified();
}

void vtkSceneContourSource::EditSourceData(vtkPolyData* source)
{
  vtkMergePoints* pointLocator = this->Collection->GetPointLocator();

  if (!source->GetPointData()->HasArray("ModifiedPointFlags"))
  {
    //since this source doesn't have a bit mask for modifications, we can't
    //do any smart processing of it
    this->InitSourceData(source);
    return;
  }

  vtkIntArray* modFlags =
    vtkIntArray::SafeDownCast(source->GetPointData()->GetArray("ModifiedPointFlags"));

  vtkCellArray* oldLines = this->Source->GetLines();
  vtkCellArray* sourceLines = source->GetLines();
  vtkCellArray* newLines = vtkCellArray::New();

  vtkIdList* oldIds = vtkIdList::New();
  vtkIdList* sourceIds = vtkIdList::New();
  vtkIdList* newIds = vtkIdList::New();

  vtkIdType sourceId = -1;
  vtkIdType oldId = -1;
  double pos[3];

  oldLines->InitTraversal();
  sourceLines->InitTraversal();
  while (sourceLines->GetNextCell(sourceIds))
  {
    oldLines->GetNextCell(oldIds);

    vtkIdType origLineNum = oldIds->GetNumberOfIds();
    vtkIdType sourceLineNum = sourceIds->GetNumberOfIds();

    bool pointsDeletedOrInserted = false;
    int flags = 0;
    for (vtkIdType i = 0; i < sourceLineNum && i < origLineNum; ++i)
    {
      oldId = oldIds->GetId(i);
      sourceId = sourceIds->GetId(i);
      source->GetPoint(sourceId, pos);

      if (modFlags->GetNumberOfTuples() <= i)
      {
        flags = 0;
      }
      else
      {
        flags = modFlags->GetValue(i);
      }

      if (flags & Point_Inserted || flags & Point_Deleted)
      {
        //since the point has a deleted flag, but we are iterating over
        //it, we know it still is a new point that was created
        pointLocator->InsertUniquePoint(pos, sourceId);
        newIds->InsertNextId(sourceId);
        pointsDeletedOrInserted = true;
      }
      else if (flags & Point_Modified && !pointsDeletedOrInserted)
      {
        //update the point location
        vtkDataArray* dataArray = this->Collection->GetPoints()->GetData();
        if (dataArray->GetDataType() == VTK_FLOAT)
        {
          vtkFloatArray* floatArray = static_cast<vtkFloatArray*>(dataArray);
          float* pt = floatArray->GetPointer(0) + 3 * oldId;
          pt[0] = static_cast<float>(pos[0]);
          pt[1] = static_cast<float>(pos[1]);
          pt[2] = static_cast<float>(pos[2]);
        }
        else
        {
          // Using the double interface
          vtkDoubleArray* doubleArray = static_cast<vtkDoubleArray*>(dataArray);
          double* pt = doubleArray->GetPointer(0) + 3 * oldId;
          pt[0] = pos[0];
          pt[1] = pos[1];
          pt[2] = pos[2];
        }

        //add this id to the extended new line id list
        newIds->InsertNextId(oldId);
      }
      else
      {
        pointLocator->InsertUniquePoint(pos, sourceId);
        newIds->InsertNextId(sourceId);
      }
    }

    //add any new points to the bowel of points
    for (vtkIdType i = origLineNum; i < sourceLineNum; ++i)
    {
      sourceId = sourceIds->GetId(i);
      source->GetPoint(sourceId, pos);
      pointLocator->InsertUniquePoint(pos, sourceId);
      newIds->InsertNextId(sourceId);
    }
  }

  newIds->Squeeze();
  newLines->InsertNextCell(newIds);

  //now time to detect if we have a closed loop
  if (newIds->GetNumberOfIds() > 1)
  {
    this->SetClosedLoop(newIds->GetId(0) == newIds->GetId(newIds->GetNumberOfIds() - 1));
  }

  //SetLines will delete the old cellArray
  this->Source->SetLines(newLines);
  newLines->FastDelete();

  oldIds->Delete();
  newIds->Delete();
  sourceIds->Delete();
  this->Modified(); //has to be called for the output to regenerated
}

void vtkSceneContourSource::UpdateSelectedNodes(vtkPolyData* source)
{
  //we need to keep the collection end nodes in sync
  //store all the old end node ids
  std::set<vtkIdType> oldEndNodes;
  for (vtkIdType i = 0; i < this->EndNodes->GetNumberOfTuples(); ++i)
  {
    oldEndNodes.insert(this->EndNodes->GetValue(i));
  }
  this->EndNodes->Reset();

  vtkMergePoints* pointLocator = this->Collection->GetPointLocator();
  vtkIntArray* selected =
    vtkIntArray::SafeDownCast(source->GetPointData()->GetArray("SelectedNodes"));
  vtkCellArray* verts = source->GetVerts();

  bool reprSelect = (selected != NULL);
  bool fileSelect = (verts && verts->GetNumberOfCells() > 0);

  if (reprSelect && !fileSelect)
  {
    //we have point data where 0 means not selected
    for (vtkIdType i = 0; i < selected->GetNumberOfTuples(); ++i)
    {
      vtkIdType id = selected->GetValue(i);
      if (id)
      {
        id = pointLocator->IsInsertedPoint(source->GetPoint(i));
        if (id >= 0)
        {
          this->EndNodes->InsertNextValue(id);
          this->Collection->SetAsEndNode(id, this->Id);
          oldEndNodes.erase(id);
        }
      }
    }
  }
  else if (fileSelect)
  {
    //we are looking at saved contours, so verts are selected nodes
    vtkIdList* vertIds = vtkIdList::New();
    verts->InitTraversal();
    while (verts->GetNextCell(vertIds))
    {
      for (vtkIdType i = 0; i < vertIds->GetNumberOfIds(); ++i)
      {
        vtkIdType id = pointLocator->IsInsertedPoint(source->GetPoint(vertIds->GetId(i)));
        if (id >= 0)
        {
          this->EndNodes->InsertNextValue(id);
          this->Collection->SetAsEndNode(id, this->Id);
          oldEndNodes.erase(id);
        }
      }
    }
    vertIds->Delete();
  }

  //now remove everything left in the set
  std::set<vtkIdType>::iterator it;
  for (it = oldEndNodes.begin(); it != oldEndNodes.end(); ++it)
  {
    this->Collection->RemoveAsEndNode((*it), this->Id);
  }

  //now force the first and last points on the arc set
  //to always be end nodes. This is law
  if (this->Source->GetLines() && this->Source->GetLines()->GetNumberOfCells() != 0)
  {
    vtkIdType npts, *pts;

    this->Source->GetLines()->GetCell(0, npts, pts);
    if (npts > 1)
    {
      this->EndNodes->InsertNextValue(pts[0]);
      this->Collection->SetAsEndNode(pts[0], this->Id);
      if (pts[0] != pts[npts - 1])
      {
        this->EndNodes->InsertNextValue(pts[npts - 1]);
        this->Collection->SetAsEndNode(pts[npts - 1], this->Id);
      }
    }
    else
    {
      this->EndNodes->SetNumberOfValues(0);
      vtkErrorMacro("Can't assign end nodes to a contour with less than two points.");
    }
  }
}

void vtkSceneContourSource::RegenerateEndNodes()
{
  //we regenerate the end nodes each time
  //we can't trust the what is in the end nodes

  if (!this->Source)
  {
    return;
  }
  vtkCellArray* arcs = this->Source->GetLines();
  if (!arcs)
  {
    return;
  }

  //build a set of old end nodes
  //while we loop through the new end nodes
  //we will remove them from this set
  //the result will be the end nodes that need
  //to be removed
  std::set<vtkIdType> oldEndNodes;
  for (vtkIdType i = 0; i < this->EndNodes->GetNumberOfTuples(); ++i)
  {
    oldEndNodes.insert(this->EndNodes->GetValue(i));
  }
  this->EndNodes->Reset();

  arcs->InitTraversal();
  vtkIdType cellIndex = 0;
  vtkIdType endIndex = arcs->GetNumberOfCells() - 1;
  vtkIdType nodeId, npts, *pts;
  while (arcs->GetNextCell(npts, pts))
  {
    if (cellIndex == 0)
    {
      nodeId = pts[0];
      this->EndNodes->InsertNextValue(nodeId);
      this->Collection->SetAsEndNode(nodeId, this->Id);
      oldEndNodes.erase(nodeId);
    }
    if (cellIndex == endIndex)
    {
      nodeId = pts[npts - 1];
      this->EndNodes->InsertNextValue(nodeId);
      this->Collection->SetAsEndNode(nodeId, this->Id);
      oldEndNodes.erase(nodeId);
    }
    vtkIdType start = (cellIndex == 0) ? 1 : 0;
    vtkIdType end = (cellIndex == endIndex) ? npts - 1 : npts;
    for (vtkIdType i = start; i < end; ++i)
    {
      nodeId = pts[i];
      if (this->Collection->IsEndNode(nodeId))
      {
        this->EndNodes->InsertNextValue(nodeId);
        //we need to register this arc as now using
        //this end node since it might not have been
        this->Collection->SetAsEndNode(nodeId, this->Id);
        oldEndNodes.erase(nodeId);
      }
    }
    ++cellIndex;
  }

  //now remove everything left in the set
  std::set<vtkIdType>::iterator it;
  for (it = oldEndNodes.begin(); it != oldEndNodes.end(); ++it)
  {
    this->Collection->RemoveAsEndNode((*it), this->Id);
  }
}

int vtkSceneContourSource::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inputVector), vtkInformationVector* outputVector)
{
  // get the ouptut
  vtkPolyData* output = vtkPolyData::SafeDownCast(
    outputVector->GetInformationObject(0)->Get(vtkDataObject::DATA_OBJECT()));

  // now move the input through to the output
  output->CopyStructure(this->Source);

  //now set the selected nodes to show up as actual verts
  vtkIdType selectedSize = (this->EndNodes) ? this->EndNodes->GetNumberOfTuples() : 0;

  //if we don't check agianst the number of points
  //we crash the renderer when creating an empty output, since
  //we always have a selected index ( xml default )
  if (selectedSize > 0 && this->Source && this->Source->GetLines())
  {
    vtkCellArray* verts = vtkCellArray::New();
    verts->InsertNextCell(selectedSize);
    for (vtkIdType i = 0; i < selectedSize; ++i)
    {
      vtkIdType idx = this->EndNodes->GetValue(i);
      verts->InsertCellPoint(idx);
    }
    output->SetVerts(verts);
    verts->FastDelete();
  }

  //we need to add the Id of this class to the output
  //so operator//filters understand which arc set this output is from
  if (output->GetFieldData()->HasArray("ArcSetId"))
  {
    output->GetFieldData()->RemoveArray("ArcSetId");
  }
  vtkIdTypeArray* arcSetId = vtkIdTypeArray::New();
  arcSetId->SetName("ArcSetId");
  arcSetId->SetNumberOfTuples(1);
  arcSetId->SetValue(0, this->Id);
  output->GetFieldData()->AddArray(arcSetId);
  arcSetId->FastDelete();

  return 1;
}

void vtkSceneContourSource::MarkedForDeletion()
{
  if (!this->EndNodes)
  {
    return;
  }

  vtkIdType nodeId;
  for (vtkIdType i = 0; i < this->EndNodes->GetNumberOfTuples(); ++i)
  {
    nodeId = this->EndNodes->GetValue(i);
    this->Collection->RemoveAsEndNode(nodeId, this->Id);
  }
}

void vtkSceneContourSource::UnMarkedForDeletion()
{
  if (!this->EndNodes)
  {
    return;
  }

  vtkIdType nodeId;
  for (vtkIdType i = 0; i < this->EndNodes->GetNumberOfTuples(); ++i)
  {
    nodeId = this->EndNodes->GetValue(i);
    this->Collection->SetAsEndNode(nodeId, this->Id);
  }
}

void vtkSceneContourSource::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Source: " << this->Source << "\n";
}

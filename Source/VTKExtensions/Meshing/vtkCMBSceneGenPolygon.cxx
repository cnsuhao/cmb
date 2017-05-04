//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkCMBSceneGenPolygon.h"

#include "vtkAppendPolyData.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkCellLocator.h"
#include "vtkCleanPolyData.h"
#include "vtkGenericCell.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkPolygon.h"

#include "smtk/extension/vtk/meshing/vtkCMBPrepareForTriangleMesher.h"
#include "smtk/extension/vtk/meshing/vtkCMBTriangleMesher.h"

#include <algorithm>
#include <deque>
#include <set>
#include <stack>
#include <sys/stat.h>
#include <sys/types.h>
#include <vtksys/SystemTools.hxx>

//for data dumping in debug
//#define DUMP_DEBUG_DATA
#ifdef DUMP_DEBUG_DATA
#define DUMP_DEBUG_DATA_DIR "E:/Work/"
#include "vtkNew.h"
#include "vtkXMLPolyDataWriter.h"
#include <sstream>
#endif

vtkStandardNewMacro(vtkCMBSceneGenPolygon);

namespace
{

class InternalPolygonDetection
{
public:
  InternalPolygonDetection(vtkPolyData* arcs);
  ~InternalPolygonDetection();

  bool FindOuterLoop(vtkIdType& startingCellId, std::deque<vtkIdType>& outerLoop);
  bool FindInnerLoop(
    const std::deque<vtkIdType>& outerLoop, std::vector<std::deque<vtkIdType> >& innerLoops);

protected:
  //these are used when finding the outside loop
  bool FindLoop(
    vtkIdType& startingCellId, std::deque<vtkIdType>& loop, const bool& outerLoopSearch);

  int FindNextCellToTraverse(
    vtkIdType& currentCellId, vtkIdType& nextCellId, bool& addToBranchStack);
  void FindConnectedCells(vtkIdType& currentCellId, std::set<vtkIdType>& connectedCells);

  //the input arcs that make up the potential polygon
  vtkPolyData* Arcs;

  //int store holds if it is visited = 1, visited & deadend = 0
  std::map<vtkIdType, int> VisitedNodes;
};

InternalPolygonDetection::InternalPolygonDetection(vtkPolyData* arcs)
{
  this->Arcs = arcs;
  this->Arcs->BuildLinks();
}

InternalPolygonDetection::~InternalPolygonDetection()
{
  this->Arcs = NULL;
}

bool InternalPolygonDetection::FindOuterLoop(
  vtkIdType& startingCellId, std::deque<vtkIdType>& outerLoop)
{
  return this->FindLoop(startingCellId, outerLoop, true);
}

bool InternalPolygonDetection::FindLoop(
  vtkIdType& startingCellId, std::deque<vtkIdType>& loop, const bool& outerLoopSearch)
{
  std::stack<vtkIdType> branchCellIds;

  //get ready to do a DFS
  if (this->Arcs->GetCell(startingCellId)->GetNumberOfPoints() > 2)
  {
    branchCellIds.push(startingCellId);
  }
  loop.push_front(startingCellId);

  vtkIdType currentCellId = startingCellId;
  vtkIdType nextCellId, lastbranchId = -1;
  bool hasBranches;
  int foundNextCellStatus;
  do
  {
    foundNextCellStatus = this->FindNextCellToTraverse(currentCellId, nextCellId, hasBranches);
    if (foundNextCellStatus == 1)
    {
      loop.push_front(nextCellId);
      if (hasBranches && (branchCellIds.size() == 0 || branchCellIds.top() != currentCellId))
      {
        branchCellIds.push(currentCellId);
      }
      currentCellId = nextCellId;
    }
    else if (foundNextCellStatus == 0)
    {
      if (outerLoopSearch)
      {
        //we completed the loop. we need to figure out
        //if we have unneeded cells on the front, or if
        //we are on a dead end

        /*need to gather all the connections, once gathered we need
        to deque items, and remove them from the gather list
        til we have 2 items left
        (the cell we came from, and the cell that complets the loop).*/
        std::set<vtkIdType> connections;
        this->FindConnectedCells(currentCellId, connections);
        while (connections.size() > 2)
        {
          nextCellId = loop.back();
          connections.erase(nextCellId);
          loop.pop_back();
        }
      }
      loop.push_front(loop.back());
    }
    else if (foundNextCellStatus == -1)
    {
      //the second is that we hit a dead branch, so we pop a single element
      //from the queue and try that one again
      if (branchCellIds.size() == 0 || loop.size() <= 1)
      {
        //we have a line or polyline.
        return false;
      }
      if (lastbranchId == currentCellId && loop.size() > 0)
      {
        loop.pop_front();
        branchCellIds.pop();
      }

      //since this is a dead end, we need to mark it visited & a dead end
      this->VisitedNodes.insert(std::pair<vtkIdType, int>(currentCellId, 0));
      while (loop.front() != branchCellIds.top() && loop.size() > 0)
      {
        //make sure we clear all the cells since the last branch
        loop.pop_front();
      }

      //try the last branching cell
      lastbranchId = currentCellId;
      currentCellId = loop.front();
    }
  } while (loop.front() != loop.back());

  return loop.size() > 3;
}

bool InternalPolygonDetection::FindInnerLoop(
  const std::deque<vtkIdType>& outerLoop, std::vector<std::deque<vtkIdType> >& innerLoops)
{
  if (outerLoop.size() - 1 == static_cast<size_t>(this->Arcs->GetNumberOfLines()))
  {
    return false;
  }

  double bounds[6];
  this->Arcs->GetBounds(bounds);

  //construct the points of the outerloop
  int numPolygonPoints = (static_cast<int>(outerLoop.size()) - 1) *
    3; //the outerloop will have the same cellId at the starte and end
  double* polygonPoints = new double[numPolygonPoints];
  double* polygon = polygonPoints; //set the pointer to look at the start
  std::deque<vtkIdType>::const_iterator it = outerLoop.begin();
  std::set<vtkIdType> usedIds;
  vtkIdType cellId;
  int index = 0;
  while (index < numPolygonPoints)
  {
    cellId = *it++;
    usedIds.insert(cellId);
    vtkPoints* cellPoints = this->Arcs->GetCell(cellId)->GetPoints();
    if (index != 0)
    {
      double p0[3], p1[3], p2[3];
      p0[0] = polygonPoints[index - 3];
      p0[1] = polygonPoints[index - 2];
      p0[2] = polygonPoints[index - 1];
      cellPoints->GetPoint(0, p1);
      cellPoints->GetPoint(1, p2);
      if (p0[0] == p1[0] && p0[1] == p1[1] && p0[2] == p1[2])
      {
        polygonPoints[index] = p2[0];
        polygonPoints[index + 1] = p2[1];
        polygonPoints[index + 2] = p2[2];
      }
      else
      {
        polygonPoints[index] = p1[0];
        polygonPoints[index + 1] = p1[1];
        polygonPoints[index + 2] = p1[2];
      }
    }
    else
    {
      cellPoints->GetPoint(0, &polygonPoints[index]);
      index += 3;
      cellPoints->GetPoint(1, &polygonPoints[index]);
    }
    index += 3;
  }

  //since we add two points on the first iteration, we need to add the last id.
  cellId = *it++;
  usedIds.insert(cellId);

  //since we added points based on cell ids, we could
  //have a leading and/or trailing point that is not part of the loop
  if (polygon[0] == polygon[index - 3] && polygon[1] == polygon[index - 2] &&
    polygon[2] == polygon[index - 1])
  {
    //first id matches second last id
    --numPolygonPoints;
  }
  else if (polygon[0] == polygon[index - 6] && polygon[1] == polygon[index - 5] &&
    polygon[2] == polygon[index - 4])
  {
    //first id matches second last id
    numPolygonPoints -= 2;
  }
  else if (polygon[3] == polygon[index - 3] && polygon[4] == polygon[index - 2] &&
    polygon[5] == polygon[index - 1])
  {
    //second id matches last id
    polygon += 3;
    --numPolygonPoints;
  }
  else if (polygon[3] == polygon[index - 6] && polygon[4] == polygon[index - 5] &&
    polygon[5] == polygon[index - 4])
  {
    //second id matches second last id
    polygon += 3;
    numPolygonPoints -= 2;
  }

  double normal[3];
  vtkPolygon::ComputeNormal(numPolygonPoints, polygonPoints, normal);

  //select the first id that isn't part of the outerloop and walk it.
  //add all its
  vtkCellArray* arcs = this->Arcs->GetLines();
  vtkIdType npts, *pts;
  cellId = this->Arcs->GetNumberOfVerts(); //this is the id of the current cell;
  for (arcs->InitTraversal(); arcs->GetNextCell(npts, pts); ++cellId)
  {
    //check each of these cells for being part of the innerloop
    if (npts == 0)
    {
      continue;
    }
    //make sure this isn't part of another loop
    if (usedIds.count(cellId) != 0)
    {
      continue;
    }

    //presumption: if the first point in the first segment of the inner loop
    //is internal all of the loop is internal
    int result = vtkPolygon::PointInPolygon(
      this->Arcs->GetPoint(pts[0]), numPolygonPoints, polygon, bounds, normal);
    if (result == -1)
    {
      continue;
    }

    std::deque<vtkIdType> innerLoop;
    //good start spot, so start the walk

    bool isLoop = this->FindLoop(cellId, innerLoop, false); //returns true only on loops, not lines
    bool isLine = innerLoop.size() > 0 && !isLoop;
    if (isLoop || isLine)
    {
      //in the future we are going to need to handle loops and lines different I expect.
      innerLoops.push_back(innerLoop);
      //we now want to mark all the innerLoop cellIds as invalid start points
      //to find other inner loops
      for (std::deque<vtkIdType>::const_iterator itIL = innerLoop.begin(); itIL != innerLoop.end();
           itIL++)
      {
        usedIds.insert(*itIL);
      }
    }
  }

  delete[] polygonPoints;
  return innerLoops.size() > 0;
}

//return codes are:
// 0 - found no next cell, and none of the points were a dead end
// 1 - found the next cell
// -1 - found a single dead end, need to retrace from previous split node
int InternalPolygonDetection::FindNextCellToTraverse(
  vtkIdType& currentCellId, vtkIdType& nextCellId, bool& addToBranchStack)
{
  addToBranchStack = false;
  bool deadEnds = false;
  unsigned short ncells;
  vtkIdType npts, *pts, *cellIds;

  this->Arcs->GetCellPoints(currentCellId, npts, pts);
  for (vtkIdType i = 0; i < npts; ++i)
  {
    this->Arcs->GetPointCells(pts[i], ncells, cellIds);
    deadEnds = (!deadEnds) ? (ncells == 1) : deadEnds;
    for (vtkIdType j = 0; j < ncells; ++j)
    {
      nextCellId = cellIds[j];
      if (currentCellId != nextCellId &&
        this->VisitedNodes.find(nextCellId) == this->VisitedNodes.end())
      {
        this->VisitedNodes.insert(std::pair<vtkIdType, int>(currentCellId, 1));
        addToBranchStack = (ncells > 2);
        return 1;
      }
    }
  }

  if (deadEnds)
  {
    return -1;
  }
  return 0;
}

void InternalPolygonDetection::FindConnectedCells(
  vtkIdType& currentCellId, std::set<vtkIdType>& connectedCells)
{
  unsigned short ncells;
  vtkIdType ci, npts, *pts, *cellIds;
  std::map<vtkIdType, int>::iterator it;

  this->Arcs->GetCellPoints(currentCellId, npts, pts);
  for (vtkIdType i = 0; i < npts; ++i)
  {
    this->Arcs->GetPointCells(pts[i], ncells, cellIds);
    for (vtkIdType j = 0; j < ncells; ++j)
    {
      ci = cellIds[j];
      it = this->VisitedNodes.find(ci);
      if (it != this->VisitedNodes.end() && it->second != 0 && ci != currentCellId)
      {
        //only push back visited nodes that are not dead ends
        connectedCells.insert(ci);
      }
    }
  }
}
}

vtkCMBSceneGenPolygon::vtkCMBSceneGenPolygon()
{
  this->SetNumberOfInputPorts(1);
}

vtkCMBSceneGenPolygon::~vtkCMBSceneGenPolygon()
{
}

int vtkCMBSceneGenPolygon::FillInputPortInformation(int /*port*/, vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPolyData");
  info->Set(vtkAlgorithm::INPUT_IS_REPEATABLE(), 1);
  return 1;
}

int vtkCMBSceneGenPolygon::RequestData(vtkInformation* /*request*/,
  vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  //setup the input mesh for the masher
  vtkNew<vtkPolyData> preMesh;

  //we need to add 3 properties to the polydata to
  //correctly prep it for meshing
  bool readyForMeshing = this->PrepForMeshing(inputVector[0], preMesh.GetPointer());

#ifdef DUMP_DEBUG_DATA
  vtkNew<vtkXMLPolyDataWriter> writer;
  writer->SetInputData(preMesh);
  std::stringstream buffer;
  buffer << DUMP_DEBUG_DATA_DIR << "preMeshedPolygon.vtp";
  writer->SetFileName(buffer.str().c_str());
  writer->Write();
  writer->Update();
#endif

  //now lets mesh it
  if (readyForMeshing)
  {
    typedef vtkCMBTriangleMesher vtkTriangleMesher;
    vtkNew<vtkTriangleMesher> mesher;
    mesher->SetMaxAreaMode(vtkTriangleMesher::RelativeToBounds);
    mesher->SetMaxArea(.0005);
    mesher->SetPreserveBoundaries(true);
    mesher->SetInputData(preMesh.GetPointer());
    mesher->Update();

    // get the ouptut
    vtkPolyData* output = vtkPolyData::SafeDownCast(
      outputVector->GetInformationObject(0)->Get(vtkDataObject::DATA_OBJECT()));

    // The arcs may have been on a z plane other than z=0 - lets set the
    // value of all the points to be the same as the first point (which
    // should belong to one of the original arcs)
    vtkPoints* points = mesher->GetOutput(0)->GetPoints();
    int i, n = points->GetNumberOfPoints();
    double p[3], z;
    if (n > 0)
    {
      // Get the first point
      points->GetPoint(0, p);
      z = p[2];
      for (i = 1; i < n; i++)
      {
        points->GetPoint(i, p);
        p[2] = z;
        points->SetPoint(i, p);
      }
    }
    // now move the input through to the output
    output->SetPoints(points);
    output->SetPolys(mesher->GetOutput(0)->GetPolys());
  }
  return 1;
}

bool vtkCMBSceneGenPolygon::PrepForMeshing(vtkInformationVector* input, vtkPolyData* mesh)
{
  this->AppendArcSets(input, mesh);
  bool valid = this->DeterminePolygon(mesh);
  return valid;
}

void vtkCMBSceneGenPolygon::AppendArcSets(vtkInformationVector* input, vtkPolyData* mesh)
{
  int numInputs = input->GetNumberOfInformationObjects();
  vtkCellArray* newCells = vtkCellArray::New();
  vtkInformation* inInfo = 0;
  for (int idx = 0; idx < numInputs; ++idx)
  {
    inInfo = input->GetInformationObject(idx);
    vtkPolyData* pd = vtkPolyData::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
    if (idx == 0)
    {
      //since the input all share the same point set, we can grab the first input
      //object points
      mesh->SetPoints(pd->GetPoints());
    }
    if (pd->GetLines()->GetNumberOfCells() > 0)
    {
      vtkIdType npts = 0;
      vtkIdType* pts = 0;
      vtkCellArray* cells = pd->GetLines();
      for (cells->InitTraversal(); cells->GetNextCell(npts, pts);)
      {
        if (npts > 2)
        {
          for (vtkIdType i = 0; i < (npts - 1); i++)
          {
            newCells->InsertNextCell(2, pts + i);
          }
        }
        else
        {
          newCells->InsertNextCell(2, pts);
        }
      }
    }
  }

  mesh->SetLines(newCells);
  newCells->FastDelete();

  //make sure we have no verts, triangles, and strips
  //reduces possible memory foot print
  mesh->SetVerts(NULL);
  mesh->SetPolys(NULL);
  mesh->SetStrips(NULL);
}

bool vtkCMBSceneGenPolygon::DeterminePolygon(vtkPolyData* mesh)
{
  typedef vtkCMBPrepareForTriangleMesher vtkPrepareForMesher;
  vtkPrepareForMesher* mapInterface = vtkPrepareForMesher::New();
  mapInterface->SetPolyData(mesh);
  mapInterface->InitializeNewMapInfo();

  //now that we have the properties set with default values
  //lets find the external poly boundary
  vtkCellLocator* locator = vtkCellLocator::New();
  locator->SetDataSet(mesh);
  locator->BuildLocator();

  //find the starting cell of the outside loop
  vtkGenericCell* cell = vtkGenericCell::New();
  vtkIdType cellId = 0;
  double bounds[6];
  mesh->GetBounds(bounds);
  //returns the cell, and the cellId
  this->FindStartingCellForMesh(locator, cell, cellId, bounds);
  locator->Delete();
  cell->Delete();
  if (cellId == -1)
  {
    return false;
  }

  /*
  now we are going to have to walk the cells to find the outside loop
  we need to setup the InternalPoylgonDetection class
  */
  std::deque<vtkIdType> outerLoop;
  std::vector<std::deque<vtkIdType> > innerLoops;
  InternalPolygonDetection ipd(mesh);

  bool outerLoopFound = ipd.FindOuterLoop(cellId, outerLoop);
  if (outerLoopFound)
  {
    bool innerLoopFound = ipd.FindInnerLoop(outerLoop, innerLoops);

    vtkIdType minId = (*std::min_element(outerLoop.begin(), outerLoop.end()));
    vtkIdType maxId = (*std::max_element(outerLoop.begin(), outerLoop.end()));
    //The *3 assumes that all cells are VTK_LINES this may not be the case later on
    vtkIdType loopId = mapInterface->AddLoop(1, -1);
    mapInterface->AddArc(minId * 3, ((maxId - minId) + 1) * 3, 0, loopId, -1, 0, 0);

    if (innerLoopFound)
    {
      for (unsigned i = 0; i < innerLoops.size(); ++i)
      {
        std::deque<vtkIdType> innerLoop(innerLoops.at(i));
        minId = (*std::min_element(innerLoop.begin(), innerLoop.end()));
        maxId = (*std::max_element(innerLoop.begin(), innerLoop.end()));
        //The *3 assumes that all cells are VTK_LINES this may not be the case later on
        vtkIdType miLoopId = mapInterface->AddLoop(-1, 1);
        mapInterface->AddArc(
          minId * 3, ((maxId - minId) + 1) * 3, 1 + i, -1, miLoopId, 1 + i, 1 + i);
        //This loop can be removed when the new mesher fully takes over
      }
    }
  }

  mapInterface->FinalizeNewMapInfo();
  mapInterface->Delete();

  return outerLoopFound;
}

bool vtkCMBSceneGenPolygon::FindStartingCellForMesh(
  vtkCellLocator* locator, vtkGenericCell* cell, vtkIdType& cellId, double bounds[6])

{
  double point[3] = { bounds[0], bounds[2], bounds[4] };
  double closestPoint[3];
  int subId;
  double dist2;
  locator->FindClosestPoint(point, closestPoint, cell, cellId, subId, dist2);
  return (cell != NULL);
}

void vtkCMBSceneGenPolygon::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

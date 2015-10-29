//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkCMBMeshTerrainWithArcs.h"


#include "vtkAppendPolyData.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkCleanPolyData.h"
#include "vtkCellLocator.h"
#include "vtkDoubleArray.h"
#include "vtkFloatArray.h"
#include "vtkGenericCell.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkIncrementalOctreePointLocator.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkPolygon.h"

#include "smtk/bridge/discrete/extension/meshing/vtkCMBPrepareForTriangleMesher.h"
#include "smtk/bridge/discrete/extension/meshing/vtkCMBTriangleMesher.h"

#include "vtkMultiBlockDataSet.h"
#include "vtkXMLPolyDataWriter.h"

#include <iostream>

vtkStandardNewMacro(vtkCMBMeshTerrainWithArcs);

namespace{
  template<class T>
  void zeroZValues(T *t, int size)
    {
    for (int i=0; i < size; i+=3,t+=3)
      {
      t[2] = 0.0;
      }
    }

  struct PInfo
    {
    int NumberOfCells;
    int Id;
    double Height;
    PInfo():
      NumberOfCells(0),
      Id(0),
      Height(0.0)
      {}
    };
  };

class vtkCMBMeshTerrainWithArcs::vtkCmbInternalTerrainInfo
  {
public:
  vtkCmbInternalTerrainInfo(vtkInformationVector* input, const double &radius,
    const double &groundElevation);
  ~vtkCmbInternalTerrainInfo()
    {
    if (this->Locator)
      {
      this->Locator->Delete();
      }
    }
  //returns the average elevation for a circle given the radius
  template<class T>
  T getElevation(T *point);
  void setRadius(const double &r)
    {
    Radius = r;
    }
  void setGroundElevation(const double &ge)
    {
    GroundElevation = ge;
    }
protected:
  double Radius;
  double GroundElevation;
  std::map<vtkIdType,double> IdToElevation;
  vtkIncrementalOctreePointLocator *Locator;
  };

class vtkCMBMeshTerrainWithArcs::vtkCmbPolygonInfo
  {
public:
  std::vector<PInfo> Info;
  int GroundIndex;
  };

//-----------------------------------------------------------------------------
vtkCMBMeshTerrainWithArcs::vtkCmbInternalTerrainInfo::vtkCmbInternalTerrainInfo(
  vtkInformationVector* input, const double &radius,const double &groundElevation):
  Radius(radius), GroundElevation(groundElevation)
  {
  //1. Go through all the input data objects and collect the size of all the point dataset
  //2. Create a super set of points while removing all the z values from the points
  //and storing them in the map.
  //3. Create locator of the resulting 2D point set

  vtkIdType numPoints = 0;
  int numInputs = input->GetNumberOfInformationObjects();
  vtkPolyData *pd = NULL;
  for (int idx = 0; idx < numInputs; ++idx)
    {
    pd = vtkPolyData::GetData(input,idx);
    if (pd)
      {
      numPoints += pd->GetNumberOfPoints();
      }
    }

  //second iteration is building the point set and elevation mapping
  vtkPoints *inputPoints = NULL;
  vtkPoints *points = vtkPoints::New();
  points->SetNumberOfPoints(numPoints);

  vtkIdType index=0;
  double p[3];
  for (int idx = 0; idx < numInputs; ++idx)
    {
    pd = vtkPolyData::GetData(input,idx);
    if (pd)
      {
      inputPoints = pd->GetPoints();
      vtkIdType size = inputPoints->GetNumberOfPoints();
      for (vtkIdType i=0; i < size; ++i)
        {
        //get the point
        inputPoints->GetPoint(i,p);

        //store the z value & flatten
        double elev = p[2];
        p[2] = 0.0;
        points->InsertPoint(index,p);
        this->IdToElevation.insert(std::pair<vtkIdType,double>(index,elev));
        ++index;
        }
      }
    }

  vtkPolyData *pointSet = vtkPolyData::New();
  pointSet->SetPoints(points);
  points->FastDelete();

  this->Locator = vtkIncrementalOctreePointLocator::New();
  this->Locator->AutomaticOn();
  this->Locator->SetTolerance(0.0);
  this->Locator->SetDataSet(pointSet);
  this->Locator->BuildLocator();
  pointSet->Delete();
  }
//-----------------------------------------------------------------------------
template<class T>
T vtkCMBMeshTerrainWithArcs::vtkCmbInternalTerrainInfo::getElevation(
  T *point)
{
  double dpoint[3];
  dpoint[0] = static_cast<double>(point[0]);
  dpoint[1] = static_cast<double>(point[1]);
  dpoint[2] = static_cast<double>(point[2]);
  vtkIdList *ids = vtkIdList::New();
  this->Locator->FindPointsWithinRadius(this->Radius,dpoint,ids);
  double sum = 0;
  std::map<vtkIdType,double>::const_iterator it;
  vtkIdType size = ids->GetNumberOfIds();
  for ( vtkIdType i=0; i < size; ++i)
    {
    //average the elevation
    it = this->IdToElevation.find(ids->GetId(i));
    if ( it != this->IdToElevation.end() )
      {
      sum += it->second;
      }
    }
  ids->Delete();

  //handle the zero size use case
  T elev = static_cast<T>((size == 0) ? this->GroundElevation : sum/size);
  return elev;
}

//-----------------------------------------------------------------------------
vtkCMBMeshTerrainWithArcs::vtkCMBMeshTerrainWithArcs()
{
  this->PolygonInfo = new vtkCmbPolygonInfo();
  this->TerrainInfo = NULL;
  this->SetNumberOfInputPorts(2);
  this->ElevationRadius = 1.0;
  this->VOIBounds[0] = 0.0;
  this->VOIBounds[1] = 1.0;
  this->VOIBounds[2] = 0.0;
  this->VOIBounds[3] = 1.0;
  this->VOIBounds[4] = 0.0;
  this->VOIBounds[5] = 1.0;

  this->NumberOfProgressSteps = 10;
  this->StepIncrement = 0.1;

  this->Mesher = smtk::bridge::discrete::vtkCMBTriangleMesher::New();
  this->MesherMaxArea = 0.125;
}

//-----------------------------------------------------------------------------
vtkCMBMeshTerrainWithArcs::~vtkCMBMeshTerrainWithArcs()
{
  delete this->PolygonInfo;
  delete this->TerrainInfo;
  this->Mesher->Delete();
}

//----------------------------------------------------------------------------
int vtkCMBMeshTerrainWithArcs::FillInputPortInformation(int port,
                                                         vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
  info->Set(vtkAlgorithm::INPUT_IS_REPEATABLE(), 1);
  if ( port == 1 )
    {
    info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
    }
  return 1;
}

//----------------------------------------------------------------------------
void vtkCMBMeshTerrainWithArcs::RemoveInputConnections()
{
  this->SetInputConnection(0, NULL);
}

//----------------------------------------------------------------------------
void vtkCMBMeshTerrainWithArcs::RemoveSourceConnections()
{
  this->SetInputConnection(1, NULL);
}

//----------------------------------------------------------------------------
void vtkCMBMeshTerrainWithArcs::DetermineNumberOfProgressSteps(const int &numInputs)
{
  //the number of progress steps is:
  // prepForMeshing calls numInputs + 2 times
  // requestData 1
  // GenerateGroundMesh 1
  // GenerateExtrudedArcSets numInputs * 3

  this->NumberOfProgressSteps = 4 + numInputs * 4;
  this->CurrentProgressStep = 0;
  this->StepIncrement = 0.9 / this->NumberOfProgressSteps;
  this->SetProgressText("vtkCMBMeshTerrainWithArcs");

  //the end of request data is where we set progress to 1.0
}

//----------------------------------------------------------------------------
void vtkCMBMeshTerrainWithArcs::NextProgressStep()
{
  if ( this->CurrentProgressStep < this->NumberOfProgressSteps )
    {
    this->UpdateProgress(this->StepIncrement * this->CurrentProgressStep);
    ++this->CurrentProgressStep;
    }
  else
    {
    vtkWarningMacro("Updating progress in vtkCMBMeshTerrainWithArcs more times than expected");
    }
}

//----------------------------------------------------------------------------
int vtkCMBMeshTerrainWithArcs::RequestData(vtkInformation*,
                                 vtkInformationVector** inputVector,
                                 vtkInformationVector* outputVector)
{
  bool validMesh = true;

  this->DetermineNumberOfProgressSteps(inputVector[1]->GetNumberOfInformationObjects());

  //setup the input mesh for the mesher
  vtkNew<vtkPolyData> groundMesh;
  vtkPolyData *mesherResult = NULL;

  //we need to add 3 properties to the polydata to
  //correctly prep it for meshing
  bool readyForGroundMeshing = this->PrepForMeshing( inputVector[1], groundMesh.GetPointer());

  //now lets mesh it
  if ( readyForGroundMeshing )
    {
    //first call to triangle will mesh only the ground plane with
    //all the arc sets as holes
    this->Mesher->SetPreserveBoundaries(false);
    this->Mesher->SetPreserveEdgesAndNodes(true);
    this->Mesher->SetMaxAreaMode(
      smtk::bridge::discrete::vtkCMBTriangleMesher::RelativeToBoundsAndSegments);
    this->Mesher->SetMaxArea(0.015);
    this->Mesher->SetUseMinAngle(true);
    this->Mesher->SetInputData(groundMesh.GetPointer());
    this->Mesher->Update();
    this->NextProgressStep();

    //Do not get the Max area, as that is just a parameter to computing
    //the actual max area depending on the max area mode.
    this->MesherMaxArea = this->Mesher->GetComputedMaxArea();

    //update the ground mesh
    mesherResult = vtkPolyData::New();
    mesherResult->ShallowCopy(this->Mesher->GetOutput());
    }
  else
    {
    return false;
    }

  if ( validMesh && mesherResult )
    {
    //we now have to call triangle with each arc sets updated
    //boundary that the ground plane generated.

    //Construct the TerrainInfo first
    this->TerrainInfo = new vtkCmbInternalTerrainInfo(inputVector[0],
      this->ElevationRadius, this->VOIBounds[4]);

    vtkPolyData *finalMesh = vtkPolyData::New();
    finalMesh->DeepCopy(mesherResult);
    int numOfArrays = finalMesh->GetCellData()->GetNumberOfArrays();
    for ( int j=numOfArrays-1; j >= 0 ; --j)
      {
      finalMesh->GetCellData()->RemoveArray(j);
      }
    finalMesh->SetLines(NULL); //don't want lines in the output

    validMesh = this->GenerateGroundMesh(finalMesh);
    if ( validMesh && this->PolygonInfo->Info.size() > 1)
      {
      //At this point mesherResult and finalMesh are the same except
      //final mesh doesn't have lines, arrays, and has extruded points
      this->GenerateExtrudedArcSets(mesherResult,finalMesh);
      }
    mesherResult->Delete();

    delete this->TerrainInfo;
    this->TerrainInfo = NULL;

    vtkPolyData *output = vtkPolyData::GetData(outputVector);
    output->ShallowCopy( finalMesh );
    finalMesh->Delete();
    }

  //force the progress to finish no matter what.
  this->UpdateProgress(1.0);
  return validMesh;
}

//----------------------------------------------------------------------------
bool vtkCMBMeshTerrainWithArcs::PrepForMeshing(vtkInformationVector* input,
                                           vtkPolyData *mesh)
{
  //make sure we have no verts, triangles, and strips
  //reduces possible memory foot print
  mesh->SetVerts(NULL);
  mesh->SetPolys(NULL);
  mesh->SetStrips(NULL);

  //clear the information each time so we don't
  //have information of the previous modified run
  this->PolygonInfo->Info.clear();

  vtkCellArray *newCells = vtkCellArray::New();

  vtkCellArray *cells = NULL;
  int numInputs = input->GetNumberOfInformationObjects();
  for (int idx = 0; idx < numInputs; ++idx)
    {
    cells = NULL;
    vtkPolyData *pd = vtkPolyData::GetData(input,idx);
    cells = pd->GetLines();
    if ( !cells )
      {
      continue;
      }

    if (cells->GetNumberOfCells() > 0 )
      {
      PInfo info;
      int numCells = 0;
      vtkIdType npts,*pts;
      bool firstCell = true;
      for (cells->InitTraversal(); cells->GetNextCell(npts,pts);)
        {
        if (firstCell)
          {
          //in the first phase  we can presume the contour has a constant z value
          //across all points
          info.Height = pd->GetPoint(pts[0])[2];
          firstCell = false;
          }
        for (vtkIdType i=0; i<(npts-1); ++i, ++numCells)
          {
          newCells->InsertNextCell(2,pts+i);
          }
        }

      //all Ids have to be 1 based
      info.Id = static_cast<int>(this->PolygonInfo->Info.size()) + 1;
      info.NumberOfCells = numCells;
      this->PolygonInfo->Info.push_back(info);
      this->NextProgressStep();
      }
    }

  //combine all the arcs together into a single polydata
  //so that we can get the combined point set
  vtkNew<vtkPoints> points;
  const int numInputsToAppend = input->GetNumberOfInformationObjects();
  if(numInputsToAppend > 0)
    {
    vtkNew<vtkAppendPolyData> append;
    for(int i=0; i < numInputsToAppend; ++i)
      {
      append->AddInputData( vtkPolyData::GetData(input,i) );
      }
    append->Update();
    //inc ref count so it doesn't go to null when append scope deletes
    points->ShallowCopy(append->GetOutput()->GetPoints());
    }

  //Insert the ground plane
  this->InsertGroundPlane(points.GetPointer(),newCells);

  vtkIdType size = newCells->GetNumberOfCells();
  mesh->SetLines(newCells);
  newCells->FastDelete();

  //bitwise and each method call to get if the mesh is valid
  bool validMesh = true;
  //Setup the ids in the cell data
  validMesh &= this->AssignPolygonIds(mesh, size);
  this->NextProgressStep();

  //when the meshing happens it will require all the points have 0 zvalues
  validMesh &= this->FlattenMesh(points.GetPointer());
  this->NextProgressStep();

  mesh->SetPoints(points.GetPointer());

  return validMesh;
}

//----------------------------------------------------------------------------
bool vtkCMBMeshTerrainWithArcs::AssignPolygonIds(vtkPolyData *mesh,
  const vtkIdType &/*size*/) const
{
  typedef smtk::bridge::discrete::vtkCMBPrepareForTriangleMesher vtkPrepareForMesher;
  vtkNew<vtkPrepareForMesher> mapInterface;
  mapInterface->SetPolyData(mesh); //Add necessary field data to the mesh
  mapInterface->SetNumberOfArcs(this->PolygonInfo->Info.size());
  mapInterface->SetNumberOfLoops(this->PolygonInfo->Info.size());
  mapInterface->InitializeNewMapInfo();

  size_t i=0;
  vtkIdType index=0;
  int numCells=0;
  //set up the cell arrays
  vtkIdType groundId = this->PolygonInfo->Info[
    this->PolygonInfo->GroundIndex].Id;
  int asId = 0;
  int endpointId = 0;
  for (i=0; i < this->PolygonInfo->Info.size(); ++i)
    {
    numCells = this->PolygonInfo->Info[i].NumberOfCells;
    asId = this->PolygonInfo->Info[i].Id;
    if ( asId == groundId )
      {
      //In this program a loop is just one arc
      vtkIdType loopId = mapInterface->AddLoop(groundId,-1);
      //endpoints don't mean anything, just make sure they are unique
      //arcs will never be part of more than one loop in this code
      mapInterface->AddArc(index*3,numCells*3,groundId,loopId,-1,endpointId,endpointId);
      endpointId++;

      index += numCells;
      }
    else
      {
      //This loop will be a hole of the ground
      vtkIdType loopId = mapInterface->AddLoop(-1,groundId);
      mapInterface->AddArc(index*3,numCells*3,asId,loopId,-1,endpointId,endpointId);
      endpointId++;

      index += numCells;
      }
    }
  mapInterface->FinalizeNewMapInfo(); //add the field data

  return true;
}
//----------------------------------------------------------------------------
bool vtkCMBMeshTerrainWithArcs::FlattenMesh(vtkPoints *points) const
{
  bool valid = true;
  vtkDataArray *dataArray = points->GetData();
  int size = points->GetNumberOfPoints() * 3;
  if (dataArray->GetDataType() == VTK_FLOAT)
    {
    vtkFloatArray *floatArray = static_cast<vtkFloatArray *>(dataArray);
    float *pt = floatArray->GetPointer(0);
    zeroZValues(pt,size);
    }
  else if (dataArray->GetDataType() == VTK_DOUBLE)
    {
    vtkDoubleArray *doubleArray = static_cast<vtkDoubleArray *>(dataArray);
    double *pt = doubleArray->GetPointer(0);
    zeroZValues(pt,size);
    }
  else
    {
    valid = false;
    }
  return valid;
}
//----------------------------------------------------------------------------
void vtkCMBMeshTerrainWithArcs::InsertGroundPlane(vtkPoints *points,vtkCellArray *newCells) const
{
  vtkIdType bl,br,tr,tl;
  bl = points->InsertNextPoint(this->VOIBounds[0],this->VOIBounds[2],0); //bottom left
  br = points->InsertNextPoint(this->VOIBounds[1],this->VOIBounds[2],0); //bottom right
  tr = points->InsertNextPoint(this->VOIBounds[1],this->VOIBounds[3],0); //top right
  tl = points->InsertNextPoint(this->VOIBounds[0],this->VOIBounds[3],0); //top left
  newCells->InsertNextCell(2);
  newCells->InsertCellPoint(br);
  newCells->InsertCellPoint(bl);
  newCells->InsertNextCell(2);
  newCells->InsertCellPoint(bl);
  newCells->InsertCellPoint(tl);
  newCells->InsertNextCell(2);
  newCells->InsertCellPoint(tl);
  newCells->InsertCellPoint(tr);
  newCells->InsertNextCell(2);
  newCells->InsertCellPoint(tr);
  newCells->InsertCellPoint(br);

  PInfo info;
  //all Ids have to be 1 based
  info.Id = static_cast<int>(this->PolygonInfo->Info.size()) + 1;
  info.NumberOfCells = 4;
  this->PolygonInfo->Info.push_back(info);
  this->PolygonInfo->GroundIndex = static_cast<int>(this->PolygonInfo->Info.size() - 1);
}

//----------------------------------------------------------------------------
bool vtkCMBMeshTerrainWithArcs::GenerateGroundMesh(vtkPolyData *finalMesh)
{
  bool valid = this->ExtrudeMeshPoints(finalMesh->GetPoints());
  this->NextProgressStep();
  return valid;
}

//----------------------------------------------------------------------------
bool vtkCMBMeshTerrainWithArcs::GenerateExtrudedArcSets(vtkPolyData *input,
  vtkPolyData* output)
{
  bool valid;
  for ( size_t i=0; i < this->PolygonInfo->Info.size(); i++ )
    {
    //skip the ground plane
    if ( static_cast<size_t>(this->PolygonInfo->GroundIndex) == i )
      {
      continue;
      }
    vtkNew<vtkPolyData> arcsetMesh;
    int id = this->PolygonInfo->Info[i].Id;
    this->CreateArcSetForMeshing(input,arcsetMesh.GetPointer(),id);
    this->NextProgressStep();

    this->Mesher->SetPreserveBoundaries(true);
    this->Mesher->SetMaxAreaMode(
        smtk::bridge::discrete::vtkCMBTriangleMesher::AbsoluteArea);
    this->Mesher->SetMaxArea(this->MesherMaxArea);
    this->Mesher->SetInputData(arcsetMesh.GetPointer());
    this->Mesher->Update();
    this->NextProgressStep();

    vtkNew<vtkPolyData> resultMesh;
    resultMesh->ShallowCopy(this->Mesher->GetOutput());

    //remove all the cell properties since they are uneeded
    int numOfArrays = resultMesh->GetCellData()->GetNumberOfArrays();
    for ( int j=numOfArrays-1; j >= 0 ; --j)
      {
      resultMesh->GetCellData()->RemoveArray(j);
      }

    valid = this->ExtrudeArcMesh(resultMesh.GetPointer(), static_cast<int>(i));
    this->NextProgressStep();
    if ( valid )
      {
      //now add this mesh on the final mesh
      vtkNew<vtkAppendPolyData> append;
      append->AddInputData(output);
      append->AddInputData(resultMesh.GetPointer());
      append->Update();
      vtkPolyData *result = append->GetOutput();
      output->ShallowCopy(result);
      }
    }

  return true;
}


//----------------------------------------------------------------------------
void vtkCMBMeshTerrainWithArcs::CreateArcSetForMeshing(vtkPolyData* input,
  vtkPolyData* output, const int &index) const
{
  vtkCellData* cellData = input->GetCellData();
  vtkPoints* points = input->GetPoints();
  if ( cellData == NULL || points == NULL)
    {
    return;
    }

  vtkPoints *newPoints = vtkPoints::New();
  newPoints->DeepCopy(points);
  output->SetPoints(newPoints);
  newPoints->Delete();


  vtkCellArray *newCells = vtkCellArray::New();
  newCells->Allocate( input->GetNumberOfLines() );
  //ElementIds will be the same as the arcSetIds
  vtkIdTypeArray *elementIds = vtkIdTypeArray::SafeDownCast(cellData->GetArray("ElementIds"));
  vtkIdType numCells = input->GetNumberOfCells();

  for (vtkIdType i = 0; i < numCells; ++i )
    {
    if ( (input->GetCellType(i) == VTK_LINE ||
          input->GetCellType(i) == VTK_POLY_LINE) &&
          elementIds->GetValue(i) == index )
      {
      newCells->InsertNextCell(input->GetCell(i));
      }
    }

  output->SetLines(newCells);
  vtkIdType size = output->GetNumberOfCells();
  newCells->FastDelete();

  typedef smtk::bridge::discrete::vtkCMBPrepareForTriangleMesher vtkPrepareForMesher;
  vtkNew<vtkPrepareForMesher> mapInterface;
  mapInterface->SetPolyData(output); //Add necessary field data to the mesh
  mapInterface->SetNumberOfArcs(1);
  mapInterface->SetNumberOfLoops(1);
  mapInterface->InitializeNewMapInfo();

  //There is only 1 arc and only 1 loop
  vtkIdType loopId = mapInterface->AddLoop(index,-1);
  mapInterface->AddArc(0,size*3,1,loopId,-1,1,1);

  mapInterface->FinalizeNewMapInfo();
}

//----------------------------------------------------------------------------
bool vtkCMBMeshTerrainWithArcs::ExtrudeArcMesh(vtkPolyData *mesh,
  const int &index) const
{
  //note this method modifies the mesh
  if ( mesh == NULL )
    {
    return false;
    }
  vtkPoints* points = mesh->GetPoints();
  if (points == NULL )
    {
    return false;
    }

  //restore this arc set to the correct height
  double height = this->PolygonInfo->Info[index].Height;
  this->UnFlattenMesh(points,height);

  //build up the cell links, so we can insert new cells
  mesh->BuildCells();

  //now convert the lines into polygons that form the sides of the
  //extrusion
  std::map<vtkIdType,vtkIdType> SidesBottomToTop;
  vtkIdType meshId, newGroundId, npts;
  vtkIdList *pIds = vtkIdList::New();
  double p[3];

  vtkCellArray *lines = mesh->GetLines();
  lines->InitTraversal();
  while( lines->GetNextCell(pIds) )
    {
    npts = pIds->GetNumberOfIds();
    for ( vtkIdType i=npts-1; i >= 0; --i)
      {
      meshId = pIds->GetId(i);
      if ( SidesBottomToTop.find(meshId) == SidesBottomToTop.end() )
        {
        points->GetPoint(meshId,p);
        p[2] = 0; //set it to zero so get the right info
        p[2] = this->TerrainInfo->getElevation(p);
        newGroundId = points->InsertNextPoint(p);
        }
      else
        {
        newGroundId = SidesBottomToTop.find(meshId)->second;
        }
      SidesBottomToTop.insert(std::pair<vtkIdType,vtkIdType>(meshId,newGroundId));
      pIds->InsertNextId(newGroundId);
      }
    mesh->InsertNextCell(VTK_POLYGON,pIds);
    pIds->Reset();
    }

  pIds->Delete();

  //now the we are done iterating the lines, delete them as they are not needed
  mesh->SetLines(NULL);

  return true;
}

//----------------------------------------------------------------------------
bool vtkCMBMeshTerrainWithArcs::UnFlattenMesh(vtkPoints *points,
  const double &height) const
{
  //get the point data void pointer
  vtkDataArray *dataArray = points->GetData();
  vtkIdType size = points->GetNumberOfPoints();
  if (dataArray->GetDataType() == VTK_FLOAT)
    {
    vtkFloatArray *floatArray = static_cast<vtkFloatArray *>(dataArray);
    float *pos = floatArray->GetPointer(0);
    for ( vtkIdType i = 0; i < size; ++i,pos+=3)
      {
      pos[2] = height;
      }
    }
  else
    {
    vtkDoubleArray *doubleArray = static_cast<vtkDoubleArray *>(dataArray);
    double *pos = doubleArray->GetPointer(0);
    for ( vtkIdType i = 0; i < size; ++i,pos+=3)
      {
      pos[2] = height;
      }
    }
  return true;
}

//----------------------------------------------------------------------------
bool vtkCMBMeshTerrainWithArcs::ExtrudeMeshPoints(vtkPoints *points) const
{
  //get the point data void pointer
  vtkDataArray *dataArray = points->GetData();
  vtkIdType size = points->GetNumberOfPoints();
  if (dataArray->GetDataType() == VTK_FLOAT)
    {
    vtkFloatArray *floatArray = static_cast<vtkFloatArray *>(dataArray);
    float *pos = floatArray->GetPointer(0);
    for ( vtkIdType i = 0; i < size; ++i,pos+=3)
      {
      pos[2] = this->TerrainInfo->getElevation(pos);
      }
    }
  else
    {
    vtkDoubleArray *doubleArray = static_cast<vtkDoubleArray *>(dataArray);
    double *pos = doubleArray->GetPointer(0);
    for ( vtkIdType i = 0; i < size; ++i,pos+=3)
      {
      pos[2] = this->TerrainInfo->getElevation(pos);
      }
    }
  return true;
}

//----------------------------------------------------------------------------
void vtkCMBMeshTerrainWithArcs::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "NumberOfProgressSteps: " << this->NumberOfProgressSteps << std::endl;
  os << indent << "CurrentProgressStep: " << this->CurrentProgressStep << std::endl;
  os << indent << "StepIncrement: " << this->StepIncrement << std::endl;
  os << indent << "VOIBounds: " << this->VOIBounds[0] << ", "
    << this->VOIBounds[1] << ", "
    << this->VOIBounds[2] << ", "
    << this->VOIBounds[3] << ", "
    << this->VOIBounds[4] << ", "
    << this->VOIBounds[5] << ", " << std::endl;
  os << indent << "ElevationRadius: " << this->ElevationRadius << std::endl;
  os << indent << "MesherMaxArea: " << this->MesherMaxArea << std::endl;
  os << indent << "Mesher: " << this->Mesher << std::endl;
}

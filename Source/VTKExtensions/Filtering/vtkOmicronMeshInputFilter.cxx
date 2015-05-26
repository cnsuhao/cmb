//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkOmicronMeshInputFilter.h"

#include "vtkObjectFactory.h"
#include "vtkInformation.h"
#include "vtkPointData.h"
#include "vtkCellData.h"
#include "vtkIdList.h"
#include "vtkIntArray.h"
#include "vtkStringArray.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkNew.h"
#include "vtkInformationVector.h"
#include "vtkPolyData.h"
#include "vtkCellLocator.h"
#include "vtkDoubleArray.h"
#include "vtkGenericCell.h"
#include "vtkCompositeDataIterator.h"
#include "vtkMultiBlockWrapper.h"


vtkStandardNewMacro(vtkOmicronMeshInputFilter);

//----------------------------------------------------------------------------
vtkOmicronMeshInputFilter::vtkOmicronMeshInputFilter()
{
  this->SetNumberOfInputPorts(1);
}

//----------------------------------------------------------------------------
vtkOmicronMeshInputFilter::~vtkOmicronMeshInputFilter()
{
}


//----------------------------------------------------------------------------
void vtkOmicronMeshInputFilter::SetInputData(vtkMultiBlockDataSet* dataSet)
{
  this->Superclass::SetInputData(dataSet);
}

//----------------------------------------------------------------------------
int vtkOmicronMeshInputFilter::RequestData(
  vtkInformation* vtkNotUsed(request),
  vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and ouptut
  vtkMultiBlockDataSet *input = vtkMultiBlockDataSet::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkMultiBlockDataSet *output = vtkMultiBlockDataSet::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  if (!input)
    {
    vtkErrorMacro("Input must be specified (and must be a vtkMultiBlockDataSet)\n");
    return 0;
    }


  vtkNew<vtkMultiBlockWrapper> wrapper;
  wrapper->SetMultiBlock(input);

  vtkNew<vtkCellLocator> locator;

  vtkPolyData *masterGeometry =  wrapper->GetMasterPolyData();
  locator->SetDataSet(masterGeometry);
  locator->BuildLocator();

  // 1st, identify soil as material at one of the corners; find corner cell
  // as cell closest to point (a little) outside the corner
  double closestPoint[3];
  vtkIdType cellId;
  int subId;
  double dist2;
  vtkNew<vtkGenericCell> cell;

  double outsideCorner[3];
  double *bounds = masterGeometry->GetBounds();
  outsideCorner[0] = bounds[0] - (bounds[1] - bounds[0]) * 0.01;
  outsideCorner[1] = bounds[2] - (bounds[3] - bounds[2]) * 0.01;
  outsideCorner[2] = bounds[4] - (bounds[5] - bounds[4]) * 0.01;
  locator->FindClosestPoint(outsideCorner, closestPoint, cell.GetPointer(), cellId,
                            subId, dist2);
  // Let find the cell's model face and its material id
  int modelFace = wrapper->GetModelFaceId(cellId);
  int soilMaterialID = wrapper->GetModelFaceMaterialId( modelFace );

  // so, we should probably be able to just use point just inside the same
  // corner; but just for grins, lets instead pick a point in the
  // the center of the volume, and do an intersection test (towards
  // bottom of bounds) per chance our soil isn't a nice box shape

  // pick point near the bottom (z) and centered in X and Y for candidate
  // soilPt, and "exteriorPt" (for line intersect test) as point with same X, Y
  // but Z just below bounds of are data.
  double soilPt[3], exteriorPt[3];
  soilPt[2] = bounds[4] + (bounds[5] - bounds[4]) * 0.01;
  exteriorPt[0] = soilPt[0] = (bounds[0] + bounds[1]) / 2.0;
  exteriorPt[1] = soilPt[1] = (bounds[2] + bounds[3]) / 2.0;
  exteriorPt[2] = bounds[4] - (bounds[5] - bounds[4]) * 0.1;

  // do the intersection test to make sure we're ok (that we're inside soil)
  double t, x[3], pCoords[3];
  if ( !locator->IntersectWithLine(soilPt, exteriorPt, 0.0001, t, x,
                                   pCoords, subId, cellId, cell.GetPointer()) )
    {
    vtkErrorMacro("Yikes... certainly not expected!  Intersection test of interior soil point failed.");

    // try something else... but what?
    return 0;
    }

  modelFace = wrapper->GetModelFaceId(cellId);
  if (soilMaterialID !=  wrapper->GetModelFaceMaterialId( modelFace ))
    {
    vtkErrorMacro("Problem finding point inside soil.");

    // should try something else, but for now, just fail
    return 0;
    }


  int numShells = wrapper->GetNumberOfShells();
  for (int i = 0; i < numShells; i++)
    {
    vtkNew<vtkPolyData> block;
    output->SetBlock(output->GetNumberOfBlocks(), block.GetPointer());

    // "FileName" ID
    vtkNew<vtkStringArray> fileNameFD;
    fileNameFD->SetName("FileName");
    std::string tempString;

    // the output material ID
    vtkNew<vtkIntArray> outputMaterialID;
    outputMaterialID->SetName( "MaterialID" );
    outputMaterialID->SetNumberOfComponents(1);

    // PointInside ID
    vtkNew<vtkDoubleArray> pointInsideFD;
    pointInsideFD->SetName( "PointInside" );
    pointInsideFD->SetNumberOfComponents(3);

    double *pointInside = wrapper->GetShellTranslationPoint(i);
    if (pointInside) // must not be soil
      {
      tempString = "\"";
      tempString += wrapper->GetShellUserName(i);
      tempString += "\"";
      pointInsideFD->InsertNextTuple( pointInside );

      // figure out the material ID
      locator->FindClosestPoint(pointInside, closestPoint,
                                cell.GetPointer(), cellId, subId, dist2);
      // Let find the cell's model face and its material id
      modelFace = wrapper->GetModelFaceId(cellId);
      outputMaterialID->
        InsertNextValue(wrapper->GetModelFaceMaterialId( modelFace ) );
      }
    else // soil
      {
      tempString = "\"soil\"";
      outputMaterialID->InsertNextValue( soilMaterialID );
      pointInsideFD->InsertNextTuple( soilPt );
      }
    fileNameFD->InsertNextValue( tempString.c_str() );
    block->GetFieldData()->AddArray( fileNameFD.GetPointer() );
    block->GetFieldData()->AddArray( outputMaterialID.GetPointer() );
    block->GetFieldData()->AddArray( pointInsideFD.GetPointer() );
    }

  return 1;
}


//----------------------------------------------------------------------------
void vtkOmicronMeshInputFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

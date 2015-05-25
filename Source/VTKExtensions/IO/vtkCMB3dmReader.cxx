//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMB3dmReader.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkDataSetRegionSurfaceFilter.h"
#include "vtkCMBMeshReader.h"
#include "vtkExtractRegionEdges.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include "vtkMultiBlockWrapper.h"



vtkStandardNewMacro(vtkCMB3dmReader);

vtkCMB3dmReader::vtkCMB3dmReader()
{
  this->FileName = 0;
  this->SetNumberOfInputPorts(0);
  this->Prep2DMeshForModelCreation = false;
  this->Is2DMesh = false;
  this->RegionIdentifiersModified = false;
}

vtkCMB3dmReader:: ~vtkCMB3dmReader()
{
  this->SetFileName(0);
}

int vtkCMB3dmReader::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{
  vtkDebugMacro("Reading a 3dm file.");
  // get the info object
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the ouptut
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  ifstream fin(this->FileName);
  if(!fin)
    {
    vtkErrorMacro(<< "File " << this->FileName << " not found");
    return 0;
    }

  vtkSmartPointer<vtkCMBMeshReader> reader =
    vtkSmartPointer<vtkCMBMeshReader>::New();
  reader->SetFileName(this->GetFileName());
  reader->SetCreateMeshElementIdArray(true);
  reader->SetRenameMaterialAsRegion(true);
  reader->SetCreateMeshMaterialIdArray(true);;
  reader->Update();
  this->RegionIdentifiersModified = false;

  // A *.3dm file can contain a 2D or 3D mesh (Shallow water ADH uses the
  // *.3dm extension for its 2D meshes) but should not 1D mesh
  if (reader->GetMeshDimension() == vtkCMBMeshReader::MESH1D)
    {
    vtkErrorMacro(<< "File contained an inappropiate mesh.");
    return 0;
    }
  this->Is2DMesh = (reader->GetMeshDimension() == vtkCMBMeshReader::MESH2D);

  if (this->Is2DMesh && this->Prep2DMeshForModelCreation)
    {
    vtkSmartPointer<vtkExtractRegionEdges> edgesFilter =
      vtkSmartPointer<vtkExtractRegionEdges>::New();
    edgesFilter->SetRegionArrayName(vtkMultiBlockWrapper::GetShellTagName());
    edgesFilter->SetInputConnection( reader->GetOutputPort() );
    edgesFilter->Update();
    this->RegionIdentifiersModified = edgesFilter->GetRegionIdentifiersModified();
    output->ShallowCopy(edgesFilter->GetOutput(0));
    }
  else
    {
    vtkSmartPointer<vtkDataSetRegionSurfaceFilter> surfaceFilter =
      vtkSmartPointer<vtkDataSetRegionSurfaceFilter>::New();
    surfaceFilter->SetInputConnection(reader->GetOutputPort());
    surfaceFilter->SetRegionArrayName(vtkMultiBlockWrapper::GetShellTagName());
    surfaceFilter->PassThroughPointIdsOn();
    surfaceFilter->Update();
    output->ShallowCopy(surfaceFilter->GetOutput(0));
    }
  return 1;
}

//----------------------------------------------------------------------------
int vtkCMB3dmReader::RequestInformation(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *vtkNotUsed(outputVector))
{
  if (!this->FileName)
    {
    vtkErrorMacro("FileName has to be specified!");
    return 0;
    }

  return 1;
}

void vtkCMB3dmReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "FileName: " << this->FileName << endl;
}

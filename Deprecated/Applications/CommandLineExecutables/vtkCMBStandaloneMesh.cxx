/*=========================================================================

Copyright (c) 2013 Kitware Inc. 28 Corporate Drive,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced, distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
=========================================================================*/
// .NAME vtkCMBStandaloneMesh.cxx -
// .SECTION Description
// .SECTION See Also

#include "vtkCMBStandaloneMesh.h"

#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkCMBModelEdgeMeshServer.h"
#include "vtkCMBStandaloneModelFaceMesh.h"
#include "vtkDataArray.h"
#include "vtkCMBMeshReader.h"
#include "vtkIdList.h"
#include "vtkIdTypeArray.h"
#include "vtkModel.h"
#include "vtkModelEdge.h"
#include "vtkModelFace.h"
#include "vtkModelItemIterator.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include "vtkUnstructuredGrid.h"

vtkStandardNewMacro(vtkCMBStandaloneMesh);

class vtkCMBStandaloneMeshInternals
{
public:
  std::map<vtkModelEdge*, vtkSmartPointer<vtkCMBModelEdgeMeshServer> > ModelEdges;
  std::map<vtkModelFace*, vtkSmartPointer<vtkCMBStandaloneModelFaceMesh> > ModelFaces;
};


//----------------------------------------------------------------------------
vtkCMBStandaloneMesh::vtkCMBStandaloneMesh()
  : vtkCMBMeshServer(), Grid(0)
{
  this->StandaloneInternal = new vtkCMBStandaloneMeshInternals;
}

//----------------------------------------------------------------------------
vtkCMBStandaloneMesh::~vtkCMBStandaloneMesh()
{
  this->Grid->Delete();
}

//----------------------------------------------------------------------------
bool vtkCMBStandaloneMesh::Load(const char *filename)
{
  bool ok = false;  // return value

  vtkCMBMeshReader *mesh_reader = vtkCMBMeshReader::New();
  mesh_reader->SetFileName(filename);
  mesh_reader->Update();
  this->Grid = mesh_reader->GetOutput();

  ok = this->Grid != 0x0;
  if (this->Grid)
    {
    ok = true;
    //this->Grid->Print(std::cout);
    }
  else
    {
    std::cout << "ERROR READING MESH " << filename << std::endl;
    }

  return ok;
}

//----------------------------------------------------------------------------
void vtkCMBStandaloneMesh::Initialize(vtkModel* model)
{
  if(model == NULL)
    {
    vtkErrorMacro("Passed in NULL model.");
    return;
    }
  if(model->GetModelDimension() != 2)
    {  // do nothing if it's not a 2d model
    return;
    }
  if(this->Model != model)
    {
    this->Reset();
    this->Model = model;
    }

  // edges
  vtkModelItemIterator* iter = model->NewIterator(vtkModelEdgeType);
  for(iter->Begin();!iter->IsAtEnd();iter->Next())
    {
    vtkModelEdge* edge =
      vtkModelEdge::SafeDownCast(iter->GetCurrentItem());
    vtkSmartPointer<vtkCMBModelEdgeMeshServer> meshRepresentation =
      vtkSmartPointer<vtkCMBModelEdgeMeshServer>::New();
    meshRepresentation->Initialize(this, edge);
    this->StandaloneInternal->ModelEdges[edge] = meshRepresentation;
    }
  iter->Delete();

  // faces
  iter = model->NewIterator(vtkModelFaceType);
  for(iter->Begin();!iter->IsAtEnd();iter->Next())
    {
    vtkModelFace* face =
      vtkModelFace::SafeDownCast(iter->GetCurrentItem());
    vtkSmartPointer<vtkCMBStandaloneModelFaceMesh> meshRepresentation =
      vtkSmartPointer<vtkCMBStandaloneModelFaceMesh>::New();
    meshRepresentation->Initialize(this, face);
    this->StandaloneInternal->ModelFaces[face] = meshRepresentation;
    }
  iter->Delete();
  this->Modified();
}

//----------------------------------------------------------------------------
vtkCMBModelEntityMesh* vtkCMBStandaloneMesh::GetModelEntityMesh(
  vtkModelGeometricEntity* entity)
{
  if(vtkModelEdge* modelEdge = vtkModelEdge::SafeDownCast(entity))
    {
    std::map<vtkModelEdge*,vtkSmartPointer<vtkCMBModelEdgeMeshServer> >::iterator it=
      this->StandaloneInternal->ModelEdges.find(modelEdge);
    if(it == this->StandaloneInternal->ModelEdges.end())
      {
      return NULL;
      }
    return it->second;
    }
  if(vtkModelFace* modelFace = vtkModelFace::SafeDownCast(entity))
    {
    std::map<vtkModelFace*,
      vtkSmartPointer<vtkCMBStandaloneModelFaceMesh> >::iterator it=
      this->StandaloneInternal->ModelFaces.find(modelFace);
    if(it == this->StandaloneInternal->ModelFaces.end())
      {
      return NULL;
      }
    return it->second;
    }
  vtkErrorMacro("Incorrect type.");
  return NULL;
}

//----------------------------------------------------------------------------
vtkPolyData* vtkCMBStandaloneMesh::GetPolyData(vtkModelFace *face)
{
  vtkPolyData *polydata = vtkPolyData::New();
  polydata->Allocate();

  // Copy all points
  polydata->SetPoints(this->Grid->GetPoints());

  // Copy all cells with same dimension and regionId
  int regionId = face->GetUniquePersistentId();
  vtkCellData *cellData = this->Grid->GetCellData();
  vtkDataArray *regionData = cellData->GetScalars("Region");
  vtkIntArray *intData = vtkIntArray::SafeDownCast(regionData);

  vtkIdList *idList = vtkIdList::New();
  vtkIdTypeArray *modelIds = vtkIdTypeArray::New();
  modelIds->SetName("ModelId");
  unsigned n = static_cast<unsigned>(regionData->GetNumberOfTuples());
  for (unsigned i=0; i<n; ++i)
    {
    int val = intData->GetValue(i);
    //std::cout << i << ": " << val << '\n';
    if (regionId == val)
      {
      idList->Reset();
      this->Grid->GetCellPoints(i, idList);
      if (idList->GetNumberOfIds() != 3)
        {
        std::cout << i << " WARNING idList size " << idList->GetNumberOfIds()
                  << " - should be 3" << std::endl;
        continue;
        }
      polydata->InsertNextCell(this->Grid->GetCellType(i), idList);
      modelIds->InsertNextValue(regionId);
      }
    }
  polydata->GetCellData()->AddArray(modelIds);

  return polydata;
}

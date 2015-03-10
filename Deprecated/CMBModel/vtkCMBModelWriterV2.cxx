/*=========================================================================

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
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

#include "vtkCMBModelWriterV2.h"

#include "Bridge.h"
#include "vtkCellData.h"
#include "vtkCharArray.h"
#include "vtkModel3dmGridRepresentation.h"
#include "vtkModelMaterial.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelEdge.h"
#include "vtkDiscreteModelEntityGroup.h"
#include "vtkDiscreteModelFace.h"
#include "vtkDiscreteModelRegion.h"
#include "vtkCMBParserBase.h"
#include "vtkModelUserName.h"
#include "vtkCMBModelWriterBase.h"
#include "vtkDoubleArray.h"
#include "vtkFieldData.h"
#include "vtkIdList.h"
#include "vtkIdTypeArray.h"
#include "vtkIntArray.h"
#include "vtkModelItemIterator.h"
#include "vtkModelVertex.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkStringArray.h"
#include "vtkUnsignedIntArray.h"
#include "vtkXMLPolyDataWriter.h"

vtkStandardNewMacro(vtkCMBModelWriterV2);

vtkCMBModelWriterV2::vtkCMBModelWriterV2()
{
  this->FileName = 0;
  this->DataMode = vtkXMLWriter::Binary;
}

vtkCMBModelWriterV2:: ~vtkCMBModelWriterV2()
{
}

bool vtkCMBModelWriterV2::Write(vtkDiscreteModel* Model)
{
  if(!Model)
    {
    vtkErrorMacro("Passed in a null model.");
    return 0;
    }


  const DiscreteMesh& mesh = Model->GetMesh();
  vtkSmartPointer<vtkPolyData> Poly = Model->GetMesh().GetAsSinglePolyData();
  this->AddFileVersion(Poly.GetPointer());

  vtkNew<vtkIdTypeArray> Classification;
  Classification->SetNumberOfComponents(1);
  Classification->SetNumberOfTuples(mesh.GetNumberOfCells());
  Classification->SetName(vtkCMBParserBase::GetModelFaceTagName());

  //iterate all edges than all faces, which is how the polydata is composed
  //this
  typedef DiscreteMesh::cell_const_iterator CellIterator;
  vtkDiscreteModel::ClassificationType& classified = Model->GetMeshClassification();
  vtkIdType index = 0;
  for(CellIterator i= mesh.CellsBegin();
      i != mesh.CellsEnd();
      ++i,++index)
    {
    vtkDiscreteModelGeometricEntity* entity = classified.GetEntity(*i);
    if(!entity)
      std::cout << *i << std::endl;
    vtkIdType id = entity->GetThisModelEntity()->GetUniquePersistentId();
    Classification->SetTupleValue(index, &id);
    }
  Poly->GetCellData()->AddArray(Classification.GetPointer());


  this->SetModelVertexData(Model, Poly.GetPointer());
  this->SetModelEdgeData(Model, Poly.GetPointer());
  this->SetModelFaceData(Model, Poly.GetPointer());
  this->SetModelRegionData(Model, Poly.GetPointer());
  this->SetMaterialData(Model, Poly.GetPointer());
  this->SetAnalysisGridData(Model, Poly.GetPointer());

  if(Model->GetNumberOfModelEntities(vtkDiscreteModelEntityGroupType))
    {
    this->SetModelEntityGroupData(Model, Poly.GetPointer());
    }
  if(Model->GetNumberOfModelEntities(vtkModelEdgeType))
    {
    this->SetFloatingEdgeData(Model, Poly.GetPointer());
    }
  this->SetUUIDData(Model, Poly.GetPointer());

  vtkNew<vtkXMLPolyDataWriter> Writer;
  Writer->SetInputData(0, Poly.GetPointer());
  Writer->SetFileName(this->GetFileName());
  Writer->SetDataMode( this->DataMode );
  Writer->Write();
  vtkDebugMacro("Finished writing a CMB file.");
  return 1;
}

void vtkCMBModelWriterV2::SetAnalysisGridData(vtkDiscreteModel* model, vtkPolyData* poly)
{
  if(vtkModel3dmGridRepresentation* analysisGridInfo =
     vtkModel3dmGridRepresentation::SafeDownCast(model->GetAnalysisGridInfo()))
    {
    vtkIdTypeArray* pointMap = analysisGridInfo->GetModelPointToAnalysisPoint();
    vtkIdTypeArray* cellMap = analysisGridInfo->GetModelCellToAnalysisCells();
    vtkCharArray* cellSides = analysisGridInfo->GetModelCellToAnalysisCellSides();
    if(pointMap != NULL && cellMap != NULL && cellSides != NULL)
      {
      pointMap->SetName(vtkDiscreteModel::GetPointMapArrayName());
      poly->GetPointData()->AddArray(pointMap);
      cellMap->SetName(vtkDiscreteModel::GetCellMapArrayName());
      poly->GetCellData()->AddArray(cellMap);
      cellSides->SetName(vtkDiscreteModel::GetCanonicalSideArrayName());
      poly->GetCellData()->AddArray(cellSides);
      }
    }
}

void vtkCMBModelWriterV2::SetModelFaceData(vtkDiscreteModel* Model, vtkPolyData* Poly)
{
  vtkNew<vtkIdTypeArray> ModelFaceAdjacentRegionsId;
  ModelFaceAdjacentRegionsId->SetNumberOfComponents(2);

  std::vector<vtkModelEntity*> Entities;
  vtkModelItemIterator* Faces = Model->NewIterator(vtkModelFaceType);
  for(Faces->Begin();!Faces->IsAtEnd();Faces->Next())
    {
    vtkModelFace* Face = vtkDiscreteModelFace::SafeDownCast(Faces->GetCurrentItem());
    Entities.push_back(Face);
    vtkIdType ids[2] = {-1, -1};
    for(int j=0;j<2;j++)
      {
      vtkModelRegion* Region = Face->GetModelRegion(j);
      if(Region)
        {
        ids[j] = Region->GetUniquePersistentId();
        }
      }
    ModelFaceAdjacentRegionsId->InsertNextTupleValue(ids);
    }
  Faces->Delete();
  ModelFaceAdjacentRegionsId->SetName(vtkCMBParserBase::GetModelFaceRegionsString());
  Poly->GetFieldData()->AddArray(ModelFaceAdjacentRegionsId.GetPointer());
  this->SetModelEntityData(Poly, Entities, "ModelFace");
}

void vtkCMBModelWriterV2::SetModelRegionData(vtkDiscreteModel* Model, vtkPolyData* Poly)
{
  vtkNew<vtkIdTypeArray> RegionMaterialId;
  RegionMaterialId->SetNumberOfComponents(1);
  vtkNew<vtkDoubleArray> PointInside;
  PointInside->SetNumberOfComponents(3);
  vtkNew<vtkIntArray> pointInsideValidity;
  pointInsideValidity->SetNumberOfComponents(1);

  std::vector<vtkModelEntity*> Entities;
  vtkModelItemIterator* Regions = Model->NewIterator(vtkModelRegionType);
  for(Regions->Begin();!Regions->IsAtEnd();Regions->Next())
    {
    vtkDiscreteModelRegion* Region = vtkDiscreteModelRegion::SafeDownCast(Regions->GetCurrentItem());
    Entities.push_back(Region);
    vtkIdType id = Region->GetMaterial()->GetUniquePersistentId();
    RegionMaterialId->InsertNextTupleValue(&id);
    if (Region->GetPointInside())
      {
      PointInside->InsertNextTupleValue(Region->GetPointInside());
      pointInsideValidity->InsertNextValue(1);
      }
    else
      {
      double dummyPt[3] = {0, 0, 0};
      PointInside->InsertNextTupleValue(dummyPt);
      pointInsideValidity->InsertNextValue(0);
      }
    }
  Regions->Delete();
  RegionMaterialId->SetName("ModelRegionMaterials");
  Poly->GetFieldData()->AddArray(RegionMaterialId.GetPointer());
  PointInside->SetName("ModelRegionPointInside");
  Poly->GetFieldData()->AddArray(PointInside.GetPointer());
  pointInsideValidity->SetName("ModelRegionPointInsideValidity");
  Poly->GetFieldData()->AddArray(pointInsideValidity.GetPointer());
  this->SetModelEntityData(Poly, Entities, "ModelRegion");
}

void vtkCMBModelWriterV2::SetMaterialData(vtkDiscreteModel* Model, vtkPolyData* Poly)
{
  // will eventually have to do warehouse id
  std::vector<vtkModelEntity*> Entities;
  vtkModelItemIterator* Materials = Model->NewIterator(vtkModelMaterialType);
  for(Materials->Begin();!Materials->IsAtEnd();Materials->Next())
    {
    vtkModelEntity* Material = vtkModelEntity::SafeDownCast(Materials->GetCurrentItem());
    Entities.push_back(Material);
    }
  Materials->Delete();
  this->SetModelEntityData(Poly, Entities, "Material");
}

// The format for storing the model entity Ids for each model entity group
// is not that straightforward since every model entity group could potentially
// be grouping a different amount of model entities.  The way that I chose to do it
// is to have a single index flat array that stores all of the needed information.
// The format of the data in the array is:
// number of model entities in first model entity group
// the Ids of the model entities in the first group
// number of model entities in second model entity group
// the Ids of the model entities in the second group
// ...
// number of model entities in last model entity group
// the Ids of the model entities in the last group
void vtkCMBModelWriterV2::SetModelEntityGroupData(vtkDiscreteModel* Model, vtkPolyData* Poly)
{
  std::vector<vtkModelEntity*> Entities;
  vtkNew<vtkIdTypeArray> GroupedEntityIds;
  GroupedEntityIds->SetNumberOfComponents(1);
  vtkModelItemIterator* EntityGroups = Model->NewIterator(vtkDiscreteModelEntityGroupType);
  for(EntityGroups->Begin();!EntityGroups->IsAtEnd();EntityGroups->Next())
    {
    vtkDiscreteModelEntityGroup* EntityGroup =
      vtkDiscreteModelEntityGroup::SafeDownCast(EntityGroups->GetCurrentItem());
    Entities.push_back(EntityGroup);
    vtkIdType NumberOfModelEntities = EntityGroup->GetNumberOfModelEntities();
    GroupedEntityIds->InsertNextTupleValue(&NumberOfModelEntities);
    if(NumberOfModelEntities>0)
      {
      vtkModelItemIterator* EntitiesIter = EntityGroup->NewModelEntityIterator();
      for(EntitiesIter->Begin();!EntitiesIter->IsAtEnd();EntitiesIter->Next())
        {
        vtkIdType EntityId = vtkModelEntity::SafeDownCast(
          EntitiesIter->GetCurrentItem())->GetUniquePersistentId();
        GroupedEntityIds->InsertNextTupleValue(&EntityId);
        }
      EntitiesIter->Delete();
      }
    }
  EntityGroups->Delete();

  GroupedEntityIds->SetName("ModelEntityGroupEntityIds");
  Poly->GetFieldData()->AddArray(GroupedEntityIds.GetPointer());

  this->SetModelEntityData(Poly, Entities, "ModelEntityGroup");
}


// The format for storing the points for each floating line in a region
// double[3] - for point1
// double[3] - for point2
// int       - for line spacing
void vtkCMBModelWriterV2::SetFloatingEdgeData(vtkDiscreteModel* Model, vtkPolyData* Poly)
{
  vtkNew<vtkIdTypeArray> EdgeRegionlId;
  EdgeRegionlId->SetNumberOfComponents(1);
  vtkNew<vtkDoubleArray> endPoints;
  endPoints->SetNumberOfComponents(3);
  vtkNew<vtkIntArray> lineSpacing;
  lineSpacing->SetNumberOfComponents(1);

  std::vector<vtkModelEntity*> Entities;
  vtkModelItemIterator* Edges = Model->NewIterator(vtkModelEdgeType);
  for(Edges->Begin();!Edges->IsAtEnd();Edges->Next())
    {
    vtkDiscreteModelEdge* edge = vtkDiscreteModelEdge::SafeDownCast(Edges->GetCurrentItem());
    if(edge->GetNumberOfAssociations(vtkModelRegionType) > 0)
      {
      Entities.push_back(edge);
      vtkIdType id = edge->GetModelRegion()->GetUniquePersistentId();
      EdgeRegionlId->InsertNextTupleValue(&id);
      double xyz[3];
      if(edge->GetAdjacentModelVertex(0)->GetPoint(xyz))
        {
        endPoints->InsertNextTupleValue(xyz);
        }
      else
        {
        vtkWarningMacro("Model vertex does not have a valid point.");
        }
      if(edge->GetAdjacentModelVertex(1)->GetPoint(xyz))
        {
        endPoints->InsertNextTupleValue(xyz);
        }
      else
        {
        vtkWarningMacro("Model vertex does not have a valid point.");
        }
      lineSpacing->InsertNextValue(edge->GetLineResolution());
      }
    }
  Edges->Delete();
  EdgeRegionlId->SetName(vtkCMBParserBase::GetFloatingEdgesString());
  Poly->GetFieldData()->AddArray(EdgeRegionlId.GetPointer());
  endPoints->SetName(vtkCMBParserBase::GetModelEdgeVerticesString());
  Poly->GetFieldData()->AddArray(endPoints.GetPointer());
  lineSpacing->SetName(vtkCMBParserBase::GetEdgeLineResolutionString());
  Poly->GetFieldData()->AddArray(lineSpacing.GetPointer());
  this->SetModelEntityData(Poly, Entities, "ModelEdge");
}

void vtkCMBModelWriterV2::SetUUIDData(vtkDiscreteModel* model, vtkPolyData* poly)
{
  vtkModelItemIterator* itit;
  std::vector<vtkModelItem*> entities;
  // The model itself
  entities.push_back(model);
  itit = model->NewIterator(vtkModelType);
  for (itit->Begin(); !itit->IsAtEnd(); itit->Next())
    entities.push_back(itit->GetCurrentItem());
  itit->Delete();

  this->SetModelItemUUIDs(model, poly, entities, vtkCMBParserBase::GetModelUUIDsString());
  // Material
  entities.clear();
  itit = model->NewIterator(vtkModelMaterialType);
  for (itit->Begin(); !itit->IsAtEnd(); itit->Next())
    entities.push_back(itit->GetCurrentItem());
  itit->Delete();
  this->SetModelItemUUIDs(model, poly, entities, vtkCMBParserBase::GetMaterialUUIDsString());
  // Groups
  entities.clear();
  itit = model->NewIterator(vtkDiscreteModelEntityGroupType);
  for (itit->Begin(); !itit->IsAtEnd(); itit->Next())
    entities.push_back(itit->GetCurrentItem());
  itit->Delete();
  this->SetModelItemUUIDs(model, poly, entities, vtkCMBParserBase::GetModelGroupUUIDsString());
  // Regions
  entities.clear();
  itit = model->NewIterator(vtkModelRegionType);
  for (itit->Begin(); !itit->IsAtEnd(); itit->Next())
    entities.push_back(itit->GetCurrentItem());
  itit->Delete();
  this->SetModelItemUUIDs(model, poly, entities, vtkCMBParserBase::GetModelRegionUUIDsString());
  // Faces
  entities.clear();
  itit = model->NewIterator(vtkModelFaceType);
  for (itit->Begin(); !itit->IsAtEnd(); itit->Next())
    entities.push_back(itit->GetCurrentItem());
  itit->Delete();
  this->SetModelItemUUIDs(model, poly, entities, vtkCMBParserBase::GetModelFaceUUIDsString());
  // Edges
  entities.clear();
  itit = model->NewIterator(vtkModelEdgeType);
  for (itit->Begin(); !itit->IsAtEnd(); itit->Next())
    entities.push_back(itit->GetCurrentItem());
  itit->Delete();
  this->SetModelItemUUIDs(model, poly, entities, vtkCMBParserBase::GetModelEdgeUUIDsString());
  // Vertices
  entities.clear();
  itit = model->NewIterator(vtkModelVertexType);
  for (itit->Begin(); !itit->IsAtEnd(); itit->Next())
    entities.push_back(itit->GetCurrentItem());
  itit->Delete();
  this->SetModelItemUUIDs(model, poly, entities, vtkCMBParserBase::GetModelVertexUUIDsString());

  // Shell
  entities.clear();
  itit = model->NewIterator(vtkModelShellUseType);
  for (itit->Begin(); !itit->IsAtEnd(); itit->Next())
    entities.push_back(itit->GetCurrentItem());
  itit->Delete();
  this->SetModelItemUUIDs(model, poly, entities, vtkCMBParserBase::GetModelShellUUIDsString());
  // Loop
  entities.clear();
  itit = model->NewIterator(vtkModelLoopUseType);
  for (itit->Begin(); !itit->IsAtEnd(); itit->Next())
    entities.push_back(itit->GetCurrentItem());
  itit->Delete();
  this->SetModelItemUUIDs(model, poly, entities, vtkCMBParserBase::GetModelLoopUUIDsString());

  // Face uses
  entities.clear();
  itit = model->NewIterator(vtkModelFaceUseType);
  for (itit->Begin(); !itit->IsAtEnd(); itit->Next())
    entities.push_back(itit->GetCurrentItem());
  itit->Delete();
  this->SetModelItemUUIDs(model, poly, entities, vtkCMBParserBase::GetModelFaceUseUUIDsString());
  // Edge uses
  entities.clear();
  itit = model->NewIterator(vtkModelEdgeUseType);
  for (itit->Begin(); !itit->IsAtEnd(); itit->Next())
    entities.push_back(itit->GetCurrentItem());
  itit->Delete();
  this->SetModelItemUUIDs(model, poly, entities, vtkCMBParserBase::GetModelEdgeUseUUIDsString());
  // Vertex uses
  entities.clear();
  itit = model->NewIterator(vtkModelVertexUseType);
  for (itit->Begin(); !itit->IsAtEnd(); itit->Next())
    entities.push_back(itit->GetCurrentItem());
  itit->Delete();
  this->SetModelItemUUIDs(model, poly, entities, vtkCMBParserBase::GetModelVertexUseUUIDsString());
}

void vtkCMBModelWriterV2::SetModelEntityData(
  vtkPolyData* Poly, std::vector<vtkModelEntity*> & ModelEntities,
  const char* BaseArrayName)
{
  vtkNew<vtkIdTypeArray> EntityIds;
  EntityIds->SetNumberOfComponents(1);
  EntityIds->SetNumberOfTuples(ModelEntities.size());
  vtkNew<vtkDoubleArray> EntityRGBA;
  EntityRGBA->SetNumberOfComponents(4);
  EntityRGBA->SetNumberOfTuples(ModelEntities.size());
  vtkNew<vtkIntArray> EntityVisibility;
  EntityVisibility->SetNumberOfComponents(1);
  EntityVisibility->SetNumberOfTuples(ModelEntities.size());
  vtkNew<vtkStringArray> EntityUserName;
  EntityUserName->SetNumberOfComponents(1);
  EntityUserName->SetNumberOfTuples(ModelEntities.size());

  vtkIdType NumEnts = ModelEntities.size();
  for(vtkIdType i=0;i<NumEnts;i++)
    {
    vtkModelEntity* Entity = ModelEntities[i];
    // unique persistent id
    EntityIds->SetValue(i, Entity->GetUniquePersistentId());
    // color
    EntityRGBA->SetTupleValue(i, Entity->GetColor());
    // visibility
    EntityVisibility->SetValue(i, Entity->GetVisibility());
    // username
    if(const char* name = vtkModelUserName::GetUserName(Entity))
      {
      EntityUserName->SetValue(i, name);
      }
    else
      {
      EntityUserName->SetValue(i, "");
      }
    }
  std::string Base(BaseArrayName);
  std::string Name = Base+"Ids";
  EntityIds->SetName(Name.c_str());
  Poly->GetFieldData()->AddArray(EntityIds.GetPointer());
  Name = Base + "Color";
  EntityRGBA->SetName(Name.c_str());
  Poly->GetFieldData()->AddArray(EntityRGBA.GetPointer());
  Name = Base + "Visibility";
  EntityVisibility->SetName(Name.c_str());
  Poly->GetFieldData()->AddArray(EntityVisibility.GetPointer());
  Name = Base + "UserName";
  EntityUserName->SetName(Name.c_str());
  Poly->GetFieldData()->AddArray(EntityUserName.GetPointer());
}

void vtkCMBModelWriterV2::SetModelItemUUIDs(
  vtkDiscreteModel* model,
  vtkPolyData* poly,
  std::vector<vtkModelItem*> & items,
  const char* arrayName)
{
  vtkUnsignedIntArray* arr = cmbsmtk::cmb::Bridge::retrieveUUIDs(model, items);
  arr->SetName(arrayName);
  poly->GetFieldData()->AddArray(arr);
  arr->Delete();
}

void vtkCMBModelWriterV2::AddFileVersion(vtkPolyData* Poly)
{
  vtkNew<vtkIntArray> Version;
  Version->SetNumberOfComponents(1);
  Version->SetNumberOfTuples(1);
  Version->SetValue(0, this->GetVersion());
  Version->SetName(vtkCMBParserBase::GetFileVersionString());
  Poly->GetFieldData()->AddArray(Version.GetPointer());
}

void vtkCMBModelWriterV2::SetDataModeToAscii()
{
  this->SetDataMode(vtkXMLWriter::Ascii);
}
void vtkCMBModelWriterV2::SetDataModeToBinary()
{
  this->SetDataMode(vtkXMLWriter::Binary);
}
void vtkCMBModelWriterV2::SetDataModeToAppended()
{
  this->SetDataMode(vtkXMLWriter::Appended);
}

void vtkCMBModelWriterV2::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  if(this->DataMode == vtkXMLWriter::Ascii)
    {
    os << indent << "DataMode: Ascii\n";
    }
  else if(this->DataMode == vtkXMLWriter::Binary)
    {
    os << indent << "DataMode: Binary\n";
    }
  else
    {
    os << indent << "DataMode: Appended\n";
    }
}

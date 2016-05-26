//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkMultiBlockWrapper.h"

#include "vtkCell.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkDoubleArray.h"
#include "vtkFieldData.h"
#include "vtkFloatArray.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInformationIntegerKey.h"
#include "vtkInstantiator.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include "vtkStringArray.h"
#include "vtkXMLPolyDataReader.h"
#include <map>
#include <set>
#include <vector>

vtkStandardNewMacro(vtkMultiBlockWrapper);
vtkInformationKeyMacro(vtkMultiBlockWrapper, BlockVisibilityMetaKey, Integer);

using std::pair;

namespace
{
  // local namespace for lazy use of char* arrays
  char ModelFaceIdsString[] = "modelfaceids";
  char ModelFaceCellIdsString[] = "modelfacecellids";
  char CMBFileVersionString[] = "version";
  char ShellUserNamesString[] = "shell names";
  char ShellColorsString[] = "shell colors";
  char ShellMaterialIdString[] = "shell material ids";
  char ShellTranslationPointString[] = "shell translation points";
  char MaterialUserNamesString[] = "material names";
  char MaterialColorsString[] = "material colors";
  char MaterialUserIdsString[] = "material user ids";
  // for now, on the model faces the data is: shell, material, model face system id
  //                   + BCS ids for model faces (block index > 0)
  char ModelFaceDataString[] = "modelfacedata";
  // convenient model face data array stored at block 0, includes four components
  // shell, materials, model face id, NOT USED (previously it is block index, should be removed)
  char ModelFaceConvenientArrayName[] = "modelfaceconvenientarray";
  char ModelFaceColorsString[] = "model face colors";
  char ModelFaceUserNamesString[] = "model face names";
  char BCSUserNamesString[] = "BCS names";
  char BCSColorsString[] = "BCS colors";
  char BCSUserIdsString[] = "BCS ids";
  char BCSBaseNameString[] = "BCSBase";
  char RCNameString[] = "reverse classification";
  char SplitFacesString[] = "splitfaces";
  char MaterialTagString[] = "cell materials";
  char ShellTagString[] = "Region";

  char NodeGroupColorsString[] = "nodal group colors";
  char NodeGroupUserNamesString[] = "nodal group names";

  enum ObjectArrayLocation
  {
    shellIdArrayLoc = 0,
    materialIdArrayLoc,
    modelFaceIdArrayLoc
    //,
    //blockNumberArrayLoc
  };

  enum MBRootBlockIndex
  {
    MasterPolyRootIndex = 0,
    ModelFaceRootIndex,
    NodalGroupRootIndex
    //,
    //blockNumberArrayLoc
  };

}

//-----------------------------------------------------------------------------
vtkMultiBlockWrapper::vtkMultiBlockWrapper()
{
  this->mb = 0;
  this->needToRemoveDeletedCells = false;
}

//-----------------------------------------------------------------------------
vtkMultiBlockWrapper::~vtkMultiBlockWrapper()
{
  if(this->mb != 0)
    {
    this->SetMultiBlock(0);
    }
  this->mb = 0;
}

//-----------------------------------------------------------------------------
const char* vtkMultiBlockWrapper::GetModelFaceTagName()
{
  return ModelFaceIdsString;
}

//-----------------------------------------------------------------------------
const char* vtkMultiBlockWrapper::GetShellTagName()
{
  return ShellTagString;
}

//-----------------------------------------------------------------------------
const char* vtkMultiBlockWrapper::GetMaterialTagName()
{
  return MaterialTagString;
}

//-----------------------------------------------------------------------------
const char* vtkMultiBlockWrapper::GetReverseClassificationTagName()
{
  return RCNameString;
}

//-----------------------------------------------------------------------------
const char* vtkMultiBlockWrapper::GetSplitFacesTagName()
{
  return SplitFacesString;
}

//-----------------------------------------------------------------------------
const char* vtkMultiBlockWrapper::GetModelFaceDataName()
{
  return ModelFaceDataString;
}

//-----------------------------------------------------------------------------
const char* vtkMultiBlockWrapper::GetModelFaceConvenientArrayName()
{
  return ModelFaceConvenientArrayName;
}

//-----------------------------------------------------------------------------
const char* vtkMultiBlockWrapper::GetMaterialUserNamesString()
{
  return MaterialUserNamesString;
}

//-----------------------------------------------------------------------------
const char* vtkMultiBlockWrapper::GetShellUserNamesString()
{
  return ShellUserNamesString;
}

//-----------------------------------------------------------------------------
const char* vtkMultiBlockWrapper::GetShellTranslationPointString()
{
  return ShellTranslationPointString;
}

//-----------------------------------------------------------------------------
const char* vtkMultiBlockWrapper::GetModelFaceUserNamesString()
{
  return ModelFaceUserNamesString;
}

//-----------------------------------------------------------------------------
const char* vtkMultiBlockWrapper::GetBCSUserNamesString()
{
  return BCSUserNamesString;
}

//-----------------------------------------------------------------------------
const char* vtkMultiBlockWrapper::GetShellColorsString()
{
  return ShellColorsString;
}

//-----------------------------------------------------------------------------
const char* vtkMultiBlockWrapper::GetModelFaceColorsString()
{
  return ModelFaceColorsString;
}

//-----------------------------------------------------------------------------
const char* vtkMultiBlockWrapper::GetMaterialColorsString()
{
  return MaterialColorsString;
}

//-----------------------------------------------------------------------------
const char* vtkMultiBlockWrapper::GetBCSColorsString()
{
  return BCSColorsString;
}

//-----------------------------------------------------------------------------
const char* vtkMultiBlockWrapper::GetModelFaceUse1String()
{
  return "ModelFaceUse1";
}

//-----------------------------------------------------------------------------
int vtkMultiBlockWrapper::GetModelFaceId(vtkPolyData* poly)
{
  vtkFieldData* field = poly->GetFieldData();
  vtkIntArray* array = vtkIntArray::SafeDownCast(
    field->GetArray(ModelFaceDataString));
  if(array)
    {
    return array->GetValue(modelFaceIdArrayLoc);
    }
  return -1;
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::SetModelFaceUse1(int modelFaceId, int shellId)
{
  vtkPolyData* modelFace = this->GetModelFaceWithId(modelFaceId);
  if(!modelFace)
    {
    vtkWarningMacro("No existing model face.");
    return;
    }
  vtkPolyData* master = this->GetMasterPolyData();
  vtkIntArray* data = vtkIntArray::SafeDownCast(
    master->GetFieldData()->GetArray(
      vtkMultiBlockWrapper::GetModelFaceUse1String()));
  if(!data)
    {
    data = vtkIntArray::New();
    data->SetNumberOfComponents(1);
    data->SetNumberOfTuples(modelFaceId+1);
    data->SetName(vtkMultiBlockWrapper::GetModelFaceUse1String());
    for(vtkIdType i=0;i<modelFaceId;i++)
      {
      data->InsertValue(i, -1); // mark as not set
      }
    data->InsertValue(modelFaceId, shellId);
    master->GetFieldData()->AddArray(data);
    data->Delete();
    }
  else
    {
    data = vtkIntArray::SafeDownCast(
      this->PerformNeededDeepCopy(data, master->GetFieldData()));
    vtkIdType dataLength = data->GetNumberOfTuples();
    for(vtkIdType i=dataLength;i<modelFaceId;i++)
      {
      data->InsertValue(i, -1);
      }
    data->InsertValue(modelFaceId, shellId);
    }
}

//-----------------------------------------------------------------------------
int vtkMultiBlockWrapper::GetModelFaceUse1(int modelFaceId)
{
  vtkPolyData* master = this->GetMasterPolyData();
  if(master)
    {
    vtkIntArray* data = vtkIntArray::SafeDownCast(
      master->GetFieldData()->GetArray(
        vtkMultiBlockWrapper::GetModelFaceUse1String()));
    if(!data || data->GetNumberOfTuples() <= modelFaceId)
      {
      return -1;
      }
    return data->GetValue(modelFaceId);
    }
  return -1;
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::SetMultiBlock(vtkMultiBlockDataSet* mb_in)
{
  if ( mb_in != this->mb)
    {
    if (this->mb)
      {
      this->mb->UnRegister(this);
      }
    this->mb = mb_in;
    if (this->mb)
      {
      this->mb->Register(this);
      }
    this->Modified();
    }
}

//-----------------------------------------------------------------------------
vtkMultiBlockDataSet* vtkMultiBlockWrapper::GetMultiBlock()
{
  return this->mb;
}

//-----------------------------------------------------------------------------
vtkPolyData* vtkMultiBlockWrapper::GetMasterPolyData()
{
  if(!this->mb)
    {
    vtkWarningMacro("No vtkMultiBlockDataSet set.");
    return 0;
    }
  return vtkPolyData::SafeDownCast(this->mb->GetBlock(MasterPolyRootIndex));
}

//-----------------------------------------------------------------------------
int vtkMultiBlockWrapper::GetNumberOfModelFaces()
{
  if(this->GetModelFaceRootBlock())
    {
    return this->GetModelFaceRootBlock()->GetNumberOfBlocks();;
    }
  vtkWarningMacro("No vtkMultiBlockDataSet or ModelFaceRootBlock set.");
  return 0;
}

//-----------------------------------------------------------------------------
vtkMultiBlockDataSet* vtkMultiBlockWrapper::GetModelFaceRootBlock()
{
  vtkMultiBlockDataSet* mfData = NULL;
  if(this->mb && this->mb->GetBlock(ModelFaceRootIndex))
    {
    mfData = vtkMultiBlockDataSet::SafeDownCast(
      this->mb->GetBlock(ModelFaceRootIndex));
    }
  return mfData;
}

//-----------------------------------------------------------------------------
int vtkMultiBlockWrapper::GetNumberOfNodalGroups()
{
  if(this->mb && this->GetNodalGroupRootBlock())
    {
    return this->GetNodalGroupRootBlock()->GetNumberOfBlocks();;
    }
  vtkWarningMacro("No vtkMultiBlockDataSet or NodalGroupRootBlock set.");
  return 0;
}

//-----------------------------------------------------------------------------
vtkMultiBlockDataSet* vtkMultiBlockWrapper::GetNodalGroupRootBlock()
{
  vtkMultiBlockDataSet* ngData = NULL;
  if(this->mb && this->mb->GetBlock(NodalGroupRootIndex))
    {
    ngData = vtkMultiBlockDataSet::SafeDownCast(
      this->mb->GetBlock(NodalGroupRootIndex));
    }
  return ngData;
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::GetModelFaceIds(vtkIdList* ids)
{
  ids->Reset();
  if(this->mb)
    {
    // the zeroth block is the entire data
    for(int i=0; i<this->GetNumberOfModelFaces(); i++)
      {
      vtkIntArray* modelFaceInfo =
        vtkIntArray::SafeDownCast(
          this->GetModelFaceWithIndex(i)->GetFieldData()->GetArray(ModelFaceDataString));
      int id = modelFaceInfo->GetValue(modelFaceIdArrayLoc);
      ids->InsertNextId(id);
      }
    }
  else
    {
    vtkWarningMacro("No vtkMultiBlockDataSet set.");
    }
  return;
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::GetBCSIds(vtkIdList* ids)
{
  ids->Reset();
  if(this->mb)
    {
    vtkPolyData* poly = this->GetMasterPolyData();
    vtkFloatArray* rgba = vtkFloatArray::SafeDownCast(
      poly->GetFieldData()->GetArray(BCSColorsString));
    if(!rgba)
      {
      return;
      }
    for(int i=0;i<rgba->GetNumberOfTuples();i++)
      {
      if(rgba->GetValue(i*4+3) >= -1)
        {
        ids->InsertNextId(i);
        }
      }
    }
  else
    {
    vtkWarningMacro("No vtkMultiBlockDataSet set.");
    }
  return;
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::GetMaterialIds(vtkIdList* ids)
{
  ids->Reset();
  if(this->mb)
    {
    vtkPolyData* poly = this->GetMasterPolyData();
    vtkFloatArray* rgba = vtkFloatArray::SafeDownCast(
      poly->GetFieldData()->GetArray(MaterialColorsString));
    for(int i=0;i<rgba->GetNumberOfTuples();i++)
      {
      if(rgba->GetValue(i*4+3) >= -1)
        {
        ids->InsertNextId(i);
        }
      }
    }
  else
    {
    vtkWarningMacro("No vtkMultiBlockDataSet set.");
    }
  return;
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::GetShellIds(vtkIdList* ids)
{
  ids->Reset();
  if(this->mb)
    {
    vtkPolyData* poly = this->GetMasterPolyData();
    vtkFloatArray* rgba = vtkFloatArray::SafeDownCast(
      poly->GetFieldData()->GetArray(ShellColorsString));
    for(int i=0;i<rgba->GetNumberOfTuples();i++)
      {
      if(rgba->GetValue(i*4+3) >= -1)
        {
        ids->InsertNextId(i);
        }
      }
    }
  else
    {
    vtkWarningMacro("No vtkMultiBlockDataSet set.");
    }
  return;
}

//-----------------------------------------------------------------------------
int vtkMultiBlockWrapper::GetShellMaterial(int shellId)
{
  if(!this->mb)
    {
    vtkWarningMacro("No vtkMultiBlockDataSet set.");
    return -1;
    }
  for(int ui=0;ui<this->GetNumberOfModelFaces();ui++)
    {
    vtkIntArray* modelFaceInfo =
      vtkIntArray::SafeDownCast(
        this->GetModelFaceWithIndex(ui)->GetFieldData()->GetArray(ModelFaceDataString));
    if(shellId == modelFaceInfo->GetValue(shellIdArrayLoc))
      {
      return modelFaceInfo->GetValue(materialIdArrayLoc);
      }
    }
  return -1;
}

//-----------------------------------------------------------------------------
double* vtkMultiBlockWrapper::GetShellTranslationPoint(vtkIdType ShellId)
{
  vtkPolyData* master = this->GetMasterPolyData();
  vtkDoubleArray* array = vtkDoubleArray::SafeDownCast(
    master->GetFieldData()->GetArray(ShellTranslationPointString));
  double* ptr = array->GetPointer(ShellId*4);
  if(ptr[3] >= 0)
    {
    return ptr;
    }
  return 0;
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::SetShellTranslationPoint(vtkIdType ShellId,
  double* translationPoint)
{
  vtkPolyData* master = this->GetMasterPolyData();
  vtkDoubleArray* array = vtkDoubleArray::SafeDownCast(
    master->GetFieldData()->GetArray(ShellTranslationPointString));
  if(ShellId < array->GetNumberOfTuples())
    {
    if(translationPoint)
      {
      double vals[4] = {translationPoint[0], translationPoint[1],
                        translationPoint[2], 1};
      array->SetTypedTuple(ShellId, vals);
      }
    else
      {
      double vals[4] = {-1, -1, -1, -1};
      array->SetTypedTuple(ShellId, vals);
      }
    }
  else
    {
    vtkWarningMacro("Trying to set the translation point of a non-existing shell.");
    }
}

//-----------------------------------------------------------------------------
vtkIntArray* vtkMultiBlockWrapper::GetBCSModelFaceIdArray(
  const char* BCSName)
{
  if(!this->mb)
    {
    vtkWarningMacro("No vtkMultiBlockDataSet set.");
    return 0;
    }
  vtkFieldData* field = this->GetMasterPolyData()->GetFieldData();
  return vtkIntArray::SafeDownCast(field->GetArray(BCSName));
}

//-----------------------------------------------------------------------------
vtkIntArray* vtkMultiBlockWrapper::GetBCSModelFaceIdArray(
  int BCSId)
{
  if(!this->mb)
    {
    vtkWarningMacro("No vtkMultiBlockDataSet set.");
    return 0;
    }
  return this->GetBCSModelFaceIdArray(
    this->GetBCSNameFromId(BCSId));
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::ProcessForWriting(vtkPolyData* poly)
{
  if(!this->mb)
    {
    vtkWarningMacro("No vtkMultiBlockDataSet set.");
    return;
    }
  poly->ShallowCopy(this->GetMasterPolyData());

  vtkCellData* cellData = poly->GetCellData();
  int i;
  // the only cell data that we store is the model face IDs
  for(i=cellData->GetNumberOfArrays()-1;i>=0;i--)
    {
    if(strcmp(cellData->GetArrayName(i),ModelFaceIdsString))
      {
      cellData->RemoveArray(cellData->GetArrayName(i));
      }
    }
  vtkFieldData* fieldData = poly->GetFieldData();
  // check to see about file version
  vtkIntArray* version = vtkIntArray::SafeDownCast(
    this->PerformNeededDeepCopy(
      fieldData->GetArray(CMBFileVersionString),fieldData));
  if(!version)
    {
    version = vtkIntArray::New();
    version->SetName(CMBFileVersionString);
    version->SetNumberOfComponents(1);
    fieldData->AddArray(version);
    version->Delete();
    }
  // version 1
  version->SetNumberOfTuples(1);
  version->SetValue(0,1);

  // clean out all unnecessary field data
//   std::vector<int> arrays(fieldData->GetNumberOfArrays());
//   for(i=0;i<fieldData->GetNumberOfArrays();i++)
//     {
//     arrays[i] = 0;
//     //cout << fieldData->GetArrayName(i) << " is array " << i << endl;
//     }
//   if(fieldData->GetArray(ModelFaceDataString, i))
//     {
//     arrays[i] = 1;
//     }
//   if(fieldData->GetArray(CMBFileVersionString, i))
//     {
//     arrays[i] = 1;
//     }
//   if(fieldData->GetArray(ShellUserNamesString, i))
//     {
//     arrays[i] = 1;
//     }
//   if(fieldData->GetArray(ShellColorsString, i))
//     {
//     arrays[i] = 1;
//     }


}

// //-----------------------------------------------------------------------------
// int vtkMultiBlockWrapper::Load(const char* FileName)
// {
//   vtkXMLPolyDataReader* reader = vtkXMLPolyDataReader::New();
//   reader->SetFileName(FileName);
//   if(!reader->CanReadFile(FileName))
//     {
//     vtkErrorMacro("Cannot read CMB file");
//     reader->Delete();
//     return 1;
//     }
//   reader->Update();
//   float rgba[4];

//   vtkPolyData* poly = reader->GetOutput();
//   vtkIntArray* modelFaceCellIds = vtkIntArray::New();
//   modelFaceCellIds->SetNumberOfComponents(1);
//   modelFaceCellIds->SetNumberOfTuples(poly->GetNumberOfCells());
//   modelFaceCellIds->SetName(ModelFaceCellIdsString);
//   poly->GetCellData()->AddArray(modelFaceCellIds);
//   modelFaceCellIds->Delete();

//   vtkMultiBlockDataSet* mb2 = vtkMultiBlockDataSet::New();
//   this->SetMultiBlock(mb2);
//   mb2->Delete();
//   this->mb->SetNumberOfBlocks(1);
//   this->mb->SetBlock(0, poly);
//   vtkMultiBlockDataSet* mfMDS = vtkMultiBlockDataSet::New();
//   vtkMultiBlockDataSet* ngMDS = vtkMultiBlockDataSet::New();
//   this->mb->SetBlock(1, mfMDS); // Model Faces Root block
//   this->mb->SetBlock(2, ngMDS); // Nodal Groups Root block.
//   mfMDS->Delete();
//   ngMDS->Delete();

//   // now create each model face...
//   vtkFieldData* fieldData = poly->GetFieldData();
//   vtkFloatArray* faceColors = vtkFloatArray::SafeDownCast(
//     fieldData->GetArray(ModelFaceColorsString));
//   int numFaces = faceColors->GetNumberOfTuples();
//   std::vector<vtkIdList*> faces(numFaces);
//   std::vector<vtkIntArray*> faceData(numFaces);
//   int i;
//   for(i=0;i<numFaces;i++)
//     {
//     faceColors->GetTypedTuple(i, rgba);
//     if(rgba[3] >= -1)
//       {
//       faces[i] = vtkIdList::New();
//       faceData[i] = vtkIntArray::New();
//       faceData[i]->SetNumberOfComponents(1);
//       faceData[i]->SetName(ModelFaceDataString);
//       faceData[i]->SetNumberOfTuples(3);
//       }
//     else
//       {
//       faces[i] = 0;
//       faceData[i] = 0;
//       }
//     }
//   // collect BCSs for model face data
//   vtkFloatArray* BCSColors = vtkFloatArray::SafeDownCast(
//     fieldData->GetArray(BCSColorsString));
//   int numTuples = BCSColors ? BCSColors->GetNumberOfTuples() : 0;
//   for(i=0;i<numTuples;i++)
//     {
//     BCSColors->GetTypedTuple(i, rgba);
//     if(rgba[3] >= -1)
//       {
//       const char* BCSName = this->GetBCSNameFromId(i);
//       vtkIntArray* BCS =
//         vtkIntArray::SafeDownCast(fieldData->GetArray(BCSName));
//       for(int j=0;j<BCS->GetNumberOfTuples();j++)
//         {
//         faceData[BCS->GetValue(j)]->InsertNextValue(i);
//         }
//       }
//     }

//   // collect the cell ids for each of the model faces
//   vtkIntArray* faceIds = vtkIntArray::SafeDownCast(
//     poly->GetCellData()->GetArray(ModelFaceIdsString));
//   for(i=0;i<faceIds->GetNumberOfTuples();i++)
//     {
//     faces[faceIds->GetValue(i)]->InsertNextId(i);
//     }

//   // From verion 0.2.0, we removed block index from the array
//   // because a block can be removed from the multiblock now.
//   vtkIntArray* fileMFData = vtkIntArray::SafeDownCast(
//     fieldData->GetArray(ModelFaceConvenientArrayName));
//   if(!fileMFData || fileMFData->GetNumberOfComponents()==4)
//     {
//     if(!fileMFData)
//       {
//       fileMFData = vtkIntArray::SafeDownCast(
//         fieldData->GetArray(ModelFaceDataString));
//       }
//     if(fileMFData)
//       {
//       // Recreate the ModelFaceConvenientArray
//       vtkIntArray* mfDataArray = vtkIntArray::New();
//       mfDataArray->SetNumberOfComponents(3);
//       mfDataArray->SetNumberOfTuples(numFaces);
//       for(i=0;i<numFaces;i++)
//         {
//         int tmpVals[4];
//         int vals[3];
//         fileMFData->GetTypedTuple(i, tmpVals);
//         for(int j=0;j<3;j++)
//           {
//           vals[j] = tmpVals[j];
//           }
//         mfDataArray->SetTypedTuple(i, vals);
//         }
//       mfDataArray->SetName(ModelFaceConvenientArrayName);
//       fieldData->AddArray(mfDataArray);
//       mfDataArray->Delete();
//       }
//     }

//   for(i=0;i<numFaces;i++)
//     {
//     faceColors->GetTypedTuple(i, rgba);
//     if(rgba[3] >= -1 && faces[i] && faces[i]->GetNumberOfIds() > 0)
//       {
//       vtkIntArray* MFData = vtkIntArray::SafeDownCast(
//         fieldData->GetArray(ModelFaceConvenientArrayName));
//       int loadingFaceId = -1;
//       if(MFData)
//         {
//         int idVals[3];
//         MFData->GetTypedTuple(i, idVals);
//         loadingFaceId = idVals[modelFaceIdArrayLoc] ;
//         for(int j=0;j<3;j++)
//           {
//           faceData[i]->SetValue(j, idVals[j]);
//           }
//         }
//       else // for versions before 0.2.0
//         {
//         vtkErrorMacro("Missing ModelFaceConvenientArray.");
//         }
//       // get any existing modelfaceuse1 information
//       vtkAbstractArray* usearray = fieldData->GetArray(
//         vtkMultiBlockWrapper::GetModelFaceUse1String());
//       int modelFaceUse1 = -1;
//       if(usearray)
//         {
//         if(usearray->GetNumberOfTuples() > i)
//           {
//           modelFaceUse1 = vtkIntArray::SafeDownCast(usearray)->GetValue(i);
//           }
//         }

//       // temporarily show this face as not existing so we can
//       // put in the model face data here.
//       this->MarkObjectAsRemoved(faceColors, i);
//       int faceId = this->CreateModelFace(rgba, faces[i], modelFaceUse1, faceData[i], 1);
//       //MFData->SetValue((i*MFData->GetNumberOfComponents())+blockNumberArrayLoc,
//       //                 this->mb->GetNumberOfBlocks()-1);
//       if(loadingFaceId != faceId)
//         {
//         vtkErrorMacro("Incorrect model face Id during load.");
//         }
//       faces[i]->Delete();
//       faceData[i]->Delete();
//       }
//     else
//       {
//       // temporarily set this non-existent model face to "exist" so that the
//       // current model face id numbering does not have to change
//       rgba[3] = 1;
//       faceColors->SetTypedTuple(i, rgba);
//       }
//     }
//   // now set the "non-existent" faces back to not existing
//   for(i=0;i<faceColors->GetNumberOfTuples();i++)
//     {
//     if(faces[i] == 0)
//       {
//       this->MarkObjectAsRemoved(faceColors, i);
//       }
//     }

//   // check to see if there exists model face names, if not add generic names
//   if(!fieldData->GetAbstractArray(ModelFaceUserNamesString))
//     {
//     vtkStringArray* mfNames = vtkStringArray::New();
//     mfNames->SetName(ModelFaceUserNamesString);
//     mfNames->SetNumberOfComponents(1);
//     mfNames->SetNumberOfTuples(faceColors->GetNumberOfTuples());
//     for(i=0;i<faceColors->GetNumberOfTuples();i++)
//       {
//       char newName[20];
//       sprintf(newName, "face %d", i);
//       mfNames->InsertValue(i, newName);
//       }
//     fieldData->AddArray(mfNames);
//     mfNames->Delete();
//     }

//   // check to see if their exists translation points, if not add them in as empty
//   if(!fieldData->GetAbstractArray(ShellTranslationPointString))
//     {
//     vtkDoubleArray* tpoints = vtkDoubleArray::New();
//     tpoints->SetNumberOfComponents(4);
//     tpoints->SetName(ShellTranslationPointString);
//     vtkIdType numshells = this->GetNumberOfShells();
//     tpoints->SetNumberOfTuples(numshells);
//     double tpoint[4] = {-1, -1, -1, -1};
//     for(vtkIdType id=0;id<numshells;id++)
//       {
//       tpoints->SetTypedTuple(id, tpoint);
//       }
//     fieldData->AddArray(tpoints);
//     tpoints->Delete();
//     }

//   reader->Delete();
//   return 0;
// }

//-----------------------------------------------------------------------------
int vtkMultiBlockWrapper::GetNumberOfShells()
{
  if(!this->mb)
    {
    vtkWarningMacro("No vtkMultiBlockDataSet set.");
    return 0;
    }
  vtkFieldData* field = this->GetMasterPolyData()->GetFieldData();
  return field->GetAbstractArray(ShellUserNamesString)->GetNumberOfTuples();
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::SetSplitModelFaces(
  int modelFaceId, vtkIntArray* newfaces)
{
  if(!this->mb)
    {
    vtkWarningMacro("No vtkMultiBlockDataSet set.");
    return;
    }

  if(newfaces)
    {
    unsigned int blockIndex = this->GetModelFaceBlockIndex(modelFaceId);
    if(this->GetModelFaceWithIndex(blockIndex))
      {
      vtkFieldData* field = this->GetModelFaceWithIndex(blockIndex)->GetFieldData();
      newfaces->SetName(SplitFacesString);
      field->AddArray(newfaces);
      }
    }
}

//-----------------------------------------------------------------------------
int vtkMultiBlockWrapper::CreateModelFace(int shellId, int materialId,
                                          float* rgba, vtkIdList* cellIds,
                                          vtkIdList* BCSIds, int modelFaceUse1)
{
  if(cellIds->GetNumberOfIds() == 0)
    {
    return -1;
    }
  if(!this->mb)
    {
    vtkWarningMacro("No vtkMultiBlockDataSet set.");
    return -1;
    }

  vtkIntArray* modelFaceData = vtkIntArray::New();
  modelFaceData->SetNumberOfComponents(1);
  int numBCSIds = 0;
  if(BCSIds)
    {
    numBCSIds = BCSIds->GetNumberOfIds();
    }
  modelFaceData->SetNumberOfTuples(3+numBCSIds);
  modelFaceData->SetName(ModelFaceDataString);
  modelFaceData->SetValue(shellIdArrayLoc, shellId);
  modelFaceData->SetValue(materialIdArrayLoc, materialId);
  modelFaceData->SetValue(modelFaceIdArrayLoc, -1); // will fill in later...
  if(BCSIds)
    {
    for(int i=0;i<BCSIds->GetNumberOfIds();i++)
      {
      modelFaceData->SetValue(3+i, BCSIds->GetId(i));
      }
    }
  // check if there exists the mapping from main grid cell id to
  // model face grid cell id
  vtkIntArray * modelFaceCellId = vtkIntArray::SafeDownCast(
    this->GetMasterPolyData()->GetCellData()->GetArray(ModelFaceCellIdsString));
  if(!modelFaceCellId)
    {
    modelFaceCellId = vtkIntArray::New();
    modelFaceCellId->SetNumberOfComponents(1);
    int numCells = this->GetMasterPolyData()->GetNumberOfCells();
    modelFaceCellId->SetNumberOfTuples(numCells);
    for(int i=0;i<numCells;i++)
      {
      modelFaceCellId->SetValue(i, -1);
      }
    modelFaceCellId->SetName(ModelFaceCellIdsString);
    this->GetMasterPolyData()->GetCellData()->AddArray(modelFaceCellId);
    modelFaceCellId->Delete();
    }
  int newModelFaceId = this->CreateModelFace(rgba, cellIds, modelFaceUse1, modelFaceData);
  modelFaceData->Delete();
  return newModelFaceId;
}

//-----------------------------------------------------------------------------
int vtkMultiBlockWrapper::CreateModelFace(vtkIdList* cellIds)
{
  if(cellIds->GetNumberOfIds() == 0)
    {
    return -1;
    }
  if(!this->mb)
    {
    vtkWarningMacro("No vtkMultiBlockDataSet set.");
    return -1;
    }
  // perform needed deep copies on main vtkPolyData
  this->PerformNeededShallowCopy(this->GetMasterPolyData(),
    MasterPolyRootIndex, this->mb);
  // first do shallow copy on model face multiblock
  this->PerformNeededShallowCopy(this->GetModelFaceRootBlock(),
    ModelFaceRootIndex, this->mb);
  //this->PerformNeededDeepCopy(this->GetMasterPolyData(), 0);

  // all of the cells in cellIds must be in oldModelFaceId
  const int oldModelFaceId = this->GetModelFaceId(cellIds->GetId(0));
  const unsigned int blockIndex = this->GetModelFaceBlockIndex(oldModelFaceId);
  vtkPolyData* oldModelFace = this->GetModelFaceWithIndex(blockIndex);
  if(!oldModelFace)
    {
    vtkErrorMacro("Bad model face information.");
    return -1;
    }
  oldModelFace = vtkPolyData::SafeDownCast(
    this->PerformNeededDeepCopy(oldModelFace, blockIndex,
    this->GetModelFaceRootBlock()));

  vtkIntArray* rcinfo = vtkIntArray::SafeDownCast(
    oldModelFace->GetCellData()->GetArray(RCNameString));
  rcinfo = vtkIntArray::SafeDownCast(
    this->PerformNeededDeepCopy(rcinfo, oldModelFace->GetCellData()));
  vtkIdType i;
  for(i=0;i<cellIds->GetNumberOfIds();i++)
    {
    if(oldModelFaceId != this->GetModelFaceId(cellIds->GetId(i)))
      {
      vtkErrorMacro("Not all cells belong to the same model face.");
      return -1;
      }
    // proper way to do this appears to be to move the "last" cell to
    // the cell that we want to delete, update that modelfacecellid on block 0,
    // and then delete the last cell
    int lastCellId = oldModelFace->GetNumberOfCells()-1;
    // now iterate backwards to really figure out the last non-deleted cell
    int lastCellType = oldModelFace->GetCellType(lastCellId);
    while(lastCellType == 0 && lastCellId !=0)
      {
      lastCellId--;
      lastCellType = oldModelFace->GetCellType(lastCellId);
      }
    if(lastCellId == 0)
      {
      vtkErrorMacro("Ran out of cells in the model face.");
      throw;
      }
    vtkIdList * ptIds = vtkIdList::New();
    oldModelFace->GetCellPoints(lastCellId, ptIds);
    vtkIdType pts[3] = {ptIds->GetId(0), ptIds->GetId(1), ptIds->GetId(2)};
    ptIds->Delete();
    oldModelFace->ReplaceCell(this->GetModelFaceCellId(cellIds->GetId(i)), 3, pts);
    this->SetModelFaceCellId(rcinfo->GetValue(lastCellId),
                             this->GetModelFaceCellId(cellIds->GetId(i)));
    rcinfo->SetValue(this->GetModelFaceCellId(cellIds->GetId(i)),
                     rcinfo->GetValue(lastCellId));
    oldModelFace->DeleteCell(lastCellId);
    this->needToRemoveDeletedCells = true;
    }

  vtkIntArray* modelFaceData = vtkIntArray::SafeDownCast(
    this->PerformNeededDeepCopy(oldModelFace->GetFieldData()->
                                GetArray(ModelFaceDataString),
                                oldModelFace->GetFieldData()));
  // deep copy on main poly data model face data
  this->PerformNeededDeepCopy(this->GetMasterPolyData()->GetFieldData()->
                              GetArray(ModelFaceConvenientArrayName),
                              this->GetMasterPolyData()->GetFieldData());
  this->PerformNeededDeepCopy(this->GetMasterPolyData()->GetFieldData()->
                              GetAbstractArray(ModelFaceUserNamesString),
                              this->GetMasterPolyData()->GetFieldData());

  // just use the old model face rgba for the new one...
  vtkFieldData* fieldData = this->GetMasterPolyData()->GetFieldData();
  vtkFloatArray* floatArray = vtkFloatArray::SafeDownCast(
    fieldData->GetArray(ModelFaceColorsString));
  float rgba[4];
  floatArray->GetTypedTuple(oldModelFaceId, rgba);
  // get any existing modelfaceuse1 information
  vtkAbstractArray* usearray = fieldData->GetArray(
    vtkMultiBlockWrapper::GetModelFaceUse1String());
  int modelFaceUse1 = -1;
  if(usearray)
    {
    if(usearray->GetNumberOfTuples() > oldModelFaceId)
      {
      modelFaceUse1 = vtkIntArray::SafeDownCast(usearray)->GetValue(oldModelFaceId);
      }
    }

  int newModelFaceId = this->CreateModelFace(rgba, cellIds, modelFaceUse1, modelFaceData);
  // would like to do this later but need to change the way we calculate
  // lastCellId above first
  this->RemoveDeletedCellsFromModelFaces();

  return newModelFaceId;
}

//-----------------------------------------------------------------------------
int vtkMultiBlockWrapper::MergeModelFaces(vtkIdType targetFaceId,
                                          vtkIdList* faceIds)
{
  if(!this->mb)
    {
    vtkWarningMacro("No vtkMultiBlockDataSet set.");
    return 0;
    }
  if(faceIds->GetNumberOfIds() == 0 ||
    (faceIds->GetNumberOfIds()==1 && faceIds->GetId(0) == targetFaceId) )
    {
    return 1;
    }

  // perform needed shallow copies on main vtkPolyData
  this->PerformNeededShallowCopy(this->GetMasterPolyData(),
    MasterPolyRootIndex, this->mb);
  // first do shallow copy on model face multiblock
  this->PerformNeededShallowCopy(this->GetModelFaceRootBlock(),
    ModelFaceRootIndex, this->mb);

  vtkPolyData* origTargetFace = this->GetModelFaceWithId(targetFaceId);
  if(!origTargetFace)
    {
    vtkWarningMacro("No target face polydata.");
    return 0;
    }

  vtkPolyData* targetFace = vtkPolyData::SafeDownCast(
    this->PerformNeededDeepCopy(origTargetFace,
    this->GetModelFaceBlockIndex(targetFaceId),
    this->GetModelFaceRootBlock()));

  vtkIntArray* targetCellIdArray = vtkIntArray::SafeDownCast(
    this->PerformNeededDeepCopy(
    targetFace->GetCellData()->GetArray(RCNameString),
    targetFace->GetCellData()));
/*
  vtkIntArray* origCellIdArray = vtkIntArray::SafeDownCast(
    origTargetFace->GetCellData()->GetArray(RCNameString));

  vtkIntArray* targetCellIdArray = vtkIntArray::New();
  targetCellIdArray->DeepCopy(origCellIdArray);
*/

  vtkCell* cell;
  vtkIdType cellId;
  vtkIdList* delFaces = vtkIdList::New();
  vtkPolyData* poly = this->GetMasterPolyData();
  for(int i=0;i<faceIds->GetNumberOfIds();i++)
    {
    vtkIdType faceId = faceIds->GetId(i);
    if(faceId == targetFaceId)
      {
      continue;
      }
    vtkPolyData* facePoly = this->GetModelFaceWithId(faceId);

    if(facePoly)
      {
      vtkIntArray* cellIdArray = vtkIntArray::SafeDownCast(
        facePoly->GetCellData()->GetArray(RCNameString));
      int numCellIds = targetCellIdArray->GetNumberOfTuples();

      for(int n=0; n<facePoly->GetNumberOfCells(); n++)
        {
        cellId = cellIdArray->GetValue(n);
        cell = poly->GetCell(cellId);
        targetFace->InsertNextCell(cell->GetCellType(), cell->GetPointIds());
        targetCellIdArray->InsertNextValue(cellId);
        this->SetModelFaceId(cellId, targetFaceId);
        this->SetModelFaceCellId(cellId, numCellIds+n);
        }
      //numCellIds = targetCellIdArray->GetNumberOfTuples();
      delFaces->InsertNextId(faceId);
      }
    }
  for(int index=0; index<delFaces->GetNumberOfIds(); index++)
    {
    this->DeleteModelFace(delFaces->GetId(index));
    }

  delFaces->Delete();
  //targetFace->GetCellData()->AddArray(targetCellIdArray);
  //this->mb->SetBlock(this->GetModelFaceBlockIndex(targetFaceId), targetFace);
  return 1;
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::CreateModelFaces(vtkIntArray* markedCells,
                                            vtkIntArray* modelFaceIds)
{
  modelFaceIds->Reset();
  // now marked has all of the cellIds to be used for creating
  // new model faces so go ahead and do it...
  std::map<int, vtkIdList*> newModelFaces;
  int i, numCells = markedCells->GetNumberOfTuples();
  for(i=0;i<numCells;i++)
    {
    if(markedCells->GetValue(i) != 0)
      {
      int modelFaceId = this->GetModelFaceId(i);
      std::map<int, vtkIdList*>::iterator it =
        newModelFaces.find(modelFaceId);
      if(it != newModelFaces.end())
        {
        it->second->InsertNextId(i);
        }
      else
        {
        vtkIdList* ids = vtkIdList::New();
        ids->InsertNextId(i);
        newModelFaces[modelFaceId] = ids;
        }
      }
    }
  modelFaceIds->SetNumberOfComponents(1);
  for(std::map<int, vtkIdList*>::iterator it=newModelFaces.begin();
      it!=newModelFaces.end();it++)
    {
    // if all the old model faces belong to the new model face,
    // we can ignore doing this model face
    if(it->second->GetNumberOfIds() !=
       this->GetModelFaceWithId(this->GetModelFaceId(it->second->GetId(0)))->GetNumberOfCells())
      {
      int newId = this->CreateModelFace(it->second);
      modelFaceIds->InsertNextValue(newId);
      }
    else
      {
      modelFaceIds->InsertNextValue(this->GetModelFaceId(it->second->GetId(0)));
      }
    it->second->Delete();
    }
}

//-----------------------------------------------------------------------------
int vtkMultiBlockWrapper::CreateModelFace(
  float* rgba, vtkIdList* cellIds,int modelFaceUse1Id,
  vtkIntArray* modelFaceData,
  int isLoadingFile)
{
  if(cellIds->GetNumberOfIds() == 0)
    {
    return -1;
    }
  if(!this->GetModelFaceRootBlock())
    {
    vtkWarningMacro("No vtkMultiBlockDataSet set.");
    return -1;
    }
  this->GetModelFaceRootBlock()->SetNumberOfBlocks(
    this->GetModelFaceRootBlock()->GetNumberOfBlocks()+1);
  int modelFaceId = this->GetNextId(ModelFaceColorsString);

  // perform deep copy if needed on model face cell ids
  vtkCellData* modelFaceCellIds = this->GetMasterPolyData()->GetCellData();
  this->PerformNeededDeepCopy(
      modelFaceCellIds->GetArray(ModelFaceCellIdsString),
      modelFaceCellIds);


  // perform deep copy if needed on model face ids
  this->PerformNeededDeepCopy(vtkIntArray::SafeDownCast(
                                this->GetMasterPolyData()->GetCellData()->GetArray(ModelFaceIdsString)),
                              this->GetMasterPolyData()->GetCellData());

  vtkPolyData* poly = this->GetMasterPolyData();

  vtkPolyData* modelFace = vtkPolyData::New();
  modelFace->SetPoints(poly->GetPoints()); //this better be a shallow copy!!!
  modelFace->Allocate(cellIds->GetNumberOfIds());
  vtkIntArray* cellIdArray = vtkIntArray::New();
  cellIdArray->SetNumberOfComponents(1);
  cellIdArray->SetNumberOfTuples(cellIds->GetNumberOfIds());
  vtkIdType i;
  for(i=0;i<cellIds->GetNumberOfIds();i++)
    {
    vtkCell* cell = poly->GetCell(cellIds->GetId(i));
    modelFace->InsertNextCell(cell->GetCellType(), cell->GetPointIds());
    cellIdArray->SetValue(i, cellIds->GetId(i));
    this->SetModelFaceId(cellIds->GetId(i), modelFaceId);
    this->SetModelFaceCellId(cellIds->GetId(i), i);
    }
  cellIdArray->SetName(RCNameString);
  modelFace->GetCellData()->AddArray(cellIdArray);
  cellIdArray->Delete();

  // add the array first since it has most of the information we need
  modelFace->GetFieldData()->AddArray(modelFaceData);
  // now perform a shallow copy if needed since we will modify it slightly
  modelFaceData = vtkIntArray::SafeDownCast(
    this->PerformNeededDeepCopy(modelFaceData,modelFace->GetFieldData()));
  modelFaceData->SetValue(modelFaceIdArrayLoc, modelFaceId);
  // 3 is for shell, material and model faceid
  int numBCSIds = modelFaceData->GetNumberOfTuples()-3;
  if(numBCSIds)
    {
    vtkFieldData* polyFieldData = poly->GetFieldData();
    for(i=3;i<modelFaceData->GetNumberOfTuples();i++)
      {
      int bcsid = modelFaceData->GetValue(i);
      // add in new model face for each BCS
      vtkIntArray* bcs =
        vtkIntArray::SafeDownCast(this->PerformNeededDeepCopy(
                                    polyFieldData->GetArray(
                                      this->GetBCSNameFromId(bcsid)),
                                    polyFieldData));
      int j=0;
      bool insert=1;
      while(insert && j<bcs->GetNumberOfTuples())
        {
        if(bcs->GetValue(j) == modelFaceId)
          {
          // it should not hit this since the model face is new
          insert = 0;
          }
        j++;
        }
      if(insert)
        {
        bcs->InsertNextValue(modelFaceId);
        }
      }
    }

  unsigned int blockIndex = this->GetModelFaceRootBlock()->GetNumberOfBlocks()-1;
  // put in model face information in block 0
  vtkIntArray* mfData = vtkIntArray::SafeDownCast(
    this->GetMasterPolyData()->GetFieldData()->GetArray(ModelFaceConvenientArrayName));
  if(!mfData)
    {
    mfData = vtkIntArray::New();
    mfData->SetNumberOfComponents(3);
    mfData->SetName(ModelFaceConvenientArrayName);
    this->GetMasterPolyData()->GetFieldData()->AddArray(mfData);
    mfData->Delete();
    }
  int data[3] = {modelFaceData->GetValue(shellIdArrayLoc),
                 modelFaceData->GetValue(materialIdArrayLoc),
                 modelFaceId};
  if(mfData->GetNumberOfTuples() == modelFaceId)
    {
    // append to the array
    mfData->InsertNextTypedTuple(data);
    }
  else
    {
    mfData->InsertTypedTuple(modelFaceId, data);
    }
  // now add in rgba data
  vtkFloatArray* RGBA = vtkFloatArray::SafeDownCast(
    this->PerformNeededDeepCopy(
      poly->GetFieldData()->GetArray(ModelFaceColorsString),
      poly->GetFieldData()));
  if(!RGBA)
    {
    RGBA = vtkFloatArray::New();
    RGBA->SetNumberOfComponents(4);
    RGBA->SetName(ModelFaceColorsString);
    RGBA->SetNumberOfTuples(1);
    poly->GetFieldData()->AddArray(RGBA);
    RGBA->Delete();
    }
  int oldSize = RGBA->GetNumberOfTuples();
  if(RGBA->GetNumberOfTuples() == modelFaceId)
    {
    RGBA->InsertNextTypedTuple(rgba);
    }
  else if(RGBA->GetNumberOfTuples() < modelFaceId)
    {
    while(RGBA->GetNumberOfTuples() < modelFaceId)
      {
      float colors[4] = {-1,-1,-1,-2};
      RGBA->InsertNextTypedTuple(colors);
      // repeat work to mark this object as not existing
      // but then it is only done in one spot
      this->MarkObjectAsRemoved(RGBA, RGBA->GetNumberOfTuples()-1);
      }
    RGBA->InsertNextTypedTuple(rgba);
    }
  else
    {
    RGBA->SetTuple(modelFaceId, rgba);
    }

  //  now add in default model face name
  char newName[20];
  sprintf(newName, "face %d", modelFaceId);
  vtkStringArray* mfNames = vtkStringArray::SafeDownCast(
    this->GetMasterPolyData()->GetFieldData()->GetAbstractArray(ModelFaceUserNamesString));
  if(!mfNames)
    {
    mfNames = vtkStringArray::New();
    mfNames->SetNumberOfComponents(1);
    mfNames->SetName(ModelFaceUserNamesString);
    mfNames->SetNumberOfTuples(1);
    this->GetMasterPolyData()->GetFieldData()->AddArray(mfNames);
    mfNames->Delete();
    }
  oldSize = mfNames->GetNumberOfTuples();
  if(oldSize == modelFaceId)
    {
    mfNames->InsertNextValue(newName);
    }
  else if(oldSize < modelFaceId)
    {
    // insert the missing names with generic names
    while(mfNames->GetNumberOfTuples() <= modelFaceId)
      {
      mfNames->InsertNextValue(newName);
      sprintf(newName, "face %d", static_cast<int>(mfNames->GetNumberOfTuples()));
      }
    }
  else if(!isLoadingFile)
    {
    // Should use the name in the file if we are loading from a CMB file.
    mfNames->SetValue(modelFaceId, newName);
    }

  this->GetModelFaceRootBlock()->SetBlock(blockIndex, modelFace);
  modelFace->Delete();

  // set modelfaceuseid
  this->SetModelFaceUse1(modelFaceId, modelFaceUse1Id);

  return modelFaceId;
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::RemoveDeletedCellsFromModelFaces()
{
  if(this->needToRemoveDeletedCells)
    {
    for(int ui=0;ui<this->GetNumberOfModelFaces();ui++)
      {
      vtkPolyData* modelFace =this->GetModelFaceWithIndex(ui);
      modelFace->RemoveDeletedCells();
      }
    this->needToRemoveDeletedCells = false;
    }
}

//-----------------------------------------------------------------------------
vtkPolyData* vtkMultiBlockWrapper::GetModelFaceWithId(int modelFaceId)
{
  if(this->mb)
    {
    int blockIndex = this->GetModelFaceBlockIndex(modelFaceId);
    return this->GetModelFaceWithIndex(blockIndex);
    }
  vtkWarningMacro("No vtkMultiBlockDataSet set.");
  return 0;
}

//-----------------------------------------------------------------------------
vtkPolyData* vtkMultiBlockWrapper::GetModelFaceWithIndex(int mfIndex)
{
  if(this->GetModelFaceRootBlock() &&
    mfIndex>=0 && mfIndex<static_cast<int>(this->GetModelFaceRootBlock()->GetNumberOfBlocks()))
    {
    return vtkPolyData::SafeDownCast(
      this->GetModelFaceRootBlock()->GetBlock(mfIndex));
    }
  vtkWarningMacro("No vtkMultiBlockDataSet set, or no ModelFace found.");
  return 0;
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::DeleteModelFace(int modelFaceId)
{
  if(!this->mb)
    {
    vtkWarningMacro("No vtkMultiBlockDataSet set.");
    return;
    }
  if(!this->GetModelFaceWithId(modelFaceId))
    {
    vtkWarningMacro("No model face found.");
    return;
    }

  // mark that there is no model face use 1 for this model face
  // since it is about to get deleted
  this->SetModelFaceUse1(modelFaceId, -1);

  this->RemoveModelFaceFromBCSs(modelFaceId);
  this->RemoveModelFaceDataInfo(modelFaceId);

  this->GetModelFaceRootBlock()->RemoveBlock(
    this->GetModelFaceBlockIndex(modelFaceId));
}

//-----------------------------------------------------------------------------
vtkIdType vtkMultiBlockWrapper::GetCellIdOnMasterPolyData(vtkIdType cellId,
                                                          int modelFaceId)
{
  vtkPolyData* poly = GetModelFaceWithId(modelFaceId);
  if(poly)
    {
    vtkIntArray* rcArray = vtkIntArray::SafeDownCast(
      poly->GetCellData()->GetArray(RCNameString));
    if(rcArray && rcArray->GetNumberOfTuples()>cellId)
      {
      return rcArray->GetValue(cellId);
      }
    }
  return -1;
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::GetMaterialUserData(int materialId, int& materialUserId,
                                               const char*& materialUserName)
{
  vtkPolyData* poly = this->GetMasterPolyData();
  vtkStringArray* userNames = vtkStringArray::SafeDownCast(
    poly->GetFieldData()->GetAbstractArray(MaterialUserNamesString));
  materialUserName = userNames->GetValue(materialId);
  vtkIntArray* userIds = vtkIntArray::SafeDownCast(
    poly->GetFieldData()->GetArray(MaterialUserIdsString));
  materialUserId = userIds->GetValue(materialId);
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::GetBCSUserData(int BCSId, int& BCSUserId,
                                          const char*& BCSUserName)
{
  vtkPolyData* poly = this->GetMasterPolyData();
  vtkStringArray* userNames = vtkStringArray::SafeDownCast(
    poly->GetFieldData()->GetAbstractArray(BCSUserNamesString));
  BCSUserName = userNames->GetValue(BCSId);
  vtkIntArray* userIds = vtkIntArray::SafeDownCast(
    poly->GetFieldData()->GetArray(BCSUserIdsString));
  BCSUserId = userIds->GetValue(BCSId);
}

//-----------------------------------------------------------------------------
const char* vtkMultiBlockWrapper::GetModelFaceUserName(int modelFaceId)
{
  vtkPolyData* poly = this->GetMasterPolyData();
  vtkStringArray* userNames = vtkStringArray::SafeDownCast(
    poly->GetFieldData()->GetAbstractArray(ModelFaceUserNamesString));
  return userNames->GetValue(modelFaceId);
}

//-----------------------------------------------------------------------------
const char* vtkMultiBlockWrapper::GetShellUserName(int shellId)
{
  vtkPolyData* poly = this->GetMasterPolyData();
  vtkStringArray* userNames = vtkStringArray::SafeDownCast(
    poly->GetFieldData()->GetAbstractArray(ShellUserNamesString));
  return userNames->GetValue(shellId);
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::GetModelFaceBCSIds(int modelFaceId,
                                              vtkIdList* list)
{
  list->Reset();
  if(!this->mb)
    {
    vtkWarningMacro("No vtkMultiBlockDataSet set.");
    return;
    }
  vtkFieldData* field = this->GetModelFaceWithId(modelFaceId)->GetFieldData();
  vtkIntArray* BCSIds = vtkIntArray::SafeDownCast(
    field->GetArray(ModelFaceDataString));
  for(int i=3;i<BCSIds->GetNumberOfTuples();i++)
    {
    list->InsertNextId(BCSIds->GetValue(i));
    }
}

//-----------------------------------------------------------------------------
int vtkMultiBlockWrapper::GetModelFaceMaterialId(int modelFaceId)
{
  if(!this->mb)
    {
    vtkWarningMacro("No vtkMultiBlockDataSet set.");
    return -1;
    }
  vtkFieldData* field = this->GetMasterPolyData()->GetFieldData();
  vtkIntArray* modelFaceData = vtkIntArray::SafeDownCast(
    field->GetArray(ModelFaceConvenientArrayName));
  int vals[3];
  for(int i=0;i<modelFaceData->GetNumberOfTuples();i++)
    {
    modelFaceData->GetTypedTuple(i, vals);
    if(vals[modelFaceIdArrayLoc] == modelFaceId)
      {
      return vals[materialIdArrayLoc];
      }
    }

  return -1;
}

//-----------------------------------------------------------------------------
int vtkMultiBlockWrapper::GetModelFaceShellId(int modelFaceId)
{
  if(!this->mb)
    {
    vtkWarningMacro("No vtkMultiBlockDataSet set.");
    return -1;
    }
  vtkFieldData* field = this->GetMasterPolyData()->GetFieldData();
  vtkIntArray* modelFaceData = vtkIntArray::SafeDownCast(
    field->GetArray(ModelFaceConvenientArrayName));
  int vals[3];
  for(int i=0;i<modelFaceData->GetNumberOfTuples();i++)
    {
    modelFaceData->GetTypedTuple(i, vals);
    if(vals[modelFaceIdArrayLoc] == modelFaceId)
      {
      return vals[shellIdArrayLoc];
      }
    }

  return -1;
}

//-----------------------------------------------------------------------------
unsigned int vtkMultiBlockWrapper::GetModelFaceBlockIndex(int modelFaceId)
{
  if(!this->GetModelFaceRootBlock())
    {
    vtkWarningMacro("No vtkMultiBlockDataSet set, or no ModelFaceRootBlock");
    return 0;
    }
  for(int i=0;  i<this->GetNumberOfModelFaces(); i++)
    {
    if(this->GetModelFaceWithIndex(i))
      {
      vtkFieldData* field = this->GetModelFaceWithIndex(i)->GetFieldData();
      vtkIntArray* modelFaceData = vtkIntArray::SafeDownCast(
        field->GetArray(ModelFaceDataString));
      if(modelFaceData->GetValue(modelFaceIdArrayLoc) == modelFaceId)
        {
        return i;
        }
      }
    }
  this->GetModelFaceBlockIndex(modelFaceId);

  vtkErrorMacro("Trying to access a non-existent model face.")
  return -1;
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::ChangeMaterialIdOfShell(int shellId,
                                                   int newMaterialId)
{
  if(!this->mb)
    {
    vtkWarningMacro("No vtkMultiBlockDataSet set.");
    return;
    }
  vtkFieldData* field = this->GetMasterPolyData()->GetFieldData();
  vtkIntArray* array = vtkIntArray::SafeDownCast(
    this->PerformNeededDeepCopy(field->GetArray(
                                  ShellMaterialIdString), field));

  if(!array)
    {
    array = vtkIntArray::New();
    array->SetNumberOfComponents(1);
    array->SetName(ShellMaterialIdString);
    field->AddArray(array);
    array->Delete();
    }
  if(array->GetNumberOfTuples() == shellId)
    {
    array->InsertNextTypedTuple(&newMaterialId);
    }
  else if(array->GetNumberOfTuples() < shellId)
    {
    while(array->GetNumberOfTuples() < shellId)
      {
      int neg1 = -1;
      array->InsertNextTypedTuple(&neg1);
      }
    array->InsertNextTypedTuple(&newMaterialId);
    }
  else
    {
    array->SetValue(shellId, newMaterialId);
    }

  // We also need to update the ModelFaceConvenientArray(
  // called ModelFaceDataString before) on block 0
  vtkIntArray* MFData = vtkIntArray::SafeDownCast(
    this->PerformNeededDeepCopy(
    field->GetArray(ModelFaceConvenientArrayName), field));
  if(MFData)
    {
    int vals[3];
    for(int i=0; i<MFData->GetNumberOfTuples(); i++)
      {
      MFData->GetTypedTuple(i, vals);
      if(vals[shellIdArrayLoc] == shellId)
        {
        vals[materialIdArrayLoc] = newMaterialId;
        MFData->InsertTypedTuple(i, vals);
        // break;
        }
      }
    }
  // first do shallow copy on model face multiblock
  this->PerformNeededShallowCopy(this->GetModelFaceRootBlock(),
    ModelFaceRootIndex, this->mb);

  for(int ui=0;ui<this->GetNumberOfModelFaces();ui++)
    {
    field = this->GetModelFaceWithIndex(ui)->GetFieldData();
    array = vtkIntArray::SafeDownCast(
      field->GetArray(ModelFaceDataString));
    if(shellId == array->GetValue(shellIdArrayLoc))
      {
      if(array->GetValue(materialIdArrayLoc) != newMaterialId)
        {
        this->PerformNeededShallowCopy(this->GetModelFaceWithIndex(ui),
          ui, this->GetModelFaceRootBlock());
        field = this->GetModelFaceWithIndex(ui)->GetFieldData();
        array = vtkIntArray::SafeDownCast(
          this->PerformNeededDeepCopy(field->GetArray(ModelFaceDataString), field));
        array->SetValue(materialIdArrayLoc, newMaterialId);
        }
      }
    }
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::ChangeUserNameOfMaterial(int materialId,
    const char* userName)
{
  if(!this->mb)
    {
    vtkWarningMacro("No vtkMultiBlockDataSet set.");
    return;
    }
  vtkFieldData* field = this->GetMasterPolyData()->GetFieldData();
  vtkStringArray* matNames = vtkStringArray::SafeDownCast(
    this->PerformNeededDeepCopy(field->GetAbstractArray(
                                  MaterialUserNamesString), field));
  matNames->SetValue(materialId, userName);
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::ChangeUserIdOfMaterial(int materialId,
    int materialUserId)
{
  if(!this->mb)
    {
    vtkWarningMacro("No vtkMultiBlockDataSet set.");
    return;
    }
  vtkFieldData* field = this->GetMasterPolyData()->GetFieldData();
  vtkIntArray* matIds = vtkIntArray::SafeDownCast(
    this->PerformNeededDeepCopy(field->GetArray(MaterialUserIdsString), field));
  matIds->SetValue(materialId, materialUserId);
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::ChangeUserColorOfMaterial(int materialId,
   float* RGBA )
{
  if(!this->mb)
    {
    vtkWarningMacro("No vtkMultiBlockDataSet set.");
    return;
    }
  vtkFieldData* field = this->GetMasterPolyData()->GetFieldData();
  vtkFloatArray* matColors = vtkFloatArray::SafeDownCast(
    this->PerformNeededDeepCopy(field->GetArray(MaterialColorsString), field));
  matColors->SetTuple(materialId, RGBA);
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::ChangeUserNameOfShell(int shellId,
    const char* userName)
{
  if(!this->mb)
    {
    vtkWarningMacro("No vtkMultiBlockDataSet set.");
    return;
    }
  vtkFieldData* field = this->GetMasterPolyData()->GetFieldData();
  vtkStringArray* shellNames = vtkStringArray::SafeDownCast(
    this->PerformNeededDeepCopy(field->GetAbstractArray(
                                  ShellUserNamesString), field));
  shellNames->SetValue(shellId, userName);
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::ChangeUserIdOfShell(int /*shellId*/,
    int /*shellUserId*/)
{
  throw 1;
/*
  if(!this->mb)
    {
    vtkWarningMacro("No vtkMultiBlockDataSet set.");
    return;
    }
  vtkFieldData* field = this->GetMasterPolyData()->GetFieldData();
  vtkIntArray* shellIds = vtkIntArray::SafeDownCast(
    this->PerformNeededDeepCopy(field->GetArray(ShellUserIdsString), field));
  shellIds->SetValue(shellId, shellUserId);*/
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::ChangeUserColorOfShell(int shellId,
   float* RGBA )
{
  if(!this->mb)
    {
    vtkWarningMacro("No vtkMultiBlockDataSet set.");
    return;
    }
  vtkFieldData* field = this->GetMasterPolyData()->GetFieldData();
  vtkFloatArray* shellColors = vtkFloatArray::SafeDownCast(
    this->PerformNeededDeepCopy(field->GetArray(ShellColorsString), field));
  shellColors->SetTuple(shellId, RGBA);
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::ChangeUserNameOfBCS(int BCSId,
    const char* userName)
{
  if(!this->mb)
    {
    vtkWarningMacro("No vtkMultiBlockDataSet set.");
    return;
    }
  vtkFieldData* field = this->GetMasterPolyData()->GetFieldData();
  vtkStringArray* BCSNames = vtkStringArray::SafeDownCast(
    this->PerformNeededDeepCopy(field->GetAbstractArray(
                                  BCSUserNamesString), field));
  BCSNames->SetValue(BCSId, userName);
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::ChangeUserIdOfBCS(int BCSId,
    int BCSUserId)
{
  if(!this->mb)
    {
    vtkWarningMacro("No vtkMultiBlockDataSet set.");
    return;
    }
  vtkFieldData* field = this->GetMasterPolyData()->GetFieldData();
  vtkIntArray* BCSIds = vtkIntArray::SafeDownCast(
    this->PerformNeededDeepCopy(field->GetArray(BCSUserIdsString), field));
  BCSIds->SetValue(BCSId, BCSUserId);
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::ChangeUserColorOfBCS(int BCSId,
   float* RGBA )
{
  if(!this->mb)
    {
    vtkWarningMacro("No vtkMultiBlockDataSet set.");
    return;
    }
  vtkFieldData* field = this->GetMasterPolyData()->GetFieldData();
  vtkFloatArray* BCSColors = vtkFloatArray::SafeDownCast(
    this->PerformNeededDeepCopy(field->GetArray(BCSColorsString), field));
  BCSColors->SetTuple(BCSId, RGBA);
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::ChangeUserColorOfModelFace(int modelFaceId,
   float* RGBA )
{
  if(!this->mb)
    {
    vtkWarningMacro("No vtkMultiBlockDataSet set.");
    return;
    }
  vtkFieldData* field = this->GetMasterPolyData()->GetFieldData();
  vtkFloatArray* modelFaceColors = vtkFloatArray::SafeDownCast(
    this->PerformNeededDeepCopy(field->GetArray(ModelFaceColorsString), field));
  modelFaceColors->SetTuple(modelFaceId, RGBA);
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::ChangeUserNameOfModelFace(int modelFaceId,
    const char* userName)
{
  if(!this->mb)
    {
    vtkWarningMacro("No vtkMultiBlockDataSet set.");
    return;
    }
  vtkFieldData* field = this->GetMasterPolyData()->GetFieldData();
  vtkStringArray* ModelFaceNames = vtkStringArray::SafeDownCast(
    this->PerformNeededDeepCopy(field->GetAbstractArray(
                                  ModelFaceUserNamesString), field));
  ModelFaceNames->SetValue(modelFaceId, userName);
}

//-----------------------------------------------------------------------------
int vtkMultiBlockWrapper::GetNextId(const char* colorsArrayName)
{
  vtkFloatArray* array = vtkFloatArray::SafeDownCast(
    this->GetMasterPolyData()->GetFieldData()->GetArray(colorsArrayName));
  if(!array)
    {
    return 0;
    }
  for(int i=0;i<array->GetNumberOfTuples();i++)
    {
    if(array->GetValue(i*4+3) < -1) // the alpha value
      {
      return i; // this is a "hole" we can fill in
      }
    }
  return array->GetNumberOfTuples(); // no "holes" so append
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::MarkObjectAsRemoved(const char* colorsArrayName,
                                               int id)
{
  this->MarkObjectAsRemoved(vtkFloatArray::SafeDownCast(
                          this->GetMasterPolyData()->GetFieldData()->
                          GetArray(colorsArrayName)), id);
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::MarkObjectAsRemoved(vtkFloatArray* colors, int id)
{
  // mark the alpha value as -2 to indicate that this is a "hole" and
  // not a real object.  also set -1 to rgb to indicate default values.
  float vals[4] = {-1, -1, -1, -2};
  colors->SetTuple(id, vals);
}

//-----------------------------------------------------------------------------
int vtkMultiBlockWrapper::GetNumberOfBCSs()
{
  if(!this->mb)
    {
    vtkWarningMacro("No vtkMultiBlockDataSet set.");
    return 0;
    }
  vtkFieldData* fieldData = this->GetMasterPolyData()->GetFieldData();
  if(!fieldData)
    {
    return 0;
    }
  // this is the slow but safe way, otherwise could assume we know
  // there aren't any "extra" arrays added to block 0
  vtkFloatArray* bcscolors = vtkFloatArray::SafeDownCast(
    this->GetMasterPolyData()->GetFieldData()->GetArray(BCSColorsString));
  int num = 0;
  for(int i=0;i<bcscolors->GetNumberOfTuples();i++)
    {
    if(bcscolors->GetValue(i*4+3) >= -1)
      {
      num++;
      }
    }
  return num;
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::GetBCSIdList(vtkIdList* IdList)
{
  IdList->Reset();
  if(!this->mb)
    {
    vtkWarningMacro("No vtkMultiBlockDataSet set.");
    return;
    }
  vtkFloatArray* bcscolors = vtkFloatArray::SafeDownCast(
    this->GetMasterPolyData()->GetFieldData()->GetArray(BCSColorsString));
  for(int i=0;i<bcscolors->GetNumberOfTuples();i++)
    {
    if(bcscolors->GetValue(i*4+3) < -1)
      {
      IdList->InsertNextId(i);
      }
    }
}

//-----------------------------------------------------------------------------
int vtkMultiBlockWrapper::AddBCS(int BCSUserId, const char* BCSUserName,
                                     float* colors, vtkIdList * modelFaceIds)
{
  if(!this->mb)
    {
    vtkWarningMacro("No vtkMultiBlockDataSet set.");
    return -1;
    }
  int BCSId = this->GetNextId(BCSColorsString);
  char* name;
  this->CreateBCSNameFromId(BCSId, &name);

  // need some type of copy here
  this->PerformNeededShallowCopy(this->GetMasterPolyData(),
    MasterPolyRootIndex, this->mb);
  // first do shallow copy on model face multiblock
  this->PerformNeededShallowCopy(this->GetModelFaceRootBlock(),
    ModelFaceRootIndex, this->mb);

  vtkFieldData* fieldData = this->GetMasterPolyData()->GetFieldData();

  vtkIntArray* array = vtkIntArray::New();
  array->SetName(name);
  delete []name;
  name = 0;
  array->SetNumberOfComponents(1);
  array->SetNumberOfTuples(modelFaceIds->GetNumberOfIds());
  for(int i=0;i<modelFaceIds->GetNumberOfIds();i++)
    {
    // first do shallow copy on the model face polydata
    unsigned int blockIndex =
      this->GetModelFaceBlockIndex(modelFaceIds->GetId(i));
    this->PerformNeededShallowCopy(this->GetModelFaceWithIndex(blockIndex),
      blockIndex, this->GetModelFaceRootBlock());
    array->SetValue(i, modelFaceIds->GetId(i));
    // need to add this BCS to the model face info
    vtkFieldData* modelFaceFieldData =
      this->GetModelFaceWithIndex(blockIndex)->GetFieldData();
    // there better be field data already associated with the model face
    vtkIntArray* dataArray = vtkIntArray::SafeDownCast(
      this->PerformNeededDeepCopy(
        modelFaceFieldData->GetArray(ModelFaceDataString),
        modelFaceFieldData));
    dataArray->InsertNextValue(BCSId);
    }
  //fieldData = this->PerformNeededDeepCopy(fieldData, this->GetMasterPolyData());
  fieldData->AddArray(array);
  array->Delete();

  vtkFloatArray* colorArray =
    vtkFloatArray::SafeDownCast(this->PerformNeededDeepCopy(
                                  fieldData->GetArray(BCSColorsString),
                                  fieldData));
  vtkStringArray* nameArray =
    vtkStringArray::SafeDownCast(this->PerformNeededDeepCopy(
                                   fieldData->GetAbstractArray(BCSUserNamesString),
                                   fieldData));
  vtkIntArray* idArray =
    vtkIntArray::SafeDownCast(this->PerformNeededDeepCopy(
                                fieldData->GetArray(BCSUserIdsString),
                                fieldData));
  if(!colorArray)
    {
    // assume that all of the arrays need to be created
    colorArray = vtkFloatArray::New();
    colorArray->SetNumberOfComponents(4);
    colorArray->SetName(BCSColorsString);
    fieldData->AddArray(colorArray);
    colorArray->Delete();
    nameArray = vtkStringArray::New();
    nameArray->SetName(BCSUserNamesString);
    fieldData->AddArray(nameArray);
    nameArray->Delete();
    idArray = vtkIntArray::New();
    idArray->SetNumberOfComponents(1);
    idArray->SetName(BCSUserIdsString);
    fieldData->AddArray(idArray);
    idArray->Delete();
    }
  if(colorArray->GetNumberOfTuples() == BCSId)
    {
    colorArray->InsertNextTuple(colors);
    nameArray->InsertNextValue(BCSUserName);
    idArray->InsertNextValue(BCSUserId);
    }
  else
    {
    colorArray->InsertTuple(BCSId, colors);
    nameArray->SetValue(BCSId, BCSUserName);
    idArray->InsertValue(BCSId, BCSUserId);
    }

  return BCSId;
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::DeleteBCS(const char* name)
{
  if(!mb)
    {
    vtkWarningMacro("No vtkMultiBlockDataSet set.");
    return;
    }

  vtkFieldData* fieldData = this->GetMasterPolyData()->GetFieldData();
  vtkIntArray *array = vtkIntArray::SafeDownCast(fieldData->GetArray(name));
  if(!array)
    {
    return;
    }
  this->PerformNeededShallowCopy(this->GetMasterPolyData(),
    MasterPolyRootIndex, this->mb);
  // first do shallow copy on model face multiblock
  this->PerformNeededShallowCopy(this->GetModelFaceRootBlock(),
    ModelFaceRootIndex, this->mb);

  fieldData = this->GetMasterPolyData()->GetFieldData();
  array = vtkIntArray::SafeDownCast(fieldData->GetArray(name));
  // remove BCS from model faces
  int BCSId = this->GetBCSIdFromName(name);
  for(vtkIdType id=0;id<array->GetNumberOfTuples();id++)
    {
    //vtkPolyData* modelFace = this->GetModelFace(array->GetValue(id));
    unsigned int blockIndex = this->GetModelFaceBlockIndex(array->GetValue(id));
    // acb test if the shallow copy is really needed below for the polydata
    // may only be needed for the actual array
    this->PerformNeededShallowCopy(this->GetModelFaceWithIndex(blockIndex),
      blockIndex, this->GetModelFaceRootBlock());
    vtkFieldData* modelFaceFieldData =
      this->GetModelFaceWithIndex(blockIndex)->GetFieldData();
    vtkIntArray* modelFaceArray =
      vtkIntArray::SafeDownCast(modelFaceFieldData->GetArray(ModelFaceDataString));
    for(vtkIdType id2=modelFaceArray->GetNumberOfTuples()-1;
        id2>2;id2--) // first three are shell, material id and model face ids
      {
      if(modelFaceArray->GetValue(id2) == BCSId)
        {
        modelFaceArray = vtkIntArray::SafeDownCast(
          this->PerformNeededDeepCopy(modelFaceArray, modelFaceFieldData));
        modelFaceArray->RemoveTuple(id2);
        }
      }
    }
  fieldData->RemoveArray(name);
  vtkFloatArray* BCSColors = vtkFloatArray::SafeDownCast(
    this->PerformNeededDeepCopy(fieldData->GetArray(BCSColorsString), fieldData));
  this->MarkObjectAsRemoved(BCSColors, BCSId);
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::DeleteBCS(int id)
{
  if(!this->mb)
    {
    vtkWarningMacro("No vtkMultiBlockDataSet set.");
    return;
    }
  const char* name = this->GetBCSNameFromId(id);
  this->DeleteBCS(name);
}

//-----------------------------------------------------------------------------
int vtkMultiBlockWrapper::AddMaterial(int MaterialUserId,
                                      const char* MaterialUserName,
                                      float* colors)
{
  if(!this->mb)
    {
    vtkWarningMacro("No vtkMultiBlockDataSet set.");
    return -1;
    }
  int MaterialId = this->GetNextId(MaterialColorsString);

  // need some type of copy here
  this->PerformNeededShallowCopy(this->GetMasterPolyData(),
    MasterPolyRootIndex, this->mb);

  vtkFieldData* fieldData = this->GetMasterPolyData()->GetFieldData();

  vtkFloatArray* colorArray =
    vtkFloatArray::SafeDownCast(this->PerformNeededDeepCopy(
                                  fieldData->GetArray(MaterialColorsString),
                                  fieldData));
  vtkStringArray* nameArray =
    vtkStringArray::SafeDownCast(this->PerformNeededDeepCopy(
                                   fieldData->GetAbstractArray(MaterialUserNamesString),
                                   fieldData));
  vtkIntArray* idArray =
    vtkIntArray::SafeDownCast(this->PerformNeededDeepCopy(
                                fieldData->GetArray(MaterialUserIdsString),
                                fieldData));
  if(!colorArray)
    {
    // assume that all of the arrays need to be created
    colorArray = vtkFloatArray::New();
    colorArray->SetNumberOfComponents(4);
    colorArray->SetName(MaterialColorsString);
    fieldData->AddArray(colorArray);
    colorArray->Delete();
    nameArray = vtkStringArray::New();
    nameArray->SetName(MaterialUserNamesString);
    fieldData->AddArray(nameArray);
    nameArray->Delete();
    idArray = vtkIntArray::New();
    idArray->SetNumberOfComponents(1);
    idArray->SetName(MaterialUserIdsString);
    fieldData->AddArray(idArray);
    idArray->Delete();
    }
  if(colorArray->GetNumberOfTuples() == MaterialId)
    {
    colorArray->InsertNextTuple(colors);
    nameArray->InsertNextValue(MaterialUserName);
    idArray->InsertNextValue(MaterialUserId);
    }
  else
    {
    colorArray->InsertTuple(MaterialId, colors);
    nameArray->SetValue(MaterialId, MaterialUserName);
    idArray->InsertValue(MaterialId, MaterialUserId);
    }

  return MaterialId;
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::DeleteMaterial(int id)
{
  if(!this->mb)
    {
    vtkWarningMacro("No vtkMultiBlockDataSet set.");
    return;
    }
  this->PerformNeededShallowCopy(this->GetMasterPolyData(), 0, this->mb);
  vtkFieldData* field = this->GetMasterPolyData()->GetFieldData();
  vtkFloatArray* rgba = vtkFloatArray::SafeDownCast(
    this->PerformNeededDeepCopy(field->GetArray(MaterialColorsString), field));
  this->MarkObjectAsRemoved(rgba, id);
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::AddBCSToModelFace(int modelFaceId,
                                                const char* BCSName)
{
  if(!this->mb)
    {
    vtkWarningMacro("No vtkMultiBlockDataSet set.");
    return;
    }
  int id = this->GetBCSIdFromName(BCSName);
  this->AddBCSToModelFace(modelFaceId, id, BCSName);
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::AddBCSToModelFace(int modelFaceId, int BCSId)
{
  if(!mb)
    {
    vtkWarningMacro("No vtkMultiBlockDataSet set.");
    return;
    }
  const char* name = this->GetBCSNameFromId(BCSId);
  this->AddBCSToModelFace(modelFaceId, BCSId, name);
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::AddBCSToModelFace(int modelFaceId, int BCSId,
                                             const char* BCSName)
{
  vtkIntArray* BCSModelFaces = vtkIntArray::SafeDownCast(
    this->GetMasterPolyData()->GetFieldData()->GetArray(BCSName));
  if(!BCSModelFaces)
    {
    vtkWarningMacro(" No corresponding BCS array found.");
    return;
    }
  int i;
  for(i=0;i<BCSModelFaces->GetNumberOfTuples();i++)
    {
    if(modelFaceId == BCSModelFaces->GetValue(i))
      {
      return; // model face already has BCS set over it
      }
    }

  this->PerformNeededShallowCopy(this->GetMasterPolyData(),
    MasterPolyRootIndex, this->mb);
  // first do shallow copy on model face multiblock
  this->PerformNeededShallowCopy(this->GetModelFaceRootBlock(),
    ModelFaceRootIndex, this->mb);

  unsigned int blockIndex = this->GetModelFaceBlockIndex(modelFaceId);
  this->PerformNeededShallowCopy(this->GetModelFaceWithIndex(blockIndex),
    blockIndex, this->GetModelFaceRootBlock());
  BCSModelFaces = vtkIntArray::SafeDownCast(
    this->PerformNeededDeepCopy(
      BCSModelFaces, this->GetMasterPolyData()->GetFieldData()));
  // check to see if this array already has modelFaceId in it
  bool insert = 1;
  i=0;
  while(insert && i<BCSModelFaces->GetNumberOfTuples())
    {
    if(BCSModelFaces->GetValue(i) == modelFaceId)
      {
      insert = 0;
      }
    i++;
    }
  if(insert)
    {
    BCSModelFaces->InsertNextValue(modelFaceId);
    }
  vtkPolyData* data = this->GetModelFaceWithId(modelFaceId);
  vtkFieldData* field = data->GetFieldData();
  vtkIntArray* modelFaceBCSs = vtkIntArray::SafeDownCast(
    this->PerformNeededDeepCopy(field->GetArray(ModelFaceDataString), field));
  modelFaceBCSs->InsertNextValue(BCSId);
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::RemoveBCSFromModelFace(int modelFaceId,
                                                     const char* BCSName)
{
  if(!mb)
    {
    vtkWarningMacro("No vtkMultiBlockDataSet set.");
    return;
    }
  int BCSId = this->GetBCSIdFromName(BCSName);
  this->RemoveBCSFromModelFace(modelFaceId, BCSId, BCSName);
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::RemoveBCSFromModelFace(int modelFaceId, int BCSId)
{
  if(!mb)
    {
    vtkWarningMacro("No vtkMultiBlockDataSet set.");
    return;
    }
  const char* name = this->GetBCSNameFromId(BCSId);
  this->RemoveBCSFromModelFace(modelFaceId, BCSId, name);
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::RemoveBCSFromModelFace(int modelFaceId, int BCSId,
                                                     const char* BCSName)
{
  vtkIntArray* BCSModelFaces = vtkIntArray::SafeDownCast(
    this->GetMasterPolyData()->GetFieldData()->GetArray(BCSName));
  int i;
  this->PerformNeededShallowCopy(this->GetMasterPolyData(),
    MasterPolyRootIndex,this->mb);
  // first do shallow copy on model face multiblock
  this->PerformNeededShallowCopy(this->GetModelFaceRootBlock(),
    ModelFaceRootIndex, this->mb);

  for(i=BCSModelFaces->GetNumberOfTuples()-1;i>-1;i--)
    {
    if(modelFaceId == BCSModelFaces->GetValue(i))
      {
      BCSModelFaces = vtkIntArray::SafeDownCast(
        this->PerformNeededDeepCopy(
          BCSModelFaces, this->GetMasterPolyData()->GetFieldData()));
      BCSModelFaces->RemoveTuple(i);
      }
    }
  unsigned int blockIndex = this->GetModelFaceBlockIndex(modelFaceId);
  this->PerformNeededShallowCopy(this->GetModelFaceWithIndex(blockIndex),
    blockIndex, this->GetModelFaceRootBlock());
  vtkDataObject* data = this->GetModelFaceWithId(modelFaceId);
  vtkIntArray* modelFaceBCSs = vtkIntArray::SafeDownCast(
    data->GetFieldData()->GetArray(ModelFaceDataString));
  for(i=modelFaceBCSs->GetNumberOfTuples()-1;i>2;i--)
    {
    if(BCSId == modelFaceBCSs->GetValue(i))
      {
      modelFaceBCSs = vtkIntArray::SafeDownCast(
        this->PerformNeededDeepCopy(modelFaceBCSs, data->GetFieldData()));
      modelFaceBCSs->RemoveTuple(i);
      }
    }
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::RemoveModelFaceFromBCSs(int modelFaceId)
{
  if(!mb)
    {
    vtkWarningMacro("No vtkMultiBlockDataSet set.");
    return;
    }
  vtkIdList* bcsIds = vtkIdList::New();
  this->GetModelFaceBCSIds(modelFaceId, bcsIds);
  for(int i=0; i<bcsIds->GetNumberOfIds(); i++)
    {
    this->RemoveBCSFromModelFace(modelFaceId, bcsIds->GetId(i));
    }
  bcsIds->Delete();
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::RemoveModelFaceDataInfo(int modelFaceId)
{
  if(!mb)
    {
    vtkWarningMacro("No vtkMultiBlockDataSet set.");
    return;
    }

  vtkPolyData* poly = vtkPolyData::SafeDownCast(
    this->PerformNeededShallowCopy(this->GetMasterPolyData(),
    MasterPolyRootIndex, this->mb));
  vtkFieldData* field = poly->GetFieldData();

  vtkIntArray* MFData = vtkIntArray::SafeDownCast(
    this->PerformNeededDeepCopy(
    field->GetArray(ModelFaceConvenientArrayName), field));
  int vals[3];
  if(MFData)
    {
    for(int i=0; i<MFData->GetNumberOfTuples(); i++)
      {
      MFData->GetTypedTuple(i, vals);
      if(vals[modelFaceIdArrayLoc] == modelFaceId)
        {
        vals[modelFaceIdArrayLoc] = -1;
        MFData->SetTypedTuple(i, vals);
        //MFData->RemoveTuple(i);
        break;
        }
      }
    }

  vtkFloatArray* rgba = vtkFloatArray::SafeDownCast(
    this->PerformNeededDeepCopy(field->GetArray(ModelFaceColorsString), field));
  this->MarkObjectAsRemoved(rgba, modelFaceId);
}

//-----------------------------------------------------------------------------
int vtkMultiBlockWrapper::AddShell(const char* ShellUserName, float* RGBA,
                                   int materialId, double* translationPoint)
{
  if(!this->mb)
    {
    vtkWarningMacro("No vtkMultiBlockDataSet set.");
    return -1;
    }
  int ShellId = this->GetNextId(ShellColorsString);

  // need some type of copy here
  this->PerformNeededShallowCopy(this->GetMasterPolyData(),
    MasterPolyRootIndex, this->mb);

  vtkFieldData* fieldData = this->GetMasterPolyData()->GetFieldData();

  vtkFloatArray* colorArray =
    vtkFloatArray::SafeDownCast(this->PerformNeededDeepCopy(
                                  fieldData->GetArray(ShellColorsString),
                                  fieldData));
  vtkStringArray* nameArray =
    vtkStringArray::SafeDownCast(this->PerformNeededDeepCopy(
                                   fieldData->GetAbstractArray(ShellUserNamesString),
                                   fieldData));

  vtkIntArray* shellMaterialArray =
    vtkIntArray::SafeDownCast(this->PerformNeededDeepCopy(
                                fieldData->GetArray(ShellMaterialIdString),
                                fieldData));

  vtkDoubleArray* translationPointArray =
    vtkDoubleArray::SafeDownCast(this->PerformNeededDeepCopy(
                                   fieldData->GetArray(ShellTranslationPointString),
                                   fieldData));
  if(!colorArray)
    {
    // assume that all of the arrays need to be created
    colorArray = vtkFloatArray::New();
    colorArray->SetNumberOfComponents(4);
    colorArray->SetName(ShellColorsString);
    fieldData->AddArray(colorArray);
    colorArray->Delete();
    nameArray = vtkStringArray::New();
    nameArray->SetName(ShellUserNamesString);
    fieldData->AddArray(nameArray);
    nameArray->Delete();
    shellMaterialArray = vtkIntArray::New();
    shellMaterialArray->SetNumberOfComponents(1);
    shellMaterialArray->SetName(ShellMaterialIdString);
    fieldData->AddArray(shellMaterialArray);
    shellMaterialArray->Delete();
    translationPointArray = vtkDoubleArray::New();
    translationPointArray->SetNumberOfComponents(4); // last one is a marker
    translationPointArray->SetName(ShellTranslationPointString);
    fieldData->AddArray(translationPointArray);
    translationPointArray->Delete();
    }
  if(colorArray->GetNumberOfTuples() == ShellId)
    {
    colorArray->InsertNextTuple(RGBA);
    nameArray->InsertNextValue(ShellUserName);
    shellMaterialArray->InsertNextValue(materialId);
    double tpoint[4] = {-1, -1, -1, -1};
    if(translationPoint)
      {
      tpoint[0] = translationPoint[0];
      tpoint[1] = translationPoint[1];
      tpoint[2] = translationPoint[2];
      tpoint[3] = 1;
      }
    translationPointArray->InsertNextTypedTuple(tpoint);
    }
  else
    {
    colorArray->InsertTuple(ShellId, RGBA);
    nameArray->SetValue(ShellId, ShellUserName);
    shellMaterialArray->SetValue(ShellId, materialId);
    double tpoint[4] = {-1, -1, -1, -1};
    if(translationPoint)
      {
      tpoint[0] = translationPoint[0];
      tpoint[1] = translationPoint[1];
      tpoint[2] = translationPoint[2];
      tpoint[3] = 1;
      }
    translationPointArray->SetTypedTuple(ShellId, tpoint);
    }

  return ShellId;
}

//-----------------------------------------------------------------------------
int vtkMultiBlockWrapper::GetModelFaceId(int cellId)
{
  vtkCellData* cellData = this->GetMasterPolyData()->GetCellData();
  vtkIntArray* array = vtkIntArray::SafeDownCast(
    cellData->GetArray(ModelFaceIdsString));
  return array->GetValue(cellId);
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::SetModelFaceId(int cellId, int modelFaceId)
{
  vtkCellData* cellData = this->GetMasterPolyData()->GetCellData();
  vtkIntArray* array = vtkIntArray::SafeDownCast(
    cellData->GetArray(ModelFaceIdsString));
  array = vtkIntArray::SafeDownCast(
    this->PerformNeededDeepCopy(array, cellData));
  array->SetValue(cellId, modelFaceId);
}

//-----------------------------------------------------------------------------
int vtkMultiBlockWrapper::GetModelFaceCellId(int cellId)
{
  vtkCellData* cellData = this->GetMasterPolyData()->GetCellData();
  vtkIntArray* array = vtkIntArray::SafeDownCast(
    cellData->GetArray(ModelFaceCellIdsString));
  return array->GetValue(cellId);
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::SetModelFaceCellId(int cellId, int modelFaceCellId)
{
  vtkCellData* cellData = this->GetMasterPolyData()->GetCellData();
  vtkIntArray* array = vtkIntArray::SafeDownCast(
    cellData->GetArray(ModelFaceCellIdsString));
  array = vtkIntArray::SafeDownCast(
    this->PerformNeededDeepCopy(array, cellData));
  array->SetValue(cellId, modelFaceCellId);
}

//-----------------------------------------------------------------------------
vtkAbstractArray* vtkMultiBlockWrapper::PerformNeededDeepCopy(
  vtkAbstractArray* array, vtkFieldData* owner)
{
  if(!array)
    {
    return array;
    }
  if(array->GetReferenceCount() != 1)
    {
    // deep copy and replace...
    vtkAbstractArray* copy = vtkAbstractArray::SafeDownCast(
      vtkInstantiator::CreateInstance(array->GetClassName()));
    copy->DeepCopy(array);
    copy->SetName(array->GetName()); // this may not be necessary
    owner->AddArray(copy);
    array = copy;
    copy->Delete();
    }
  return array;
}

//-----------------------------------------------------------------------------
vtkDataObject* vtkMultiBlockWrapper::PerformNeededShallowCopy(
  vtkDataObject* data, unsigned int blockIndex,
  vtkMultiBlockDataSet* mbDataSet)
{
  // debug to make sure we mean blockIndex and not model face id
  if(mbDataSet && mbDataSet->GetBlock(blockIndex) != data)
    {
    vtkErrorMacro("Wrong block index for vtkDataObject");
    return 0;
    }

  if(data->GetReferenceCount() != 1)
    {
    // shallow copy and replace...
    vtkDataObject* copy = vtkDataObject::SafeDownCast(
      vtkInstantiator::CreateInstance(data->GetClassName()));
    copy->ShallowCopy(data);
    mbDataSet->SetBlock(blockIndex, copy);
    data = copy;
    copy->Delete();
    }
  return data;
}

//-----------------------------------------------------------------------------
vtkDataObject* vtkMultiBlockWrapper::PerformNeededDeepCopy(
  vtkDataObject* data, unsigned int blockIndex,
  vtkMultiBlockDataSet* mbDataSet)
{
  // debug to make sure we mean blockIndex and not model face id
  if(mbDataSet && mbDataSet->GetBlock(blockIndex) != data)
    {
    vtkErrorMacro("Wrong block index for vtkDataObject");
    return 0;
    }

  vtkPolyData* polyCopy = vtkPolyData::SafeDownCast(data);
  // we do a deep copy if the polydata or any of its cell arrays (verts, lines or polys)
  // has a reference count greater than 1!
  if(data->GetReferenceCount() != 1 ||
     (polyCopy && ((polyCopy->GetVerts() && polyCopy->GetVerts()->GetReferenceCount() != 1) ||
       (polyCopy->GetPolys() && polyCopy->GetPolys()->GetReferenceCount() != 1) ||
                   (polyCopy->GetLines() && polyCopy->GetLines()->GetReferenceCount() != 1) ) ) )
    {
    vtkDataObject* copy = vtkDataObject::SafeDownCast(
      vtkInstantiator::CreateInstance(data->GetClassName()));
    /*
    // if this is a polydata, do not copy pointdata
    // Still has some problems with MergeFacesFilter test
    if(polyCopy)
      {
      vtkPolyData* targetCopy = vtkPolyData::SafeDownCast(copy);
      targetCopy->CopyStructure(polyCopy);
      targetCopy->GetPointData()->PassData(polyCopy->GetPointData());
      targetCopy->GetCellData()->DeepCopy(polyCopy->GetCellData());
      targetCopy->GetFieldData()->DeepCopy(polyCopy->GetFieldData());
      targetCopy->BuildLinks();
      }
    else
      {
      copy->DeepCopy(data);
      }
    */
    // since the points shouldn't change, we go back and do a shallow copy on them
    // from the original data to save memory.  it is inefficient but allows me to
    // avoid having to do a deep copy on the proper member data that I'm interested
    // in of the polydata object (I may mess that up!)
    copy->DeepCopy(data);
    vtkPolyData::SafeDownCast(copy)->SetPoints(vtkPolyData::SafeDownCast(data)->GetPoints());
    // WARNING: if data->GetReferenceCount() ==1 here, than data will be invalid
    // after SetBlock()!
    mbDataSet->SetBlock(blockIndex, copy);
    copy->Delete();
    return copy;
    }
  return data;
}

//-----------------------------------------------------------------------------
/*vtkFieldData* vtkMultiBlockWrapper::PerformNeededDeepCopy(vtkFieldData* field,
                                                             vtkDataObject* data)
{
  if(field->GetReferenceCount() != 1)
    {
    // deep copy and replace...
    vtkFieldData* copy = vtkFieldData::SafeDownCast(
      vtkInstantiator::CreateInstance(field->GetClassName()));
    copy->DeepCopy(field);
    data->SetFieldData(copy);
    field = copy;
    field->Delete();
    }
  return field;
}*/

//-----------------------------------------------------------------------------
int vtkMultiBlockWrapper::GetBCSIdFromName(const char* name)
{
  int id;
  sscanf(name, "%*s %d", &id);
  return id;
}

//-----------------------------------------------------------------------------
// This is a roundabout way to return the string name but this way we don't
// have to worry about allocation/deallocation of the string.
const char* vtkMultiBlockWrapper::GetBCSNameFromId(int id)
{
  char name[20];
  sprintf(name,"%s %d",BCSBaseNameString,id);
  vtkFieldData* fieldData = this->GetMasterPolyData()->GetFieldData();
  vtkDataArray* array = fieldData->GetArray(name);
  if(!array)
    {
    return 0;
    }
  return array->GetName();
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::CreateBCSNameFromId(int BCSId, char **name)
{
  *name = new char[20];
  sprintf(*name, "%s %d", BCSBaseNameString, BCSId);
  return;
}

/*
//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::SetFaceVisibility(
  int face, int visible)
{
  if(!this->mb)
    {
    vtkWarningMacro("No vtkMultiBlockDataSet set, or no requested block.");
    return;
    }
  int block = this->GetModelFaceBlockIndex(face);
  if(!this->mb->GetBlock(block))
    {
    vtkWarningMacro("No requested block.");
    return;
    }

  this->mb->GetMetaData(block)->Set(
    vtkMultiBlockWrapper::BlockVisibilityMetaKey(), visible);
}

//-----------------------------------------------------------------------------
int vtkMultiBlockWrapper::GetFaceVisibility(int face)
{
  if(!this->mb)
    {
    vtkWarningMacro("No requested block.");
    return 0;
    }

  int block = this->GetModelFaceBlockIndex(face);
  if(!this->mb->GetBlock(block))
    {
    vtkWarningMacro("No requested block.");
    return 0;
    }

  if(this->mb->HasMetaData(block) &&
    this->mb->GetMetaData(block)->Has(vtkMultiBlockWrapper::BlockVisibilityMetaKey()))
    {
    return this->mb->GetMetaData(block)->Get(
      vtkMultiBlockWrapper::BlockVisibilityMetaKey());
    }

  return 0;
}
*/

//-----------------------------------------------------------------------------
int vtkMultiBlockWrapper::CreateNodalGroup(vtkIntArray* ptIds,
  vtkIntArray* /*modelFaceIds*/, float* rgba, int isLoadingFile)
{
  if(ptIds->GetNumberOfTuples() == 0)
    {
    return -1;
    }
  if(!this->GetNodalGroupRootBlock())
    {
    vtkWarningMacro("No vtkMultiBlockDataSet set.");
    return -1;
    }

  // perform needed deep copies on main vtkPolyData
  this->PerformNeededShallowCopy(this->GetMasterPolyData(),
    MasterPolyRootIndex, this->mb);

  vtkMultiBlockDataSet* ngMBRoot = this->GetNodalGroupRootBlock();
  // first do shallow copy on nodal group multiblock
  this->PerformNeededShallowCopy(ngMBRoot, NodalGroupRootIndex, this->mb);

  ngMBRoot->SetNumberOfBlocks(ngMBRoot->GetNumberOfBlocks()+1);
  vtkIdType ngId = this->GetNextId(NodeGroupColorsString);

  vtkPolyData* poly = this->GetMasterPolyData();
  vtkIdType numPts =  ptIds->GetNumberOfTuples();

  vtkPolyData* ngPoly = vtkPolyData::New();
  vtkPoints *newPts = vtkPoints::New();
  newPts->SetDataTypeToDouble();
  newPts->Allocate(numPts);
  vtkCellArray *newVerts = vtkCellArray::New();
  newVerts->Allocate(numPts);
  vtkIntArray* ptIdArray = vtkIntArray::New();
  ptIdArray->SetNumberOfComponents(1);
  ptIdArray->SetNumberOfTuples(numPts);

  double pts[3];
  vtkIdType ptId;
  for (vtkIdType idx = 0;  idx < numPts; idx++)
    {
    ptId = ptIds->GetValue(idx);
    poly->GetPoint(ptId, pts);
    newPts->InsertNextPoint(pts);
    newVerts->InsertNextCell(1, &idx);
    ptIdArray->SetValue(idx, ptId);
    }

  ngPoly->SetPoints(newPts);
  newPts->Delete();
  ngPoly->SetVerts(newVerts);
  newVerts->Delete();

  ptIdArray->SetName(RCNameString);
  ngPoly->GetPointData()->AddArray(ptIdArray);
  ptIdArray->Delete();

  vtkFieldData* fieldData = poly->GetFieldData();

  // now add in rgba data
  vtkFloatArray* RGBA = vtkFloatArray::SafeDownCast(
    this->PerformNeededDeepCopy(
      fieldData->GetArray(NodeGroupColorsString), fieldData));

  if(!RGBA)
    {
    RGBA = vtkFloatArray::New();
    RGBA->SetNumberOfComponents(4);
    RGBA->SetName(NodeGroupColorsString);
    RGBA->SetNumberOfTuples(1);
    poly->GetFieldData()->AddArray(RGBA);
    RGBA->Delete();
    }
  vtkIdType oldSize = RGBA->GetNumberOfTuples();
  if(RGBA->GetNumberOfTuples() == ngId)
    {
    RGBA->InsertNextTypedTuple(rgba);
    }
  else if(RGBA->GetNumberOfTuples() < ngId)
    {
    while(RGBA->GetNumberOfTuples() < ngId)
      {
      float colors[4] = {-1,-1,-1,-2};
      RGBA->InsertNextTypedTuple(colors);
      // repeat work to mark this object as not existing
      // but then it is only done in one spot
      this->MarkObjectAsRemoved(RGBA, RGBA->GetNumberOfTuples()-1);
      }
    RGBA->InsertNextTypedTuple(rgba);
    }
  else
    {
    RGBA->SetTuple(ngId, rgba);
    }

  //  now add in default model face name
  char newName[20];
  sprintf(newName, "face %d", static_cast<int>(ngId));
  vtkStringArray* ngNames = vtkStringArray::SafeDownCast(
    this->GetMasterPolyData()->GetFieldData()->GetAbstractArray(ModelFaceUserNamesString));
  if(!ngNames)
    {
    ngNames = vtkStringArray::New();
    ngNames->SetNumberOfComponents(1);
    ngNames->SetName(NodeGroupUserNamesString);
    ngNames->SetNumberOfTuples(1);
    poly->GetFieldData()->AddArray(ngNames);
    ngNames->Delete();
    }
  oldSize = ngNames->GetNumberOfTuples();
  if(oldSize == ngId)
    {
    ngNames->InsertNextValue(newName);
    }
  else if(oldSize < ngId)
    {
    // insert the missing names with generic names
    while(ngNames->GetNumberOfTuples() <= ngId)
      {
      ngNames->InsertNextValue(newName);
      sprintf(newName, "face %d", static_cast<int>(ngNames->GetNumberOfTuples()));
      }
    }
  else if(!isLoadingFile)
    {
    // Should use the name in the file if we are loading from a CMB file.
    ngNames->SetValue(ngId, newName);
    }

  int blockIndex = this->GetNodalGroupRootBlock()->GetNumberOfBlocks()-1;
  this->GetModelFaceRootBlock()->SetBlock(blockIndex, ngPoly);
  ngPoly->Delete();

  return ngId;
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::CreateNodalGroups(
  vtkIntArray* /*markedPoints*/, vtkIntArray* /*NGIds*/)
{
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::DeleteNodalGroup(int /*ngId*/)
{
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::RemoveDeletedCellsFromNodalGroups()
{
}

//-----------------------------------------------------------------------------
int vtkMultiBlockWrapper::AddNodalBCS(
  int /*BCSUserId*/, const char* /*BCSUserName*/, float* /*colors*/, vtkIdList* /*ngIds*/)
{
  return -1;
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::AddBCSToNodalGroup(
  int /*NodalGroupId*/, const char* /*BCSName*/)
{
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::AddBCSToNodalGroup(int /*NodalGroupId*/, int /*BCSId*/)
{
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::RemoveBCSFromNodalGroup(int /*NodalGroupId*/,
                              const char* /*BCSName*/)
{
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::RemoveBCSFromNodalGroup(
  int /*NodalGroupId*/, int /*BCSSsytemId*/)
{
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::ChangeUserNameOfNodalBCS(
  int /*BCSId*/, const char* /*userName*/)
{
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::ChangeUserIdOfNodalBCS(
  int /*BCSId*/, int /*BCSUserId*/)
{
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::ChangeUserColorOfNodalBCS(
  int /*BCSId*/, float* /*userRGBA*/)
{
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::ChangeUserColorOfNodalGroup(
  int /*ngId*/, float* /*userRGBA*/)
{
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::ChangeUserNameOfNodalGroup(
  int /*ngId*/, const char* /*userName*/)
{
}

//-----------------------------------------------------------------------------
void vtkMultiBlockWrapper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "vtkMultiBlockDataSet: "
     << this->mb << "\n";
  os << indent << "needToRemoveDeletedCells: "
     << this->needToRemoveDeletedCells << "\n";
}

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

vtkMultiBlockWrapper::vtkMultiBlockWrapper()
{
  this->mb = 0;
  this->needToRemoveDeletedCells = false;
}

vtkMultiBlockWrapper::~vtkMultiBlockWrapper()
{
  this->mb = 0;
}

const char* vtkMultiBlockWrapper::GetModelFaceTagName()
{
  return ModelFaceIdsString;
}

const char* vtkMultiBlockWrapper::GetShellTagName()
{
  return ShellTagString;
}

const char* vtkMultiBlockWrapper::GetMaterialTagName()
{
  return MaterialTagString;
}

const char* vtkMultiBlockWrapper::GetReverseClassificationTagName()
{
  return RCNameString;
}

const char* vtkMultiBlockWrapper::GetSplitFacesTagName()
{
  return SplitFacesString;
}

const char* vtkMultiBlockWrapper::GetModelFaceDataName()
{
  return ModelFaceDataString;
}

const char* vtkMultiBlockWrapper::GetModelFaceConvenientArrayName()
{
  return ModelFaceConvenientArrayName;
}

const char* vtkMultiBlockWrapper::GetMaterialUserNamesString()
{
  return MaterialUserNamesString;
}

const char* vtkMultiBlockWrapper::GetShellUserNamesString()
{
  return ShellUserNamesString;
}

const char* vtkMultiBlockWrapper::GetShellTranslationPointString()
{
  return ShellTranslationPointString;
}

const char* vtkMultiBlockWrapper::GetModelFaceUserNamesString()
{
  return ModelFaceUserNamesString;
}

const char* vtkMultiBlockWrapper::GetBCSUserNamesString()
{
  return BCSUserNamesString;
}

const char* vtkMultiBlockWrapper::GetShellColorsString()
{
  return ShellColorsString;
}

const char* vtkMultiBlockWrapper::GetModelFaceColorsString()
{
  return ModelFaceColorsString;
}

const char* vtkMultiBlockWrapper::GetMaterialColorsString()
{
  return MaterialColorsString;
}

const char* vtkMultiBlockWrapper::GetBCSColorsString()
{
  return BCSColorsString;
}

const char* vtkMultiBlockWrapper::GetModelFaceUse1String()
{
  return "ModelFaceUse1";
}

void vtkMultiBlockWrapper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "vtkMultiBlockDataSet: " << this->mb << "\n";
  os << indent << "needToRemoveDeletedCells: " << this->needToRemoveDeletedCells << "\n";
}

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkSceneGenVegetationReader.h"

#include "vtkCellArray.h"
#include "vtkCharArray.h"
#include "vtkDoubleArray.h"
#include "vtkErrorCode.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkLongArray.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include "vtkStringArray.h"
#include "vtkTransform.h"

#include "vtkTransformPolyDataFilter.h"
#
#include "smtk/extension/vtk/reader/vtkCMBGeometryReader.h"
#include <sys/stat.h>
#include <sys/types.h>

vtkStandardNewMacro(vtkSceneGenVegetationReader);

struct ModelInstance
{
  std::string ID;
  double Scale;
  double ZRotation;
  double Translation[3];
};

//-----------------------------------------------------------------------------
vtkSceneGenVegetationReader::vtkSceneGenVegetationReader()
{
  this->FileName = 0;
  this->SetNumberOfInputPorts(0);
  this->MetWindHeight = -1;
  this->StartSimTime = -1;
  this->EndSimTime = -1;
  this->InputFluxFile = -1;
}

//-----------------------------------------------------------------------------
vtkSceneGenVegetationReader::~vtkSceneGenVegetationReader()
{
  this->SetFileName(0);
  this->ClearModel();
}

//-----------------------------------------------------------------------------
int vtkSceneGenVegetationReader::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inputVector), vtkInformationVector* outputVector)
{
  // get the info object
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  // get the ouptut
  vtkMultiBlockDataSet* output =
    vtkMultiBlockDataSet::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

  ifstream fin(this->FileName);
  if (!fin)
  {
    vtkErrorMacro(<< "File " << this->FileName << " not found");
    return 0;
  }

  char buffer[2048];
  this->ClearModel();

  std::map<std::string, VegetationModel>::iterator iterModel;

  std::string cmd, vegID;
  std::vector<ModelInstance> instances;
  while (!fin.eof())
  {
    fin >> cmd;

    if (cmd[0] == '#')
    {
      fin.getline(buffer, 2047); // finish reading the current line
    }

    if (cmd == "MODELS")
    {
      std::string fileName;
      int numModels;
      fin >> numModels;
      for (int i = 0; i < numModels; i++)
      {
        fin >> vegID >> fileName;
        iterModel = this->Models.find(vegID);
        if (iterModel != this->Models.end())
        {
          // leaf was specified 1st??? probably don't need to do this check!
          iterModel->second.FileName = fileName;
        }
        else
        {
          VegetationModel vegModel;
          vegModel.FileName = fileName;
          this->Models[vegID] = vegModel;
        }
      }
    }

    if (cmd == "LEAFSIZE" || cmd == "LEAF_SIZE")
    {
      double leafSize;
      int numLeafSizes;
      fin >> numLeafSizes;
      for (int i = 0; i < numLeafSizes; i++)
      {
        fin >> vegID >> leafSize;
        iterModel = this->Models.find(vegID);
        if (iterModel != this->Models.end())
        {
          iterModel->second.LeafSize = leafSize;
        }
        else
        {
          VegetationModel vegModel;
          vegModel.LeafSize = leafSize;
          this->Models[vegID] = vegModel;
        }
      }
    }

    if (cmd == "MATERIALCODE" || cmd == "MATERIAL_CODE")
    {
      long material;
      int numMaterials;
      fin >> numMaterials;
      for (int i = 0; i < numMaterials; i++)
      {
        fin >> vegID >> material;
        iterModel = this->Models.find(vegID);
        if (iterModel != this->Models.end())
        {
          iterModel->second.Material = material;
        }
        else
        {
          VegetationModel vegModel;
          vegModel.Material = material;
          this->Models[vegID] = vegModel;
        }
      }
    }

    if (cmd == "NODEFILE")
    {
      fin >> this->NodeFile;
    }

    if (cmd == "ENSIGHT_NODEFILE")
    {
      fin >> this->EnsightNodeFile;
    }

    if (cmd == "ENSIGHT_STOMATAL")
    {
      std::string name;
      this->EnsightStomatal.push_back(name);
    }

    if (cmd == "MET_FILE")
    {
      fin >> this->MetFile;
    }

    if (cmd == "START_SIM_TIME")
    {
      fin >> this->StartSimTime;
    }

    if (cmd == "END_SIM_TIME")
    {
      fin >> this->EndSimTime;
    }

    if (cmd == "MET_WIND_HEIGHT")
    {
      fin >> this->MetWindHeight;
    }

    if (cmd == "OUTPUT_MESH")
    {
      fin >> this->OutputMesh;
    }

    if (cmd == "ENSIGHT_OUTPUT_MESH")
    {
      fin >> this->EnsightOutputMesh;
    }

    if (cmd == "INPUT_FLUX_FILE")
    {
      fin >> this->InputFluxFile;
    }

    if (cmd == "INSTANCE")
    {
      int numInstances;
      fin >> numInstances;

      for (int i = 0; i < numInstances; i++)
      {
        ModelInstance instance;
        fin >> instance.ID >> instance.Scale >> instance.ZRotation >> instance.Translation[0] >>
          instance.Translation[1] >> instance.Translation[2];
        instances.push_back(instance);
      }
    }
    cmd.clear();
  }
  fin.close();

  // add field data that applies to all blocks
  if (this->NodeFile.size())
  {
    vtkSmartPointer<vtkStringArray> nodefileFD = vtkStringArray::New();
    nodefileFD->SetName("NodeFile");
    nodefileFD->InsertNextValue(this->NodeFile);
    output->GetFieldData()->AddArray(nodefileFD);
  }
  if (this->EnsightNodeFile.size())
  {
    vtkSmartPointer<vtkStringArray> nodefileFD = vtkStringArray::New();
    nodefileFD->SetName("EnsightNodeFile");
    nodefileFD->InsertNextValue(this->EnsightNodeFile);
    output->GetFieldData()->AddArray(nodefileFD);
  }
  if (this->MetFile.size())
  {
    vtkSmartPointer<vtkStringArray> metFileFD = vtkStringArray::New();
    metFileFD->SetName("MetFile");
    metFileFD->InsertNextValue(this->MetFile);
    output->GetFieldData()->AddArray(metFileFD);
  }
  if (this->OutputMesh.size())
  {
    vtkSmartPointer<vtkStringArray> meshFD = vtkStringArray::New();
    meshFD->SetName("OutputMesh");
    meshFD->InsertNextValue(this->OutputMesh);
    output->GetFieldData()->AddArray(meshFD);
  }
  if (this->EnsightOutputMesh.size())
  {
    vtkSmartPointer<vtkStringArray> meshFD = vtkStringArray::New();
    meshFD->SetName("EnsightOutputMesh");
    meshFD->InsertNextValue(this->EnsightOutputMesh);
    output->GetFieldData()->AddArray(meshFD);
  }
  if (this->MetWindHeight >= 0)
  {
    vtkSmartPointer<vtkDoubleArray> metWindHeightFD = vtkSmartPointer<vtkDoubleArray>::New();
    metWindHeightFD->SetName("MetWindHeight");
    metWindHeightFD->InsertNextValue(this->MetWindHeight);
    output->GetFieldData()->AddArray(metWindHeightFD);
  }
  if (this->StartSimTime >= 0)
  {
    vtkSmartPointer<vtkLongArray> startSimTimeFD = vtkSmartPointer<vtkLongArray>::New();
    startSimTimeFD->SetName("StartSimTime");
    startSimTimeFD->InsertNextValue(this->StartSimTime);
    output->GetFieldData()->AddArray(startSimTimeFD);
  }
  if (this->EndSimTime >= 0)
  {
    vtkSmartPointer<vtkLongArray> endSimTimeFD = vtkSmartPointer<vtkLongArray>::New();
    endSimTimeFD->SetName("EndSimTime");
    endSimTimeFD->InsertNextValue(this->EndSimTime);
    output->GetFieldData()->AddArray(endSimTimeFD);
  }
  if (this->InputFluxFile >= 0)
  {
    vtkSmartPointer<vtkLongArray> inputFluxFileFD = vtkSmartPointer<vtkLongArray>::New();
    inputFluxFileFD->SetName("InputFluxFile");
    inputFluxFileFD->InsertNextValue(this->InputFluxFile);
    output->GetFieldData()->AddArray(inputFluxFileFD);
  }
  if (this->EnsightStomatal.size())
  {
    // I've had some iffy expereiences with vtkStringArray when more than one
    // string; thus using vtkCharArray, which is less elegant
    vtkSmartPointer<vtkCharArray> ensightStomatalFD = vtkSmartPointer<vtkCharArray>::New();
    ensightStomatalFD->SetName("ensightStomatalFD");
    size_t maxSize = 0;
    for (size_t i = 0; i < this->EnsightStomatal.size(); i++)
    {
      if (this->EnsightStomatal[i].size() > maxSize)
      {
        maxSize = this->EnsightStomatal[i].size();
      }
    }
    ensightStomatalFD->SetNumberOfComponents(static_cast<int>(maxSize + 1));
    ensightStomatalFD->SetNumberOfTuples(this->EnsightStomatal.size());
    for (int i = 0; i < static_cast<int>(this->EnsightStomatal.size()); i++)
    {
      ensightStomatalFD->SetTypedTuple(i, this->EnsightStomatal[i].c_str());
    }
    output->GetFieldData()->AddArray(ensightStomatalFD);
  }

  // now add a block for each instance
  double color[3] = { 0.2, 0.4, 0.2 };
  for (std::vector<ModelInstance>::iterator i = instances.begin(); i != instances.end(); i++)
  {
    iterModel = this->Models.find(i->ID);
    if (iterModel == this->Models.end())
    {
      vtkErrorMacro("Veg instance not found: " << i->ID);
      continue;
    }
    this->AddBlock(output, iterModel->second, i->Scale, i->ZRotation, i->Translation, color);
  }

  return 1;
}

//-----------------------------------------------------------------------------
int vtkSceneGenVegetationReader::AddBlock(vtkMultiBlockDataSet* output, VegetationModel& model,
  double scale, double zRotation, double translation[3], double color[3])
{
  vtkPolyData* block = vtkPolyData::New();
  if (!model.Dataset)
  {
    vtkSmartPointer<vtkCMBGeometryReader> reader = vtkSmartPointer<vtkCMBGeometryReader>::New();
    reader->SetFileName(model.FileName.c_str());
    reader->Update();
    model.Dataset = vtkPolyData::New();
    if (reader->GetErrorCode() == vtkErrorCode::NoError)
    {
      model.Dataset->ShallowCopy(reader->GetOutput());
    }
  }
  // may resuse the geometry, but the field data (transformation) will be different
  block->ShallowCopy(model.Dataset);
  output->SetBlock(output->GetNumberOfBlocks(), block);

  vtkSmartPointer<vtkDoubleArray> translationFD = vtkSmartPointer<vtkDoubleArray>::New();
  translationFD->SetName("Translation");
  translationFD->SetNumberOfComponents(3);
  translationFD->InsertNextTuple(translation);
  block->GetFieldData()->AddArray(translationFD);

  vtkSmartPointer<vtkDoubleArray> rotationFD = vtkSmartPointer<vtkDoubleArray>::New();
  rotationFD->SetName("Rotation");
  rotationFD->SetNumberOfComponents(3);
  rotationFD->InsertNextTuple3(90, 0, 360 - zRotation);
  block->GetFieldData()->AddArray(rotationFD);

  //orientoccluder[ito].RotationEuler(0.0, 0.0, 90.0);  //plants are sideways

  vtkSmartPointer<vtkDoubleArray> scaleFD = vtkSmartPointer<vtkDoubleArray>::New();
  scaleFD->SetName("Scale");
  scaleFD->SetNumberOfComponents(1);
  scaleFD->InsertNextValue(scale);
  block->GetFieldData()->AddArray(scaleFD);

  vtkSmartPointer<vtkDoubleArray> colorFD = vtkSmartPointer<vtkDoubleArray>::New();
  colorFD->SetName("Color");
  colorFD->SetNumberOfComponents(3);
  colorFD->InsertNextTuple(color);
  block->GetFieldData()->AddArray(colorFD);

  // now combine the components into a single Transformation
  vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
  transform->PreMultiply();
  transform->Translate(translation);
  transform->RotateZ(zRotation);
  transform->RotateX(90);
  transform->Scale(scale, scale, scale);

  vtkSmartPointer<vtkDoubleArray> transformFD = vtkSmartPointer<vtkDoubleArray>::New();
  transformFD->SetName("Transformation");
  transformFD->SetNumberOfComponents(16);
  double transformData[16];
  vtkMatrix4x4::DeepCopy(transformData, transform->GetMatrix());
  transformFD->InsertNextTuple(transformData);
  block->GetFieldData()->AddArray(transformFD);

  return VTK_OK;
}

//-----------------------------------------------------------------------------
void vtkSceneGenVegetationReader::ClearModel()
{
  for (std::map<std::string, VegetationModel>::iterator i = this->Models.begin();
       i != this->Models.end(); i++)
  {
    if (i->second.Dataset)
    {
      i->second.Dataset->Delete();
      i->second.Dataset = 0;
    }
  }
  this->Models.clear();
  this->EnsightStomatal.clear();
}

//-----------------------------------------------------------------------------
void vtkSceneGenVegetationReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "File Name: " << (this->FileName ? this->FileName : "(none)") << "\n";
}

//----------------------------------------------------------------------------
int vtkSceneGenVegetationReader::RequestInformation(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inputVector), vtkInformationVector* vtkNotUsed(outputVector))
{
  if (!this->FileName)
  {
    vtkErrorMacro("FileName has to be specified!");
    return 0;
  }

  return 1;
}

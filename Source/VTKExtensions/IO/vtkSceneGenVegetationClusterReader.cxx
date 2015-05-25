//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkSceneGenVegetationClusterReader.h"

#include "vtkCellArray.h"
#include "vtkDoubleArray.h"
#include "vtkLongArray.h"
#include "vtkStringArray.h"
#include "vtkGlyph3D.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkNew.h"
#include "vtkPolyData.h"
#include "vtkCharArray.h"
#include "vtkCMBGeometryReader.h"
#include "vtkErrorCode.h"
#include "vtkStripper.h"
#include "vtkTransform.h"
#include "vtkTransformPolyDataFilter.h"
#
#include <sys/types.h>
#include <sys/stat.h>


vtkStandardNewMacro(vtkSceneGenVegetationClusterReader);

struct ModelInstance
  {
  std::string ID;
  double Scale;
  double ZRotation;
  double Translation[3];
  };

//-----------------------------------------------------------------------------
vtkSceneGenVegetationClusterReader::vtkSceneGenVegetationClusterReader()
{
  this->FileName = 0;
  this->SetNumberOfInputPorts(0);
  this->MetWindHeight = -1;
  this->StartSimTime = -1;
  this->EndSimTime = -1;
  this->InputFluxFile = -1;

}

//-----------------------------------------------------------------------------
vtkSceneGenVegetationClusterReader::~vtkSceneGenVegetationClusterReader()
{
  this->SetFileName(0);
  this->ClearModel();
}

//-----------------------------------------------------------------------------
int vtkSceneGenVegetationClusterReader::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{
  // get the info object
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the ouptut
  vtkMultiBlockDataSet *output = vtkMultiBlockDataSet::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  ifstream fin(this->FileName);
  if(!fin)
    {
    vtkErrorMacro(<< "File " << this->FileName << " not found");
    return 0;
    }

  char buffer[2048];
  this->ClearModel();

  std::map< std::string, VegetationModel>::iterator iterModel;

  std::string cmd, vegID;
  std::vector< ModelInstance > instances;
  while(!fin.eof())
    {
    fin >> cmd;

    if (cmd[0]=='#')
      {
      fin.getline(buffer,2047);  // finish reading the current line
      }

    if (cmd == "MODELS")
      {
      std::string fileName;
      int numModels;
      fin >> numModels;
      for (int i = 0; i < numModels; i++)
        {
        fin >> vegID >> fileName;
        iterModel = this->Models.find( vegID );
        if (iterModel != this->Models.end())
          {
          // leaf was specified 1st??? probably don't need to do this check!
          iterModel->second.FileName = fileName;
          }
        else
          {
          VegetationModel vegModel;
          vegModel.FileName = fileName;
          this->Models[ vegID ] = vegModel;
          }
        }
      }

    if (cmd=="LEAFSIZE" || cmd=="LEAF_SIZE")
      {
      double leafSize;
      int numLeafSizes;
      fin >> numLeafSizes;
      for (int i = 0; i < numLeafSizes; i++)
        {
        fin >> vegID >> leafSize;
        iterModel = this->Models.find( vegID );
        if (iterModel != this->Models.end())
          {
          iterModel->second.LeafSize = leafSize;
          }
        else
          {
          VegetationModel vegModel;
          vegModel.LeafSize = leafSize;
          this->Models[ vegID ] = vegModel;
          }
        }
      }

    if (cmd=="MATERIALCODE" || cmd=="MATERIAL_CODE")
      {
      long material;
      int numMaterials;
      fin >> numMaterials;
      for (int i = 0; i < numMaterials; i++)
        {
        fin >> vegID >> material;
        iterModel = this->Models.find( vegID );
        if (iterModel != this->Models.end())
          {
          iterModel->second.Material = material;
          }
        else
          {
          VegetationModel vegModel;
          vegModel.Material = material;
          this->Models[ vegID ] = vegModel;
          }
        }
      }

    if (cmd=="NODEFILE")
      {
      fin >> this->NodeFile;
      }

    if (cmd=="ENSIGHT_NODEFILE")
      {
      fin >> this->EnsightNodeFile;
      }

    if (cmd=="ENSIGHT_STOMATAL")
      {
      std::string name;
      this->EnsightStomatal.push_back(name);
      }

    if (cmd=="MET_FILE")
      {
      fin >> this->MetFile;
      }

    if (cmd=="START_SIM_TIME")
      {
      fin >> this->StartSimTime;
      }

    if (cmd=="END_SIM_TIME")
      {
      fin >> this->EndSimTime;
      }

    if (cmd=="MET_WIND_HEIGHT")
      {
      fin >> this->MetWindHeight;
      }

    if (cmd=="OUTPUT_MESH")
      {
      fin >> this->OutputMesh;
      }

    if (cmd=="ENSIGHT_OUTPUT_MESH")
      {
      fin >> this->EnsightOutputMesh;
      }

    if (cmd=="INPUT_FLUX_FILE")
      {
      fin >> this->InputFluxFile;
      }

    if (cmd=="INSTANCE")
      {
      int numInstances;
      fin >> numInstances;

      for (int i = 0; i < numInstances; i++)
        {
        ModelInstance instance;
        fin >> instance.ID >> instance.Scale >> instance.ZRotation >>
          instance.Translation[0] >> instance.Translation[1] >>
          instance.Translation[2];
        instances.push_back( instance );
        }
      }
    cmd.clear();
    }
  fin.close();


  // add field data that applies to all blocks
  if (this->NodeFile.size())
    {
    vtkNew<vtkStringArray> nodefileFD;
    nodefileFD->SetName("NodeFile");
    nodefileFD->InsertNextValue(this->NodeFile);
    output->GetFieldData()->AddArray( nodefileFD.GetPointer() );
    }
  if (this->EnsightNodeFile.size())
    {
    vtkNew<vtkStringArray> nodefileFD;
    nodefileFD->SetName("EnsightNodeFile");
    nodefileFD->InsertNextValue(this->EnsightNodeFile);
    output->GetFieldData()->AddArray( nodefileFD.GetPointer() );
    }
  if (this->MetFile.size())
    {
    vtkNew<vtkStringArray> metFileFD;
    metFileFD->SetName("MetFile");
    metFileFD->InsertNextValue(this->MetFile);
    output->GetFieldData()->AddArray( metFileFD.GetPointer() );
    }
  if (this->OutputMesh.size())
    {
    vtkNew<vtkStringArray> meshFD;
    meshFD->SetName("OutputMesh");
    meshFD->InsertNextValue(this->OutputMesh);
    output->GetFieldData()->AddArray( meshFD.GetPointer() );
    }
  if (this->EnsightOutputMesh.size())
    {
    vtkNew<vtkStringArray> meshFD;
    meshFD->SetName("EnsightOutputMesh");
    meshFD->InsertNextValue(this->EnsightOutputMesh);
    output->GetFieldData()->AddArray( meshFD.GetPointer() );
    }
  if (this->MetWindHeight >= 0)
    {
    vtkNew<vtkDoubleArray> metWindHeightFD;
    metWindHeightFD->SetName("MetWindHeight");
    metWindHeightFD->InsertNextValue( this->MetWindHeight );
    output->GetFieldData()->AddArray( metWindHeightFD.GetPointer() );
    }
  if (this->StartSimTime >= 0)
    {
    vtkNew<vtkLongArray> startSimTimeFD;
    startSimTimeFD->SetName("StartSimTime");
    startSimTimeFD->InsertNextValue( this->StartSimTime );
    output->GetFieldData()->AddArray( startSimTimeFD.GetPointer() );
    }
  if (this->EndSimTime >= 0)
    {
    vtkNew<vtkLongArray> endSimTimeFD;
    endSimTimeFD->SetName("EndSimTime");
    endSimTimeFD->InsertNextValue( this->EndSimTime );
    output->GetFieldData()->AddArray( endSimTimeFD.GetPointer() );
    }
  if (this->InputFluxFile >= 0)
    {
    vtkNew<vtkLongArray> inputFluxFileFD;
    inputFluxFileFD->SetName("InputFluxFile");
    inputFluxFileFD->InsertNextValue( this->InputFluxFile );
    output->GetFieldData()->AddArray( inputFluxFileFD.GetPointer() );
    }
  if (this->EnsightStomatal.size())
    {
    // I've had some iffy expereiences with vtkStringArray when more than one
    // string; thus using vtkCharArray, which is less elegant
    vtkNew<vtkCharArray> ensightStomatalFD;
    ensightStomatalFD->SetName("ensightStomatalFD");
    unsigned int maxSize = 0;
    for (size_t i = 0; i < this->EnsightStomatal.size(); i++)
      {
      if (this->EnsightStomatal[i].size() > maxSize)
        {
        maxSize = this->EnsightStomatal[i].size();
        }
      }
    ensightStomatalFD->SetNumberOfComponents( maxSize + 1 );
    ensightStomatalFD->SetNumberOfTuples( this->EnsightStomatal.size() );
    for (int i = 0; i < static_cast<int>(this->EnsightStomatal.size()); i++)
      {
      ensightStomatalFD->SetTupleValue(i, this->EnsightStomatal[i].c_str());
      }
    output->GetFieldData()->AddArray( ensightStomatalFD.GetPointer() );
    }

  // now add a block for each instance
  double color[3] = { 0.2, 0.4, 0.2 };
  double pnt[3];
  for(std::vector<ModelInstance>::iterator i = instances.begin();
      i != instances.end(); i++)
    {
    iterModel = this->Models.find( i->ID );
    if (iterModel == this->Models.end())
      {
      vtkErrorMacro("Veg instance not found: " << i->ID);
      continue;
      }
    // Is the model loaded?
    if (!iterModel->second.Dataset)
      {
      vtkNew<vtkCMBGeometryReader> reader;
      reader->SetFileName( iterModel->second.FileName.c_str() );
      reader->Update();
      iterModel->second.Dataset = vtkPolyData::New();
      if (reader->GetErrorCode() == vtkErrorCode::NoError)
        {
        iterModel->second.Dataset->ShallowCopy( reader->GetOutput() );
        }
      iterModel->second.Plants = vtkPolyData::New();
      vtkPoints *points = vtkPoints::New();
      points->SetDataTypeToDouble();
      iterModel->second.Plants->SetPoints(points);
      points->Delete();
      }
    iterModel->second.Scale += i->Scale;
    pnt[0] = i->Translation[0];
    pnt[1] = i->Translation[1];
    pnt[2] = i->Translation[2];
    iterModel->second.Plants->GetPoints()->InsertNextPoint(pnt);
    }

  for(std::map< std::string, VegetationModel>::iterator iter =
        this->Models.begin(); iter != this->Models.end(); iter++)
    {
    if (iter->second.Dataset)
      {
      this->AddBlock(output, iter->second, color);
      }
    }
  return 1;
}


//-----------------------------------------------------------------------------
int vtkSceneGenVegetationClusterReader::AddBlock(vtkMultiBlockDataSet *output,
                                          VegetationModel &model, double color[3])
{
  // create the transform of the plant - note we need to do an average for the scale
  double scale = model.Scale / static_cast<double>(model.Plants->GetPoints()->GetNumberOfPoints());
  vtkNew<vtkTransform> transform;
  transform->PreMultiply();
  transform->RotateX( 90 );
  transform->Scale( scale, scale, scale );
  vtkNew<vtkTransformPolyDataFilter> trans;
  trans->SetInputData(model.Dataset);
  trans->SetTransform(transform.GetPointer());

  // Create TriStrips of the plant
  vtkNew<vtkStripper> stripper;
  stripper->SetInputConnection(trans->GetOutputPort());
  // Lets create a glyph filter for the instances (of the same geometry)
  vtkNew<vtkGlyph3D> glyph;
  glyph->SetInputData(model.Plants);
  glyph->SetSourceConnection(stripper->GetOutputPort());
  glyph->Update();
  vtkNew<vtkPolyData> block;
  // may resuse the geometry, but the field data (transformation) will be different
  block->ShallowCopy( glyph->GetOutput() );
  output->SetBlock(output->GetNumberOfBlocks(), block.GetPointer());
  double dummy[3];
  dummy[0] = dummy[1] = dummy[2] = 0.0;
  vtkNew<vtkDoubleArray> translationFD;
  translationFD->SetName( "Translation" );
  translationFD->SetNumberOfComponents(3);
  translationFD->InsertNextTuple( dummy );
  block->GetFieldData()->AddArray( translationFD.GetPointer() );

  vtkNew<vtkDoubleArray> rotationFD;
  rotationFD->SetName( "Rotation" );
  rotationFD->SetNumberOfComponents(3);
  rotationFD->InsertNextTuple3( 0, 0, 0 );
  block->GetFieldData()->AddArray( rotationFD.GetPointer() );

  //orientoccluder[ito].RotationEuler(0.0, 0.0, 90.0);  //plants are sideways
  vtkNew<vtkDoubleArray> scaleFD;
  scaleFD->SetName( "Scale" );
  scaleFD->SetNumberOfComponents(1);
  scaleFD->InsertNextValue( 1.0 );
  block->GetFieldData()->AddArray( scaleFD.GetPointer() );

  vtkNew<vtkDoubleArray> colorFD;
  colorFD->SetName( "Color" );
  colorFD->SetNumberOfComponents(3);
  colorFD->InsertNextTuple( color );
  block->GetFieldData()->AddArray( colorFD.GetPointer() );

  vtkNew<vtkDoubleArray> transformFD;
  transformFD->SetName( "Transformation" );
  transformFD->SetNumberOfComponents(16);
  transform->Identity();
  transformFD->InsertNextTuple( transform->GetMatrix()[0][0] );
  block->GetFieldData()->AddArray( transformFD.GetPointer() );

  return VTK_OK;
}


//-----------------------------------------------------------------------------
void vtkSceneGenVegetationClusterReader::ClearModel()
{
  for(std::map< std::string, VegetationModel>::iterator i =
    this->Models.begin(); i != this->Models.end(); i++)
    {
    if (i->second.Dataset)
      {
      i->second.Dataset->Delete();
      i->second.Dataset = 0;
      }
    if (i->second.Plants)
      {
      i->second.Plants->Delete();
      i->second.Plants = 0;
      }
    }
  this->Models.clear();
  this->EnsightStomatal.clear();
}



//-----------------------------------------------------------------------------
void vtkSceneGenVegetationClusterReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "File Name: "
     << (this->FileName ? this->FileName : "(none)") << "\n";
}


//----------------------------------------------------------------------------
int vtkSceneGenVegetationClusterReader::RequestInformation(
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


/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkOmicronModelInputReader.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkOmicronModelInputReader.h"

#include "vtkCellArray.h"
#include "vtkDoubleArray.h"
#include "vtkStringArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkPolyData.h"
#include "vtkCharArray.h"
#include "vtkSmartPointer.h"
#include "vtkCMBGeometryReader.h"
#include "vtkErrorCode.h"
#include "vtkTransform.h"
#include "vtkOutlineSource.h"

#include "vtkTransformPolyDataFilter.h"
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>


vtkStandardNewMacro(vtkOmicronModelInputReader);

//-----------------------------------------------------------------------------
vtkOmicronModelInputReader::vtkOmicronModelInputReader()
{
  this->FileName = 0;
  this->LoadGeometry = false;
  this->SetNumberOfInputPorts(0);

}

//-----------------------------------------------------------------------------
vtkOmicronModelInputReader::~vtkOmicronModelInputReader()
{
  this->SetFileName(0);
}

//-----------------------------------------------------------------------------
int vtkOmicronModelInputReader::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{
  // get the info object
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the ouptut
  vtkMultiBlockDataSet *output = vtkMultiBlockDataSet::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkFieldData *topFieldData = output->GetFieldData();

  ifstream fin(this->FileName);
  if(!fin)
    {
    vtkErrorMacro(<< "File " << this->FileName << " not found");
    return 0;
    }

  char buffer[2048];

  // 1st line is the SurfaceFileName
  std::string fileName;
  fin >> fileName;
  fin.getline(buffer, 2047);  //finish the line

  // Next we get the four corners of our area of interest
  double boundaryCoordinates[4][2];
  fin.getline(buffer, 2047, ':');  //read until the :
  fin >> boundaryCoordinates[0][0] >> boundaryCoordinates[0][1]
      >> boundaryCoordinates[1][0] >> boundaryCoordinates[1][1]
      >> boundaryCoordinates[2][0] >> boundaryCoordinates[2][1]
      >> boundaryCoordinates[3][0] >> boundaryCoordinates[3][1];
  fin.getline(buffer, 2047);  //finish the line

  // Surface transformation info
  double translation[3], rotation[3], scale, color[3] = {0.3, 0.2, 0.01};
  fin.getline(buffer, 2047, ':');  //read until the :
  // NOTE: rotations are order z, y, x in the file
  fin >> translation[0] >> translation[1] >> translation[2]
      >> rotation[2] >> rotation[1] >> rotation[0]
      >> scale;
  fin.getline(buffer, 2047);  //finish the line

  vtkPolyData *surface =
    this->AddBlock(output, fileName.c_str(), translation, rotation, scale,
    color, "soil");

  // DomainBottom and factor
  double domainBottom, frameSpace;
  fin.getline(buffer, 2047, ':');  //read until the :
  fin >> domainBottom >> frameSpace;
  fin.getline(buffer, 2047);  //finish the line

  vtkSmartPointer<vtkDoubleArray> frameSpaceFD =
    vtkSmartPointer<vtkDoubleArray>::New();
  frameSpaceFD->SetName("FrameSpace");
  frameSpaceFD->InsertNextValue( frameSpace );
  topFieldData->AddArray( frameSpaceFD );

  this->AddROIBlock(output, boundaryCoordinates, surface,
    translation, domainBottom);

  // area constraint
  double areaConstraint;
  fin.getline(buffer, 2047, ':');  //read until the :
  fin >> areaConstraint;
  fin.getline(buffer, 2047);  //finish the line
  vtkSmartPointer<vtkDoubleArray> areaConstraintFD =
    vtkSmartPointer<vtkDoubleArray>::New();
  areaConstraintFD->SetName("AreaConstraint");
  areaConstraintFD->InsertNextValue( areaConstraint );
  topFieldData->AddArray( areaConstraintFD );

  // volume_constraint: 8.9e-005
  double volumeConstraint;
  fin.getline(buffer, 2047, ':');  //read until the :
  fin >> volumeConstraint;
  fin.getline(buffer, 2047);  //finish the line
  vtkSmartPointer<vtkDoubleArray> volumeConstraintFD =
    vtkSmartPointer<vtkDoubleArray>::New();
  volumeConstraintFD->SetName("VolumeConstraint");
  volumeConstraintFD->InsertNextValue( volumeConstraint );
  topFieldData->AddArray( volumeConstraintFD );

  // disc_constraint / disc_radius
  double discConstraint;
  fin.getline(buffer, 2047, ':');  //read until the :
  fin >> discConstraint;
  fin.getline(buffer, 2047);  //finish the line
  vtkSmartPointer<vtkDoubleArray> discConstraintFD =
    vtkSmartPointer<vtkDoubleArray>::New();
  discConstraintFD->SetName("DiscConstraint");
  discConstraintFD->InsertNextValue( discConstraint );
  topFieldData->AddArray( discConstraintFD );

  // number_of_regions: 24
  int numberOfRegions;
  fin.getline(buffer, 2047, ':');  //read until the :
  fin >> numberOfRegions;
  fin.getline(buffer, 2047);  //finish the line

  color[0] = 0.3; color[1] = 0.4; color[2] = 0.3;
  for (vtkIdType i = 0; i < numberOfRegions && !fin.eof(); i++)
    {
    fin >> fileName;
    fin.getline(buffer, 2047);  //finish the line

    // transformation info
    fin.getline(buffer, 2047, ':');
    // NOTE: rotations are order z, y, x in the file
    fin >> translation[0] >> translation[1] >> translation[2]
        >> rotation[2] >> rotation[1] >> rotation[0]
        >> scale;
    fin.getline(buffer, 2047);  //finish the line

    this->AddBlock(output, fileName.c_str(), translation, rotation,
      scale, color);
    }
  fin.close();

  return 1;
}


//-----------------------------------------------------------------------------
vtkPolyData* vtkOmicronModelInputReader::AddBlock(vtkMultiBlockDataSet *output,
                                                  const char *fileName,
                                                  double translation[3],
                                                  double rotation[3],
                                                  double scale,
                                                  double color[3],
                                                  const char *additionalIdentifier/*=0*/)
{
  vtkPolyData *block;

  if (this->LoadGeometry)
    {
    vtkSmartPointer<vtkCMBGeometryReader> reader =
      vtkSmartPointer<vtkCMBGeometryReader>::New();
    reader->SetFileName( fileName );
    reader->Update();
    if (reader->GetErrorCode() == vtkErrorCode::NoError)
      {
      // transform the point data IF non-(0,0,0) translation
      if (additionalIdentifier &&
        (translation[0] != 0 || translation[1] != 0 || translation[2] != 0))
        {
        // RSB!!! currently only setting the additinoalInentifier for the
        // surface / soil.  Barry indicated that "some LIDAR data was captured
        // with UTM coordinates that could not be stored in floats without
        // lack of precision.  Offsetting to a zero value meant that OpenGL
        // could handle the points with subsequent translation and rotation."
        // I had oriingally planned to just handle this translation within
        // the transformation (as the first operation on each point as opposed
        // to the last) but given Barry's "note" above, am going to go ahead
        // and just transform the data as ScenGen does for now.
        vtkSmartPointer<vtkTransformPolyDataFilter> tFilter =
          vtkSmartPointer<vtkTransformPolyDataFilter>::New();
        tFilter->SetInputConnection( reader->GetOutputPort() );
        vtkSmartPointer<vtkTransform> transform =
          vtkSmartPointer<vtkTransform>::New();
        transform->PreMultiply();
        transform->Translate( translation );
        tFilter->SetTransform( transform );
        tFilter->Update();
        block = tFilter->GetOutput();
        output->SetBlock(output->GetNumberOfBlocks(), block);
        }
      else
        {
        block = reader->GetOutput();
        output->SetBlock(output->GetNumberOfBlocks(), block);
        }
      }
    else // if don't know how to read it, add empty block
      {
      block = vtkPolyData::New();
      output->SetBlock(output->GetNumberOfBlocks(), block);
      block->Delete();
      }
    }
  else
    {
    block = vtkPolyData::New();

    // FileName is set in CMBGeometryReader if we are loading geometry
    vtkSmartPointer<vtkStringArray> fileNameFD =
      vtkSmartPointer<vtkStringArray>::New();
    fileNameFD->SetName("FileName");
    fileNameFD->InsertNextValue(fileName);
    block->GetFieldData()->AddArray( fileNameFD );

    output->SetBlock(output->GetNumberOfBlocks(), block);
    block->Delete();
    }

  vtkSmartPointer<vtkDoubleArray> translationFD =
    vtkSmartPointer<vtkDoubleArray>::New();
  translationFD->SetName( "Translation" );
  translationFD->SetNumberOfComponents(3);
  translationFD->InsertNextTuple( translation );
  block->GetFieldData()->AddArray( translationFD );

  vtkSmartPointer<vtkDoubleArray> rotationFD =
    vtkSmartPointer<vtkDoubleArray>::New();
  rotationFD->SetName( "Rotation" );
  rotationFD->SetNumberOfComponents(3);
  rotationFD->InsertNextTuple( rotation );
  block->GetFieldData()->AddArray( rotationFD );

  vtkSmartPointer<vtkDoubleArray> scaleFD =
    vtkSmartPointer<vtkDoubleArray>::New();
  scaleFD->SetName( "Scale" );
  scaleFD->SetNumberOfComponents(3);
  scaleFD->InsertNextTuple3( scale, scale, scale );
  block->GetFieldData()->AddArray( scaleFD );

  vtkSmartPointer<vtkDoubleArray> colorFD =
    vtkSmartPointer<vtkDoubleArray>::New();
  colorFD->SetName( "Color" );
  colorFD->SetNumberOfComponents(3);
  colorFD->InsertNextTuple( color );
  block->GetFieldData()->AddArray( colorFD );

  // now combine the components into a single Transformation
  vtkSmartPointer<vtkTransform> transform =
    vtkSmartPointer<vtkTransform>::New();
  transform->PreMultiply();
  if (!additionalIdentifier)
    {
    transform->Translate(translation);
    }
  transform->RotateZ( rotation[2] );
  transform->RotateY( rotation[1] );
  transform->RotateX( rotation[0] );
  transform->Scale( scale, scale, scale );
  // RSB!!! Instead doing this with a transform filter
  //if (additionalIdentifier)
  //  {
  //  transform->Translate(translation);
  //  }
  vtkSmartPointer<vtkDoubleArray> transformFD =
    vtkSmartPointer<vtkDoubleArray>::New();
  transformFD->SetName( "Transformation" );
  transformFD->SetNumberOfComponents(16);
  transformFD->InsertNextTuple( transform->GetMatrix()[0][0] );
  block->GetFieldData()->AddArray( transformFD );

  vtkSmartPointer<vtkStringArray> objectType =
    vtkSmartPointer<vtkStringArray>::New();
  objectType->SetName("ObjectType");


  // PointInside is same as Translation, unless we indicated this is the soil
  if (additionalIdentifier)
    {
    vtkSmartPointer<vtkStringArray> extraFD =
      vtkSmartPointer<vtkStringArray>::New();
    extraFD->SetName("Identifier");
    extraFD->InsertNextValue(additionalIdentifier);
    block->GetFieldData()->AddArray( extraFD );

    objectType->InsertNextValue("Surface");
    }
  else
    {
    vtkSmartPointer<vtkDoubleArray> pointInsideFD =
      vtkSmartPointer<vtkDoubleArray>::New();
    pointInsideFD->SetName( "PointInside" );
    pointInsideFD->SetNumberOfComponents(3);
    pointInsideFD->InsertNextTuple( translation );
    block->GetFieldData()->AddArray( pointInsideFD );

    objectType->InsertNextValue("Solid");
    }
  block->GetFieldData()->AddArray( objectType );

  return block;
}


//-----------------------------------------------------------------------------
int vtkOmicronModelInputReader::AddROIBlock(vtkMultiBlockDataSet *output,
                                            double (*boundaryCoords)[2],
                                            vtkPolyData *surface,
                                            double translation[3],
                                            double domainBottom)
{
  vtkPolyData *block;

  if (this->LoadGeometry)
    {
    // is the case, just use a default
    double height;
    double minX, maxX, minY, maxY;
    minX = maxX = boundaryCoords[0][0];
    minY = maxY = boundaryCoords[0][1];
    for (int i = 1; i < 3; i++) // no need to check the 4th point
      {
      if (boundaryCoords[i][0] < minX) { minX = boundaryCoords[i][0]; }
      else if (boundaryCoords[i][0] > maxX) { maxX = boundaryCoords[i][0]; }
      if (boundaryCoords[i][1] < minY) { minY = boundaryCoords[i][1]; }
      else if (boundaryCoords[i][1] > maxY) { maxY = boundaryCoords[i][1]; }
      }
    if (surface->GetNumberOfPoints() < 1)
      {
      // we may have specified to LoadGeometry, but not been able to load the
      // surface, which is need to come up with the height of the bbox.  If
      // that is the case, use the average x,y dimension
      height = ((maxX - minX) + (maxY - minY)) / 2.0;
      }
    else
      {
      // come up with the "height" of the bbox
      double bounds[6];
      surface->GetBounds(bounds);

      // as Barry did in SceneGen
      height = 2 * (bounds[5] - domainBottom);
      }

    vtkSmartPointer<vtkOutlineSource> outlineSource =
      vtkSmartPointer<vtkOutlineSource>::New();
    //outlineSource->SetBounds(-1, 1, -1, 1, -1, 1);
    //minX + translation[0], maxX + translation[0],
    //  minY + translation[1], maxY + translation[1], domainBottom,
    //  domainBottom + height);
    outlineSource->Update();
    block = outlineSource->GetOutput();


    double voiTranslation[3], rotation[3] = {0, 0, 0}, scale[3];

    voiTranslation[0] = translation[0] + (maxX + minX) / 2.0;
    voiTranslation[1] = translation[1] + (maxY + minY) / 2.0;
    voiTranslation[2] = domainBottom + height / 2.0;

    scale[0] = (maxX - minX) / 2.0;
    scale[1] = (maxY - minY) / 2.0;
    scale[2] = height / 2.0;

    vtkSmartPointer<vtkDoubleArray> translationFD =
      vtkSmartPointer<vtkDoubleArray>::New();
    translationFD->SetName( "Translation" );
    translationFD->SetNumberOfComponents(3);
    translationFD->InsertNextTuple( voiTranslation );
    block->GetFieldData()->AddArray( translationFD );

    vtkSmartPointer<vtkDoubleArray> rotationFD =
      vtkSmartPointer<vtkDoubleArray>::New();
    rotationFD->SetName( "Rotation" );
    rotationFD->SetNumberOfComponents(3);
    rotationFD->InsertNextTuple( rotation );
    block->GetFieldData()->AddArray( rotationFD );

    vtkSmartPointer<vtkDoubleArray> scaleFD =
      vtkSmartPointer<vtkDoubleArray>::New();
    scaleFD->SetName( "Scale" );
    scaleFD->SetNumberOfComponents(3);
    scaleFD->InsertNextTuple( scale );
    block->GetFieldData()->AddArray( scaleFD );

    output->SetBlock(output->GetNumberOfBlocks(), block);
    }
  else
    {
    block = vtkPolyData::New();
    output->SetBlock(output->GetNumberOfBlocks(), block);
    block->Delete();
    }

  vtkSmartPointer<vtkStringArray> objectType =
    vtkSmartPointer<vtkStringArray>::New();
  objectType->SetName("ObjectType");
  objectType->InsertNextValue("RegionOfInterest");
  block->GetFieldData()->AddArray( objectType );

  vtkSmartPointer<vtkDoubleArray> boundaryCoordinatesFD =
    vtkSmartPointer<vtkDoubleArray>::New();
  boundaryCoordinatesFD->SetName( "ROICoordinates" );
  boundaryCoordinatesFD->SetNumberOfValues(8);
  boundaryCoordinatesFD->SetNumberOfComponents(2);
  boundaryCoordinatesFD->SetTuple(0, boundaryCoords[0]);
  boundaryCoordinatesFD->SetTuple(1, boundaryCoords[1]);
  boundaryCoordinatesFD->SetTuple(2, boundaryCoords[2]);
  boundaryCoordinatesFD->SetTuple(3, boundaryCoords[3]);
  block->GetFieldData()->AddArray( boundaryCoordinatesFD );

  vtkSmartPointer<vtkDoubleArray> bottomFD =
    vtkSmartPointer<vtkDoubleArray>::New();
  bottomFD->SetName("ROIBottom");
  bottomFD->InsertNextValue( domainBottom );
  block->GetFieldData()->AddArray( bottomFD );

  return VTK_OK;
}

//-----------------------------------------------------------------------------
void vtkOmicronModelInputReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "File Name: "
     << (this->FileName ? this->FileName : "(none)") << "\n";
}


//----------------------------------------------------------------------------
int vtkOmicronModelInputReader::RequestInformation(
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


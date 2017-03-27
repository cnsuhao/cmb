//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkOSDLReader.h"

#include "vtkCellArray.h"
#include "vtkDoubleArray.h"
#include "vtkStringArray.h"
#include "vtkInformation.h"
#include "vtkErrorCode.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkSmartPointer.h"
#
#include "vtkOmicronModelInputReader.h"
#include "vtkSceneGenVegetationClusterReader.h"
#include "vtkSceneGenVegetationReader.h"
#include "vtkCompositeDataIterator.h"
#include "vtksys/SystemTools.hxx"
#include <sys/types.h>
#include <sys/stat.h>

vtkStandardNewMacro(vtkOSDLReader);

//-----------------------------------------------------------------------------
vtkOSDLReader::vtkOSDLReader()
{
  this->FileName = NULL;
  this->SetNumberOfInputPorts(0);
}

//-----------------------------------------------------------------------------
vtkOSDLReader::~vtkOSDLReader()
{
  this->SetFileName(0);
}

//-----------------------------------------------------------------------------
int vtkOSDLReader::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{
  if (!this->FileName || (strlen(this->FileName) == 0))
    {
    vtkErrorMacro("FileName has to be specified!");
    return 0;
    }

  // get the info object
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  vtkMultiBlockDataSet *output = vtkMultiBlockDataSet::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  ifstream fin(this->FileName);
  if(!fin)
    {
    vtkErrorMacro("File " << this->FileName << " not found");
    this->SetErrorCode( vtkErrorCode::CannotOpenFileError );
    return 0;
    }


  std::string currentDirectory =  vtksys::SystemTools::CollapseFullPath(
    vtksys::SystemTools::GetCurrentWorkingDirectory().c_str() );
  std::string fullPath = vtksys::SystemTools::CollapseFullPath( this->FileName );
  std::string dataPath = vtksys::SystemTools::GetFilenamePath( fullPath );
  vtksys::SystemTools::ChangeDirectory( dataPath.c_str() );


  //the first line is our File ID
  std::string tag;
  fin >> tag;
  if (tag.find("OSDL") == std::string::npos)
    {
    vtkErrorMacro("Warning: Not an OSD file.\n");
    this->SetErrorCode( vtkErrorCode::FileFormatError );
    return 0;
    }

// order doesn't matter
  while (!(tag=="}") && !fin.eof())
    {
    fin >> tag;

    if ((tag[0]=='/') && (tag[1]=='/'))
      {
      //fin.getline(inln, 199);  //finish the line
      continue;
      }

  //  if (tag == "version")
  //    {
  //    fin >> version; //compare version #
  //    }

  ////  if (tag == "object_geometry")
  ////  {
  ////    if (!ReadObjectGeometry( fin )) return false;
  ////  }

  ////  if (tag == "area_of_interest")
  ////  {
  ////    if (!ReadAreaOfInterest( fin )) return false;
  ////  }

    if (tag == "%Omicron_Input")
      {
      fin >> tag;
      vtkSmartPointer<vtkOmicronModelInputReader> reader =
        vtkSmartPointer<vtkOmicronModelInputReader>::New();
      reader->SetFileName( tag.c_str() );
      reader->LoadGeometryOn();
      reader->Update();
      this->AppendBlocks( output, reader->GetOutput() );
      }

    if (tag == "%Veg_Input")
      {
      fin >> tag;
      vtkSmartPointer<vtkSceneGenVegetationReader> reader =
        vtkSmartPointer<vtkSceneGenVegetationReader>::New();
      reader->SetFileName( tag.c_str() );
      reader->Update();
      this->AppendBlocks( output, reader->GetOutput() );
      }

  //  if (tag == "%Omicron_Output")
  //  {
  //    fin >> tag;
  //    outputs.AddOmicronFileName( tag );
  //    //cout << "Omicron Output: " << tag << "\n";
  //  }

  //  if (tag == "%Veg_Output")
  //  {
  //    fin >> tag;
  //    outputs.AddVegFileName( tag );
  //    cout << "Veg Output: " << tag << "\n";
  //  }

  //  if (tag == "%POVRay_Output")
  //  {
  //    fin >> tag;
  //    outputs.AddPOVRayFileName( tag );
  //    cout << "POV-Ray Output: " << tag << "\n";
  //  }

  //  if (tag == "%Wavefront_Output")
  //  {
  //    fin >> tag;
  //    outputs.AddWavefrontFileName( tag );
  //    cout << "Wavefront Output: " << tag << "\n";
  //  }

  //}
    }
  fin.close();




//
//
//  char buffer[2048];
//  //1st 5 lines are of no interest... at least for now
//  fin.getline(buffer, 2048);
//  fin.getline(buffer, 2048);
//  fin.getline(buffer, 2048);
//  fin.getline(buffer, 2048);
//  fin.getline(buffer, 2048);
//
//  // volume_constraint: 8.9e-005
//  fin.getline(buffer, 2048);
//  double volumeConstraint;
//  sscanf(buffer,"volume_constraint: %lf", &volumeConstraint);
//  vtkSmartPointer<vtkDoubleArray> volumeConstraintFD = vtkSmartPointer<vtkDoubleArray>::New();
//  volumeConstraintFD->SetName("VolumeConstraint");
//  volumeConstraintFD->InsertNextValue( volumeConstraint );
//
//  // disc_constraint... don't care about
//  fin.getline(buffer, 2048);
//
//  // number_of_regions: 24
//  fin.getline(buffer, 2048);
//  int numberOfRegions;
//  sscanf(buffer,"number_of_regions: %d", &numberOfRegions);
//
//  vtkPoints *newPts = vtkPoints::New();
//  newPts->SetDataTypeToDouble();
//  newPts->Allocate(numberOfRegions);
//  vtkCellArray *newVerts = vtkCellArray::New();
//  newVerts->Allocate(numberOfRegions * 2);
//
//  vtkSmartPointer<vtkCharArray> objectFileNameFD =
//    vtkSmartPointer<vtkCharArray>::New();
//  objectFileNameFD->SetName( "RegionNames" );
////  objectFileNameFD->SetNumberOfValues(numberOfRegions);
//
//  vtkSmartPointer<vtkDoubleArray> pointInsideFD =
//    vtkSmartPointer<vtkDoubleArray>::New();
//  pointInsideFD->SetName( "PointsInside" );
//  pointInsideFD->SetNumberOfValues(numberOfRegions * 3);
//  pointInsideFD->SetNumberOfComponents(3);
//
//  std::vector< std::string > objectNames;
//  double pointInside[3];
//  unsigned int maxSize = 0;
//  for (vtkIdType i = 0; i < numberOfRegions && !fin.eof(); i++)
//    {
//    // the filename for the object/region
//    fin.getline(buffer, 2048);
//    objectNames.push_back( buffer );
//    if (strlen(buffer) > maxSize)
//      {
//      maxSize = strlen(buffer);
//      }
//
//    // (X_translation,Y_translation,Z_translation,rotz,roty,rotx,scale): 13.94 1.481 9.588 0 0 0 1
//    fin.getline(buffer, 2048);
//    sscanf(buffer,"(X_translation,Y_translation,Z_translation,rotz,roty,rotx,scale): %lf %lf %lf",
//      pointInside, pointInside + 1, pointInside +2);
//    pointInsideFD->SetTuple(i, pointInside);
//    newPts->InsertNextPoint( pointInside );
//    newVerts->InsertNextCell(1, &i);
//    }
//  fin.close();
//
//  // make sure we have enough compoenents to add "soil" later on
//  // (in vtkOmicronMeshInputFilter)
//  objectFileNameFD->SetNumberOfComponents( maxSize > 3 ? maxSize+1 : 5 );
//  objectFileNameFD->SetNumberOfTuples(numberOfRegions);
//  for (unsigned int i = 0; i < objectNames.size(); i++)
//    {
//    objectFileNameFD->SetTypedTuple(i, objectNames[i].c_str());
//    }
//
//  output->GetFieldData()->AddArray( volumeConstraintFD );
//  output->GetFieldData()->AddArray( objectFileNameFD );
//  output->GetFieldData()->AddArray( pointInsideFD );
//
//
//  //std::string tempString;
//  //for (int i = 0; i < output->GetFieldData()->GetNumberOfArrays(); i++)
//  //  {
//  //  tempString = output->GetFieldData()->GetArray(i)->GetName();
//  //  }
//
//  output->SetPoints(newPts);
//  newPts->Delete();
//
//  output->SetVerts(newVerts);
//  newVerts->Delete();

  vtksys::SystemTools::ChangeDirectory( currentDirectory.c_str() );

  return 1;
}

#include "vtkTransformPolyDataFilter.h"
#include "vtkTransform.h"

//-----------------------------------------------------------------------------
void vtkOSDLReader::AppendBlocks( vtkMultiBlockDataSet *output,
                                      vtkDataObject *dataObject )
{
  vtkCompositeDataSet* mds = vtkCompositeDataSet::SafeDownCast(dataObject);
  if (mds)
    {
    if (dataObject->GetFieldData()->GetNumberOfArrays() > 0)
      {
      // if the aray already exists... tough!
      for (int i = 0; i < dataObject->GetFieldData()->GetNumberOfArrays(); i++)
        {
        output->GetFieldData()->AddArray(
          dataObject->GetFieldData()->GetAbstractArray(i) );
        }
      }
    vtkCompositeDataIterator* iter = mds->NewIterator();
    iter->InitTraversal();
    while (!iter->IsDoneWithTraversal())
      {
      output->SetBlock(output->GetNumberOfBlocks(),
        iter->GetCurrentDataObject());

      // for now, make sure everything has an object type... this is not where
      // this hsould be done... but covering all bases for now
      vtkStringArray *objectType = vtkStringArray::SafeDownCast(
        iter->GetCurrentDataObject()->GetFieldData()->GetAbstractArray( "ObjectType" ) );
      if (!objectType)
        {
        objectType = vtkStringArray::New();
        objectType->SetName("ObjectType");
        objectType->InsertNextValue("Other");
        iter->GetCurrentDataObject()->GetFieldData()->AddArray( objectType );
        objectType->Delete();
        }

      iter->GoToNextItem();
      }
    iter->Delete();
    }
  else
    {
    output->SetBlock(output->GetNumberOfBlocks(), dataObject);
    vtkStringArray *objectType = vtkStringArray::SafeDownCast(
      dataObject->GetFieldData()->GetAbstractArray( "ObjectType" ) );
    if (!objectType)
      {
      objectType = vtkStringArray::New();
      objectType->SetName("ObjectType");
      objectType->InsertNextValue("Other");
      dataObject->GetFieldData()->AddArray( objectType );
      objectType->Delete();
      }
    }
}


//-----------------------------------------------------------------------------
void vtkOSDLReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "File Name: "
     << (this->FileName ? this->FileName : "(none)") << "\n";
}


//----------------------------------------------------------------------------
int vtkOSDLReader::RequestInformation(
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


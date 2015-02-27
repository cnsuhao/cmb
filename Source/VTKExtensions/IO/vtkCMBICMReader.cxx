/*=========================================================================

Program:   Visualization Toolkit
Module:    vtkCMBICMReader.cxx

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
All rights reserved.
See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkCMBICMReader.h"

#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkCellType.h"
#include "vtkCMBReaderHelperFunctions.h"
#include "vtkErrorCode.h"
#include "vtkFieldData.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkLine.h"
#include "vtkObjectFactory.h"
#include "vtkPoints.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkStringArray.h"
#include "vtkUnstructuredGrid.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <vtksys/SystemTools.hxx>


using namespace ReaderHelperFunctions;

vtkStandardNewMacro(vtkCMBICMReader);
//=============================================================================
vtkCMBICMReader::vtkCMBICMReader()
{
  //By default data is to be read in long/lat and
  //East will be facing the positive direction
  this->DataIsLatLong = false;
  this->OldDataIsLatLong = true;

  this->DataIsPositiveEast = true;
  this->OldDataIsPositiveEast = false;

  this->FileName = NULL;
  this->DataFileName = NULL;
  this->OldFileName = NULL;
  this->OldDataFileName = NULL;
  this->SetNumberOfInputPorts(0);
  this->NumberOfTimeSteps = 0;
  this->TimeSteps = NULL;
  this->points = NULL;
  this->vectors = NULL;
  this->HexCells = NULL;
}
//-----------------------------------------------------------------------------
vtkCMBICMReader::~vtkCMBICMReader()
{
  this->SetFileName(0);
  this->SetDataFileName(0);
  if(this->points)
    {
    this->points->Delete();
    }
  if(this->vectors)
    {
    this->vectors->Delete();
    }
  if(this->HexCells)
    {
    this->HexCells->Delete();
    }
  if (this->TimeSteps)
    {
    delete [] this->TimeSteps;
    this->TimeSteps = 0;
    this->NumberOfTimeSteps = 0;
    }
}
//-----------------------------------------------------------------------------
int vtkCMBICMReader::ReadTemporalData(vtkInformationVector* outputVector)
{
  //Help the user get the full path to the temporal data
  if (!this->DataFileName || strcmp(this->DataFileName,"") == 0)
    {
    //If no data file is specified use a default name
    this->SetDataFileName((vtksys::SystemTools::GetFilenamePath(this->FileName)+
          "/"+vtksys::SystemTools::GetFilenameWithoutExtension(this->FileName)+
          "-timedata.dat").c_str());
    }
  if(vtksys::SystemTools::GetFilenameExtension(this->DataFileName)=="")
    {
    this->SetDataFileName((std::string(this->DataFileName)+".dat").c_str());
    }
  if(vtksys::SystemTools::GetFilenamePath(this->DataFileName) == "")
    {
    this->SetDataFileName((vtksys::SystemTools::GetFilenamePath(this->FileName)+"/"+this->DataFileName).c_str());
    }
  std::string dataFileNameStr = this->DataFileName;
  struct stat dfs;
  if (stat(dataFileNameStr.c_str(), &dfs) != 0)
    {
    vtkErrorMacro("Unable to open file: "<< dataFileNameStr);
    this->SetErrorCode( vtkErrorCode::CannotOpenFileError );
    return 0;
    }
  /*Setup variables*/
  std::ifstream file(dataFileNameStr.c_str(), std::ios::in | std::ios::binary );
  std::stringstream line(std::stringstream::in | std::stringstream::out);

  //Read in data
  if(!file)
    {
    vtkErrorMacro("Unable to open file: " << dataFileNameStr);
    return 0;
    }

  this->UpdateProgress(0);
  file.seekg(0,ios::end);
  double numBytes = file.tellg();
  file.clear();
  file.seekg(0,ios::beg);

  this->TimeStepRange[0] = 0;
  std::vector<double> TimeSteps_vector;

  while(readNextLine(file,line))
    {
    float current_time;
    line >> current_time;
    TimeSteps_vector.push_back(current_time);
    vtkSmartPointer<vtkFloatArray> cellData = vtkSmartPointer<vtkFloatArray>::New();
    cellData->SetNumberOfComponents(1);
    cellData->Allocate((this->NumberOfCells*50),(this->NumberOfCells));

    //each timestep should have numPoints/8 in it
    for(int i = 0; i < (this->NumberOfCells); i++)
      {
      if(!readNextLine(file,line))
        {
        vtkErrorMacro("Incorrect number of lines in " << dataFileNameStr << "\n  num_lines must be multiple of: " << (this->NumberOfCells)+1 );
        }
      float val;
      line >> val;
      cellData->InsertTuple1(i,val);
      }


    cellData->SetName(vtksys::SystemTools::GetFilenameWithoutExtension(this->DataFileName).c_str());
    this->CellData.push_back(cellData);
    this->UpdateProgress(static_cast<double>(file.tellg())/static_cast<double>(numBytes));
    }
  this->NumberOfTimeSteps = TimeSteps_vector.size();
  if(this->TimeSteps)
    {
    delete []this->TimeSteps;
    }
  this->TimeSteps = new double[this->NumberOfTimeSteps];
  std::copy(TimeSteps_vector.begin(), TimeSteps_vector.end(), this->TimeSteps);
  this->TimeStepRange[0] = this->TimeSteps[0];
  this->TimeStepRange[1] = this->TimeSteps[this->NumberOfTimeSteps-1];
  this->TimeStep = 0;
  this->TimeValue = this->TimeSteps[0];
  file.close();
  return 1;
}
//-----------------------------------------------------------------------------
int vtkCMBICMReader::ReadGeometryData(vtkInformationVector *outputVector)
{
  std::string fileNameStr = this->FileName;
  struct stat fs;
  if (stat(fileNameStr.c_str(), &fs) != 0)
    {
    vtkErrorMacro("Unable to open file: "<< fileNameStr);
    this->SetErrorCode( vtkErrorCode::CannotOpenFileError );
    return 0;
    }
  /*Setup variables*/
  std::ifstream file(fileNameStr.c_str(), ios::in | ios::binary );
  std::stringstream line(std::stringstream::in | std::stringstream::out);
  if(!file)
    {
    vtkErrorMacro("Unable to open file: " << fileNameStr);
    return 1;
    }

  this->UpdateProgress(0);
  file.seekg(0,ios::end);
  double numBytes = file.tellg();
  double numBytes2 = numBytes * 2;//*2 because the file will be read twice
  file.clear();
  file.seekg(0,ios::beg);
  //Read File

  //Starting First Pass
  // This file is an ide followed by 8
  // pieces of point data
  // idnum
  // id1 lon lat z lonj lati k
  // id2 lon lat z lonj lati k
  // id3 lon lat z lonj lati k
  // id4 lon lat z lonj lati k
  // id5 lon lat z lonj lati k
  // id6 lon lat z lonj lati k
  // id7 lon lat z lonj lati k
  // id8 lon lat z lonj lati k
  while(readNextLine(file,line))
    {
    for(int count = 0; count < 8 ; count++)
      {
      if(!readNextLine(file,line)){vtkErrorMacro("Incorrect file format: " << fileNameStr);}
      this->NumberOfPoints++;
      }
    this->UpdateProgress(static_cast<double>(file.tellg())/static_cast<double>(numBytes2));
    }
  this->NumberOfCells = this->NumberOfPoints/8;
  file.clear();
  file.seekg(0,ios::beg);
  if(!file)
    {
    vtkErrorMacro("Unable to open file: " << fileNameStr);
    return 1;
    }

  //Start Second Pass
  this->points->SetNumberOfPoints(this->NumberOfPoints);
  this->vectors->SetNumberOfComponents(3);
  this->vectors->SetNumberOfTuples(this->NumberOfPoints);
  this->HexCells->Allocate(this->NumberOfCells);

  vtkIdType pointId = 0;
  while(readNextLine(file,line))
    {
    //skip over box number
    vtkSmartPointer<vtkHexahedron> hex = vtkSmartPointer<vtkHexahedron>::New();
    for(int count = 0; count < 8 ; count++)
      {
      //By default read in long/lat data
      float lon, lat, z;
      float lonj,lati,k;
      int unused;
      if(!readNextLine(file,line)){ vtkErrorMacro("Incorrect file format: " << fileNameStr);}
      line >> unused >> lon >> lat >> z >> lonj >> lati >> k;
      if(this->DataIsLatLong)
        {
        //If the data being passed in is lat/long swap values
        std::swap(lat, lon);
        std::swap(lati,lonj);
        }
      if(!this->DataIsPositiveEast)
        {
        //Longitude will be negated if positive is west
        lon = -lon;
        lonj = -lonj;
        }

      this->points->SetPoint(pointId,lon,lat,z);
      hex->GetPointIds()->SetId(count,pointId);
      this->vectors->SetTuple3(pointId,lonj,lati,k);
      pointId++;
      }
    this->HexCells->InsertNextCell(hex);
    //Numbytes2 is twice as big as the acutal number of bytes in the file
    //This is the second time reading the file so add an offset
    this->UpdateProgress((static_cast<double>(file.tellg())+numBytes)/static_cast<double>(numBytes2));
    }
  //Clean up pass 2
  file.close();
  return 1;
}
//-----------------------------------------------------------------------------
  int vtkCMBICMReader::RequestData(
      vtkInformation *vtkNotUsed(request),
      vtkInformationVector **vtkNotUsed(inputVector),
      vtkInformationVector *outputVector)
{
  // get the info object
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the ouptut
  vtkUnstructuredGrid *output = vtkUnstructuredGrid::SafeDownCast(
      outInfo->Get(vtkDataObject::DATA_OBJECT()));

  if(!output)
    {
    return 0;
    }

  //Set up geometry and read in timesteps if
  //it hasn't been done yet or the file names
  //changed
  if(!this->OldFileName ||
      strcmp(this->FileName, this->OldFileName) != 0 ||
      this->OldDataIsLatLong != this->DataIsLatLong ||
      this->OldDataIsPositiveEast != this->DataIsPositiveEast )
    {
    //Clean up old data
    if(this->points)
      {
      this->points->Delete();
      }
    if(this->vectors)
      {
      this->vectors->Delete();
      }
    if(this->HexCells)
      {
      this->HexCells->Delete();
      }

    this->NumberOfPoints = 0;
    this->NumberOfCells = 0;
    this->points = vtkPoints::New();
    this->vectors = vtkFloatArray::New();
    this->vectors->SetName(vtksys::SystemTools::GetFilenameWithoutExtension(this->FileName).c_str());
    this->HexCells = vtkCellArray::New();

    if(!ReadGeometryData(outputVector))
      {
      return 0;
      }
    //Don't reread geometry unless the filename changed
    this->SetOldFileName(this->FileName);
    this->OldDataIsLatLong = this->DataIsLatLong;
    this->OldDataIsPositiveEast = this->DataIsPositiveEast;
    //force rereading of temporal data
    this->OldDataFileName = 0;
    }
  if(!this->OldDataFileName || strcmp(this->DataFileName, this->OldDataFileName) != 0)
    {
    this->CellData.clear();
    if (this->TimeSteps)
      {
      delete [] this->TimeSteps;
      }
    this->TimeSteps = NULL;
    this->TimeStep = 0;
    this->NumberOfTimeSteps = 0;
    this->TimeStepRange[0] = 0;
    this->TimeStepRange[1] = 0;
    if(!ReadTemporalData(outputVector))
      {
      return 0;
      }
    this->SetOldDataFileName(this->DataFileName);
    }

  if(outInfo->Has(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP()))
    {
    this->TimeValue = outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP());
    //convert TimeValue to TimeIndex
    for(int i = 0; i < this->NumberOfTimeSteps; i++)
      {
      if(this->TimeSteps[i] == this->TimeValue)
        {
        this->TimeStep = i;
        break;
        }
      }
    }
  //Setup out info
  outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_STEPS(), this->TimeSteps, this->NumberOfTimeSteps);
  outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_RANGE(),this->TimeStepRange,2);
  //Apply geometry to output
  output->Allocate(this->NumberOfCells,1024);
  output->SetPoints(this->points);
  output->GetPointData()->SetVectors(this->vectors);
  output->SetCells(VTK_HEXAHEDRON,this->HexCells);
  //apply timestep to geometry
  output->GetCellData()->SetScalars(this->CellData[this->TimeStep]);
  return 1;
}
//-----------------------------------------------------------------------------
void vtkCMBICMReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "File Name: "
    << (this->FileName ? this->FileName : "(none)") << "\n";
}
//----------------------------------------------------------------------------
int vtkCMBICMReader::RequestInformation(
    vtkInformation * /*request*/,
    vtkInformationVector ** /*inputVector*/,
    vtkInformationVector *outputVector)
{
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  if(this->TimeSteps)
    {
    outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_STEPS(), this->TimeSteps, this->NumberOfTimeSteps);
    outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_RANGE(),this->TimeStepRange,2);
    }
  else
    {
    //Create fake timesteps if none exist yet
    //This makes sure paraview knows that this class has time fields
    this->TimeSteps = new double[1];
    this->NumberOfTimeSteps = 1;
    this->TimeSteps[0] = 0;
    this->TimeStepRange[0] = 0;
    this->TimeStepRange[1] = 0;
    outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_STEPS(), this->TimeSteps, this->NumberOfTimeSteps);
    outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_RANGE(),this->TimeStepRange,2);
    }
  if (!this->FileName)
    {
    vtkErrorMacro("FileName has to be specified!");
    return 0;
    }
  return 1;
}

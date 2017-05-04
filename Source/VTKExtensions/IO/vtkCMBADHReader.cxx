//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkCMBADHReader.h"
#include "smtk/extension/vtk/reader/vtkCMBReaderHelperFunctions.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkCellType.h"
#include "vtkErrorCode.h"
#include "vtkFieldData.h"
#include "vtkFloatArray.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkLine.h"
#include "vtkObjectFactory.h"
#include "vtkPoints.h"
#include "vtkPolyDataWriter.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkStringArray.h"
#include "vtkUnstructuredGrid.h"
#include <algorithm>
#include <map>
#include <sys/stat.h>
#include <sys/types.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtksys/SystemTools.hxx>

using namespace ReaderHelperFunctions;

//BTX
//Helper class for the reader
class ADHTemporalData
{
public:
  ADHTemporalData()
  {
    this->TimeSteps = NULL;
    this->TimeStepRange[0] = -1;
    this->TimeStepRange[1] = -1;
    this->NumberOfTimeSteps = 0;

    this->NumberOfPoints = -1;
    this->NumberOfCells = -1;
    this->ObjId = -1;
    this->IsVector = false;
    this->Name = std::string("No Name Specified");
  }
  ~ADHTemporalData()
  {
    std::map<double, vtkFloatArray*>::iterator iter;
    for (iter = DataArray.begin(); iter != DataArray.end(); ++iter)
    {
      (*iter).second->Delete();
    }
    DataArray.clear();
    if (TimeSteps)
    {
      delete[] TimeSteps;
    }
  }
  void UpdateArrayNames(const char* name)
  {
    std::map<double, vtkFloatArray*>::iterator iter;
    for (iter = DataArray.begin(); iter != DataArray.end(); ++iter)
    {
      (*iter).second->SetName(name);
    }
  }

protected:
  double* TimeSteps; //in seconds

  int NumberOfTimeSteps;
  int NumberOfPoints;
  int NumberOfCells;
  int ObjId;
  bool IsVector;
  std::string Name;

  // Descriptions:
  // Store the range of time steps
  double TimeStepRange[2]; //In Frames
  std::map<double, std::streampos>
    tsToBytePosition; //array that maps timevalue to its line's byte position
  std::map<double, vtkFloatArray*> DataArray;
  friend class vtkCMBADHReader;
};
//ETX
//=============================================================================

vtkStandardNewMacro(vtkCMBADHReader);

//=============================================================================
vtkCMBADHReader::vtkCMBADHReader()
{
  this->FileName = NULL;
  this->OldFileName = NULL;
  this->Suffix = NULL;
  this->OldSuffix = NULL;
  this->Prefix = NULL;
  this->OldPrefix = NULL;
  this->TimeValue = 0;
  this->DataSet = 0;
  this->PrimaryDataSet = 0;
  this->CacheSize = 100;
}

vtkCMBADHReader::~vtkCMBADHReader()
{
  this->SetFileName(0);
  this->SetOldFileName(0);
  this->SetPrefix(0);
  this->SetOldPrefix(0);
  this->SetSuffix(0);
  this->SetOldSuffix(0);
  std::vector<ADHTemporalData*>::iterator iter;
  for (iter = this->DataSets.begin(); iter != this->DataSets.end(); iter++, this->DataSet++)
  {
    delete (*iter);
  }
  this->DataSets.clear();
}

int vtkCMBADHReader::GetNumberOfTimeSteps()
{
  return DataSets[PrimaryDataSet]->NumberOfTimeSteps;
}

double* vtkCMBADHReader::GetTimeStepRange()
{
  return DataSets[PrimaryDataSet]->TimeStepRange;
}

//Do an initial read of the file scanning the header

/*
 DATASET
 OBJTYPE typestr
 BEGSCL
 BEGVEC
 ND numpoints
 NC numcells
 NAME string

 TS istat time
 t1
 t2
 ...
 tn
ENDDS
*/

int vtkCMBADHReader::ScanFile()
{
  int operation_succeded = 0;
  ifstream file(this->FileName, ios::in | ios::binary);
  if (!file)
  {
    vtkErrorMacro("Unable to open file: " << this->FileName);
    return 0;
  }
  //Get number of bytes in the file
  file.seekg(0, ios::end);
  double numBytes = file.tellg();
  file.clear();
  file.seekg(0, ios::beg);

  /*Setup variables*/
  std::stringstream line(std::stringstream::in | std::stringstream::out);
  std::string card = "";

  ADHTemporalData* timeData = new ADHTemporalData();
  std::vector<double> timeValues;
  while (true)
  {
    std::streampos pos = file.tellg();
    if (!readNextLine(file, line, card))
    {
      break;
    }
    if (card == "TS")
    {
      double istat = 0, time = 0;
      line >> istat >> time;
      for (int i = 0; i < timeData->NumberOfPoints; i++)
      {
        //Skip over time data that we don't need to read yet
        readNextLine(file, line);
        if (istat != 0)
        {
          readNextLine(file, line);
        }
      }
      timeValues.push_back(time);
      timeData->tsToBytePosition[time] = pos;
    }
    else if (card == "NAME")
    {
      char tmpname[512];
      line.getline(tmpname, 512);
      timeData->Name = tmpname;
      //Remove quotation marks
      timeData->Name.erase(
        remove(timeData->Name.begin(), timeData->Name.end(), '\"'), timeData->Name.end());
      //Remove trailing and leading whitespace
      timeData->Name = timeData->Name.substr(
        timeData->Name.find_first_not_of(" \t"), timeData->Name.find_last_not_of(" \t"));
    }
    else if (card == "ND")
    {
      line >> timeData->NumberOfPoints;
    }
    else if (card == "NC")
    {
      line >> timeData->NumberOfCells;
    }
    else if (card == "BEGVEC")
    {
      timeData->IsVector = true;
    }
    else if (card == "BEGSCL")
    {
      timeData->IsVector = false;
    }
    else if (card == "ENDDS")
    {
      timeData->NumberOfTimeSteps = static_cast<int>(timeValues.size());
      timeData->TimeSteps = new double[timeData->NumberOfTimeSteps];
      std::copy(timeValues.begin(), timeValues.end(), timeData->TimeSteps);
      timeData->TimeStepRange[0] = timeData->TimeSteps[0];
      timeData->TimeStepRange[1] = timeData->TimeSteps[timeData->NumberOfTimeSteps - 1];
      this->DataSets.push_back(timeData);
      timeData = new ADHTemporalData();
      timeValues.clear();
      operation_succeded = 1;
    }
    this->UpdateProgress(static_cast<double>(file.tellg()) / static_cast<double>(numBytes));
  }
  delete timeData;
  file.close();
  return operation_succeded;
}

//This is a reader for the SMS ADH file for help on the format of the file go to
//http://www.ems-i.com/smshelp/SMS-Help.htm#File_Formats/SMS_Project_Files.htm
int vtkCMBADHReader::ReadTemporalData()
{
  std::string fileNameStr = this->FileName;
  struct stat fs;
  if (stat(fileNameStr.c_str(), &fs) != 0)
  {
    vtkErrorMacro("Unable to open file: " << fileNameStr);
    this->SetErrorCode(vtkErrorCode::CannotOpenFileError);
    return 0;
  }

  //Open File
  ifstream file(fileNameStr.c_str(), ios::in | ios::binary);
  if (!file)
  {
    vtkErrorMacro("Unable to open file: " << fileNameStr);
    return 1;
  }
  //Read File
  {
    ADHTemporalData* timeData = this->DataSets[this->DataSet];

    /*Setup variables*/
    std::stringstream line(std::stringstream::in | std::stringstream::out);
    std::string card = "";

    //Jump to the wanted data
    file.clear();
    file.seekg(timeData->tsToBytePosition[this->TimeValue]);
    readNextLine(file, line, card);
    if (card != "TS")
    {
      vtkErrorMacro("Error While Reading File");
      return 0;
    }

    int istat = -1;
    double time = 0;
    line >> istat >> time;

    if (time != this->TimeValue)
    {
      vtkErrorMacro("Error While Reading File");
      return 0;
    }

    readNextLine(file, line);

    int* stat_arr = 0;
    if (istat == 1)
    {
      stat_arr = new int[timeData->NumberOfPoints];
      for (int i = 0; i < timeData->NumberOfPoints; i++)
      {
        line >> stat_arr[i];
        readNextLine(file, line);
        this->UpdateProgress(i / timeData->NumberOfPoints * 2);
      }
    }

    //There will be an unknown number of tuples
    //do a quick check on how many values exist
    //TODO not the best way to do this recode later
    int numTuples = 0;
    char tmpline[512];
    line.getline(tmpline, 512);
    std::vector<vtksys::String> vals = vtksys::SystemTools::SplitString(tmpline, ' ');
    for (std::vector<vtksys::String>::iterator iter = vals.begin(); iter != vals.end(); ++iter)
    {
      if ((*iter) != " ")
      {
        numTuples++;
      }
    }
    //rewind the string stream after it is known
    //how many tuples there will be
    line.clear();
    line.seekg(0, ios::beg);

    //To begin an array of celldata the
    //file must be starting or the
    //previous dataset must be done
    std::string dname;
    if (this->Prefix)
    {
      dname += this->Prefix;
    }
    dname += timeData->Name;
    if (this->Suffix)
    {
      dname += this->Suffix;
    }

    vtkFloatArray* dataArr = vtkFloatArray::New();
    dataArr->SetName(dname.c_str());
    dataArr->SetNumberOfComponents(numTuples);
    dataArr->Allocate(timeData->NumberOfPoints);

    float* tup = new float[numTuples];
    for (int i = 0; i < timeData->NumberOfPoints; i++)
    {
      for (int j = 0; j < numTuples; j++)
      {
        line >> tup[j];
      }
      dataArr->InsertTuple(i, tup);
      readNextLine(file, line);
      this->UpdateProgress(static_cast<double>((i * (istat + 1))) /
        static_cast<double>((timeData->NumberOfPoints * (istat + 1))));
    }
    delete[] tup;
    timeData->DataArray[time] = dataArr;
    if (stat_arr)
    {
      delete[] stat_arr;
    }
    file.close();
  }

  return 0;
}

int vtkCMBADHReader::RequestData(vtkInformation* /*request*/, vtkInformationVector** inputVector,
  vtkInformationVector* outputVector)
{
  vtkPointSet* input = vtkPointSet::GetData(inputVector[0]);
  if (input == 0)
  {
    vtkErrorMacro("Must set Input!");
    return 0;
  }

  if (!this->FileName || strcmp(this->FileName, "") == 0)
  {
    vtkErrorMacro("ERROR: File Name Must Be Specified!");
    return 0;
  }

  if (this->DataSets.size() == 0)
  {
    vtkErrorMacro("ERROR: No Data Available!");
    return 0;
  }

  this->UpdateProgress(0);
  // get the info object
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  // get the ouptut
  vtkPointSet* output = vtkPointSet::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

  bool needToUpdateNames = false;
  // Check to see if the prefix or suffix has changed - the first check is to see if
  // both current and old are NULL, the second is to see if they have the strings
  if ((this->OldPrefix != this->Prefix) &&
    ((this->OldPrefix == NULL) || (strcmp(this->Prefix, this->OldPrefix) != 0)))
  {
    this->SetOldPrefix(this->Prefix);
    needToUpdateNames = true;
  }

  if ((this->OldSuffix != this->Suffix) &&
    ((this->OldSuffix == NULL) || (strcmp(this->Suffix, this->OldSuffix) != 0)))
  {
    this->SetOldSuffix(this->Suffix);
    needToUpdateNames = true;
  }

  std::vector<ADHTemporalData*>::iterator iter;
  if (needToUpdateNames && (this->DataSets.size() > 0))
  {
    std::string dname;
    if (this->Prefix)
    {
      dname += this->Prefix;
    }
    dname += (*this->DataSets.begin())->Name;
    if (this->Suffix)
    {
      dname += this->Suffix;
    }
    for (iter = this->DataSets.begin(); iter != this->DataSets.end(); iter++)
    {
      (*iter)->UpdateArrayNames(dname.c_str());
    }
  }
  output->DeepCopy(input);

  this->DataSet = 0;
  for (iter = this->DataSets.begin(); iter != this->DataSets.end(); iter++, this->DataSet++)
  {
    ADHTemporalData* currentDataSet = (*iter);
    if (outInfo->Has(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP()))
    {
      this->TimeValue = outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP());
    }

    //If we haven't read the data already
    if (currentDataSet->DataArray.find(this->TimeValue) == currentDataSet->DataArray.end())
    {
      ReadTemporalData();
    }
    //Keep memory footprint small
    if (currentDataSet->DataArray.size() > static_cast<size_t>(this->CacheSize))
    {
      std::map<double, vtkFloatArray*>::iterator toRemove = currentDataSet->DataArray.begin();
      if ((*toRemove).first == this->TimeValue)
      {
        for (int i = 0; i < this->CacheSize; i++)
        {
          toRemove++;
        }
      }
      (*toRemove).second->Delete();
      currentDataSet->DataArray.erase(toRemove);
    }
    if (currentDataSet->DataArray.find(this->TimeValue) != currentDataSet->DataArray.end())
    {
      vtkFloatArray* currentData = currentDataSet->DataArray[this->TimeValue];
      output->GetPointData()->AddArray(currentData);
    }
    else
    {
      vtkWarningMacro("WARNING: No data exists for time " << this->TimeValue << " In Dataset  \""
                                                          << currentDataSet->Name << "\"");
    }
  }
  this->DataSet = this->PrimaryDataSet;
  //Setup out info
  outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_STEPS(),
    this->DataSets[this->PrimaryDataSet]->TimeSteps,
    this->DataSets[this->PrimaryDataSet]->NumberOfTimeSteps);
  outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_RANGE(),
    this->DataSets[this->PrimaryDataSet]->TimeStepRange, 2);

  return 1;
}

//This is here because of a bug in vtkPointSetAlgorithm
//It can be removed whenever the ExecuteInformation function in
//vtkPointSetAlgorithm is changed to RequestInformation
int vtkCMBADHReader::ExecuteInformation(
  vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  return this->RequestInformation(request, inputVector, outputVector);
}

int vtkCMBADHReader::RequestInformation(vtkInformation* /*request*/,
  vtkInformationVector** /*inputVector*/, vtkInformationVector* outputVector)
{
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  if (this->FileName && (strcmp(this->FileName, "") != 0))
  {
    // Has the filename chnaged?
    if (!this->OldFileName || strcmp(this->FileName, this->OldFileName) != 0)
    {
      this->DataSets.clear();
      this->TimeValue = 0;
      this->DataSet = 0;
      if (!ScanFile())
      {
        vtkErrorMacro("ERROR: Invalid ADH File");
        return 0;
      }
      this->DataSet = 0;
      this->PrimaryDataSet = 0;
      this->SetOldFileName(this->FileName);
      this->SetOldPrefix(this->Prefix);
      this->SetOldSuffix(this->Suffix);
    }
  }
  if (this->DataSets.size() > 0)
  {
    outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_STEPS(),
      this->DataSets[this->PrimaryDataSet]->TimeSteps,
      this->DataSets[this->PrimaryDataSet]->NumberOfTimeSteps);
    outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_RANGE(),
      this->DataSets[this->PrimaryDataSet]->TimeStepRange, 2);
  }
  else
  {
    //Tell the pipeline there will be timesteps even though
    //What they are is not yet known
    double fakeTimeSteps[1] = { 0 };
    double fakeTimeStepRange[2] = { 0, 0 };
    outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_STEPS(), fakeTimeSteps, 1);
    outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_RANGE(), fakeTimeStepRange, 2);
  }
  return 1;
}

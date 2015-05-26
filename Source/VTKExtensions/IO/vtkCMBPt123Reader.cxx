//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkCMBPt123Reader.h"
#include "vtkCMBMeshReader.h"
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkDoubleArray.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkPolyData.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkUnstructuredGrid.h>
#include <vtksys/SystemTools.hxx>
#include <map>
#include <string>
#include <vtkNew.h>

#include <fstream>



// Helper class
class Pt123TemporalData
{
  public:
    Pt123TemporalData()
      {
      this->Filename = "";
      this->Name = "";
      this->NumDimensions = 0;
      this->NumPoints = 0;
      this->NumberOfTimeSteps = 0;
      this->TimeStepRange[0] = -1;
      this->TimeStepRange[1] = -1;
      this->TimeSteps = NULL;
      this->IsAScalar = true;
      }
    ~Pt123TemporalData()
      {
      std::map<double, vtkDoubleArray* >::iterator iter;
      if(this->DataArray.size() > 0)
        {
        for(iter = DataArray.begin(); iter != DataArray.end(); ++iter)
          {
          (*iter).second->Delete();
          }
        }
      DataArray.clear();
      if(TimeSteps)
        {
        delete[] TimeSteps;
        }
      }

    double *TimeSteps; //in seconds
    bool IsAScalar;
    std::string Name;
    std::string Filename;
    int NumberOfTimeSteps;
    int NumDimensions;
    int NumPoints;
    double TimeStepRange[2]; //In Frames
    std::map<double, std::streampos> tsToBytePosition;//map timevalue to the byte positino in the file for quick reading
    std::map<double,vtkDoubleArray* > DataArray;//map of timevalue to dataset
};
// -----------------------------------------------------------------------------

vtkStandardNewMacro(vtkCMBPt123Reader);

// -----------------------------------------------------------------------------
vtkCMBPt123Reader::vtkCMBPt123Reader()
{
  this->CacheSize = 100;
  this->FileName = 0;
  this->FileNamePath = 0;
  this->NemcData = 0;
  this->NumberOfTimeSteps = 0;
  this->OldFileName = 0;
  this->PrereadGeometry = 0;
  this->SetNumberOfInputPorts(0);
  this->TimeStepRange[0] = -1;
  this->TimeStepRange[1] = -1;
  this->TimeSteps = 0;
  this->VelData = 0;
}
// -----------------------------------------------------------------------------
vtkCMBPt123Reader::~vtkCMBPt123Reader()
{
  this->FileNamePath = 0;
  this->ResetAllData();
  this->SetFileName( 0 );
}
// -----------------------------------------------------------------------------
void vtkCMBPt123Reader::ResetAllData()
{
  this->SetOldFileName( 0 );
  if(VelData)
    {
    delete VelData;
    VelData = 0;
    }
  if(NemcData)
    {
    delete NemcData;
    NemcData = 0;
    }
  if(PrereadGeometry)
    {
    PrereadGeometry->Delete();
    }
  this->TimeSteps = 0;
  this->NumberOfTimeSteps = -1;
  this->TimeStepRange[0] = -1;
  this->TimeStepRange[1] = -1;
}
// -----------------------------------------------------------------------------
void vtkCMBPt123Reader::PrintSelf( ostream& os, vtkIndent indent )
{
  this->Superclass::PrintSelf( os, indent );
  os << indent << "FileName: " << ( this->FileName ? this->FileName : "(null)" ) << "\n" <<
     "CacheSize: " << CacheSize << "\n" <<
     "NumberOfTimeSteps: " << NumberOfTimeSteps << "\n";
}
// -----------------------------------------------------------------------------
vtkDoubleArray*
vtkCMBPt123Reader::GetDataAtTime(Pt123TemporalData *dat, double requestedTime)
{
  // Find the time span that contains this time
  double dataTime = requestedTime;
  int i;
  //If we have already read the data don't do it again
  if(dat->DataArray.find(dataTime) != dat->DataArray.end())
    {
    return dat->DataArray[dataTime];
    }
  //Read the data because it hasn't been done yet
  vtkDoubleArray *arr = vtkDoubleArray::New();
  arr->SetName(dat->Name.c_str());
  if (dat->IsAScalar)
    {
    arr->SetNumberOfComponents(1);
    }
  else
    {
    arr->SetNumberOfComponents(3);
    }

  arr->SetNumberOfTuples(dat->NumPoints);

  // Open the file and jump to the start of the time instance
  ifstream tf(dat->Filename.c_str());
  if(!tf)
    {
    vtkErrorMacro("Unable to open Temporal File: " << dat->Filename);
    return 0;
    }

  tf.seekg(dat->tsToBytePosition[dataTime]);
  double val[3];
  val[2] = 0.0; // in case of 2D
  for (i = 0; i < dat->NumPoints; i++)
    {
    if (dat->IsAScalar)
      {
      tf >> val[0];
      }
    // else if (dat->NumDimensions == 2)
    //   {
    //   tf >> val[0] >> val[1];
    //   }
    else
      {
      tf >> val[0] >> val[1] >> val[2];
      }
    arr->SetTuple(i, val);
    this->UpdateProgress(static_cast<double>(i)/static_cast<double>(dat->NumPoints));
    }
  tf.close();

  //Keep memory footprint small
  //Cache only 20 things at a time so see if we need to get rid of something
  // if so then get rid of the first thing in the map - Note a better thing
  // might be to get rid of the element that is furthest from the dataTime
  if(dat->DataArray.size() > static_cast<size_t>(this->CacheSize-1))
    {
    std::map<double,vtkDoubleArray* >::iterator toRemove =
        dat->DataArray.begin();
    (*toRemove).second->Delete();
    dat->DataArray.erase(toRemove);
    }
  // insert the new data
  dat->DataArray[dataTime] = arr;
  return arr;
}
// -----------------------------------------------------------------------------
// Scan the temporal data to make a note of where the useful information is so
// We can access it very quickly later in the program
int vtkCMBPt123Reader::ScanTemporalData(Pt123TemporalData* dat, const char* filename,
                                        bool isAScalar)
{
  //Open File
  ifstream tf(filename);
  if(!tf)
    {
    vtkErrorMacro("Unable to open Temporal File: " << filename << " for Scanning");
    return 0;
    }

  // Read in the header - number of points, dimensionality
  dat->Filename = filename;
  dat->IsAScalar = isAScalar;
  tf >> dat->NumPoints;
  if (isAScalar)
    {
    tf  >> dat->NumberOfTimeSteps;
    }
  else
    {
    tf  >> dat->NumDimensions >> dat->NumberOfTimeSteps;
    }

  if ((dat->NumPoints <= 0) || (dat->NumberOfTimeSteps <= 0))
    {
    vtkErrorMacro("Temporal File: " << filename << " has bad header");
    return 0;
    }

  dat->TimeSteps = new double[dat->NumberOfTimeSteps];

  tf.ignore(1000, '\n');
  int i, j;
  double val;
  std::string card;
  // Scan the file and note where each time step is
  for (i = 0; i < dat->NumberOfTimeSteps; i++)
    {
    tf >> card >> dat->TimeSteps[i];
    if (card != "TS")
      {
      vtkErrorMacro("Could not find Time Step: " << i << " for file: " << filename);
      return 0;
      }
    // Record where we are in the file
    dat->tsToBytePosition[dat->TimeSteps[i]] = tf.tellg();
    // See if we need to skip lines to get to the next TS
    if (i != (dat->NumberOfTimeSteps - 1))
      {
      // Get to the next line
      tf >> val;
      for (j = 0; j < dat->NumPoints; j++)
        {
        tf.ignore(1000, '\n');
        }
      }
    this->UpdateProgress(static_cast<double>(i)/static_cast<double>(dat->NumberOfTimeSteps));
    }
  dat->TimeStepRange[0] = dat->TimeSteps[0];
  dat->TimeStepRange[1] = dat->TimeSteps[dat->NumberOfTimeSteps-1];
  tf.close();
  return 1;
}
//-----------------------------------------------------------------------------
int vtkCMBPt123Reader::ReadPts2File(vtkPolyData* polyData,const char* filename)
{
  if(!polyData)
    {
    vtkErrorMacro("Please pass in a new vtkPolyData");
    return 0;
    }
  //Open File
  ifstream pf(filename);
  if(!pf)
    {
    vtkErrorMacro("Unable to open Points File: " << filename);
    return 0;
    }

  // Read in the RK Scheme and the element tracking
  int rkScheme, elementTracking;
  pf >> rkScheme >> elementTracking;
  pf.ignore(1000, '\n'); // Skip rest of line
  std::string card;
  vtkIdType numPnts, i;
  // Read in the number of points
  pf >> numPnts;
  pf.ignore(1000, '\n'); // skip rest of line

  vtkNew<vtkCellArray> verts;
  vtkNew<vtkPoints> points;
  vtkNew<vtkIntArray> globalIds;

  points->SetNumberOfPoints(numPnts);
  globalIds->SetNumberOfComponents(1);
  globalIds->SetNumberOfTuples(numPnts);
  globalIds->SetName("GlobalElementId");
  vtkIdType gid;
  double pnt[3];
  // In the case of ID or 2D problems the third coordinate is 0.0
  // So lets set it just in case
  pnt[2] = 0.0;
  for (i = 0; i < numPnts; i++)
    {
    pf >> gid >> pnt[0] >> pnt[1];
    if (this->SpaceDimension == 3)
      {
      pf >> pnt[2];
      }
    pf.ignore(1000, '\n'); // Skip rest of line
    points->SetPoint(i, pnt);
    globalIds->SetValue(i, gid);
    verts->InsertNextCell(1, &i);
    }
  pf.close();
  polyData->GetPointData()->SetScalars(globalIds.GetPointer());
  polyData->SetPoints(points.GetPointer());
  polyData->SetVerts(verts.GetPointer());
  return 1;
}

//-----------------------------------------------------------------------------
int vtkCMBPt123Reader::ReadBinaryStreams(vtkPolyData* polyData,const char* filename)
{
  ifstream file (filename, ios::in|ios::binary);
  char junk[4];
  int nPnts, nDims;
  int i, nLocs, j;
  int maxSize = 200;
  double *pnts, *timeSteps;
  vtkIdType *pids;
  pnts = new double[maxSize*3]; // Points are 3D reagardless of domain dimensionality
  timeSteps = new double[maxSize];
  pids = new vtkIdType[maxSize];

  vtkNew<vtkCellArray> lines;
  vtkNew<vtkPoints> points;
  vtkNew<vtkDoubleArray> timeData;
  vtkNew<vtkIdTypeArray> idData;
  points->SetDataTypeToDouble();
  timeData->SetNumberOfComponents(1);
  timeData->SetName("Time");
  idData->SetNumberOfComponents(1);
  idData->SetName("TraceIds");

  file.read(junk, 4);
  file.read(reinterpret_cast<char *>(&nPnts), sizeof(int));
  file.read(reinterpret_cast<char *>(&nDims), sizeof(int)); // Domain dimensions (1 : 1D, 2 : 2D, 3 : 3D)
  file.read(junk, 4);
  //cout << "NPts = " << nPnts << " NDims = " << nDims << "\n";
  vtkIdType nextPointId = 0;
  idData->SetNumberOfTuples(nPnts);
  for (i = 0; i < nPnts; i++)
    {
    file.read(junk, 4);
    file.read(reinterpret_cast<char *>(&nLocs), sizeof(int));
    file.read(junk, 4);
    idData->SetValue(i, i);
    //cout << "Particle = " << i << " NLocs = " << nLocs << "\n";
    //Lets see if we need to increase our arrays
    if (nLocs > maxSize)
      {
      maxSize = nLocs * 1.2; // Lets increase the arrays to be 20% larger than nLocs
      delete[] timeSteps;
      delete[] pnts;
      delete[] pids;
      pnts = new double[maxSize*3]; // Points are 3D regardless of domain dimensionality
      timeSteps = new double[maxSize];
      pids = new vtkIdType[maxSize];
      }
    file.read(junk, 4);
    file.read(reinterpret_cast<char *>(timeSteps), sizeof(double)*nLocs);
    file.read(junk, 4);
    file.read(junk, 4);
    //file.read(reinterpret_cast<char *>(pnts), sizeof(double)*nLocs*nDims);
    file.read(reinterpret_cast<char *>(pnts), sizeof(double)*nLocs*3); // Always read X,Y,Z
    file.read(junk, 4);

    // Adding the time information is straight forward though the point information is based
    // on FORTRAN arrys
    for (j = 0; j < nLocs; j++)
      {
      timeData->InsertNextValue(timeSteps[j]);
      pids[j] = nextPointId++;
      //if (nDims == 3)
      //  {
      //  points->InsertNextPoint(pnts[j], pnts[j+nLocs], pnts[j+(2*nLocs)]);
      //  }
      //else
      //  {
      //  points->InsertNextPoint(pnts[j], pnts[j+nLocs], 0.0);
      //  }
      points->InsertNextPoint(pnts[j], pnts[j+nLocs], pnts[j+(2*nLocs)]);
      }
    // Insert the trace
    lines->InsertNextCell(nLocs, pids);
    }
  polyData->GetPointData()->AddArray(timeData.GetPointer());
  polyData->GetCellData()->AddArray(idData.GetPointer());
  polyData->GetPointData()->SetActiveScalars(timeData->GetName());
  polyData->GetCellData()->SetActiveScalars(idData->GetName());
  polyData->SetLines(lines.GetPointer());
  polyData->SetPoints(points.GetPointer());
  file.close();
  delete[] timeSteps;
  delete[] pnts;
  delete[] pids;
  return 1;
}

// -----------------------------------------------------------------------------
int vtkCMBPt123Reader::ReadSUPFile(const char* filename)
{
  //Open File
  ifstream supStream(filename);
  if(!supStream)
    {
    vtkErrorMacro("Unable to open file: " << filename);
    return 1;
    }
  if(!PrereadGeometry)
    {
    PrereadGeometry = vtkMultiBlockDataSet::New();
    }
  std::string line, cardType;
  // Read in the dimension space
  supStream >> this->SpaceDimension;
  supStream.ignore(1000, '\n'); // Skip rest of line
  // Read in Velocity Info
  supStream >> this->TransientVelocity;
  supStream >> this->VelocityFormat;
  supStream.ignore(1000, '\n'); // Skip rest of line
  // Read in Margin and Open Boundary Info - for the
  // time being we are skipping these
  supStream.ignore(1000, '\n'); // Skip rest of line
  supStream.ignore(1000, '\n'); // Skip rest of line

  // We need something to help join the file path to
  // filenames back into a full path
  std::vector<std::string> fileParts(2);
  fileParts[0] = this->FileNamePath;
  // BUG - Join should probably add the needed / but it doesnt
  fileParts[0]+="/";
  // Process the rest of the file which should be
  // card based with associated filenames
  while(supStream  >> cardType >> fileParts[1])
    {
    // All of the remaining cards have filenames associated with
    // them so lets create the full path
    std::string filepath = vtksys::SystemTools::JoinPath(fileParts);

    if (cardType == "GEOM")
      {
      vtkNew<vtkCMBMeshReader> meshReader;
      meshReader->SetFileName(filepath.c_str());
      meshReader->Update();
      vtkSmartPointer<vtkUnstructuredGrid> geom = meshReader->GetOutput();
      PrereadGeometry->SetBlock(0,geom);
      continue;
      }

    if ((cardType == "PTS2") || (cardType == "PTSP"))
      {
      vtkNew<vtkPolyData> pd;
      ReadPts2File(pd.GetPointer(),filepath.c_str());
      PrereadGeometry->SetBlock(1,pd.GetPointer());
      continue;
      }

    if (cardType == "VNAS")
      {
      if(!this->VelData)
        {
        this->VelData = new Pt123TemporalData();
        }
      this->VelData->Name = "Velocity";
      this->ScanTemporalData(this->VelData,filepath.c_str(), false);
      continue;
      }

    if (cardType == "NEMA")
      {
      if(!this->NemcData)
        {
        this->NemcData = new Pt123TemporalData();
        }
      this->NemcData->Name = "Moisture";
      this->ScanTemporalData(this->NemcData,filepath.c_str(), true);
      continue;
      }

    if (cardType == "SAPT")
      {
      continue;
      }

    if (cardType == "SBPT" || cardType == "SBP2")
      {
      vtkNew<vtkPolyData> polydata;
      this->ReadBinaryStreams(polydata.GetPointer(), filepath.c_str());
      PrereadGeometry->SetBlock(2,polydata.GetPointer());
      continue;
      }
    }
  supStream.close();
  return 1;
}
// -----------------------------------------------------------------------------
int vtkCMBPt123Reader::UpdateTimeData(Pt123TemporalData* dat, double timeValue)
{
  if(!dat)
    {
    return 0;
    }
  vtkDoubleArray *arr = this->GetDataAtTime(dat,timeValue);
  if(!arr)
    {
    vtkErrorMacro("Timestep doesn't exist in "+dat->Filename);
    return 0;
    }
  vtkUnstructuredGrid* ug = vtkUnstructuredGrid::SafeDownCast(PrereadGeometry->GetBlock(0));
  if(ug->GetPointData()->HasArray(arr->GetName()))
    {
    ug->GetPointData()->RemoveArray(arr->GetName());
    }
  ug->GetPointData()->AddArray(arr);
  this->TimeSteps = dat->TimeSteps;
  this->TimeStepRange[0] = dat->TimeStepRange[0];
  this->TimeStepRange[1] = dat->TimeStepRange[1];
  this->NumberOfTimeSteps = dat->NumberOfTimeSteps;
  return 1;

}
// -----------------------------------------------------------------------------
int vtkCMBPt123Reader::RequestData(
    vtkInformation* /*request*/, vtkInformationVector** /*inputVector*/, vtkInformationVector* outputVector )
{
  // get the info object
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  this->UpdateProgress(0);
  if ( ! this->FileName )
    {
    return 0;
    }
  std::string fnp = vtksys::SystemTools::GetFilenamePath(this->FileName);
  this->FileNamePath = fnp.c_str();
  vtkMultiBlockDataSet* output = vtkMultiBlockDataSet::SafeDownCast(
      outputVector->GetInformationObject( 0 )->Get(vtkDataObject::DATA_OBJECT()));
  if ( ! output )
    {
    return 0;
    }
  //Redo everything if then name has changed
  if(!this->OldFileName || strcmp(this->FileName, this->OldFileName) != 0)
    {
    this->ResetAllData();
    if(!this->ReadSUPFile(this->FileName))
      {
      return 0;
      }
    this->SetOldFileName(this->FileName);
    }
  if(outInfo->Has(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP()))
    {
    double requestedTimeStep =
      outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_TIME_STEP());
    UpdateTimeData(VelData ,requestedTimeStep);
    UpdateTimeData(NemcData,requestedTimeStep);
    }
  outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_STEPS(),
      this->TimeSteps,
      this->NumberOfTimeSteps);
  outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_RANGE(),
      this->TimeStepRange,2);

  if(!this->PrereadGeometry)
    {
    return 0;
    }
  output->DeepCopy(this->PrereadGeometry);

  return 1 ;
}
// -----------------------------------------------------------------------------
int vtkCMBPt123Reader::RequestInformation(
    vtkInformation* /*request*/, vtkInformationVector** /*inputVector*/, vtkInformationVector* outputVector )
{
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  if(this->TimeSteps)
    {
    outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_STEPS(),
        this->TimeSteps,
        this->NumberOfTimeSteps);
    outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_RANGE(),
        this->TimeStepRange,2);
    }
  else
    {
    //Tell the pipeline that this class has time steps even
    //though we don't know what they are yet
    double fakeTimeSteps[1] = {0};
    double fakeTimeStepRange[2] = {0,0};
    outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_STEPS(),
        fakeTimeSteps,1);
    outInfo->Set(vtkStreamingDemandDrivenPipeline::TIME_RANGE(),
        fakeTimeStepRange,2);
    }
  if (!this->FileName)
    {
    vtkErrorMacro("FileName has to be specified!");
    return 0;
    }
  return 1;
}

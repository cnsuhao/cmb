//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkLIDARMultiFilesReader.h"

#include "vtkAppendPolyData.h"
#include "vtkCellArray.h"
#include "vtkPointData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkLASReader.h"
#include "vtkLIDARReader.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkStringArray.h"
#include "vtkTransform.h"
#include "vtkWeakPointer.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <vtksys/SystemTools.hxx>



class vtkLIDARMultiFilesReader::InternalLIDARFileInfo
{
public:
  InternalLIDARFileInfo(const char* filename)
    {
    this->FileName = filename;
    this->CreateReader();
    }
  ~InternalLIDARFileInfo()
    {
    this->FileName = "";
    if(this->Reader)
      {
      this->Reader->Delete();
      this->Reader = NULL;
      }
    this->SetTransform(static_cast<vtkTransform*>(0));
    }
  void SetTransform(vtkTransform* inTransform)
    {
    if(this->LIDARReader)
      {
      this->LIDARReader->SetTransform(this->Transform);
      }
    if(this->LASReader)
      {
      //this->LASReader->SetTransform(this->Transform);
      }
    }
  vtkTransform* GetTransform()
    {
    if(this->LIDARReader)
      {
      return this->LIDARReader->GetTransform();
      }
    else if(this->LASReader)
      {
      //this->LASReader->GetTransform();
      }
    return NULL;
    }
  void SetConvertFromLatLongToXYZ(bool mode)
    {
    if(this->LIDARReader)
      {
      this->LIDARReader->SetConvertFromLatLongToXYZ(mode);
      }
    if(this->LASReader)
      {
      this->LASReader->SetConvertFromLatLongToXYZ(mode);
      }
    }
  bool GetConvertFromLatLongToXYZ()
    {
    return this->ConvertFromLatLongToXYZ;
    }

  vtkAlgorithm* GetReader()
    {
    if(!this->Reader)
      {
      this->CreateReader();
      }
    return this->Reader;
    }

  std::string FileName;
  vtkWeakPointer<vtkLIDARReader> LIDARReader;
  vtkWeakPointer<vtkLASReader> LASReader;

protected:
  void CreateReader()
    {
    if (this->FileName.find("bin.pts") != std::string::npos ||
      this->FileName.find(".bin") != std::string::npos)
      {
      this->Reader = vtkLIDARReader::New();
      this->LIDARReader = this->Reader;
      this->LIDARReader->SetFileName(this->FileName.c_str());
      }
    else if(this->FileName.find(".las")!= std::string::npos)
      {
      this->Reader = vtkLASReader::New();
      this->LASReader = this->Reader;
      this->LASReader->SetFileName(this->FileName.c_str());
      }
    }

  vtkAlgorithm* Reader;
  bool ConvertFromLatLongToXYZ;
  vtkTransform *Transform;
};

vtkStandardNewMacro(vtkLIDARMultiFilesReader);

//-----------------------------------------------------------------------------
vtkLIDARMultiFilesReader::vtkLIDARMultiFilesReader()
{
  this->SetNumberOfInputPorts(0);
  this->Initialize();
}

//-----------------------------------------------------------------------------
vtkLIDARMultiFilesReader::~vtkLIDARMultiFilesReader()
{
  this->SetCurrentFileName(0);
}

//-----------------------------------------------------------------------------
void vtkLIDARMultiFilesReader::Initialize()
{
  this->RemoveAllFileNames();
  this->DataBounds[0] = this->DataBounds[2] = this->DataBounds[4] = VTK_DOUBLE_MAX;
  this->DataBounds[1] = this->DataBounds[3] = this->DataBounds[5] = VTK_DOUBLE_MIN;
  this->RequestedReadFilePieces.clear();
  this->LimitReadToBounds = false;
  this->ReadBounds[0] = this->ReadBounds[2] = this->ReadBounds[4] = VTK_DOUBLE_MAX;
  this->ReadBounds[1] = this->ReadBounds[3] = this->ReadBounds[5] = VTK_DOUBLE_MIN;
  this->TransformOutputData = false;

  this->MaxNumberOfPoints = 1000000;
  this->LimitToMaxNumberOfPoints = false;

  this->OutputDataTypeIsDouble = false;
  this->CurrentFileName = 0;
}

//-----------------------------------------------------------------------------
void vtkLIDARMultiFilesReader::AddFileName(const char* fname)
{
  if(this->LIDARFiles.find(fname) == this->LIDARFiles.end())
    {
    InternalLIDARFileInfo* lidarFileObj = new InternalLIDARFileInfo(fname);
    lidarFileObj->FileName = fname;
    this->LIDARFiles[fname] = lidarFileObj;
    }
}
//-----------------------------------------------------------------------------
void vtkLIDARMultiFilesReader::RemoveAllFileNames()
{
  std::map<std::string, InternalLIDARFileInfo*>::iterator it=
    this->LIDARFiles.begin();
  while(it != this->LIDARFiles.end())
    {
    if(it->second)
      {
      delete it->second;
      }
    ++it;
    }
  this->LIDARFiles.clear();
}
//-----------------------------------------------------------------------------
const char* vtkLIDARMultiFilesReader::GetFileName(unsigned int idx)
{
  std::map<std::string, InternalLIDARFileInfo*>::iterator it=
    this->LIDARFiles.begin();
  std::advance( it, idx );
  return (it!= this->LIDARFiles.end() && it->second) ?
    it->second->FileName.c_str() : NULL;
}

//-----------------------------------------------------------------------------
unsigned int vtkLIDARMultiFilesReader::GetNumberOfFileNames()
{
  return static_cast<unsigned int>(this->LIDARFiles.size());
}

//-----------------------------------------------------------------------------
void vtkLIDARMultiFilesReader::AddRequestedPieceForRead(
  const char* filename,int pieceIdx, int onRatio)
{
  if(filename && pieceIdx >= 0 && onRatio > 0)
    {
    this->RequestedReadFilePieces[filename].push_back(
      std::make_pair(pieceIdx, onRatio));
    this->Modified();
    }
}

//-----------------------------------------------------------------------------
void vtkLIDARMultiFilesReader::RemoveAllRequestedReadPieces()
{
  std::map<std::string, InternalLIDARFileInfo*>::iterator it=
  this->LIDARFiles.begin();
  while(it != this->LIDARFiles.end())
    {
    if(it->second->LIDARReader)
      {
      it->second->LIDARReader->RemoveAllRequestedReadPieces();
      }
    ++it;
    }
  this->RequestedReadFilePieces.clear();
  this->Modified();
}
//-----------------------------------------------------------------------------
int vtkLIDARMultiFilesReader::GetKnownNumberOfPieces(
  const char* filename)
{
  if(filename && this->LIDARFiles.find(filename)!=
    this->LIDARFiles.end())
    {
    return this->LIDARFiles[filename]->LIDARReader->
      GetKnownNumberOfPieces();
    }
  return 0;
}
//-----------------------------------------------------------------------------
int vtkLIDARMultiFilesReader::GetKnownNumberOfPieces()
{
  std::map<std::string, InternalLIDARFileInfo*>::iterator it=
  this->LIDARFiles.begin();
  int total = 0;
  while(it != this->LIDARFiles.end())
    {
    total += this->GetKnownNumberOfPieces(it->first.c_str());
    ++it;
    }
  return total;
}
//-----------------------------------------------------------------------------
vtkIdType vtkLIDARMultiFilesReader::GetNumberOfPointsInPiece(
  const char* filename, int pieceIndex)
{
  if(filename && this->LIDARFiles.find(filename)!=
     this->LIDARFiles.end())
    {
    return this->LIDARFiles[filename]->LIDARReader->
      GetNumberOfPointsInPiece(pieceIndex);
    }
  return 0;
}

//-----------------------------------------------------------------------------
vtkIdType vtkLIDARMultiFilesReader::GetTotalNumberOfPoints(const char* filename)
{
  if(filename && this->LIDARFiles.find(filename)!=
    this->LIDARFiles.end())
    {
    return this->LIDARFiles[filename]->LIDARReader->
      GetTotalNumberOfPoints();
    }
  return 0;
}

//-----------------------------------------------------------------------------
vtkIdType vtkLIDARMultiFilesReader::GetTotalNumberOfPoints()
{
  std::map<std::string, InternalLIDARFileInfo*>::iterator it=
  this->LIDARFiles.begin();
  int total = 0;
  while(it != this->LIDARFiles.end())
    {
    total += this->GetTotalNumberOfPoints(it->first.c_str());
    ++it;
    }
  return total;
}

//-----------------------------------------------------------------------------
vtkIdType vtkLIDARMultiFilesReader::GetRealNumberOfOutputPoints(const char* filename)
{
  if(filename && this->LIDARFiles.find(filename)!=
    this->LIDARFiles.end())
    {
    return this->LIDARFiles[filename]->LIDARReader->
      GetRealNumberOfOutputPoints();
    }
  return 0;
}

//-----------------------------------------------------------------------------
vtkIdType vtkLIDARMultiFilesReader::GetRealNumberOfOutputPoints()
{
  std::map<std::string, InternalLIDARFileInfo*>::iterator it=
    this->LIDARFiles.begin();
  int total = 0;
  while(it != this->LIDARFiles.end())
    {
    total += this->GetRealNumberOfOutputPoints(it->first.c_str());
    ++it;
    }
  return total;
}

//-----------------------------------------------------------------------------
void vtkLIDARMultiFilesReader::SetTransformToAll(double elements[16])
{
  vtkTransform *tmpTransform = vtkTransform::New();
  tmpTransform->SetMatrix(elements);
  this->SetTransformToAll(tmpTransform);
  tmpTransform->Delete();
}

//-----------------------------------------------------------------------------
void vtkLIDARMultiFilesReader::SetTransformToAll(vtkTransform *inTransform)
{
  std::map<std::string, InternalLIDARFileInfo*>::iterator it=
    this->LIDARFiles.begin();
  while(it != this->LIDARFiles.end())
    {
    this->SetTransform(it->first.c_str(), inTransform);
    ++it;
    }
}

//-----------------------------------------------------------------------------
vtkTransform* vtkLIDARMultiFilesReader::GetTransform()
{
  std::map<std::string, InternalLIDARFileInfo*>::iterator it=
    this->LIDARFiles.begin();
  if(it != this->LIDARFiles.end())
    {
    return this->GetTransform(it->first.c_str());
    }
  return NULL;
}

//-----------------------------------------------------------------------------
void vtkLIDARMultiFilesReader::SetTransform(
  const char* filename,vtkTransform *transform)
{
  if(this->LIDARFiles.find(filename) != this->LIDARFiles.end())
    {
    this->LIDARFiles[filename]->SetTransform(transform);
    this->Modified();
    }
}

//-----------------------------------------------------------------------------
void vtkLIDARMultiFilesReader::SetTransform(
  const char* filename, double elements[16])
{
  if(this->LIDARFiles.find(filename) != this->LIDARFiles.end())
    {
    vtkTransform *tmpTransform = vtkTransform::New();
    tmpTransform->SetMatrix(elements);
    this->SetTransform(filename, tmpTransform);
    tmpTransform->Delete();
    }
}

//-----------------------------------------------------------------------------
vtkTransform* vtkLIDARMultiFilesReader::GetTransform(const char* filename)
{
  if(this->LIDARFiles.find(filename) != this->LIDARFiles.end())
    {
    return this->LIDARFiles[filename]->GetTransform();
    }
  return NULL;
}

//-----------------------------------------------------------------------------
void vtkLIDARMultiFilesReader::SetConvertFromLatLongToXYZToAll(bool mode)
{
  std::map<std::string, InternalLIDARFileInfo*>::iterator it=
  this->LIDARFiles.begin();
  while(it != this->LIDARFiles.end())
    {
    this->SetConvertFromLatLongToXYZ(it->first.c_str(), mode);
    ++it;
    }
}

//-----------------------------------------------------------------------------
bool vtkLIDARMultiFilesReader::GetConvertFromLatLongToXYZ()
{
  std::map<std::string, InternalLIDARFileInfo*>::iterator it=
    this->LIDARFiles.begin();
  if(it != this->LIDARFiles.end())
    {
    return this->GetConvertFromLatLongToXYZ(it->first.c_str());
    }
  return false;
}

//-----------------------------------------------------------------------------
void vtkLIDARMultiFilesReader::SetConvertFromLatLongToXYZ(
  const char* filename, bool mode)
{
  if(this->LIDARFiles.find(filename) != this->LIDARFiles.end())
    {
    this->LIDARFiles[filename]->SetConvertFromLatLongToXYZ(mode);
    this->Modified();
    }
}
//-----------------------------------------------------------------------------
bool vtkLIDARMultiFilesReader::GetConvertFromLatLongToXYZ(
  const char* filename)
{
  if(this->LIDARFiles.find(filename) != this->LIDARFiles.end())
    {
    return this->LIDARFiles[filename]->GetConvertFromLatLongToXYZ();
    }
  return false;
}

//-----------------------------------------------------------------------------
int vtkLIDARMultiFilesReader::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{
  int res = this->ReadFilesInfo();
  if(res == vtkLIDARReader::READ_ERROR)
    {
    this->Initialize();
    return 0;
    }
  else if(res == vtkLIDARReader::READ_ABORT)
    {
    this->Initialize();
    return 1;
    }
  if(this->GetTotalNumberOfPoints()==0)
    {
    vtkErrorMacro(<<"There is no data found in the files.");
    this->Initialize();
    return 0;
    }

  // get the info object
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the outtut
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkBoundingBox bbox;
  int readresult= vtkLIDARReader::READ_OK;
  vtkAppendPolyData* appendPoly=NULL;
  if(this->RequestedReadFilePieces.size()==0)// All pieces
    {
    if(this->LIDARFiles.size()>1)
      {
      appendPoly = vtkAppendPolyData::New();
      }
    std::map<std::string, InternalLIDARFileInfo*>::iterator it=
      this->LIDARFiles.begin();
    while(it != this->LIDARFiles.end())
      {
      if(!(it->second && it->second->GetReader()))
        {
        vtkWarningMacro(<<"No Reader created for " << it->first);
        ++it;
        continue;
        }
      readresult = vtkLIDARReader::READ_OK;

      if(it->second->LIDARReader)
        {
        this->InitLIDARReader(it->second->LIDARReader);
        it->second->LIDARReader->Update();
        if(it->second->LIDARReader->GetErrorCode())
          {
          vtkWarningMacro(<<"Error while reading file: " << it->first);
          continue;
          }
        }
      else if(it->second->LASReader)
        {
        //readresult = it->second->LASReader->ReadFileInfo();
        }
      bbox.AddBounds(it->second->LIDARReader->GetDataBounds());
      if(appendPoly)
        {
        appendPoly->AddInputConnection(
          it->second->GetReader()->GetOutputPort(0));
        }
      else
        {
        output->ShallowCopy(it->second->GetReader()->GetOutputDataObject(0));
        }
      ++it;
      }
    }
  else // single pieces
    {
    if(this->RequestedReadFilePieces.size()>1)
      {
      appendPoly = vtkAppendPolyData::New();
      }
    std::map< std::string, std::vector< std::pair<int, int> > >::iterator it;
    for(it = this->RequestedReadFilePieces.begin();
      it != this->RequestedReadFilePieces.end(); it++)
      {
      if(this->LIDARFiles.find(it->first)!=this->LIDARFiles.end())
        {
        this->InitLIDARReader(this->LIDARFiles[it->first]->LIDARReader);
        for(std::vector< std::pair<int, int> >::iterator vit=
          it->second.begin();
          vit != it->second.end(); vit++)
          {
          this->LIDARFiles[it->first]->LIDARReader->AddRequestedPieceForRead(
            (*vit).first,(*vit).second);
          }
        this->LIDARFiles[it->first]->LIDARReader->Update();
        if(this->LIDARFiles[it->first]->LIDARReader->GetErrorCode())
          {
          vtkWarningMacro(<<"Error while reading file: " << it->first);
          continue;
          }
        }
      else
        {
        vtkWarningMacro(<<"Can't find file info for: " << it->first);
        continue;
        }
      bbox.AddBounds(this->LIDARFiles[it->first]->LIDARReader->GetDataBounds());
      if(appendPoly)
        {
        appendPoly->AddInputConnection(
          this->LIDARFiles[it->first]->GetReader()->GetOutputPort(0));
        }
      else
        {
        output->ShallowCopy(this->LIDARFiles[it->first]->GetReader()->
          GetOutputDataObject(0));
        }
      }
    }

  if(appendPoly)
    {
    appendPoly->Update();
    output->ShallowCopy(appendPoly->GetOutput(0));
    appendPoly->Delete();
    }
  this->UpdateProgress( 1.0 );
  bbox.GetBounds(this->DataBounds);
  return 1;
}

//-----------------------------------------------------------------------------
void vtkLIDARMultiFilesReader::InitLIDARReader(vtkLIDARReader* lidarReader)
{
  lidarReader->SetLimitReadToBounds(this->LimitReadToBounds);
  lidarReader->SetReadBounds(this->ReadBounds);
  lidarReader->SetLimitToMaxNumberOfPoints(this->LimitToMaxNumberOfPoints);
  lidarReader->SetOutputDataTypeIsDouble(this->OutputDataTypeIsDouble);
  lidarReader->SetTransformOutputData(this->TransformOutputData);
  vtkIdType totPts = this->GetTotalNumberOfPoints();
  double ratio = static_cast<double>(lidarReader->GetTotalNumberOfPoints())/static_cast<double>(totPts);
  if(this->LimitToMaxNumberOfPoints)
    {
    lidarReader->SetMaxNumberOfPoints(this->MaxNumberOfPoints*ratio);
    }
}

//-----------------------------------------------------------------------------
vtkIdType vtkLIDARMultiFilesReader::GetEstimatedNumOfOutPoints()
{
  vtkIdType numOutputPts = 0;
  std::map<std::string, InternalLIDARFileInfo*>::iterator it=
    this->LIDARFiles.begin();
  while(it != this->LIDARFiles.end())
    {
    if(it->second->LIDARReader)
      {
      numOutputPts = it->second->LIDARReader->
        GetEstimatedNumOfOutPoints();
      }
    ++it;
    }
  return numOutputPts;
}

//-----------------------------------------------------------------------------
int vtkLIDARMultiFilesReader::ReadFilesInfo()
{
  if(this->LIDARFiles.size()==0)
    {
    vtkErrorMacro(<< "There are no files set for the reader.");
    this->Initialize();
    return vtkLIDARReader::READ_ERROR;
    }

  std::map<std::string, InternalLIDARFileInfo*>::iterator it=
    this->LIDARFiles.begin();
  int readresult= vtkLIDARReader::READ_OK;
  while(it != this->LIDARFiles.end())
    {
    if(!it->second)
      {
      vtkWarningMacro(<<"No File info found for " << it->first);
      ++it;
      continue;
      }
    if(!(it->second->GetReader()))
      {
      vtkWarningMacro(<<"No Reader created for " << it->first);
      ++it;
      continue;
      }
    readresult = vtkLIDARReader::READ_OK;

    if(it->second->LIDARReader)
      {
      readresult = it->second->LIDARReader->ReadFileInfo();
      }
    else if(it->second->LASReader)
      {
      //readresult = it->second->LASReader->ReadFileInfo();
      }
    if(readresult != vtkLIDARReader::READ_OK)
      {
      if(readresult == vtkLIDARReader::READ_ERROR)
        {
        vtkWarningMacro(<<"Error while reader file: " << it->first);
        }
      break;
      }
    ++it;
    }
  return readresult;
}

//----------------------------------------------------------------------------
int vtkLIDARMultiFilesReader::RequestInformation(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *vtkNotUsed(outputVector))
{
  if (this->LIDARFiles.size()==0)
    {
    vtkErrorMacro("FileNames has to be specified!");
    return 0;
    }

  return 1;
}

//-----------------------------------------------------------------------------
void vtkLIDARMultiFilesReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Current File Name: "
    << (this->CurrentFileName ? this->CurrentFileName : "(none)") << "\n";
  os << indent << "Limit Read To Bounds: " <<
    (this->LimitReadToBounds ? "On" : "Off");
  os << indent << "MaxNumberOfPoints: " << this->MaxNumberOfPoints;
  os << indent << "ReadBounds: " << this->ReadBounds;
  os << indent << "DataBounds: " << this->DataBounds;
  os << indent << "LimitToMaxNumberOfPoints: " <<
    (this->LimitToMaxNumberOfPoints ? "On" : "Off");
  os << indent << "OutputDataTypeIsDouble: " <<
    (this->OutputDataTypeIsDouble ? "On" : "Off");
  os << indent << "TransformOutputData: " <<
    (this->TransformOutputData ? "On" : "Off");
}

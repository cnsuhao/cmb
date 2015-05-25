//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkADHHotStartWriter.h"

#include "vtkInformation.h"
#include "vtkObjectFactory.h"
#include "vtkPointSet.h"
#include "vtkDataArray.h"
#include "vtkPointData.h"
#include "vtkCellType.h"

#include <sstream>
#include <vtksys/SystemTools.hxx>
#include <vector>

#define HOTSTART_SEPARATOR "  "

vtkStandardNewMacro(vtkADHHotStartWriter);

//-----------------------------------------------------------------------------
struct vtkADHHotStartWriterInternal
{
public:
  vtkADHHotStartWriterInternal() {}
  ~vtkADHHotStartWriterInternal()
  {  this->OutputArrays.clear(); }

  std::vector<std::string> OutputArrays;
};

//-----------------------------------------------------------------------------
vtkADHHotStartWriter::vtkADHHotStartWriter()
{
  this->SetNumberOfInputConnections(0, 1);
  this->Implementation = new vtkADHHotStartWriterInternal;
  this->FileName = 0;
}

//-----------------------------------------------------------------------------
vtkADHHotStartWriter::~vtkADHHotStartWriter()
{
  if(this->Implementation)
    {
    delete this->Implementation;
    this->Implementation = 0;
    }
}
//----------------------------------------------------------------------------
int vtkADHHotStartWriter::FillInputPortInformation(int,
  vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataObject");
  return 1;
}

//----------------------------------------------------------------------------
ostream* vtkADHHotStartWriter::OpenFile()
{
  if (!this->FileName || !this->FileName[0])
    {
    vtkErrorMacro("FileName has to be specified.");
    return NULL;
    }

  ostream* fp = new ofstream(this->FileName, ios::out);
  if (fp->fail())
    {
    vtkErrorMacro(<< "Unable to open file: "<< this->FileName);
    delete fp;
    return NULL;
    }
  return fp;
}

//----------------------------------------------------------------------------
void vtkADHHotStartWriter::CloseFile(ostream* fp)
{
  if (fp)
    {
    delete fp;
    fp = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkADHHotStartWriter::WriteData()
{
  size_t noOfArrays = this->Implementation->OutputArrays.size();
  if(noOfArrays<1)
    {
    vtkErrorMacro("No input arrays to the writer.");
    return;
    }
  ostream* file = this->OpenFile();
  if (!file)
    {
    vtkErrorMacro("Can't open file to write." << this->FileName);
    }
  if (!this->WriteArrays(*file))
    {
    vtkErrorMacro("Write failed");
    }

  this->CloseFile(file);
}

//-----------------------------------------------------------------------------
void vtkADHHotStartWriter::AddInputPointArrayToProcess(const char* name)
{
  this->Implementation->OutputArrays.push_back(std::string(name));
  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkADHHotStartWriter::ClearInputPointArrayToProcess()
{
  this->Implementation->OutputArrays.clear();
  this->Modified();
}

//----------------------------------------------------------------------------
bool vtkADHHotStartWriter::WriteArrays(ostream& fp)
{
  vtkDataObject* input = this->GetInput();
  vtkPointSet* inMesh= vtkPointSet::SafeDownCast(input);
  size_t noOfArrays = this->Implementation->OutputArrays.size();
  for(size_t i=0; i < noOfArrays; ++i)
    {
    //Get the data array to be processed
    vtkDataArray* inArray = inMesh->GetPointData()->GetArray(
      this->Implementation->OutputArrays[i].c_str());
    if (!inArray)
      {
      vtkErrorMacro("Could not find the Array to process, "<<
        this->Implementation->OutputArrays[i].c_str());
      return false;
      }
      if(!this->WriteArrayHeader(fp, inArray) ||
      !this->WriteArray(fp, inArray) ||
      !this->WriteArrayFooter(fp))
      {
      vtkErrorMacro("Error writing the Array, "<<
        this->Implementation->OutputArrays[i].c_str());
      return false;
      }
    }
  return true;
}
//----------------------------------------------------------------------------
bool vtkADHHotStartWriter::WriteArray(ostream& fp, vtkDataArray* darray)
{
  vtkIdType i, n = darray->GetNumberOfTuples();
  int k=darray->GetNumberOfComponents();
  double* v;
  if(k>1)
    {
    for (i = 0; i < n; i++)
      {
      v = darray->GetTuple(i);
      for(int j=0; j<k-1; j++)
        {
        fp << v[j] << HOTSTART_SEPARATOR;
        }
      fp << v[k-1] << endl;
      }
    }
  else
    {
    for (i = 0; i < n; i++)
      {
      v = darray->GetTuple(i);
      fp << v[0] << endl;
      }
    }
  return true;
}
//----------------------------------------------------------------------------
bool vtkADHHotStartWriter::WriteArrayHeader(ostream& fp, vtkDataArray* darray)
{
  vtkDataObject* input = this->GetInput();
  vtkPointSet* inMesh= vtkPointSet::SafeDownCast(input);
  int cellType = inMesh->GetCellType(0);
  const char* meshDim = cellType==VTK_TRIANGLE ? "\"mesh2d\"" : "\"mesh3d\"";
  fp << "DATASET" << endl;
  fp << "OBJTYPE" << HOTSTART_SEPARATOR << meshDim << endl;
  if(darray->GetNumberOfComponents()>1)
    {
    fp << "BEGVEC" << endl;
    }
  else
    {
    fp << "BEGSCL" << endl;
    }
  fp << "ND" << HOTSTART_SEPARATOR << inMesh->GetNumberOfPoints() << endl;
  fp << "NC" << HOTSTART_SEPARATOR << inMesh->GetNumberOfCells() << endl;
  fp << "NAME" << HOTSTART_SEPARATOR << darray->GetName() << endl;
  fp << "TS 0 0.00000000e+00" << endl;

  return true;
}
//----------------------------------------------------------------------------
bool vtkADHHotStartWriter::WriteArrayFooter(ostream& fp)
{
  fp << "ENDDS" << endl;
  return true;
}

//-----------------------------------------------------------------------------
void vtkADHHotStartWriter::PrintSelf(ostream &os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "File Name: " <<
    (this->FileName ? this->FileName : "(none)") << endl;
}

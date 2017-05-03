//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkCMBSceneV2WriterHelper.h"

#include "vtkErrorCode.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"

#
#include <sys/stat.h>
#include <sys/types.h>
#include <vtksys/SystemTools.hxx>

vtkStandardNewMacro(vtkCMBSceneV2WriterHelper);

//-----------------------------------------------------------------------------
// The internals class mostly holds templated ivars that we don't want to
// expose in the header file.
class vtkCMBSceneV2WriterHelper::vtkSceneGenInternal
{
public:
  std::vector<vtkStdString> ObjectFileNames;
};
//-----------------------------------------------------------------------------
vtkCMBSceneV2WriterHelper::vtkCMBSceneV2WriterHelper()
{
  this->SetNumberOfInputPorts(0);

  this->FileName = 0;
  this->Text = 0;
  this->Mode = 0;
  this->PackageScene = 0;

  this->Internal = new vtkCMBSceneV2WriterHelper::vtkSceneGenInternal();
}

//-----------------------------------------------------------------------------
vtkCMBSceneV2WriterHelper::~vtkCMBSceneV2WriterHelper()
{
  this->SetFileName(0);
  this->SetText(0);

  if (this->Internal)
  {
    delete this->Internal;
  }
}

//-----------------------------------------------------------------------------
int vtkCMBSceneV2WriterHelper::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inputVector), vtkInformationVector* vtkNotUsed(outputVector))
{
  if (this->GetMode() == 1)
  {
    //we are in check directory info mode
    this->ProcessFileNames();
  }
  else if (this->GetMode() == 2)
  {
    //we are in xml file writing mode
    if (this->FileName == NULL || this->Text == NULL)
    {
      return 0;
    }

    std::ofstream out(this->FileName);
    out.write(this->Text, strlen(this->Text));
    out.close();
  }
  return 1;
}

//-----------------------------------------------------------------------------
void vtkCMBSceneV2WriterHelper::AddObjectFileName(const char* fname)
{
  this->Internal->ObjectFileNames.push_back(fname);
  this->Modified();
}

void vtkCMBSceneV2WriterHelper::RemoveAllObjectFileNames()
{
  this->Internal->ObjectFileNames.clear();
  this->Modified();
}

unsigned int vtkCMBSceneV2WriterHelper::GetNumberOfObjectFileNames()
{
  return static_cast<unsigned int>(this->Internal->ObjectFileNames.size());
}

const char* vtkCMBSceneV2WriterHelper::GetObjectFileName(unsigned int idx)
{
  return this->Internal->ObjectFileNames[idx].c_str();
}

//-----------------------------------------------------------------------------
void vtkCMBSceneV2WriterHelper::ProcessFileNames()
{
  std::string fullPath = vtksys::SystemTools::CollapseFullPath(this->FileName);
  std::string dataPath = vtksys::SystemTools::GetFilenamePath(fullPath);
  std::string relPath, fname, fnameFullPath;
  std::string fPath, fnameComp, newFile;

  for (unsigned int i = 0; i < this->Internal->ObjectFileNames.size(); i++)
  {
    fname = this->Internal->ObjectFileNames[i];

    fnameFullPath = vtksys::SystemTools::CollapseFullPath(fname.c_str());
    if (this->GetPackageScene())
    {
      // See if the file is in the same directory as the scene file
      // if its not we need to copy the file - do this by comparing th
      // paths
      fPath = vtksys::SystemTools::GetFilenamePath(fnameFullPath);
      fnameComp = vtksys::SystemTools::GetFilenameName(fname);

      if (fPath != dataPath)
      {
        newFile = dataPath;
        newFile.append("/");
        newFile.append(fnameComp);
        vtksys::SystemTools::CopyFileAlways(fnameFullPath.c_str(), newFile.c_str());
      }
      relPath = fnameComp;
    }
    else
    {
      relPath = vtksys::SystemTools::RelativePath(dataPath.c_str(), fnameFullPath.c_str());
    }
    this->Internal->ObjectFileNames[i] = relPath;
  }
}

//-----------------------------------------------------------------------------
void vtkCMBSceneV2WriterHelper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "File Name: " << (this->FileName ? this->FileName : "(none)") << "\n";

  os << indent << "Text: " << (this->Text ? this->Text : "(none)") << "\n";

  for (unsigned int i = 0; i < this->Internal->ObjectFileNames.size(); i++)
  {
    os << indent << "Object FileName[" << i << "]: " << this->Internal->ObjectFileNames[i] << endl;
  }

  os << indent << "Mode: " << this->Mode << endl;
  os << indent << "PackageScene: " << this->PackageScene << endl;
}

//----------------------------------------------------------------------------
int vtkCMBSceneV2WriterHelper::RequestInformation(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inputVector), vtkInformationVector* vtkNotUsed(outputVector))
{
  return VTK_OK;
}

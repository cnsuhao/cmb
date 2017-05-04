//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "pqCMBSceneReader.h"
#include "pqCMBSceneTree.h"
#include "pqCMBSceneV1Reader.h"
#include "pqCMBSceneV2Reader.h"
#include "vtkSmartPointer.h"
#include "vtkXMLDataElement.h"
#include "vtkXMLUtilities.h"

//----------------------------------------------------------------------------
pqCMBSceneReader::pqCMBSceneReader()
{
  this->Tree = NULL;
  this->UseBoundsConstraint = 0;
  for (int i = 0; i < 6; i++)
  {
    this->BoundsConstraint[i] = 0.0;
  }
  this->FilterObjectByType = 0;
}

//----------------------------------------------------------------------------
pqCMBSceneReader::~pqCMBSceneReader()
{
}

//----------------------------------------------------------------------------
int pqCMBSceneReader::process(const char* data)
{
  if (!this->Tree)
  {
    this->Status = "Scene Tree has not been set";
    return -1;
  }
  if (!data || !(*data))
  {
    this->Status = "The scene file is empty";
    return -1;
  }

  this->Tree->empty();
  this->Status = "";
  vtkSmartPointer<vtkXMLDataElement> root;
  root.TakeReference(vtkXMLUtilities::ReadElementFromString(data));

  if (!root)
  {
    this->Status = "Could not retrieve XML";
    return -1;
  }

  if (strcmp(root->GetName(), "Scene"))
  {
    this->Status = "Root Element is not a Scene";
    return -1;
  }
  const char* cData;
  std::string sData;

  // Check Version
  cData = root->GetAttribute("Version");
  if (!cData)
  {
    this->Status = "Missing Version Attribute";
    return -1;
  }

  int status = 0;
  if (!strcmp(cData, "1.0"))
  {
    pqCMBSceneV1Reader reader;
    reader.setTree(this->Tree);
    reader.setFileName(this->FileName.c_str());
    status = reader.process(root);
    this->Status = reader.getStatusMessage();
  }
  else if (!strcmp(cData, "2.0"))
  {
    pqCMBSceneV2Reader reader;
    reader.setTree(this->Tree);
    reader.setFileName(this->FileName.c_str());
    reader.setUseBoundsConstraint(this->UseBoundsConstraint);
    reader.setBoundsConstraint(this->BoundsConstraint);
    reader.setFilterObjectByType(this->FilterObjectByType);
    reader.setFilterObjectTypes(this->FilterObjectTypes);
    status = reader.process(root);
    this->Status = reader.getStatusMessage();
  }
  else
  {
    this->Status = "Version ";
    this->Status += cData;
    this->Status += " is not supported";
    status = -1;
  }
  return status;
}

//----------------------------------------------------------------------------
int pqCMBSceneReader::getUserDefinedObjectTypes(const char* data, QStringList& objTypes)
{
  this->Status = "";
  vtkSmartPointer<vtkXMLDataElement> root;
  root.TakeReference(vtkXMLUtilities::ReadElementFromString(data));
  if (!root)
  {
    this->Status = "Could not retrieve XML";
    return -1;
  }

  if (strcmp(root->GetName(), "Scene"))
  {
    this->Status = "Root Element is not a Scene";
    return -1;
  }
  const char* cData = root->GetAttribute("Version");
  if (!cData || strcmp(cData, "2.0"))
  {
    this->Status = "The getUserDefinedObjectTypes() is only supported in version2.0 reader.";
    return -1;
  }

  int status = 0;
  pqCMBSceneV2Reader reader;
  status = reader.getUserDefinedObjectTypes(root, objTypes);
  this->Status = reader.getStatusMessage();
  return status;
}

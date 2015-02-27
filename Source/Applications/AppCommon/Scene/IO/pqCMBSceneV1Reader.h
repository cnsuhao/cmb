/*=========================================================================

  Program:   CMB
  Module:    pqCMBSceneV1Reader.h

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME pqCMBSceneV1Reader - reads a Version 1 XML representation of a Scene.
// .SECTION Description
// .SECTION Caveats

#ifndef __pqCMBSceneV1Reader_h
#define __pqCMBSceneV1Reader_h

#include "cmbAppCommonExport.h"
#include "cmbSystemConfig.h"
#include <string>
#include <vector>

class vtkXMLDataElement;
class pqCMBSceneTree;
class SceneNode;
class pqCMBSceneObjectBase;
class QProgressDialog;
class CMBAPPCOMMON_EXPORT pqCMBSceneV1Reader
{
public:
  pqCMBSceneV1Reader() : Tree(NULL), Progress(NULL){}
  virtual ~pqCMBSceneV1Reader();

  void setTree(pqCMBSceneTree *tree)
    {this->Tree = tree;}

  pqCMBSceneTree *getTree() const
    {return this->Tree;}

  void setFileName(const char *name)
    { this->FileName = name;}
  std::string getFileName() const
    {return this->FileName;}

  int process(vtkXMLDataElement *root);

  std::string getStatusMessage() const
    {return this->Status;}

  static int convertToType(const char *tname,
                           int &surfaceType);
protected:
  void processTypes(vtkXMLDataElement *);
  void processFiles(vtkXMLDataElement *);
  void processTextureFiles(vtkXMLDataElement *);
  void processObjects(vtkXMLDataElement *);
  void processObject(vtkXMLDataElement *);
  pqCMBSceneObjectBase *processVOI(vtkXMLDataElement *elem);
  pqCMBSceneObjectBase *processLine(vtkXMLDataElement *elem);
  pqCMBSceneObjectBase *processContour(vtkXMLDataElement *elem);
  pqCMBSceneObjectBase *processPlane(vtkXMLDataElement *elem);
  pqCMBSceneObjectBase *processPoints(vtkXMLDataElement *elem);
  pqCMBSceneObjectBase *processFacetedObject(vtkXMLDataElement *elem,
                                       int surfaceType);
  void processConstraints(vtkXMLDataElement *, pqCMBSceneObjectBase *);
  int processTextureInfo(vtkXMLDataElement *, pqCMBSceneObjectBase *);
  void appendStatus(const std::string &newStatus);

  pqCMBSceneTree *Tree;
  std::vector<std::string> FileNames;
  std::vector<std::string> TextureFileNames;
  std::vector<pqCMBSceneObjectBase *> Sources;
  std::string Status;
  std::string FileName;
  QProgressDialog *Progress;
};

#endif /* __pqCMBSceneV1Reader_h */

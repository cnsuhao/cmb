/*=========================================================================

  Program:   CMB
  Module:    pqCMBSceneV1Writer.h

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME pqCMBSceneV1Writer - writes out an Version 1 XML representation of a Scene.
// .SECTION Description
// .SECTION Caveats

#ifndef __pqCMBSceneV1Writer_h
#define __pqCMBSceneV1Writer_h

#include <string>
#include <map>
#include "cmbAppCommonExport.h"
#include "cmbSystemConfig.h"

class vtkXMLDataElement;
class pqCMBSceneTree;
class SceneNode;
class pqCMBSceneObjectBase;
class CMBAPPCOMMON_EXPORT pqCMBSceneV1Writer
{
public:
  pqCMBSceneV1Writer()
    {this->Tree = NULL;}
  virtual ~pqCMBSceneV1Writer();

  void setFileName(const char *name)
    { this->FileName = name;}
  std::string getFileName() const
    {return this->FileName;}

  void setTree(pqCMBSceneTree *tree)
    {this->Tree = tree;}

  pqCMBSceneTree *getTree() const
    {return this->Tree;}

  void write(bool packageScene = false);
protected:
  vtkXMLDataElement *getTypes();
  vtkXMLDataElement *getDirectory(bool packageScene);
  vtkXMLDataElement *getTextureDirectory(bool packageScene);
  vtkXMLDataElement *getObjects();
  vtkXMLDataElement *getObjectDescription(SceneNode *);
  vtkXMLDataElement *addConstraints(pqCMBSceneObjectBase *obj);
  vtkXMLDataElement *addTextureInfo(pqCMBSceneObjectBase *obj);
  void processVOI(SceneNode *node, vtkXMLDataElement *elem);
  void processLine(SceneNode *node, vtkXMLDataElement *elem);
  void processContour(SceneNode *node, vtkXMLDataElement *elem);
  void processPlane(SceneNode *node, vtkXMLDataElement *elem);
  void processFileBasedObj(SceneNode *node, vtkXMLDataElement *elem);
  void processGeometricProperties(SceneNode *node, vtkXMLDataElement *elem);
  std::string FileName;
  pqCMBSceneTree *Tree;
  std::map<std::string, int> DirectoryMap;
  std::map<std::string, int> TextureDirectoryMap;
};

#endif /* __pqCMBSceneV1Writer_h */

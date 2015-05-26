//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqCMBSceneV2Writer - writes out an Version 1 XML representation of a Scene.
// .SECTION Description
// .SECTION Caveats

#ifndef __pqCMBSceneV2Writer_h
#define __pqCMBSceneV2Writer_h

#include <string>
#include <map>
#include <QList>

#include "cmbAppCommonExport.h"
#include "cmbSystemConfig.h"

class vtkXMLDataElement;
class pqCMBSceneTree;
class pqCMBSceneNode;
class pqCMBSceneObjectBase;
class pqCMBTexturedObject;
class pqPipelineSource;
class pqOutputPort;
class vtkStringList;

class CMBAPPCOMMON_EXPORT pqCMBSceneV2Writer
{
public:
  pqCMBSceneV2Writer();
  virtual ~pqCMBSceneV2Writer();

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
  vtkXMLDataElement *generateContourFile( bool packageScene );
  vtkXMLDataElement *getObjectDescription(pqCMBSceneNode *);
  vtkXMLDataElement *getUserDefinedObjectTypes();
  vtkXMLDataElement *addConstraints(pqCMBSceneObjectBase *obj);
  vtkXMLDataElement *addTextureInfo(pqCMBTexturedObject *obj);
  vtkXMLDataElement *addBathyMetryInfo(pqCMBTexturedObject *obj);
  void processVOI(pqCMBSceneNode *node, vtkXMLDataElement *elem);
  void processLine(pqCMBSceneNode *node, vtkXMLDataElement *elem);
  void processContour(pqCMBSceneNode *node, vtkXMLDataElement *elem);
  void processPlane(pqCMBSceneNode *node, vtkXMLDataElement *elem);
  void processCone(pqCMBSceneNode *node, vtkXMLDataElement *elem);
  void processPoints(pqCMBSceneNode *node, vtkXMLDataElement *elem);
  void processFacetedObject(pqCMBSceneNode *node, vtkXMLDataElement *elem);
  void processSolidMesh(pqCMBSceneNode *node, vtkXMLDataElement *elem);
  void processGlyphObject(pqCMBSceneNode *node, vtkXMLDataElement *elem);
  void processPolygon(pqCMBSceneNode *node, vtkXMLDataElement *elem);
  void processUniformGrid(pqCMBSceneNode *node, vtkXMLDataElement *elem);
  void processGeometricProperties(pqCMBSceneNode *node, vtkXMLDataElement *elem);
  void processNonTransformProperties(pqCMBSceneNode *node, vtkXMLDataElement *elem);
  void updateDirectoryInfo(const std::string &fname,
                           vtkStringList *absFiles, int &nextEntry);
  std::string FileName;
  pqCMBSceneTree *Tree;
  std::map<std::string, int> DirectoryMap;
  std::map<std::string, int> TextureDirectoryMap;
  // This map is to keep track of which user defined object names are used
  std::map<std::string, int> UserDefinedObjectTypes;

  QList<pqOutputPort*>* ContourInputs;
  std::string getContourFileName(bool pacakageScene,std::string absoluteFileName);
  pqPipelineSource *WriterSource;
};

#endif /* __pqCMBSceneV2Writer_h */

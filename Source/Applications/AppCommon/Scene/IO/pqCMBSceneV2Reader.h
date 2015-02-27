/*=========================================================================

  Program:   CMB
  Module:    pqCMBSceneV2Reader.h

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME pqCMBSceneV2Reader - reads a Version 1 XML representation of a Scene.
// .SECTION Description
// .SECTION Caveats

#ifndef __pqCMBSceneV2Reader_h
#define __pqCMBSceneV2Reader_h

#include "cmbAppCommonExport.h"
#include "cmbSystemConfig.h"
#include <string>
#include <vector>
#include <queue>
#include <QStringList>

class vtkXMLDataElement;
class pqCMBSceneTree;
class SceneNode;
class pqCMBSceneObjectBase;
class pqCMBArc;
class QProgressDialog;
class vtkSMSourceProxy;

class CMBAPPCOMMON_EXPORT pqCMBSceneV2Reader
{
public:
  pqCMBSceneV2Reader();
  virtual ~pqCMBSceneV2Reader();

  void setTree(pqCMBSceneTree *tree)
    {this->Tree = tree;}

  pqCMBSceneTree *getTree() const
    {return this->Tree;}

  void setFileName(const char *name)
    { this->FileName = name;}
  std::string getFileName() const
    {return this->FileName;}

  void setUseBoundsConstraint(int val)
  { this->UseBoundsConstraint = val; }
  void setFilterObjectByType(int val)
  { this->FilterObjectByType = val; }
  void setBoundsConstraint(double bounds[6])
  {
    for(int i=0; i<6; i++)
    {
      this->BoundsConstraint[i] = bounds[i];
    }
  }
  void setFilterObjectTypes(QStringList& objTypes)
  {
    this->FilterObjectTypes.clear();
    this->FilterObjectTypes << objTypes;
  }

  int getUserDefinedObjectTypes(vtkXMLDataElement* root,
    QStringList& objTypes);
  int process(vtkXMLDataElement *root);

  std::string getStatusMessage() const
    {return this->Status;}

protected:
  void processTypes(vtkXMLDataElement *);
  void processFiles(vtkXMLDataElement *);
  void processTextureFiles(vtkXMLDataElement *);
  void processObjects(vtkXMLDataElement *);
  void processObject(vtkXMLDataElement *);
  void processUserDefinedObjectTypes(vtkXMLDataElement *);
  void processContourFile(vtkXMLDataElement *);
  pqCMBSceneObjectBase *processVOI(vtkXMLDataElement *elem);
  pqCMBSceneObjectBase *processLine(vtkXMLDataElement *elem);
  pqCMBSceneObjectBase *processCone(vtkXMLDataElement *elem);
  pqCMBSceneObjectBase *processContour(vtkXMLDataElement *elem);
  pqCMBSceneObjectBase *processPlane(vtkXMLDataElement *elem);
  pqCMBSceneObjectBase *processPoints(vtkXMLDataElement *elem);
  pqCMBSceneObjectBase *processPolygons(vtkXMLDataElement *elem);
  pqCMBSceneObjectBase *processFacetedObject(vtkXMLDataElement *elem);
  pqCMBSceneObjectBase *processGlyphObject(vtkXMLDataElement *elem);
  pqCMBSceneObjectBase *processUniformGrid(vtkXMLDataElement *elem);
  pqCMBSceneObjectBase *processSolidMesh(vtkXMLDataElement *elem);
  pqCMBSceneObjectBase *processUnknownObject(vtkXMLDataElement *elem);
  void processConstraints(vtkXMLDataElement *, pqCMBSceneObjectBase *);
  int processTextureInfo(vtkXMLDataElement *, pqCMBSceneObjectBase *);
  int processBathymetryInfo(vtkXMLDataElement *, pqCMBSceneObjectBase *);
  void appendStatus(const std::string &newStatus);

  void createContour(vtkSMSourceProxy *proxy);
  // Used to clean up stuff in the case of an aborted loading
  // of a scene file
  void abortSceneLoading();
  pqCMBSceneTree *Tree;
  std::vector<std::string> FileNames;
  std::vector<std::string> TextureFileNames;
  std::vector<pqCMBSceneObjectBase *> Sources;
  std::queue<pqCMBArc*> Contours;
  std::string Status;
  std::string FileName;
  QProgressDialog *Progress;

  int UseBoundsConstraint;
  double BoundsConstraint[6];
  int FilterObjectByType;
  QStringList FilterObjectTypes;

};

#endif /* __pqCMBSceneV2Reader_h */

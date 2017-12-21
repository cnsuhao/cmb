//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "pqCMBSceneV2Reader.h"
#include "pqCMBArc.h"
#include "pqCMBConicalRegion.h"
#include "pqCMBFacetedObject.h"
#include "pqCMBGlyphObject.h"
#include "pqCMBLine.h"
#include "pqCMBPlane.h"
#include "pqCMBPoints.h"
#include "pqCMBPolygon.h"
#include "pqCMBSceneNode.h"
#include "pqCMBSceneTree.h"
#include "pqCMBSolidMesh.h"
#include "pqCMBUniformGrid.h"
#include "pqCMBVOI.h"
#include "qtCMBArcWidgetManager.h"
#include "vtkBoundingBox.h"
#include "vtkDoubleArray.h"
#include "vtkIdTypeArray.h"
#include "vtkSMSceneContourSourceProxy.h"
#include "vtkSMSourceProxy.h"
#include "vtkXMLDataElement.h"
#include "vtkXMLUtilities.h"
#include <QFileInfo>
#include <QProgressDialog>
#include <QTreeWidget>
#include <sstream>
#include <vtksys/SystemTools.hxx>

#include "pqApplicationCore.h"
#include "pqObjectBuilder.h"
#include "pqPipelineSource.h"
#include "pqSMAdaptor.h"

template <typename T>
void InitSceneGenCoutourNodesArray(vtkXMLDataElement* e, void* dstArray)
{
  const char* elem_cdata = e->GetCharacterData();
  std::stringstream pointData;
  pointData << elem_cdata;

  int dataLength = 0;
  int dataBufferSize = 64;

  T* dataBuffer = new T[dataBufferSize];
  T element;

  while (pointData >> element)
  {
    if (dataLength == dataBufferSize)
    {
      int newSize = dataBufferSize * 2;
      T* newBuffer = new T[newSize];
      memcpy(newBuffer, dataBuffer, dataLength * sizeof(T));
      delete[] dataBuffer;
      dataBuffer = newBuffer;
      dataBufferSize = newSize;
    }
    dataBuffer[dataLength++] = element;
  }

  memcpy(dstArray, dataBuffer, dataLength * sizeof(T));
}

pqCMBSceneV2Reader::pqCMBSceneV2Reader()
{
  this->Tree = NULL;
  this->Progress = NULL;
  this->UseBoundsConstraint = 0;
  for (int i = 0; i < 6; i++)
  {
    this->BoundsConstraint[i] = 0.0;
  }
  this->FilterObjectByType = 0;
}

pqCMBSceneV2Reader::~pqCMBSceneV2Reader()
{
  if (this->Progress)
  {
    delete this->Progress;
  }
}

int pqCMBSceneV2Reader::process(vtkXMLDataElement* root)
{
  if (!this->Tree)
  {
    this->Status = "Scene Tree has not been set";
    return -1;
  }

  this->Tree->empty();
  this->Sources.clear();
  this->FileNames.clear();
  this->TextureFileNames.clear();
  while (!this->Contours.empty())
  {
    this->Contours.pop();
  }
  this->Status = "";

  vtkXMLDataElement* elem;
  if (strcmp(root->GetName(), "Scene"))
  {
    this->Status = "Root Element is not a Scene";
    return -1;
  }
  const char* cData;
  std::string sData;

  // Check Version
  cData = root->GetAttribute("Version");
  if ((!cData) || strcmp(cData, "2.0"))
  {
    this->Status = "This is not a 2.0 Version Scene Representation";
    return -1;
  }

  // Get Units
  elem = root->FindNestedElementWithName("Units");
  if (!elem)
  {
    this->Tree->setUnits(cmbSceneUnits::Unknown);
  }
  else
  {
    this->Tree->setUnits(cmbSceneUnits::convertFromString(elem->GetAttribute("Value")));
  }

  // Lets get the number of things we need to process for the Progress Bar
  int num = 1; // Theres the Type Definitions and the Object File Directory to start with

  elem = root->FindNestedElementWithName("ObjectDefinitions");
  if (elem)
  {
    num += elem->GetNumberOfNestedElements();
  }

  this->Progress =
    new QProgressDialog("Reading V2 File", "Abort Load", 0, num, this->Tree->getWidget());
  this->Progress->setWindowTitle("Loading SceneGen");
  this->Progress->setWindowModality(Qt::WindowModal);
  this->Progress->setLabelText("Processing Type Definitions");
  this->Progress->setValue(0);
  elem = root->FindNestedElementWithName("TypeDefinitions");
  if (elem)
  {
    this->processTypes(elem);
  }
  if (this->Progress->wasCanceled())
  {
    this->abortSceneLoading();
    return -1;
  }
  this->Progress->setLabelText("Processing File Information");
  this->Progress->setValue(1);
  elem = root->FindNestedElementWithName("ObjectFileDirectory");
  if (elem)
  {
    this->processFiles(elem);
  }

  elem = root->FindNestedElementWithName("TextureFileDirectory");
  if (elem)
  {
    this->processTextureFiles(elem);
  }
  if (this->Progress->wasCanceled())
  {
    this->abortSceneLoading();
    return -1;
  }

  elem = root->FindNestedElementWithName("UserDefinedTypeDefinitions");
  if (elem)
  {
    this->processUserDefinedObjectTypes(elem);
  }
  if (this->Progress->wasCanceled())
  {
    this->abortSceneLoading();
    return -1;
  }

  elem = root->FindNestedElementWithName("ContourDefinitions");
  if (elem)
  {
    this->processContourFile(elem);
  }
  if (this->Progress->wasCanceled())
  {
    this->abortSceneLoading();
    return -1;
  }

  elem = root->FindNestedElementWithName("ObjectDefinitions");
  if (elem)
  {
    this->processObjects(elem);
  }

  if (this->Status == "")
  {
    return 0;
  }

  return -1;
}

int pqCMBSceneV2Reader::getUserDefinedObjectTypes(vtkXMLDataElement* root, QStringList& objTypes)
{
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
  vtkXMLDataElement* elem = root->FindNestedElementWithName("UserDefinedTypeDefinitions");
  objTypes.clear();
  if (elem)
  {
    for (int i = 0; i < elem->GetNumberOfNestedElements(); i++)
    {
      objTypes << elem->GetNestedElement(i)->GetAttribute("Name");
    }
  }
  else
  {
    this->Status = "Can't find UserDefinedTypeDefinitions element in XML.";
    status = -1;
  }
  return status;
}

void pqCMBSceneV2Reader::processTypes(vtkXMLDataElement* typeSection)
{
  pqCMBSceneNode *parent, *node;
  vtkXMLDataElement *te, *ce;
  const char* data;
  double color[4];
  int n = typeSection->GetNumberOfNestedElements();
  int i;
  for (i = 0; i < n; i++)
  {
    te = typeSection->GetNestedElement(i);
    data = te->GetAttribute("Parent");
    if (!data)
    {
      node = this->Tree->createRoot(te->GetAttribute("Name"));
    }
    else
    {
      parent = this->Tree->findNode(data);
      node = this->Tree->createNode(te->GetAttribute("Name"), parent, NULL, NULL);
    }
    ce = te->FindNestedElementWithName("Color");
    if (ce)
    {
      ce->GetVectorAttribute("Value", 4, color);
      node->setExplicitColor(color);
    }

    if (te->FindNestedElementWithName("Invisible"))
    {
      node->setVisibility(false);
    }
    if (te->FindNestedElementWithName("Locked"))
    {
      node->setIsLocked(true);
    }
  }
}

void pqCMBSceneV2Reader::processUserDefinedObjectTypes(vtkXMLDataElement* typesSection)
{
  vtkXMLDataElement* fe;
  int n = typesSection->GetNumberOfNestedElements();
  int i;
  for (i = 0; i < n; i++)
  {
    fe = typesSection->GetNestedElement(i);
    this->Tree->addUserDefinedType(fe->GetAttribute("Name"), false);
  }
  this->Tree->cleanUpUserDefinedTypes();
}

void pqCMBSceneV2Reader::processContourFile(vtkXMLDataElement* contourSection)
{
  std::string fullOrigPath = vtksys::SystemTools::CollapseFullPath(this->FileName.c_str());
  std::string dataPath = vtksys::SystemTools::GetFilenamePath(fullOrigPath);

  QStringList files;
  files << vtksys::SystemTools::CollapseFullPath(
             contourSection->GetAttribute("FileName"), dataPath.c_str())
             .c_str();

  //now that we have the contour file name, lets read it in and generate
  //all the server side contour objects
  pqObjectBuilder* builder = pqApplicationCore::instance()->getObjectBuilder();
  builder->blockSignals(true);
  pqPipelineSource* reader = NULL;
  reader =
    builder->createReader("sources", "XMLPolyDataReader", files, this->Tree->getCurrentServer());
  builder->blockSignals(false);

  if (reader)
  {
    pqPipelineSource* extractContour =
      builder->createFilter("filters", "CmbExtractContours", reader);
    vtkSMSourceProxy* proxy = vtkSMSourceProxy::SafeDownCast(extractContour->getProxy());
    proxy->UpdatePipeline();
    proxy->UpdatePropertyInformation();
    int max = pqSMAdaptor::getElementProperty(proxy->GetProperty("NumberOfContoursInfo")).toInt();
    this->createContour(proxy);
    for (int i = 1; i < max; ++i)
    {
      pqSMAdaptor::setElementProperty(proxy->GetProperty("ContourIndex"), i);
      proxy->UpdateProperty("ContourIndex");
      proxy->UpdatePipeline();
      this->createContour(proxy);
    }
  }
}

void pqCMBSceneV2Reader::createContour(vtkSMSourceProxy* proxy)
{
  pqCMBArc* obj = new pqCMBArc(proxy);
  this->Contours.push(obj);
}

void pqCMBSceneV2Reader::processFiles(vtkXMLDataElement* dirSection)
{
  std::string fullOrigPath = vtksys::SystemTools::CollapseFullPath(this->FileName.c_str());
  std::string dataPath = vtksys::SystemTools::GetFilenamePath(fullOrigPath);
  std::string fullPath;
  vtkXMLDataElement* fe;
  int n = dirSection->GetNumberOfNestedElements();
  int i;
  for (i = 0; i < n; i++)
  {
    fe = dirSection->GetNestedElement(i);
    fullPath = vtksys::SystemTools::CollapseFullPath(fe->GetAttribute("Name"), dataPath.c_str());
    this->FileNames.push_back(fullPath);
    this->Sources.push_back(NULL);
  }
}

void pqCMBSceneV2Reader::processTextureFiles(vtkXMLDataElement* dirSection)
{
  std::string fullOrigPath = vtksys::SystemTools::CollapseFullPath(this->FileName.c_str());
  std::string dataPath = vtksys::SystemTools::GetFilenamePath(fullOrigPath);
  std::string fullPath;
  vtkXMLDataElement* fe;
  int n = dirSection->GetNumberOfNestedElements();
  int i;
  for (i = 0; i < n; i++)
  {
    fe = dirSection->GetNestedElement(i);
    fullPath = vtksys::SystemTools::CollapseFullPath(fe->GetAttribute("Name"), dataPath.c_str());
    this->TextureFileNames.push_back(fullPath);
  }
}

void pqCMBSceneV2Reader::processObjects(vtkXMLDataElement* objSection)
{
  vtkXMLDataElement* oe;
  int baseCount = this->Progress->value() + 1;
  int n = objSection->GetNumberOfNestedElements();
  int i;

  vtkBoundingBox bbox;
  vtkXMLDataElement* pe = NULL;
  double position[3];
  bbox.SetBounds(this->BoundsConstraint);

  QString baseText("Creating Node:");
  for (i = 0; i < n; i++)
  {
    oe = objSection->GetNestedElement(i);
    this->Progress->setLabelText(baseText + oe->GetAttribute("Name"));
    this->Progress->setValue(baseCount + i);
    if (this->Progress->wasCanceled())
    {
      this->Tree->empty();
      this->Status = "Loading aborted by User";
      return;
    }

    // check if these objects should be processed.
    if (this->FilterObjectByType &&
      !this->FilterObjectTypes.contains(oe->GetAttribute("UserDefinedType")))
    {
      continue;
    }
    if (this->UseBoundsConstraint)
    {
      // We need the position
      pe = oe->FindNestedElementWithName("Position");
      if (!pe)
      {
        continue;
      }
      pe->GetVectorAttribute("Value", 3, position);
      if (!bbox.ContainsPoint(position))
      {
        continue;
      }
    }

    this->processObject(oe);
  }
}

pqCMBSceneObjectBase* pqCMBSceneV2Reader::processVOI(vtkXMLDataElement* elem)
{
  pqCMBSceneObjectBase* obj;
  vtkXMLDataElement* e;
  // Make sure its a bounds type VOI
  if (strcmp("Bounds", elem->GetAttribute("VOIType")))
  {
    std::string estring = elem->GetAttribute("Name");
    estring += " is a VOI of an unsupported type: ";
    estring += elem->GetAttribute("VOIType");
    this->appendStatus(estring);
    return NULL; // We don't support this!
  }
  double bounds[6];
  double position[3];

  // We need the position any way for VOI so lets grab it
  e = elem->FindNestedElementWithName("Position");
  if (e)
  {
    e->GetVectorAttribute("Value", 3, position);
  }
  else
  {
    position[0] = position[1] = position[2] = 0.0;
  }

  e = elem->FindNestedElementWithName("Bounds");
  e->GetVectorAttribute("Value", 6, bounds);
  obj = new pqCMBVOI(
    position, bounds, this->Tree->getCurrentServer(), this->Tree->getCurrentView(), false);
  return obj;
}

pqCMBSceneObjectBase* pqCMBSceneV2Reader::processLine(vtkXMLDataElement* elem)
{
  pqCMBSceneObjectBase* obj;
  vtkXMLDataElement* e;
  // Make sure its a staight line type LineObject
  if (strcmp("StraightLine", elem->GetAttribute("LineType")))
  {
    std::string estring = elem->GetAttribute("Name");
    estring += " is a line of an unsupported type: ";
    estring += elem->GetAttribute("LineType");
    this->appendStatus(estring);
    return NULL;
  }
  double point1[3], point2[3];
  e = elem->FindNestedElementWithName("Point1WorldPosition");
  e->GetVectorAttribute("Value", 3, point1);
  e = elem->FindNestedElementWithName("Point2WorldPosition");
  e->GetVectorAttribute("Value", 3, point2);

  obj = new pqCMBLine(
    point1, point2, this->Tree->getCurrentServer(), this->Tree->getCurrentView(), false);
  this->Tree->setLineWidgetCallbacks(static_cast<pqCMBLine*>(obj));

  return obj;
}

pqCMBSceneObjectBase* pqCMBSceneV2Reader::processContour(vtkXMLDataElement* elem)
{
  vtkXMLDataElement* e;
  int positionNormal;
  double positionOffset;
  e = elem->FindNestedElementWithName("PlaneProjectionNormal");
  e->GetScalarAttribute("Value", positionNormal);
  e = elem->FindNestedElementWithName("PlanePositionOffset");
  e->GetScalarAttribute("Value", positionOffset);

  if (this->Contours.empty())
  {
    return NULL;
  }

  pqCMBArc* obj = this->Contours.front();
  obj->setPlaneProjectionNormal(positionNormal);
  obj->setPlaneProjectionPosition(positionOffset);

  this->Contours.pop();
  return obj;
}

pqCMBSceneObjectBase* pqCMBSceneV2Reader::processCone(vtkXMLDataElement* elem)
{
  pqCMBSceneObjectBase* obj;
  vtkXMLDataElement* e;
  double tr, br, h, p1[3], dir[3];
  int r;
  e = elem->FindNestedElementWithName("BaseCenter");
  if (e)
  {
    e->GetVectorAttribute("Value", 3, p1);
  }
  else
  {
    return NULL;
  }

  e = elem->FindNestedElementWithName("Direction");
  if (e)
  {
    e->GetVectorAttribute("Value", 3, dir);
  }
  else
  {
    return NULL;
  }

  e = elem->FindNestedElementWithName("Height");
  if (e)
  {
    e->GetScalarAttribute("Value", h);
  }
  else
  {
    return NULL;
  }

  e = elem->FindNestedElementWithName("BaseRadius");
  if (e)
  {
    e->GetScalarAttribute("Value", br);
  }
  else
  {
    return NULL;
  }

  e = elem->FindNestedElementWithName("TopRadius");
  if (e)
  {
    e->GetScalarAttribute("Value", tr);
  }
  else
  {
    return NULL;
  }

  e = elem->FindNestedElementWithName("Resolution");
  if (e)
  {
    e->GetScalarAttribute("Value", r);
  }
  else
  {
    return NULL;
  }

  obj = new pqCMBConicalRegion(
    p1, br, h, tr, dir, r, this->Tree->getCurrentServer(), this->Tree->getCurrentView(), false);
  return obj;
}

pqCMBSceneObjectBase* pqCMBSceneV2Reader::processPlane(vtkXMLDataElement* elem)
{
  pqCMBSceneObjectBase* obj;
  vtkXMLDataElement* e;
  double position[3], p1[3], p2[3];

  e = elem->FindNestedElementWithName("Point1");
  if (e)
  {
    e->GetVectorAttribute("Value", 3, p1);
  }
  else
  {
    return NULL;
  }

  e = elem->FindNestedElementWithName("Point2");
  if (e)
  {
    e->GetVectorAttribute("Value", 3, p2);
  }
  else
  {
    return NULL;
  }

  obj = new pqCMBPlane(p1, p2, this->Tree->getCurrentServer(), this->Tree->getCurrentView(), false);
  // We need the position
  e = elem->FindNestedElementWithName("Position");
  if (e)
  {
    e->GetVectorAttribute("Value", 3, position);
  }
  else
  {
    position[0] = position[1] = position[2] = 0.0;
  }

  obj->setPosition(position, false);
  return obj;
}

pqCMBSceneObjectBase* pqCMBSceneV2Reader::processPoints(vtkXMLDataElement* elem)
{
  pqCMBSceneObjectBase* obj;
  vtkXMLDataElement* e;
  double position[3];

  // if LIDAR data then (if not older version of sg file) will have
  // target/max number of points to load (to match state when saved to disk).
  // If older version of file, just default to 1,000,000 points.
  int lidarMaxNumberOfPoints = 1000000;
  elem->GetScalarAttribute("MaxNumberOfPoints", lidarMaxNumberOfPoints);

  // newset version support for LIDAR (pts and las) does pieces seperately
  // if pieceId is set, then must be the newest version
  int pieceId = -1;
  elem->GetScalarAttribute("PieceId", pieceId);

  bool doublePrecision = false;
  const char* precisionString = elem->GetAttribute("DataPrecision");
  if (precisionString && !strcmp(precisionString, "double"))
  {
    doublePrecision = true;
  }

  // See if we have already created a source for this object
  int index;
  elem->GetScalarAttribute("FileEntry", index);
  if (index < 0)
  {
    std::string estring = elem->GetAttribute("Name");
    estring += " does not have a points file associated with it";
    this->appendStatus(estring);
    return NULL; // We don't support this!
  }

  // if pieceId != -1, then LAS or LIDARPts file, which we handle differently
  // since may be multiple pieces in the file that will all use the same reader
  // to read
  if (pieceId != -1)
  {
    pqPipelineSource* source = 0;
    if (this->Sources[index])
    {
      source = dynamic_cast<pqCMBPoints*>(this->Sources[index])->getReaderSource();
    }
    else
    {
      QStringList files;
      files << this->FileNames[index].c_str();
      pqObjectBuilder* builder = pqApplicationCore::instance()->getObjectBuilder();

      QFileInfo finfo(this->FileNames[index].c_str());
      builder->blockSignals(true);
      if (finfo.completeSuffix().toLower() == "las")
      {
        source =
          builder->createReader("sources", "LASReader", files, this->Tree->getCurrentServer());
      }
      else // currently (for what we support) has to by LIDAR pts format
      {
        source =
          builder->createReader("sources", "LIDARReader", files, this->Tree->getCurrentServer());
      }
      builder->blockSignals(false);
    }
    int onRatio = 1;
    elem->GetScalarAttribute("PieceOnRatio", onRatio);
    pqCMBPoints* pobj = new pqCMBPoints(this->Tree->getCurrentServer(),
      this->Tree->getCurrentView(), source, pieceId, onRatio, doublePrecision);
    pobj->setFileName(this->FileNames[index].c_str());
    obj = pobj;
    this->Sources[index] = obj;
  }
  else if (this->Sources[index]) // Old format but we already have the source read in
  {
    obj = this->Sources[index]->duplicate(
      this->Tree->getCurrentServer(), this->Tree->getCurrentView(), false);
  }
  else // Old format that has not yet been read in
  {
    obj = new pqCMBPoints(this->FileNames[index].c_str(), this->Tree->getCurrentServer(),
      this->Tree->getCurrentView(), lidarMaxNumberOfPoints, false);
    this->Sources[index] = obj;
  }
  // We need the position
  e = elem->FindNestedElementWithName("Position");
  if (e)
  {
    e->GetVectorAttribute("Value", 3, position);
  }
  else
  {
    position[0] = position[1] = position[2] = 0.0;
  }

  obj->setPosition(position, false);
  return obj;
}

pqCMBSceneObjectBase* pqCMBSceneV2Reader::processFacetedObject(vtkXMLDataElement* elem)
{
  pqCMBSceneObjectBase* obj;
  vtkXMLDataElement* e;
  double position[3];

  // See if we have already created a source for this object
  int index;
  elem->GetScalarAttribute("FileEntry", index);
  if (index < 0)
  {
    std::string estring = elem->GetAttribute("Name");
    estring += " does not have a faceted file associated with it";
    this->appendStatus(estring);
    return NULL; // We don't support this!
  }

  pqCMBSceneObjectBase::enumSurfaceType otype =
    pqCMBSceneObjectBase::convertStringToSurfaceType(elem->GetAttribute("SurfaceType"));

  // Have we read in the file already?
  if (this->Sources[index])
  {
    obj = this->Sources[index]->duplicate(
      this->Tree->getCurrentServer(), this->Tree->getCurrentView(), false);
  }
  else
  {
    obj = new pqCMBFacetedObject(this->FileNames[index].c_str(), this->Tree->getCurrentServer(),
      this->Tree->getCurrentView(), false);
    this->Sources[index] = obj;
  }

  pqCMBFacetedObject* fobj = dynamic_cast<pqCMBFacetedObject*>(obj);
  fobj->setSurfaceType(static_cast<pqCMBSceneObjectBase::enumSurfaceType>(otype));
  // We need the position
  e = elem->FindNestedElementWithName("Position");
  if (e)
  {
    e->GetVectorAttribute("Value", 3, position);
  }
  else
  {
    position[0] = position[1] = position[2] = 0.0;
  }

  obj->setPosition(position, false);
  return obj;
}

pqCMBSceneObjectBase* pqCMBSceneV2Reader::processUniformGrid(vtkXMLDataElement* elem)
{
  pqCMBSceneObjectBase* obj;
  vtkXMLDataElement* e;
  double position[3];

  // See if we have already created a source for this object
  int index;
  elem->GetScalarAttribute("FileEntry", index);
  if (index < 0)
  {
    std::string estring = elem->GetAttribute("Name");
    estring += " does not have an image file associated with it";
    this->appendStatus(estring);
    return NULL; // We don't support this!
  }

  // Have we read in the file already?
  if (this->Sources[index])
  {
    obj = this->Sources[index]->duplicate(
      this->Tree->getCurrentServer(), this->Tree->getCurrentView(), false);
  }
  else
  {
    obj = new pqCMBUniformGrid(this->FileNames[index].c_str(), this->Tree->getCurrentServer(),
      this->Tree->getCurrentView(), false);
    this->Sources[index] = obj;
  }

  pqCMBUniformGrid* fobj = dynamic_cast<pqCMBUniformGrid*>(obj);
  // See if this is a Raw DEM File
  e = elem->FindNestedElementWithName("RawDEMInfo");
  if (e)
  {
    vtkIdType a[2], b[2];
    int c;
    elem->GetScalarAttribute("ReadGroupOfFiles", c);
    fobj->setReadGroupOfFiles((c == 1));
    elem->GetScalarAttribute("OnRatio", c);
    fobj->setOnRatio(c);
    e->GetVectorAttribute("RowExtents", 2, a);
    e->GetVectorAttribute("ColumnExtents", 2, b);
    fobj->setExtents(a, b);
  }
  // We need the position
  e = elem->FindNestedElementWithName("Position");
  if (e)
  {
    e->GetVectorAttribute("Value", 3, position);
  }
  else
  {
    position[0] = position[1] = position[2] = 0.0;
  }

  obj->setPosition(position, false);
  return obj;
}

pqCMBSceneObjectBase* pqCMBSceneV2Reader::processUnknownObject(vtkXMLDataElement* elem)
{
  std::string estring = elem->GetAttribute("Name");
  estring += " is of unknown type and is not supported by this reader";
  this->appendStatus(estring);
  return NULL; // We don't support this!
}

pqCMBSceneObjectBase* pqCMBSceneV2Reader::processGlyphObject(vtkXMLDataElement* elem)
{
  pqCMBGlyphObject* obj;
  vtkXMLDataElement* e;
  double position[3];

  // See if we have already created a source for this object
  int gindex, pindex;
  elem->GetScalarAttribute("GlyphFileEntry", gindex);
  if (gindex < 0)
  {
    std::string estring = elem->GetAttribute("Name");
    estring += " does not have a glyph file associated with it";
    this->appendStatus(estring);
    return NULL; // We don't support this!
  }

  elem->GetScalarAttribute("PointsFileEntry", pindex);
  if (pindex < 0)
  {
    std::string estring = elem->GetAttribute("Name");
    estring += " does not have a points file associated with it";
    this->appendStatus(estring);
    return NULL; // We don't support this!
  }

  pqCMBSceneObjectBase::enumSurfaceType otype =
    pqCMBSceneObjectBase::convertStringToSurfaceType(elem->GetAttribute("SurfaceType"));

  // Have we read in the glyph file already?
  // NOTE: that if we have not read in the file for a faceted object that also
  // uses the same source geometry as the glyph we do not cache the read in source
  // The reason for this is that we would need to keep track of these faceted scene
  // objects so we can later delete them.  This implementation is not 100% optimal but
  // is easier to maintain and the cost is almost 0
  obj = new pqCMBGlyphObject(this->FileNames[gindex].c_str(), this->FileNames[pindex].c_str(),
    this->Tree->getCurrentServer(), this->Tree->getCurrentView(), false);
  // We need the position
  e = elem->FindNestedElementWithName("Position");
  if (e)
  {
    e->GetVectorAttribute("Value", 3, position);
  }
  else
  {
    position[0] = position[1] = position[2] = 0.0;
  }

  obj->setPosition(position, false);
  obj->setSurfaceType(static_cast<pqCMBSceneObjectBase::enumSurfaceType>(otype));
  return obj;
}

pqCMBSceneObjectBase* pqCMBSceneV2Reader::processPolygons(vtkXMLDataElement* elem)
{
  pqCMBPolygon* obj = NULL;
  vtkXMLDataElement* e;

  // See if we have already created a source for this object
  int pindex;
  elem->GetScalarAttribute("PolygonFileEntry", pindex);
  if (pindex < 0)
  {
    std::string estring = elem->GetAttribute("Name");
    estring += " does not have a polygon file associated with it";
    this->appendStatus(estring);
    return NULL; // We don't support this!
  }

  // We need the min angle
  double minAngle = 0; //default to disable min angle
  elem->GetScalarAttribute("MinAngle", minAngle);

  // We need the edge length
  double edgeLength = 0; //default to disabled
  elem->GetScalarAttribute("EdgeLength", edgeLength);

  e = elem->FindNestedElementWithName("Arcs");
  std::vector<pqCMBSceneNode*> inputNodes;
  pqCMBSceneNode* temp = NULL;
  if (e)
  {
    int size = 0;
    const char* arcName;
    e->GetScalarAttribute("Size", size);
    for (int i = 0; i < size; ++i)
    {
      arcName = e->GetNestedElement(i)->GetAttribute("Name");
      if (!arcName)
      {
        continue;
      }
      //look up this name in the scene tree
      temp = this->Tree->findNode(arcName);
      if (temp)
      {
        inputNodes.push_back(temp);
      }
    }
    //create the polygon filter client item
    obj = new pqCMBPolygon(minAngle, edgeLength, inputNodes);
  }

  return obj;
}

void pqCMBSceneV2Reader::processObject(vtkXMLDataElement* elem)
{

  pqCMBSceneObjectBase::enumObjectType otype =
    pqCMBSceneObjectBase::convertStringToType(elem->GetAttribute("ObjectType"));

  pqCMBSceneNode* parent = this->Tree->findNode(elem->GetAttribute("NodeType"));
  pqCMBSceneObjectBase* obj;
  vtkXMLDataElement *e, *colorElement;
  double data3[3];

  if (otype == pqCMBSceneObjectBase::VOI)
  {
    obj = this->processVOI(elem);
  }
  else if (otype == pqCMBSceneObjectBase::Line)
  {
    obj = this->processLine(elem);
  }
  else if (otype == pqCMBSceneObjectBase::Arc)
  {
    obj = this->processContour(elem);
  }
  else if (otype == pqCMBSceneObjectBase::GroundPlane)
  {
    obj = this->processPlane(elem);
  }
  else if (otype == pqCMBSceneObjectBase::Points)
  {
    obj = this->processPoints(elem);
  }
  else if (otype == pqCMBSceneObjectBase::Faceted)
  {
    obj = this->processFacetedObject(elem);
  }
  else if (otype == pqCMBSceneObjectBase::Glyph)
  {
    obj = this->processGlyphObject(elem);
  }
  else if (otype == pqCMBSceneObjectBase::Polygon)
  {
    obj = this->processPolygons(elem);
  }
  else if (otype == pqCMBSceneObjectBase::GeneralCone)
  {
    obj = this->processCone(elem);
  }
  else if (otype == pqCMBSceneObjectBase::UniformGrid)
  {
    obj = this->processUniformGrid(elem);
  }
  else if (otype == pqCMBSceneObjectBase::SolidMesh)
  {
    obj = this->processSolidMesh(elem);
  }
  else
  {
    obj = this->processUnknownObject(elem);
  }
  if (obj == NULL)
  {
    return;
  }

  e = elem->FindNestedElementWithName("Units");
  if (e)
  {
    obj->setUnits(cmbSceneUnits::convertFromString(e->GetAttribute("Value")));
  }

  e = elem->FindNestedElementWithName("Orientation");
  if (e)
  {
    e->GetVectorAttribute("Value", 3, data3);
    obj->setOrientation(data3, false);
  }
  e = elem->FindNestedElementWithName("Scale");
  if (e)
  {
    e->GetVectorAttribute("Value", 3, data3);
    obj->setScale(data3, false);
  }
  e = elem->FindNestedElementWithName("Origin");
  if (e)
  {
    e->GetVectorAttribute("Value", 3, data3);
    obj->setOrigin(data3, false);
  }

  e = elem->FindNestedElementWithName("Constraints");
  if (e)
  {
    this->processConstraints(e, obj);
  }

  e = elem->FindNestedElementWithName("TextureInfo");
  if (e)
  {
    int status = this->processTextureInfo(e, obj);
    if (status == -1)
    {
      std::string estring = elem->GetAttribute("Name");
      estring += " does not have a texture file associated with it";
      this->appendStatus(estring);
    }
    else if (status == -2)
    {
      std::string estring = elem->GetAttribute("Name");
      estring += " can not have a texture associated with it";
      this->appendStatus(estring);
    }
  }

  e = elem->FindNestedElementWithName("ShowElevation");
  if (e)
  {
    pqCMBTexturedObject* tobj = dynamic_cast<pqCMBTexturedObject*>(obj);
    if (tobj == NULL)
    {
      std::string estring = elem->GetAttribute("Name");
      estring += " can not have elevation associated with it";
      this->appendStatus(estring);
    }
    else
    {
      tobj->showElevation(true);
    }
  }

  pqCMBSceneNode* node = this->Tree->createNode(elem->GetAttribute("Name"), parent, obj, NULL);

  double color[4];
  colorElement = elem->FindNestedElementWithName("Color");
  if (colorElement && colorElement->FindNestedElementWithName("Inherited") == NULL)
  {
    // Explicit Color
    colorElement->GetVectorAttribute("Value", 4, color);
    node->setExplicitColor(color);
  }

  if (elem->FindNestedElementWithName("Invisible"))
  {
    node->setVisibility(false);
  }
  if (elem->FindNestedElementWithName("Locked"))
  {
    node->setIsLocked(true);
  }
  std::string userType = elem->GetAttribute("UserDefinedType");
  obj->setUserDefinedType(userType.c_str());
  obj->updateRepresentation();
  if (elem->FindNestedElementWithName("SnapTarget"))
  {
    this->Tree->setSnapTarget(node);
  }
}

int pqCMBSceneV2Reader::processTextureInfo(vtkXMLDataElement* elem, pqCMBSceneObjectBase* obj)
{
  int index;
  int npoints;
  double points[12];

  elem->GetScalarAttribute("TextureFileEntry", index);
  if (index < 0)
  {
    return -1;
  }

  pqCMBTexturedObject* tobj = dynamic_cast<pqCMBTexturedObject*>(obj);
  if (tobj == NULL)
  {
    return -2;
  }

  elem->GetScalarAttribute("NumberOfRegistrationPoints", npoints);
  elem->GetVectorAttribute("Points", npoints * 4, points);

  tobj->setTextureMap(this->TextureFileNames[index].c_str(), npoints, points);
  this->Tree->addTextureFileName(this->TextureFileNames[index].c_str());
  return 0;
}

void pqCMBSceneV2Reader::processConstraints(
  vtkXMLDataElement* constraintSection, pqCMBSceneObjectBase* obj)
{
  obj->clearConstraints();
  vtkXMLDataElement* ce;
  int n = constraintSection->GetNumberOfNestedElements();
  int i;
  std::string name;
  for (i = 0; i < n; i++)
  {
    ce = constraintSection->GetNestedElement(i);
    name = ce->GetName();

    if (name == "XTransConstraint")
    {
      obj->setXTransConstraint(true);
    }
    else if (name == "YTransConstraint")
    {
      obj->setYTransConstraint(true);
    }
    else if (name == "ZTransConstraint")
    {
      obj->setZTransConstraint(true);
    }
    else if (name == "XRotationConstraint")
    {
      obj->setXRotationConstraint(true);
    }
    else if (name == "YRotationConstraint")
    {
      obj->setYRotationConstraint(true);
    }
    else if (name == "ZRotationConstraint")
    {
      obj->setZRotationConstraint(true);
    }
    else if (name == "IsotropicScalingConstraint")
    {
      obj->setIsotropicScalingConstraint(true);
    }
    else if (name == "XScalingConstraint")
    {
      obj->setXScalingConstraint(true);
    }
    else if (name == "YScalingConstraint")
    {
      obj->setYScalingConstraint(true);
    }
    else if (name == "ZScalingConstraint")
    {
      obj->setZScalingConstraint(true);
    }
    else
    {
      std::string estring = name;
      estring += " is an unsupported constraint";
      this->appendStatus(estring);
    }
  }
}

void pqCMBSceneV2Reader::appendStatus(const std::string& newStatus)
{
  if (this->Status != "")
  {
    this->Status += "\n";
  }
  this->Status += newStatus;
}

pqCMBSceneObjectBase* pqCMBSceneV2Reader::processSolidMesh(vtkXMLDataElement* elem)
{
  pqCMBSceneObjectBase* obj;
  vtkXMLDataElement* e;
  double position[3];

  // See if we have already created a source for this object
  int index;
  elem->GetScalarAttribute("FileEntry", index);
  if (index < 0)
  {
    std::string estring = elem->GetAttribute("Name");
    estring += " does not have a faceted file associated with it";
    this->appendStatus(estring);
    return NULL; // We don't support this!
  }

  // Have we read in the file already?
  if (this->Sources[index])
  {
    obj = this->Sources[index]->duplicate(
      this->Tree->getCurrentServer(), this->Tree->getCurrentView(), false);
  }
  else
  {
    obj = new pqCMBSolidMesh(this->FileNames[index].c_str(), this->Tree->getCurrentServer(),
      this->Tree->getCurrentView(), false);
    this->Sources[index] = obj;
  }

  // We need the position
  e = elem->FindNestedElementWithName("Position");
  if (e)
  {
    e->GetVectorAttribute("Value", 3, position);
  }
  else
  {
    position[0] = position[1] = position[2] = 0.0;
  }

  obj->setPosition(position, false);
  return obj;
}

void pqCMBSceneV2Reader::abortSceneLoading()
{
  this->Tree->empty();
  this->Status = "Loading aborted by User";
}

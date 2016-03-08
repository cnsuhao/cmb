//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "pqCMBSceneV1Reader.h"
#include "pqCMBSceneTree.h"
#include "pqCMBSceneNode.h"
#include "pqCMBFacetedObject.h"
#include "pqCMBLine.h"
#include "pqCMBPlane.h"
#include "pqCMBPoints.h"
#include "pqCMBArc.h"
#include "pqCMBVOI.h"
#include "qtCMBArcWidgetManager.h"
#include "vtkDoubleArray.h"
#include "vtkIdTypeArray.h"
#include "vtkXMLDataElement.h"
#include "vtkXMLUtilities.h"
#include <vtksys/SystemTools.hxx>
#include <QFileInfo>
#include <QProgressDialog>
#include <QTreeWidget>
#include <sstream>

#include "pqObjectBuilder.h"
#include "pqPipelineSource.h"
#include "pqApplicationCore.h"

template <typename T>
void InitSceneGenCoutourNodesArray(
  vtkXMLDataElement* e, void* dstArray)
{
  const char *elem_cdata = e->GetCharacterData();
  std::stringstream pointData;
  pointData << elem_cdata;

  int dataLength = 0;
  int dataBufferSize = 64;

  T* dataBuffer = new T[dataBufferSize];
  T element;

  while(pointData >> element)
    {
    if(dataLength == dataBufferSize)
      {
      int newSize = dataBufferSize*2;
      T* newBuffer = new T[newSize];
      memcpy(newBuffer, dataBuffer, dataLength*sizeof(T));
      delete [] dataBuffer;
      dataBuffer = newBuffer;
      dataBufferSize = newSize;
      }
    dataBuffer[dataLength++] = element;
    }

  memcpy(dstArray, dataBuffer, dataLength*sizeof(T));
}

//----------------------------------------------------------------------------
pqCMBSceneV1Reader::~pqCMBSceneV1Reader()
{
  if (this->Progress)
    {
    delete this->Progress;
    }
}

//----------------------------------------------------------------------------
int pqCMBSceneV1Reader::process(vtkXMLDataElement *root)
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
  this->Status = "";

  vtkXMLDataElement *elem;
  if (strcmp(root->GetName(), "Scene"))
    {
    this->Status = "Root Element is not a Scene";
    return -1;
    }
  const char *cData;
  std::string sData;

  // Check Version
  cData = root->GetAttribute("Version");
  if ((!cData) || strcmp(cData, "1.0"))
    {
    this->Status = "This is not a 1.0 Version Scene Representation";
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
    this->Tree->setUnits(cmbSceneUnits::convertFromString(
                           elem->GetAttribute("Value")));
    }

  // Lets get the number of things we need to process for the Progress Bar
  int num = 2; // Theres the Type Definitions and the Object File Directory to start with

  elem = root->FindNestedElementWithName("ObjectDefinitions");
  if (elem)
    {
    num += elem->GetNumberOfNestedElements();
    }

  this->Progress = new QProgressDialog("", "Abort Load", 0, num,
                                       this->Tree->getWidget());
  this->Progress->setWindowTitle("Loading SceneGen");
  //this->Progress->setWindowModality(Qt::WindowModal);
  this->Progress->setLabelText("Processing Type Definitions");
  elem = root->FindNestedElementWithName("TypeDefinitions");
  if (elem)
    {
    this->processTypes(elem);
    }
  if (this->Progress->wasCanceled())
    {
    this->Tree->empty();
    this->Status = "Loading aborted by User";
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
    this->Tree->empty();
    this->Status = "Loading aborted by User";
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

//----------------------------------------------------------------------------
void pqCMBSceneV1Reader::processTypes(vtkXMLDataElement *typeSection)
{
  pqCMBSceneNode *parent, *node;
  vtkXMLDataElement *te, *ce;
  const char *data;
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
    }
}

//----------------------------------------------------------------------------
void pqCMBSceneV1Reader::processFiles(vtkXMLDataElement *dirSection)
{
  std::string fullOrigPath =
    vtksys::SystemTools::CollapseFullPath( this->FileName.c_str());
  std::string dataPath = vtksys::SystemTools::GetFilenamePath( fullOrigPath );
  std::string fullPath;
  vtkXMLDataElement *fe;
  int n = dirSection->GetNumberOfNestedElements();
  int i;
  for (i = 0; i < n; i++)
    {
    fe = dirSection->GetNestedElement(i);
    fullPath =
      vtksys::SystemTools::CollapseFullPath(fe->GetAttribute("Name"),
                                            dataPath.c_str());
    this->FileNames.push_back(fullPath);
    this->Sources.push_back(NULL);
    }
}
//----------------------------------------------------------------------------
void pqCMBSceneV1Reader::processTextureFiles(vtkXMLDataElement *dirSection)
{
  std::string fullOrigPath =
    vtksys::SystemTools::CollapseFullPath( this->FileName.c_str());
  std::string dataPath = vtksys::SystemTools::GetFilenamePath( fullOrigPath );
  std::string fullPath;
  vtkXMLDataElement *fe;
  int n = dirSection->GetNumberOfNestedElements();
  int i;
  for (i = 0; i < n; i++)
    {
    fe = dirSection->GetNestedElement(i);
    fullPath =
      vtksys::SystemTools::CollapseFullPath(fe->GetAttribute("Name"),
                                            dataPath.c_str());
    this->TextureFileNames.push_back(fullPath);
    }
}
//----------------------------------------------------------------------------
void pqCMBSceneV1Reader::processObjects(vtkXMLDataElement *objSection)
{
  vtkXMLDataElement *oe;
  int baseCount = this->Progress->value() + 1;
  int n = objSection->GetNumberOfNestedElements();
  int i;
  QString baseText("Creating Node:");
  for (i = 0; i < n; i++)
    {
    oe = objSection->GetNestedElement(i);
    if (this->Progress->wasCanceled())
      {
      this->Tree->empty();
      this->Status = "Loading aborted by User";
      return;
      }
    this->Progress->setLabelText(baseText + oe->GetAttribute("Name"));
    this->Progress->setValue(baseCount+i);
    this->processObject(oe);
    }
  return;
}
//----------------------------------------------------------------------------

pqCMBSceneObjectBase *pqCMBSceneV1Reader::processVOI(vtkXMLDataElement * elem)
{
  pqCMBSceneObjectBase *obj;
  vtkXMLDataElement *e;
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
  if(e)
    {
    e->GetVectorAttribute("Value", 3, position);
    }
  else
    {
   position[0] = position[1] = position[2] = 0.0;
   }

  e = elem->FindNestedElementWithName("Bounds");
  e->GetVectorAttribute("Value", 6, bounds);
  obj = new pqCMBVOI(position, bounds,
                        this->Tree->getCurrentServer(),
                        this->Tree->getCurrentView(),
                        false);
  return obj;
}
//----------------------------------------------------------------------------

pqCMBSceneObjectBase *pqCMBSceneV1Reader::processLine(vtkXMLDataElement * elem)
{
  pqCMBSceneObjectBase *obj;
  vtkXMLDataElement *e;
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

  obj = new pqCMBLine(point1, point2,
                         this->Tree->getCurrentServer(),
                         this->Tree->getCurrentView(),
                         false);
  this->Tree->setLineWidgetCallbacks(static_cast<pqCMBLine*>(obj));

  return obj;
}
//----------------------------------------------------------------------------

pqCMBSceneObjectBase *pqCMBSceneV1Reader::processContour(vtkXMLDataElement * elem)
{
  pqCMBSceneObjectBase *obj;
  vtkXMLDataElement *e;
  // Make sure its a form of contour the reader supports
  if (strcmp("BoundedPlane", elem->GetAttribute("ContourType")))
    {
    std::string estring = elem->GetAttribute("Name");
    estring += " is a contour of an unsupported type: ";
    estring += elem->GetAttribute("ContourType");
    this->appendStatus(estring);
    return NULL; // We don't support this!
    }
  int closedLoop, positionNormal;
  double positionOffset;
  e = elem->FindNestedElementWithName("PlaneProjectionNormal");
  e->GetScalarAttribute("Value", positionNormal);
  e = elem->FindNestedElementWithName("PlanePositionOffset");
  e->GetScalarAttribute("Value", positionOffset);
  e = elem->FindNestedElementWithName("ContourClosedLoop");
  e->GetScalarAttribute("Value", closedLoop);

  e = elem->FindNestedElementWithName("NodeWorldPositions");
  int numNodes, numValues;
  e->GetScalarAttribute("NumberOfNodes", numNodes);
  e->GetScalarAttribute("NumberOfValues", numValues);
  vtkDoubleArray* nodePositions = vtkDoubleArray::New();
  nodePositions->SetNumberOfComponents(3);
  nodePositions->SetNumberOfTuples(numNodes);
  InitSceneGenCoutourNodesArray<double>(e, nodePositions->GetVoidPointer(0));

  vtkIdTypeArray* SelIndices = NULL;
  e = elem->FindNestedElementWithName("SelectedNodeIndices");
  if(e)
    {
    e->GetScalarAttribute("NumberOfValues", numValues);
    SelIndices = vtkIdTypeArray::New();
    SelIndices->SetNumberOfComponents(1);
    SelIndices->SetNumberOfTuples(numValues);
    InitSceneGenCoutourNodesArray<vtkIdType>(e, SelIndices->GetVoidPointer(0));
    }
  /*
  This isn't the nicest way, but it keeps back wards compatibility
  we are going to create a contour representation than shuttle to the server.
  */
  obj = this->Tree->getArcWidgetManager()->createLegacyV1Contour(positionNormal,
                          positionOffset, closedLoop, nodePositions, SelIndices );

  nodePositions->Delete();
  if(SelIndices)
    {
    SelIndices->Delete();
    }

  return obj;
}
//----------------------------------------------------------------------------

pqCMBSceneObjectBase *pqCMBSceneV1Reader::processPlane(vtkXMLDataElement * elem)
{
  pqCMBSceneObjectBase *obj;
  vtkXMLDataElement *e;
  double position[3], p1[3], p2[3];

  e = elem->FindNestedElementWithName("Point1");
  if(e)
    {
    e->GetVectorAttribute("Value", 3, p1);
    }
  else
    {
    return NULL;
    }

  e = elem->FindNestedElementWithName("Point2");
  if(e)
    {
    e->GetVectorAttribute("Value", 3, p2);
    }
  else
    {
    return NULL;
    }

  obj = new pqCMBPlane(p1, p2,
                          this->Tree->getCurrentServer(),
                          this->Tree->getCurrentView(),
                          false);
  // We need the position
  e = elem->FindNestedElementWithName("Position");
  if(e)
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
//----------------------------------------------------------------------------

pqCMBSceneObjectBase *pqCMBSceneV1Reader::processPoints(vtkXMLDataElement * elem)
{
  pqCMBSceneObjectBase *obj;
  vtkXMLDataElement *e;
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

 // See if we have already created a source for this object
  int index;
  elem->GetScalarAttribute("FileEntry", index);
  if (index < 0)
    {
    std::string estring = elem->GetAttribute("Name");
    estring += " does not have a file associated with it";
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
        source = builder->createReader("sources", "LASReader", files,
          this->Tree->getCurrentServer());
        }
      else // currently (for what we support) has to by LIDAR pts format
        {
        source = builder->createReader("sources", "LIDARReader", files,
          this->Tree->getCurrentServer());
        }
      builder->blockSignals(false);
      }
    int onRatio = 1;
    elem->GetScalarAttribute("PieceOnRatio", onRatio);
    pqCMBPoints *pobj =
      new pqCMBPoints(
        this->Tree->getCurrentServer(),
        this->Tree->getCurrentView(), source, pieceId, onRatio, false);
    pobj->setFileName(this->FileNames[index].c_str());
    obj = pobj;
    this->Sources[index] = obj;
    }
  else if (this->Sources[index])
    {
    obj = this->Sources[index]->duplicate(this->Tree->getCurrentServer(),
                                          this->Tree->getCurrentView(),
                                          false);
    }
  else
    {
    obj = new pqCMBPoints(this->FileNames[index].c_str(),
                             this->Tree->getCurrentServer(),
                             this->Tree->getCurrentView(),
                             lidarMaxNumberOfPoints,
                             false);
    this->Sources[index] = obj;
    }
  // We need the position
  e = elem->FindNestedElementWithName("Position");
  if(e)
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
//----------------------------------------------------------------------------

pqCMBSceneObjectBase *pqCMBSceneV1Reader::processFacetedObject(vtkXMLDataElement * elem,
                                                       int surfaceType)
{
  pqCMBSceneObjectBase *obj;
  vtkXMLDataElement *e;
  double position[3];
  pqCMBFacetedObject *fobj;

 // See if we have already created a source for this object
  int index;
  elem->GetScalarAttribute("FileEntry", index);
  if (index < 0)
    {
    std::string estring = elem->GetAttribute("Name");
    estring += " does not have a file associated with it";
    this->appendStatus(estring);
    return NULL; // We don't support this!
    }

  if (this->Sources[index])
    {
    obj = this->Sources[index]->duplicate(this->Tree->getCurrentServer(),
                                          this->Tree->getCurrentView(),
                                          false);
    fobj = dynamic_cast<pqCMBFacetedObject*>(obj);
    }
  else
    {
    fobj = new pqCMBFacetedObject(this->FileNames[index].c_str(),
                                    this->Tree->getCurrentServer(),
                                    this->Tree->getCurrentView(),
                                    false);
    obj = fobj;
    this->Sources[index] = obj;
    }
  fobj->setSurfaceType(static_cast<pqCMBSceneObjectBase::enumSurfaceType>(surfaceType));
  // We need the position
  e = elem->FindNestedElementWithName("Position");
  if(e)
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
//----------------------------------------------------------------------------

void pqCMBSceneV1Reader::processObject(vtkXMLDataElement * elem)
{

  std::string otypeName = elem->GetAttribute("ObjectType");
  int stype;
  pqCMBSceneObjectBase::enumObjectType otype =
    static_cast<pqCMBSceneObjectBase::enumObjectType>
    (this->convertToType(otypeName.c_str(), stype));

  pqCMBSceneNode *parent = this->Tree->findNode(elem->GetAttribute("NodeType"));
  pqCMBSceneObjectBase *obj;
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
  else
    {
    obj = this->processFacetedObject(elem, stype);
    }

  if (obj == NULL)
    {
    return;
    }

  e = elem->FindNestedElementWithName("Units");
  if(e)
    {
    obj->setUnits(cmbSceneUnits::convertFromString(e->GetAttribute("Value")));
    }

  e = elem->FindNestedElementWithName("Orientation");
  if(e)
    {
    e->GetVectorAttribute("Value", 3, data3);
    obj->setOrientation(data3, false);
    }
  e = elem->FindNestedElementWithName("Scale");
  if(e)
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

  pqCMBSceneNode *node =
    this->Tree->createNode(elem->GetAttribute("Name"), parent, obj, NULL);

  double color[4];
  colorElement = elem->FindNestedElementWithName("Color");
  if(colorElement &&
    colorElement->FindNestedElementWithName("Inherited") == NULL)
    {
    // Explicit Color
    colorElement->GetVectorAttribute("Value", 4, color);
    node->setExplicitColor(color);
    }

  otypeName.insert(0, "-");
  obj->setUserDefinedType(otypeName.c_str());
  if (elem->FindNestedElementWithName("Invisible"))
    {
    node->setVisibility(false);
    }
  obj->updateRepresentation();
}

//----------------------------------------------------------------------------
int pqCMBSceneV1Reader::processTextureInfo(vtkXMLDataElement *elem,
                                          pqCMBSceneObjectBase *obj)
{
  int index;
  int npoints;
  double points[12];

  elem->GetScalarAttribute("TextureFileEntry", index);
  if (index < 0)
    {
    return -1;
    }

  pqCMBTexturedObject *tobj = dynamic_cast<pqCMBTexturedObject *>(obj);
  if (tobj == NULL)
    {
    return -2;
    }

  elem->GetScalarAttribute("NumberOfRegistrationPoints", npoints);
  elem->GetVectorAttribute("Points", npoints*4, points);

  tobj->setTextureMap(this->TextureFileNames[index].c_str(), npoints, points);
  this->Tree->addTextureFileName(this->TextureFileNames[index].c_str());
  return 0;
}
//----------------------------------------------------------------------------
void pqCMBSceneV1Reader::processConstraints(vtkXMLDataElement *constraintSection,
                                          pqCMBSceneObjectBase *obj)
{
  obj->clearConstraints();
  vtkXMLDataElement *ce;
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
//----------------------------------------------------------------------------
void pqCMBSceneV1Reader::appendStatus(const std::string &newStatus)
{
  if (this->Status != "")
    {
    this->Status += "\n";
    }
  this->Status += newStatus;
}
//----------------------------------------------------------------------------
int pqCMBSceneV1Reader::convertToType(const char *tname, int &stype)
{
  // First see if the name matches a known type
  stype = pqCMBSceneObjectBase::Other;
  int otype = pqCMBSceneObjectBase::convertStringToType(tname);
  if (otype != pqCMBSceneObjectBase::Unknown)
    {
    return otype;
    }
  std::string n = tname;
  if (n == "LIDAR")
    {
    return pqCMBSceneObjectBase::Points;
    }

  // This must be a faceted object - get the surface type
  stype = pqCMBSceneObjectBase::convertStringToSurfaceType(tname);
  return pqCMBSceneObjectBase::Faceted;
}
//----------------------------------------------------------------------------

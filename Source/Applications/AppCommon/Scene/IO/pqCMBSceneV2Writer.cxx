//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqCMBSceneV2Writer - writes out an Version 2 XML representation of a Scene.

#include "pqCMBSceneV2Writer.h"
#include "pqApplicationCore.h"
#include "pqCMBArc.h"
#include "pqCMBConicalRegion.h"
#include "pqCMBFacetedObject.h"
#include "pqCMBGlyphObject.h"
#include "pqCMBLine.h"
#include "pqCMBPlane.h"
#include "pqCMBPoints.h"
#include "pqCMBPolygon.h"
#include "pqCMBSceneNode.h"
#include "pqCMBSceneNodeIterator.h"
#include "pqCMBSceneTree.h"
#include "pqCMBSolidMesh.h"
#include "pqCMBUniformGrid.h"
#include "pqCMBVOI.h"
#include "pqObjectBuilder.h"
#include "pqOutputPort.h"
#include "pqPipelineSource.h"
#include "pqSMAdaptor.h"
#include "pqServer.h"
#include "smtk/bridge/discrete/kernel/Model/vtkUUID.h"
#include "vtkDoubleArray.h"
#include "vtkIdTypeArray.h"
#include "vtkSMSourceProxy.h"
#include "vtkStringWriter.h"
#include "vtkXMLDataElement.h"
#include "vtkXMLUtilities.h"
#include <sstream>
#include <vtksys/SystemTools.hxx>

#include <QList>

#include "vtkNew.h"
#include "vtkPVCMBSceneV2WriterInformation.h"
#include "vtkProcessModule.h"
#include "vtkSMStringVectorProperty.h"
#include "vtkStringList.h"

pqCMBSceneV2Writer::pqCMBSceneV2Writer()
{
  this->Tree = NULL;
  this->WriterSource = NULL;
}

pqCMBSceneV2Writer::~pqCMBSceneV2Writer()
{
}

void pqCMBSceneV2Writer::write(bool packageScene)
{
  // generate the server writer
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();
  this->WriterSource =
    builder->createSource("sources", "CmbSceneV2WriterHelper", core->getActiveServer());
  vtkSMSourceProxy* proxy = vtkSMSourceProxy::SafeDownCast(this->WriterSource->getProxy());

  //generate the contour inputs collection
  this->ContourInputs = new QList<pqOutputPort*>;

  //push the file name to the server
  pqSMAdaptor::setElementProperty(proxy->GetProperty("FileName"), this->FileName.c_str());
  proxy->UpdateProperty("FileName");

  this->UserDefinedObjectTypes.clear();
  vtkXMLDataElement* scene = vtkXMLDataElement::New();
  scene->SetName("Scene");
  scene->SetAttribute("Version", "2.0");
  vtkXMLDataElement* part;
  part = vtkXMLDataElement::New();
  part->SetName("Units");
  part->SetAttribute("Value", cmbSceneUnits::convertToString(this->Tree->getUnits()).c_str());
  scene->AddNestedElement(part);
  part->Delete();
  part = this->getTypes();
  scene->AddNestedElement(part);
  part->Delete();

  part = this->getDirectory(packageScene);
  scene->AddNestedElement(part);
  part->Delete();

  part = this->getTextureDirectory(packageScene);
  scene->AddNestedElement(part);
  part->Delete();

  part = this->getObjects();
  scene->AddNestedElement(part);
  part->Delete();

  //note: this also causes the server to write the contour file
  if (this->ContourInputs->size() > 0)
  {
    part = this->generateContourFile(packageScene);
    scene->AddNestedElement(part);
    part->Delete();
  }
  part = this->getUserDefinedObjectTypes();
  scene->AddNestedElement(part);
  part->Delete();
  vtkIndent indent;

  //now that the xml is generated we are going to push it to the server as a
  //single string
  std::stringstream buffer;
  vtkXMLUtilities::FlattenElement(scene, buffer, &indent);

  pqSMAdaptor::setElementProperty(proxy->GetProperty("Text"), buffer.str().c_str());
  //set the mode to be writing of the xml file
  pqSMAdaptor::setElementProperty(proxy->GetProperty("Mode"), 2);

  proxy->UpdateVTKObjects();
  proxy->UpdatePipeline();

  builder->destroy(this->WriterSource);

  scene->Delete();

  //generate the contour inputs collection
  delete this->ContourInputs;
  this->ContourInputs = NULL;
}

vtkXMLDataElement* pqCMBSceneV2Writer::getUserDefinedObjectTypes()
{
  vtkXMLDataElement* typeSection = vtkXMLDataElement::New();
  typeSection->SetName("UserDefinedTypeDefinitions");
  std::map<std::string, int>::iterator it;
  vtkXMLDataElement* te;
  for (it = this->UserDefinedObjectTypes.begin(); it != this->UserDefinedObjectTypes.end(); ++it)
  {
    te = vtkXMLDataElement::New();
    te->SetName("UserType");
    te->SetAttribute("Name", (*it).first.c_str());
    typeSection->AddNestedElement(te);
    te->Delete();
  }
  return typeSection;
}

vtkXMLDataElement* pqCMBSceneV2Writer::getTypes()
{
  vtkXMLDataElement* typeSection = vtkXMLDataElement::New();
  typeSection->SetName("TypeDefinitions");
  pqCMBSceneNodeIterator iter(this->Tree->getRoot());
  pqCMBSceneNode* n;
  vtkXMLDataElement *te, *ce;
  double color[4];

  while ((n = iter.next()))
  {
    if (!n->isTypeNode())
    {
      continue;
    }
    te = vtkXMLDataElement::New();
    te->SetName("Type");
    te->SetAttribute("Name", n->getName());

    if (n->hasExplicitColor())
    {
      ce = vtkXMLDataElement::New();
      ce->SetName("Color");
      n->getColor(color);
      ce->SetVectorAttribute("Value", 4, color);
      te->AddNestedElement(ce);
      ce->Delete();
    }

    if (!n->isVisible())
    {
      vtkXMLDataElement* ve = vtkXMLDataElement::New();
      ve->SetName("Invisible");
      te->AddNestedElement(ve);
      ve->Delete();
    }

    if (n->isLocked())
    {
      vtkXMLDataElement* ve = vtkXMLDataElement::New();
      ve->SetName("Locked");
      te->AddNestedElement(ve);
      ve->Delete();
    }

    if (n->getParent())
    {
      te->SetAttribute("Parent", n->getParent()->getName());
    }
    typeSection->AddNestedElement(te);
    te->Delete();
  }
  return typeSection;
}

vtkXMLDataElement* pqCMBSceneV2Writer::getDirectory(bool packageScene)
{
  //the goal is to push to the server a list of files that need to be checked
  //and potentially copied. After this process is over we will fetch
  //from the server the updated list of files and add it to the xml tree
  vtkSMSourceProxy* proxy = vtkSMSourceProxy::SafeDownCast(this->WriterSource->getProxy());

  //set if we are packaging the scene
  pqSMAdaptor::setElementProperty(proxy->GetProperty("PackageScene"), packageScene);
  proxy->UpdateProperty("PackageScene");

  //set the mode to be directory parsing
  pqSMAdaptor::setElementProperty(proxy->GetProperty("Mode"), 1);
  proxy->UpdateProperty("Mode");

  pqCMBSceneNodeIterator iter(this->Tree->getRoot());
  pqCMBSceneNode* n;
  int nextEntry = 0;
  vtkStringList* absFiles = vtkStringList::New();
  std::string fname;
  while ((n = iter.next()))
  {
    if (!n->getDataObject())
    {
      continue;
    }
    // is this a file based object (Points or FacetedObject), and image or glyph object?
    fname = "";

    // Is this a points object?
    pqCMBPoints* pobj = dynamic_cast<pqCMBPoints*>(n->getDataObject());
    if (pobj != NULL)
    {
      this->updateDirectoryInfo(pobj->getFileName(), absFiles, nextEntry);
      continue;
    }

    // Is this a faceted Object?
    pqCMBFacetedObject* fobj = dynamic_cast<pqCMBFacetedObject*>(n->getDataObject());

    if (fobj != NULL)
    {
      this->updateDirectoryInfo(fobj->getFileName(), absFiles, nextEntry);
      continue;
    }

    // Is this a solid mesh?
    pqCMBSolidMesh* solobj = dynamic_cast<pqCMBSolidMesh*>(n->getDataObject());

    if (solobj != NULL)
    {
      this->updateDirectoryInfo(solobj->getFileName(), absFiles, nextEntry);
      continue;
    }

    // Is this an Uniform Grid Object?
    pqCMBUniformGrid* uobj = dynamic_cast<pqCMBUniformGrid*>(n->getDataObject());

    if (uobj != NULL)
    {
      this->updateDirectoryInfo(uobj->getFileName(), absFiles, nextEntry);
      continue;
    }

    // Is this a Glyph Object?
    pqCMBGlyphObject* gobj = dynamic_cast<pqCMBGlyphObject*>(n->getDataObject());
    if (gobj != NULL)
    {
      this->updateDirectoryInfo(gobj->getGlyphFileName(), absFiles, nextEntry);
      // Do we have to create a file for the point infomation?
      fname = gobj->getPointsFileName();
      if (fname == "")
      {
        unsigned char uuid[16];
        if (vtkUUID::GenerateUUID(uuid) == -1)
        {
          // We need to use the less unique Id
          vtkUUID::ConstructUUID(uuid);
        }
        // Get the path that the Scene is being written to
        fname = vtksys::SystemTools::GetFilenamePath(this->FileName);
        //now add on the glyph component
        fname += "/GlyphPoints_";
        std::string uname;
        vtkUUID::ConvertBinaryUUIDToString(uuid, uname);
        fname += uname;
        fname += ".vtk";
        gobj->setPointsFileName(fname.c_str());
      }
      this->updateDirectoryInfo(fname, absFiles, nextEntry);
      continue;
    }

    pqCMBPolygon* pgObject = dynamic_cast<pqCMBPolygon*>(n->getDataObject());
    if (pgObject != NULL)
    {
      unsigned char uuid[16];
      if (vtkUUID::GenerateUUID(uuid) == -1)
      {
        // We need to use the less unique Id
        vtkUUID::ConstructUUID(uuid);
      }
      // Get the path that the Scene is being written to
      fname = vtksys::SystemTools::GetFilenamePath(this->FileName);
      //now add on the polygon component
      fname += "/PolygonMesh_";
      std::string uname;
      vtkUUID::ConvertBinaryUUIDToString(uuid, uname);
      fname += uname;
      fname += ".vtp";
      pgObject->setFileName(fname);
      this->updateDirectoryInfo(fname, absFiles, nextEntry);
      continue;
    }
  }

  vtkSMStringVectorProperty* dirProperty =
    vtkSMStringVectorProperty::SafeDownCast(proxy->GetProperty("AddObjectFileName"));
  dirProperty->SetElements(absFiles);

  proxy->UpdateProperty("AddObjectFileName");
  proxy->UpdatePipeline();

  //get the updated paths from the server
  proxy->UpdatePropertyInformation(dirProperty);
  vtkNew<vtkPVCMBSceneV2WriterInformation> info;

  proxy->GatherInformation(info.GetPointer());

  vtkXMLDataElement* dirSection = vtkXMLDataElement::New();
  dirSection->SetName("ObjectFileDirectory");
  vtkXMLDataElement* fe;
  for (int i = 0; i < absFiles->GetNumberOfStrings(); ++i)
  {
    fe = vtkXMLDataElement::New();
    fe->SetName("File");
    fe->SetAttribute("Name", info->GetObjectFileName(i));
    dirSection->AddNestedElement(fe);
    fe->Delete();
  }

  absFiles->Delete();
  return dirSection;
}

void pqCMBSceneV2Writer::updateDirectoryInfo(
  const std::string& fname, vtkStringList* absFiles, int& nextEntry)
{
  if (fname == "")
  {
    return;
  }

  if (this->DirectoryMap.find(fname) == this->DirectoryMap.end())
  {
    //only pass to the server unique strings
    //we need a directory mapping for lookup
    this->DirectoryMap[fname] = nextEntry++;
    absFiles->AddString(fname.c_str());
  }
}

vtkXMLDataElement* pqCMBSceneV2Writer::getTextureDirectory(bool packageScene)
{

  //the goal is to push to the server a list of files that need to be checked
  //and potentially copied. After this process is over we will fetch
  //from the server the updated list of files and add it to the xml tree
  vtkSMSourceProxy* proxy = vtkSMSourceProxy::SafeDownCast(this->WriterSource->getProxy());

  //set if we are packaging the scene
  pqSMAdaptor::setElementProperty(proxy->GetProperty("PackageScene"), packageScene);
  proxy->UpdateProperty("PackageScene");

  //set the mode to be directory parsing
  pqSMAdaptor::setElementProperty(proxy->GetProperty("Mode"), 1);
  proxy->UpdateProperty("Mode");

  pqCMBSceneNodeIterator iter(this->Tree->getRoot());
  pqCMBSceneNode* n;

  int nextEntry = 0;
  vtkStringList* absFiles = vtkStringList::New();
  pqCMBTexturedObject* tobj;
  while ((n = iter.next()))
  {
    if (!n->getDataObject())
    {
      continue;
    }
    tobj = dynamic_cast<pqCMBTexturedObject*>(n->getDataObject());
    // Skip objects w/o textures
    if ((tobj == NULL) || (!tobj->hasTexture()))
    {
      continue;
    }
    std::string fname = tobj->getTextureFileName().toStdString();
    if (this->TextureDirectoryMap.find(fname) == this->TextureDirectoryMap.end())
    {
      //only pass to the server unique strings
      //we need a directory mapping for lookup
      this->TextureDirectoryMap[fname] = nextEntry++;
      absFiles->AddString(fname.c_str());
    }
  }

  vtkSMStringVectorProperty* dirProperty =
    vtkSMStringVectorProperty::SafeDownCast(proxy->GetProperty("AddObjectFileName"));
  dirProperty->SetElements(absFiles);

  proxy->UpdateProperty("AddObjectFileName");
  proxy->UpdatePipeline();

  //get the updated paths from the server
  proxy->UpdatePropertyInformation(dirProperty);
  vtkNew<vtkPVCMBSceneV2WriterInformation> info;

  proxy->GatherInformation(info.GetPointer());

  vtkXMLDataElement* dirSection = vtkXMLDataElement::New();
  dirSection->SetName("TextureFileDirectory");
  vtkXMLDataElement* fe;
  for (int i = 0; i < absFiles->GetNumberOfStrings(); ++i)
  {
    fe = vtkXMLDataElement::New();
    fe->SetName("File");

    QString relativePath = pqSMAdaptor::getMultipleElementProperty(dirProperty, i).toString();
    fe->SetAttribute("Name", info->GetObjectFileName(i));
    dirSection->AddNestedElement(fe);
    fe->Delete();
  }

  absFiles->Delete();
  return dirSection;
}

vtkXMLDataElement* pqCMBSceneV2Writer::getObjects()
{
  vtkXMLDataElement* objSection = vtkXMLDataElement::New();
  vtkXMLDataElement* oe;
  objSection->SetName("ObjectDefinitions");
  SceneObjectNodeIterator iter(this->Tree->getRoot());
  pqCMBSceneNode* n;
  while ((n = iter.next()))
  {
    oe = getObjectDescription(n);
    objSection->AddNestedElement(oe);
    oe->Delete();
  }
  return objSection;
}

vtkXMLDataElement* pqCMBSceneV2Writer::getObjectDescription(pqCMBSceneNode* node)
{
  pqCMBSceneObjectBase* obj = node->getDataObject();
  if (!obj)
  {
    return NULL;
  }

  vtkXMLDataElement* elem = vtkXMLDataElement::New();
  elem->SetName("Object");
  elem->SetAttribute("Name", node->getName());
  elem->SetAttribute("NodeType", node->getParent()->getName());
  elem->SetAttribute("ObjectType", obj->getTypeAsString().c_str());
  std::string uname = obj->getUserDefinedType();
  elem->SetAttribute("UserDefinedType", uname.c_str());
  if (this->UserDefinedObjectTypes.find(uname) == this->UserDefinedObjectTypes.end())
  {
    this->UserDefinedObjectTypes[uname] = 1;
  }

  if (dynamic_cast<pqCMBVOI*>(obj) != NULL)
  {
    this->processVOI(node, elem);
  }
  else if (dynamic_cast<pqCMBPoints*>(obj) != NULL)
  {
    this->processPoints(node, elem);
  }
  else if (dynamic_cast<pqCMBLine*>(obj) != NULL)
  {
    this->processLine(node, elem);
  }
  else if (dynamic_cast<pqCMBPlane*>(obj) != NULL)
  {
    this->processPlane(node, elem);
  }
  else if (dynamic_cast<pqCMBConicalRegion*>(obj) != NULL)
  {
    this->processCone(node, elem);
  }
  else if (dynamic_cast<pqCMBFacetedObject*>(obj) != NULL)
  {
    this->processFacetedObject(node, elem);
  }
  else if (dynamic_cast<pqCMBGlyphObject*>(obj) != NULL)
  {
    this->processGlyphObject(node, elem);
  }
  else if (dynamic_cast<pqCMBUniformGrid*>(obj) != NULL)
  {
    this->processUniformGrid(node, elem);
  }
  else if (dynamic_cast<pqCMBArc*>(obj) != NULL)
  {
    this->processContour(node, elem);
  }
  else if (dynamic_cast<pqCMBPolygon*>(obj) != NULL)
  {
    this->processPolygon(node, elem);
  }
  else if (dynamic_cast<pqCMBSolidMesh*>(obj) != NULL)
  {
    this->processSolidMesh(node, elem);
  }

  return elem;
}

void pqCMBSceneV2Writer::processVOI(pqCMBSceneNode* node, vtkXMLDataElement* elem)
{
  pqCMBVOI* obj = dynamic_cast<pqCMBVOI*>(node->getDataObject());
  elem->SetAttribute("VOIType", "Bounds");

  double bounds[6];
  vtkXMLDataElement* be = vtkXMLDataElement::New();
  be->SetName("Bounds");
  obj->getDataBounds(bounds);
  be->SetVectorAttribute("Value", 6, bounds);
  elem->AddNestedElement(be);
  be->Delete();
  this->processGeometricProperties(node, elem);
}

void pqCMBSceneV2Writer::processLine(pqCMBSceneNode* node, vtkXMLDataElement* elem)
{
  pqCMBLine* obj = dynamic_cast<pqCMBLine*>(node->getDataObject());
  elem->SetAttribute("LineType", "StraightLine");

  double pos[3];
  vtkXMLDataElement* be1 = vtkXMLDataElement::New();
  be1->SetName("Point1WorldPosition");
  (static_cast<pqCMBLine*>(obj))->getPoint1Position(pos[0], pos[1], pos[2]);
  be1->SetVectorAttribute("Value", 3, pos);
  elem->AddNestedElement(be1);
  be1->Delete();
  vtkXMLDataElement* be2 = vtkXMLDataElement::New();
  be2->SetName("Point2WorldPosition");
  (static_cast<pqCMBLine*>(obj))->getPoint2Position(pos[0], pos[1], pos[2]);
  be2->SetVectorAttribute("Value", 3, pos);
  elem->AddNestedElement(be2);
  be2->Delete();
  this->processNonTransformProperties(node, elem);
}

void pqCMBSceneV2Writer::processContour(pqCMBSceneNode* node, vtkXMLDataElement* elem)
{
  pqCMBArc* obj = dynamic_cast<pqCMBArc*>(node->getDataObject());
  if (!obj)
  {
    return;
  }
  pqOutputPort* port = obj->getSource()->getOutputPort(0);
  if (!port)
  {
    return;
  }
  //add this contour to the collection of filters as input for the contour writer
  this->ContourInputs->push_back(port);

  //write out the xml info for this contour
  vtkSmartPointer<vtkXMLDataElement> normalElement = vtkSmartPointer<vtkXMLDataElement>::New();
  normalElement->SetName("PlaneProjectionNormal");
  normalElement->SetIntAttribute("Value", obj->getPlaneProjectionNormal());
  elem->AddNestedElement(normalElement);

  vtkSmartPointer<vtkXMLDataElement> positionElement = vtkSmartPointer<vtkXMLDataElement>::New();
  positionElement->SetName("PlanePositionOffset");
  positionElement->SetDoubleAttribute("Value", obj->getPlaneProjectionPosition());
  elem->AddNestedElement(positionElement);

  vtkSmartPointer<vtkXMLDataElement> closedElement = vtkSmartPointer<vtkXMLDataElement>::New();
  closedElement->SetName("ClosedLoop");
  closedElement->SetIntAttribute("Value", obj->getClosedLoop());
  elem->AddNestedElement(closedElement);

  this->processNonTransformProperties(node, elem);
}

void pqCMBSceneV2Writer::processPlane(pqCMBSceneNode* node, vtkXMLDataElement* elem)
{
  pqCMBPlane* obj = dynamic_cast<pqCMBPlane*>(node->getDataObject());
  double p1[3], p2[3];
  vtkXMLDataElement* be;
  obj->getPlaneInfo(p1, p2);
  be = vtkXMLDataElement::New();
  be->SetName("Point1");
  be->SetVectorAttribute("Value", 3, p1);
  elem->AddNestedElement(be);
  be->Delete();
  be = vtkXMLDataElement::New();
  be->SetName("Point2");
  be->SetVectorAttribute("Value", 3, p2);
  elem->AddNestedElement(be);
  be->Delete();
  this->processGeometricProperties(node, elem);
}

void pqCMBSceneV2Writer::processCone(pqCMBSceneNode* node, vtkXMLDataElement* elem)
{
  pqCMBConicalRegion* obj = dynamic_cast<pqCMBConicalRegion*>(node->getDataObject());
  double p1[3], dir[3], a;
  int b;
  vtkXMLDataElement* be;
  obj->getBaseCenter(p1);
  be = vtkXMLDataElement::New();
  be->SetName("BaseCenter");
  be->SetVectorAttribute("Value", 3, p1);
  elem->AddNestedElement(be);
  be->Delete();
  be = vtkXMLDataElement::New();
  obj->getDirection(dir);
  be->SetName("Direction");
  be->SetVectorAttribute("Value", 3, dir);
  elem->AddNestedElement(be);
  be->Delete();
  be = vtkXMLDataElement::New();
  a = obj->getHeight();
  be->SetName("Height");
  be->SetDoubleAttribute("Value", a);
  elem->AddNestedElement(be);
  be->Delete();
  be = vtkXMLDataElement::New();
  a = obj->getBaseRadius();
  be->SetName("BaseRadius");
  be->SetDoubleAttribute("Value", a);
  elem->AddNestedElement(be);
  be->Delete();
  be = vtkXMLDataElement::New();
  a = obj->getTopRadius();
  be->SetName("TopRadius");
  be->SetDoubleAttribute("Value", a);
  elem->AddNestedElement(be);
  be->Delete();
  be = vtkXMLDataElement::New();
  b = obj->getResolution();
  be->SetName("Resolution");
  be->SetIntAttribute("Value", b);
  elem->AddNestedElement(be);
  be->Delete();
  this->processGeometricProperties(node, elem);
}

void pqCMBSceneV2Writer::processFacetedObject(pqCMBSceneNode* node, vtkXMLDataElement* elem)
{
  pqCMBFacetedObject* obj = dynamic_cast<pqCMBFacetedObject*>(node->getDataObject());
  elem->SetAttribute("SurfaceType", obj->getSurfaceTypeAsString().c_str());
  std::map<std::string, int>::iterator iter = this->DirectoryMap.find(obj->getFileName());
  if (iter == this->DirectoryMap.end())
  {
    elem->SetIntAttribute("FileEntry", -2);
  }
  else
  {
    elem->SetIntAttribute("FileEntry", (*iter).second);
  }
  this->processGeometricProperties(node, elem);
}

void pqCMBSceneV2Writer::processSolidMesh(pqCMBSceneNode* node, vtkXMLDataElement* elem)
{
  pqCMBSolidMesh* obj = dynamic_cast<pqCMBSolidMesh*>(node->getDataObject());
  std::map<std::string, int>::iterator iter = this->DirectoryMap.find(obj->getFileName());
  if (iter == this->DirectoryMap.end())
  {
    elem->SetIntAttribute("FileEntry", -2);
  }
  else
  {
    elem->SetIntAttribute("FileEntry", (*iter).second);
  }
  this->processGeometricProperties(node, elem);
}

void pqCMBSceneV2Writer::processUniformGrid(pqCMBSceneNode* node, vtkXMLDataElement* elem)
{
  pqCMBUniformGrid* obj = dynamic_cast<pqCMBUniformGrid*>(node->getDataObject());
  std::map<std::string, int>::iterator iter = this->DirectoryMap.find(obj->getFileName());
  if (iter == this->DirectoryMap.end())
  {
    elem->SetIntAttribute("FileEntry", -2);
  }
  else
  {
    elem->SetIntAttribute("FileEntry", (*iter).second);
  }

  if (obj->isRawDEM())
  {
    vtkXMLDataElement* ue = vtkXMLDataElement::New();
    ue->SetName("RawDEMInfo");
    vtkIdType a[2], b[2];
    a[0] = (obj->getReadGroupOfFiles()) ? 1 : 0;
    ue->SetIntAttribute("ReadGroupOfFiles", a[0]);
    ue->SetIntAttribute("OnRatio", obj->getOnRatio());
    obj->getExtents(a, b);
    ue->SetVectorAttribute("RowExtents", 2, a);
    ue->SetVectorAttribute("ColumnExtents", 2, b);
    elem->AddNestedElement(ue);
    ue->Delete();
  }
  this->processGeometricProperties(node, elem);
}

void pqCMBSceneV2Writer::processPoints(pqCMBSceneNode* node, vtkXMLDataElement* elem)
{
  pqCMBPoints* obj = dynamic_cast<pqCMBPoints*>(node->getDataObject());
  if (obj->getReaderSource())
  {
    if (obj->getPieceId() == -1)
    {
      // old style, all pieces appended together.  Should only happen if read
      // in an old sg file
      vtkSMSourceProxy* sourceProxy = vtkSMSourceProxy::SafeDownCast(obj->getSource()->getProxy());
      sourceProxy->UpdatePropertyInformation();
      int maxNumberOfPoints =
        pqSMAdaptor::getElementProperty(sourceProxy->GetProperty("GetMaxNumberOfPoints")).toInt();
      elem->SetIntAttribute("MaxNumberOfPoints", maxNumberOfPoints);
    }
    else
    {
      elem->SetIntAttribute("PieceId", obj->getPieceId());
      elem->SetIntAttribute("PieceOnRatio", obj->getPieceOnRatio());
    }
    elem->SetAttribute("DataPrecision", (obj->getDoubleDataPrecision() ? "double" : "float"));
  }

  std::map<std::string, int>::iterator iter = this->DirectoryMap.find(obj->getFileName());
  if (iter == this->DirectoryMap.end())
  {
    elem->SetIntAttribute("FileEntry", -2);
  }
  else
  {
    elem->SetIntAttribute("FileEntry", (*iter).second);
  }
  this->processGeometricProperties(node, elem);
}

void pqCMBSceneV2Writer::processGlyphObject(pqCMBSceneNode* node, vtkXMLDataElement* elem)
{
  pqCMBGlyphObject* obj = dynamic_cast<pqCMBGlyphObject*>(node->getDataObject());
  elem->SetAttribute("SurfaceType", obj->getSurfaceTypeAsString().c_str());
  obj->writePointsFile();
  std::map<std::string, int>::iterator iter = this->DirectoryMap.find(obj->getGlyphFileName());
  if (iter == this->DirectoryMap.end())
  {
    elem->SetIntAttribute("GlyphFileEntry", -2);
  }
  else
  {
    elem->SetIntAttribute("GlyphFileEntry", (*iter).second);
  }

  iter = this->DirectoryMap.find(obj->getPointsFileName());
  if (iter == this->DirectoryMap.end())
  {
    elem->SetIntAttribute("PointsFileEntry", -2);
  }
  else
  {
    elem->SetIntAttribute("PointsFileEntry", (*iter).second);
  }
  this->processGeometricProperties(node, elem);
}

void pqCMBSceneV2Writer::processPolygon(pqCMBSceneNode* node, vtkXMLDataElement* elem)
{
  pqCMBPolygon* obj = dynamic_cast<pqCMBPolygon*>(node->getDataObject());
  std::map<std::string, int>::iterator iter = this->DirectoryMap.find(obj->getFileName());
  if (iter == this->DirectoryMap.end())
  {
    elem->SetIntAttribute("PolygonFileEntry", -2);
  }
  else
  {
    elem->SetIntAttribute("PolygonFileEntry", (*iter).second);
    obj->writeToFile();
  }

  elem->SetDoubleAttribute("MinAngle", obj->getMinAngle());
  elem->SetDoubleAttribute("EdgeLength", obj->getEdgeLength());

  //get the list of arcs this node is mapped to.
  //these need to be saved so that we can restore the manager
  const std::set<pqCMBArc*>& arcs = obj->arcsUsedByPolygon();
  if (arcs.size() > 0)
  {
    vtkXMLDataElement* arcsDE = vtkXMLDataElement::New();
    arcsDE->SetName("Arcs");
    arcsDE->SetIntAttribute("Size", static_cast<int>(arcs.size()));
    typedef std::set<pqCMBArc*>::const_iterator arc_it;
    for (arc_it i = arcs.begin(); i != arcs.end(); ++i)
    {
      vtkXMLDataElement* arcDE = vtkXMLDataElement::New();
      arcDE->SetName("Arc");
      arcDE->SetAttribute("Name", this->Tree->findNode(*i)->getName());
      arcsDE->AddNestedElement(arcDE);
      arcDE->Delete();
    }
    elem->AddNestedElement(arcsDE);
    arcsDE->Delete();
  }
  this->processGeometricProperties(node, elem);
}

void pqCMBSceneV2Writer::processNonTransformProperties(
  pqCMBSceneNode* node, vtkXMLDataElement* elem)
{
  pqCMBSceneObjectBase* obj = node->getDataObject();
  vtkXMLDataElement* ue = vtkXMLDataElement::New();
  ue->SetName("Units");
  ue->SetAttribute("Value", cmbSceneUnits::convertToString(obj->getUnits()).c_str());
  elem->AddNestedElement(ue);
  ue->Delete();

  vtkXMLDataElement* ce = vtkXMLDataElement::New();
  double color[4];
  ce->SetName("Color");
  obj->getColor(color);
  ce->SetVectorAttribute("Value", 4, color);
  // Is the color inherited?
  if (!node->hasExplicitColor())
  {
    vtkXMLDataElement* ie = vtkXMLDataElement::New();
    ie->SetName("Inherited");
    ce->AddNestedElement(ie);
    ie->Delete();
  }
  elem->AddNestedElement(ce);
  ce->Delete();

  if (!node->isVisible())
  {
    vtkXMLDataElement* ve = vtkXMLDataElement::New();
    ve->SetName("Invisible");
    elem->AddNestedElement(ve);
    ve->Delete();
  }

  if (node->isLocked())
  {
    vtkXMLDataElement* ve = vtkXMLDataElement::New();
    ve->SetName("Locked");
    elem->AddNestedElement(ve);
    ve->Delete();
  }

  if (obj->isSnapTarget())
  {
    vtkXMLDataElement* ve = vtkXMLDataElement::New();
    ve->SetName("SnapTarget");
    elem->AddNestedElement(ve);
    ve->Delete();
  }
}

void pqCMBSceneV2Writer::processGeometricProperties(pqCMBSceneNode* node, vtkXMLDataElement* elem)
{
  this->processNonTransformProperties(node, elem);

  pqCMBSceneObjectBase* obj = node->getDataObject();

  double data[3];
  vtkXMLDataElement* placement = vtkXMLDataElement::New();
  placement->SetName("Position");
  obj->getPosition(data);
  placement->SetVectorAttribute("Value", 3, data);
  elem->AddNestedElement(placement);
  placement->Delete();

  placement = vtkXMLDataElement::New();
  placement->SetName("Orientation");
  obj->getOrientation(data);
  placement->SetVectorAttribute("Value", 3, data);
  elem->AddNestedElement(placement);
  placement->Delete();

  placement = vtkXMLDataElement::New();
  placement->SetName("Scale");
  obj->getScale(data);
  placement->SetVectorAttribute("Value", 3, data);
  elem->AddNestedElement(placement);
  placement->Delete();

  placement = vtkXMLDataElement::New();
  placement->SetName("Origin");
  obj->getOrigin(data);
  placement->SetVectorAttribute("Value", 3, data);
  elem->AddNestedElement(placement);
  placement->Delete();

  vtkXMLDataElement* ce = this->addConstraints(obj);
  elem->AddNestedElement(ce);
  ce->Delete();

  pqCMBTexturedObject* tobj = dynamic_cast<pqCMBTexturedObject*>(obj);
  if (tobj != NULL)
  {
    if (tobj->hasTexture())
    {
      ce = this->addTextureInfo(tobj);
      elem->AddNestedElement(ce);
      ce->Delete();
    }
    // Save Elevation
    if (tobj->showingElevation())
    {
      vtkXMLDataElement* ve = vtkXMLDataElement::New();
      ve->SetName("ShowElevation");
      elem->AddNestedElement(ve);
      ve->Delete();
    }
  }
}

vtkXMLDataElement* pqCMBSceneV2Writer::addConstraints(pqCMBSceneObjectBase* obj)
{
  vtkXMLDataElement* elem = vtkXMLDataElement::New();
  elem->SetName("Constraints");

  if (obj->isXTransConstrain())
  {
    vtkXMLDataElement* ce = vtkXMLDataElement::New();
    ce->SetName("XTransConstraint");
    elem->AddNestedElement(ce);
    ce->Delete();
  }

  if (obj->isYTransConstrain())
  {
    vtkXMLDataElement* ce = vtkXMLDataElement::New();
    ce->SetName("YTransConstraint");
    elem->AddNestedElement(ce);
    ce->Delete();
  }

  if (obj->isZTransConstrain())
  {
    vtkXMLDataElement* ce = vtkXMLDataElement::New();
    ce->SetName("ZTransConstraint");
    elem->AddNestedElement(ce);
    ce->Delete();
  }

  if (obj->isXRotationConstrain())
  {
    vtkXMLDataElement* ce = vtkXMLDataElement::New();
    ce->SetName("XRotationConstraint");
    elem->AddNestedElement(ce);
    ce->Delete();
  }

  if (obj->isYRotationConstrain())
  {
    vtkXMLDataElement* ce = vtkXMLDataElement::New();
    ce->SetName("YRotationConstraint");
    elem->AddNestedElement(ce);
    ce->Delete();
  }

  if (obj->isZRotationConstrain())
  {
    vtkXMLDataElement* ce = vtkXMLDataElement::New();
    ce->SetName("ZRotationConstraint");
    elem->AddNestedElement(ce);
    ce->Delete();
  }

  if (obj->isIsotropicScalingConstrain())
  {
    vtkXMLDataElement* ce = vtkXMLDataElement::New();
    ce->SetName("IsotropicScalingConstraint");
    elem->AddNestedElement(ce);
    ce->Delete();
  }

  if (obj->isXScalingConstrain())
  {
    vtkXMLDataElement* ce = vtkXMLDataElement::New();
    ce->SetName("XScalingConstraint");
    elem->AddNestedElement(ce);
    ce->Delete();
  }

  if (obj->isYScalingConstrain())
  {
    vtkXMLDataElement* ce = vtkXMLDataElement::New();
    ce->SetName("YScalingConstraint");
    elem->AddNestedElement(ce);
    ce->Delete();
  }

  if (obj->isZScalingConstrain())
  {
    vtkXMLDataElement* ce = vtkXMLDataElement::New();
    ce->SetName("ZScalingConstraint");
    elem->AddNestedElement(ce);
    ce->Delete();
  }
  return elem;
}

vtkXMLDataElement* pqCMBSceneV2Writer::addTextureInfo(pqCMBTexturedObject* obj)
{
  vtkXMLDataElement* elem = vtkXMLDataElement::New();
  elem->SetName("TextureInfo");

  std::map<std::string, int>::iterator iter =
    this->TextureDirectoryMap.find(obj->getTextureFileName().toStdString());
  if (iter == this->TextureDirectoryMap.end())
  {
    elem->SetIntAttribute("TextureFileEntry", -2);
  }
  else
  {
    elem->SetIntAttribute("TextureFileEntry", (*iter).second);
  }

  int n = obj->getNumberOfRegistrationPoints();
  int i, j;
  double data[12];
  for (i = 0, j = 0; i < n; i++, j += 4)
  {
    obj->getRegistrationPointPair(i, &(data[j]), &(data[j + 2]));
  }

  elem->SetIntAttribute("NumberOfRegistrationPoints", n);
  elem->SetVectorAttribute("Points", 4 * n, data);
  return elem;
}

std::string pqCMBSceneV2Writer::getContourFileName(bool packageScene, std::string absoluteFileName)
{
  //the scene file only uses relative paths, so we need to query the writer
  //to get the correct path for the contour file
  vtkSMSourceProxy* proxy = vtkSMSourceProxy::SafeDownCast(this->WriterSource->getProxy());

  //set if we are packaging the scene
  pqSMAdaptor::setElementProperty(proxy->GetProperty("PackageScene"), packageScene);
  proxy->UpdateProperty("PackageScene");

  //set the mode to be directory parsing
  pqSMAdaptor::setElementProperty(proxy->GetProperty("Mode"), 1);
  proxy->UpdateProperty("Mode");

  //need to specify the contour file that this scene is linked too

  vtkStringList* absFiles = vtkStringList::New();
  absFiles->AddString(absoluteFileName.c_str());

  vtkSMStringVectorProperty* dirProperty =
    vtkSMStringVectorProperty::SafeDownCast(proxy->GetProperty("AddObjectFileName"));
  dirProperty->SetElements(absFiles);

  proxy->UpdateProperty("AddObjectFileName");
  proxy->UpdatePipeline();

  //get the updated paths from the server
  proxy->UpdatePropertyInformation(dirProperty);
  vtkNew<vtkPVCMBSceneV2WriterInformation> info;

  proxy->GatherInformation(info.GetPointer());

  absFiles->Delete();
  std::string relativeFileName = info->GetObjectFileName(0);
  return relativeFileName;
}

vtkXMLDataElement* pqCMBSceneV2Writer::generateContourFile(bool packageScene)
{
  //drop the .sg extension and add _contour.vtp
  std::string absFileName = this->FileName;
  size_t pos = absFileName.rfind(".");
  if (pos != std::string::npos)
  {
    absFileName.resize(pos);
  }
  //now add on the contour tag
  absFileName += "_contour.vtp";

  std::string relativeFileName = this->getContourFileName(packageScene, absFileName);

  vtkXMLDataElement* objSection = vtkXMLDataElement::New();
  objSection->SetName("ContourDefinitions");
  objSection->SetAttribute("FileName", relativeFileName.c_str());

  //add the inputs to the QMap
  QMap<QString, QList<pqOutputPort*> > namedInputs;
  namedInputs["Input"] = *this->ContourInputs;

  //now that we have collected all the contour info, write out the file
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();
  pqPipelineSource* writer = builder->createFilter(
    "writers", "SceneGenV2ContourWriter", namedInputs, core->getActiveServer());

  vtkSMSourceProxy* proxy = vtkSMSourceProxy::SafeDownCast(writer->getProxy());

  pqSMAdaptor::setElementProperty(proxy->GetProperty("FileName"), absFileName.c_str());
  proxy->UpdateProperty("FileName");
  proxy->UpdatePipeline();

  //now that the file has been written, delete the writer
  builder->destroy(writer);

  return objSection;
}

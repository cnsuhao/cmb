/*=========================================================================

   Program: ConceptualModelBuilder
   Module:    pqCMBSceneV1Writer.cxx

   Copyright (c) Kitware Inc.
   All rights reserved.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#include "pqCMBSceneV1Writer.h"
#include "pqCMBSceneTree.h"
#include "SceneNode.h"
#include "SceneNodeIterator.h"
#include "pqCMBLine.h"
#include "CmbScenePolyline.h"
#include "pqPipelineSource.h"
#include "pqSMAdaptor.h"
#include "vtkDoubleArray.h"
#include "vtkIdTypeArray.h"
#include "vtkPVContourRepresentationInfo.h"
#include "vtkSMSourceProxy.h"
#include "vtkXMLDataElement.h"
#include "vtkXMLUtilities.h"
#include <vtksys/SystemTools.hxx>
#include <sstream>

//----------------------------------------------------------------------------
pqCMBSceneV1Writer::~pqCMBSceneV1Writer()
{
}

//----------------------------------------------------------------------------
void pqCMBSceneV1Writer::write(bool packageScene)
{
  vtkXMLDataElement *scene = vtkXMLDataElement::New();
  scene->SetName("Scene");
  scene->SetAttribute("Version", "1.0");
  vtkXMLDataElement *part;
  part = vtkXMLDataElement::New();
  part->SetName("Units");
  part->SetAttribute("Value",
                     cmbSceneUnits::convertToString(this->Tree->getUnits()).c_str());
  scene->AddNestedElement(part);
  part->Delete();
  part= this->getTypes();
  scene->AddNestedElement(part);
  part->Delete();

  part= this->getDirectory(packageScene);
  scene->AddNestedElement(part);
  part->Delete();

  part= this->getTextureDirectory(packageScene);
  scene->AddNestedElement(part);
  part->Delete();

  part = this->getObjects();
  scene->AddNestedElement(part);
  part->Delete();

  vtkIndent indent;
  vtkXMLUtilities::WriteElementToFile(scene, this->FileName.c_str(), &indent);
  scene->Delete();
}

//----------------------------------------------------------------------------
vtkXMLDataElement *pqCMBSceneV1Writer::getTypes()
{
  vtkXMLDataElement *typeSection = vtkXMLDataElement::New();
  typeSection->SetName("TypeDefinitions");
  SceneNodeIterator iter(this->Tree->getRoot());
  SceneNode *n;
  vtkXMLDataElement *te, *ce;
  double color[4];

  while(n = iter.next())
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

    if (n->getParent())
      {
      te->SetAttribute("Parent", n->getParent()->getName());
      }
    typeSection->AddNestedElement(te);
    te->Delete();
    }
  return typeSection;
}

//----------------------------------------------------------------------------
vtkXMLDataElement *pqCMBSceneV1Writer::getDirectory(bool packageScene)
{
  vtkXMLDataElement *dirSection = vtkXMLDataElement::New();
  dirSection->SetName("ObjectFileDirectory");
  SceneNodeIterator iter(this->Tree->getRoot());
  SceneNode *n;
  vtkXMLDataElement *fe;
  int nextEntry = 0;
  this->DirectoryMap.clear();
  std::string fullPath =
    vtksys::SystemTools::CollapseFullPath( this->FileName.c_str());
  std::string dataPath = vtksys::SystemTools::GetFilenamePath( fullPath );
  std::string fnameComp, fPath, newFile;
  std::string relPath;
  while(n = iter.next())
    {
    if (!n->getDataObject())
      {
      continue;
      }
    // Skip all nodes that  do not have filenames
    if ((n->getDataObject()->getType() == pqCMBSceneObjectBase::VOI) ||
        (n->getDataObject()->getType() == pqCMBSceneObjectBase::GroundPlane) ||
        (n->getDataObject()->getType() == pqCMBSceneObjectBase::Line) ||
        (n->getDataObject()->getType() == pqCMBSceneObjectBase::Contour))
      {
      continue;
      }
    std::string fname = n->getDataObject()->getFileName();
    if (this->DirectoryMap.find(fname) !=
        this->DirectoryMap.end())
      {
      continue;
      }

    std::string fnameFullPath =
      vtksys::SystemTools::CollapseFullPath( fname.c_str());
    if (packageScene)
      {
      // See if the file is in the same directory as the scene file
      // if its not we need to copy the file - do this by comparing th
      // paths
      fPath = vtksys::SystemTools::GetFilenamePath( fnameFullPath );
      fnameComp = vtksys::SystemTools::GetFilenameName( fname );

      if (fPath != dataPath)
        {
        newFile = dataPath;
        newFile.append("/");
        newFile.append(fnameComp);
        vtksys::SystemTools::CopyFileAlways(fnameFullPath.c_str(),
                                            newFile.c_str());
        }
      relPath = fnameComp;
      }
    else
      {
      relPath = vtksys::SystemTools::RelativePath(dataPath.c_str(),
                                                  fnameFullPath.c_str());
      }
    this->DirectoryMap[fname] = nextEntry++;
    fe = vtkXMLDataElement::New();
    fe->SetName("File");
    fe->SetAttribute("Name", relPath.c_str());
    dirSection->AddNestedElement(fe);
    fe->Delete();
    }
  return dirSection;
}

//----------------------------------------------------------------------------
vtkXMLDataElement *pqCMBSceneV1Writer::getTextureDirectory(bool packageScene)
{
  vtkXMLDataElement *dirSection = vtkXMLDataElement::New();
  dirSection->SetName("TextureFileDirectory");
  SceneNodeIterator iter(this->Tree->getRoot());
  SceneNode *n;
  vtkXMLDataElement *fe;
  int nextEntry = 0;
  this->TextureDirectoryMap.clear();
  std::string fullPath =
    vtksys::SystemTools::CollapseFullPath( this->FileName.c_str());
  std::string dataPath = vtksys::SystemTools::GetFilenamePath( fullPath );
  std::string relPath;
  std::string fnameComp, fPath, newFile;
  while(n = iter.next())
    {
    if (!n->getDataObject())
      {
      continue;
      }
    // Skip objects w/o textures
    if (!n->getDataObject()->hasTexture())
      {
      continue;
      }
    std::string fname = n->getDataObject()->getTextureFileName();
    if (this->TextureDirectoryMap.find(fname) !=
        this->TextureDirectoryMap.end())
      {
      continue;
      }

    std::string fnameFullPath =
      vtksys::SystemTools::CollapseFullPath( fname.c_str());

    if (packageScene)
      {
      // See if the file is in the same directory as the scene file
      // if its not we need to copy the file - do this by comparing th
      // paths
      fPath = vtksys::SystemTools::GetFilenamePath( fnameFullPath );
      fnameComp = vtksys::SystemTools::GetFilenameName( fname );

      if (fPath != dataPath)
        {
        newFile = dataPath;
        newFile.append("/");
        newFile.append(fnameComp);
        vtksys::SystemTools::CopyFileAlways(fnameFullPath.c_str(),
                                            newFile.c_str());
        }
      relPath = fnameComp;
      }
    else
      {
      relPath = vtksys::SystemTools::RelativePath(dataPath.c_str(),
                                                  fnameFullPath.c_str());
      }
    this->TextureDirectoryMap[fname] = nextEntry++;
    fe = vtkXMLDataElement::New();
    fe->SetName("File");
    fe->SetAttribute("Name", relPath.c_str());
    dirSection->AddNestedElement(fe);
    fe->Delete();
    }
  return dirSection;
}

//----------------------------------------------------------------------------
vtkXMLDataElement *pqCMBSceneV1Writer::getObjects()
{
  vtkXMLDataElement *objSection = vtkXMLDataElement::New();
  vtkXMLDataElement *oe;
  objSection->SetName("ObjectDefinitions");
  SceneObjectNodeIterator iter(this->Tree->getRoot());
  SceneNode *n;
  while(n = iter.next())
    {
    oe = getObjectDescription(n);
    objSection->AddNestedElement(oe);
    oe->Delete();
    }
  return objSection;
}

//----------------------------------------------------------------------------
vtkXMLDataElement *pqCMBSceneV1Writer::getObjectDescription(SceneNode *node)
{
  pqCMBSceneObjectBase *obj = node->getDataObject();
  if (!obj)
    {
    return NULL;
    }

  vtkXMLDataElement *elem = vtkXMLDataElement::New();
  elem->SetName("Object");
  elem->SetAttribute("Name", node->getName());
  elem->SetAttribute("NodeType", node->getParent()->getName());
  elem->SetAttribute("ObjectType", obj->getTypeAsString().c_str());
  if (obj->getType() == pqCMBSceneObjectBase::LIDAR && obj->getReaderSource())
    {
    if (obj->getPieceId() == -1)
      {
      // old style, all pieces appended together.  Should only happen if read
      // in an old sg file
      vtkSMSourceProxy* sourceProxy = vtkSMSourceProxy::SafeDownCast(
        obj->getSource()->getProxy() );
      sourceProxy->UpdatePropertyInformation();
      int maxNumberOfPoints = pqSMAdaptor::getElementProperty(
        sourceProxy->GetProperty("GetMaxNumberOfPoints")).toInt();
      elem->SetIntAttribute("MaxNumberOfPoints", maxNumberOfPoints);
      }
    else
      {
      elem->SetIntAttribute("PieceId", obj->getPieceId());
      elem->SetIntAttribute("PieceOnRatio", obj->getPieceOnRatio());
      }
    }

  if (obj->getType() == pqCMBSceneObjectBase::VOI)
    {
    this->processVOI(node, elem);
    }
  else if (obj->getType() == pqCMBSceneObjectBase::Line)
    {
    this->processLine(node, elem);
    }
  else if (obj->getType() == pqCMBSceneObjectBase::Contour)
    {
    this->processContour(node, elem);
    }
  else if (obj->getType() == pqCMBSceneObjectBase::GroundPlane)
    {
    this->processPlane(node, elem);
    }
  else // File Based Object
    {
    this->processFileBasedObj(node, elem);
    }

  return elem;
}

void pqCMBSceneV1Writer::processVOI(SceneNode *node, vtkXMLDataElement *elem)
{
  pqCMBSceneObjectBase *obj = node->getDataObject();
  elem->SetAttribute("VOIType", "Bounds");

  double bounds[6];
  vtkXMLDataElement *be = vtkXMLDataElement::New();
  be->SetName("Bounds");
  obj->getDataBounds(bounds);
  be->SetVectorAttribute("Value", 6, bounds);
  elem->AddNestedElement(be);
  be->Delete();
  this->processGeometricProperties(node, elem);
}

void pqCMBSceneV1Writer::processLine(SceneNode *node, vtkXMLDataElement *elem)
{
  pqCMBSceneObjectBase *obj = node->getDataObject();
  elem->SetAttribute("LineType", "StraightLine");

  double pos[3];
  vtkXMLDataElement *be1 = vtkXMLDataElement::New();
  be1->SetName("Point1WorldPosition");
  (static_cast<pqCMBLine*>(obj))->getPoint1Position(pos[0],pos[1], pos[2]);
  be1->SetVectorAttribute("Value", 3, pos);
  elem->AddNestedElement(be1);
  be1->Delete();
  vtkXMLDataElement *be2 = vtkXMLDataElement::New();
  be2->SetName("Point2WorldPosition");
  (static_cast<pqCMBLine*>(obj))->getPoint2Position(pos[0],pos[1], pos[2]);
  be2->SetVectorAttribute("Value", 3, pos);
  elem->AddNestedElement(be2);
  be2->Delete();
 }

void pqCMBSceneV1Writer::processContour(SceneNode *node, vtkXMLDataElement *elem)
{
  pqCMBSceneObjectBase *obj = node->getDataObject();
  elem->SetAttribute("ContourType", "BoundedPlane");

  CmbScenePolyline* contourObj =
    static_cast<CmbScenePolyline*>(obj);
  if(contourObj)
    {
    vtkPVContourRepresentationInfo* contourInfo = contourObj->getContourInfo();

    if(contourInfo && contourInfo->GetNumberOfAllNodes())
      {
      vtkSmartPointer<vtkXMLDataElement> normalElement =
        vtkSmartPointer<vtkXMLDataElement>::New();
      normalElement->SetName("PlaneProjectionNormal");
      normalElement->SetIntAttribute("Value",
                                     contourObj->GetPlaneProjectionNormal());
      elem->AddNestedElement(normalElement);

      vtkSmartPointer<vtkXMLDataElement> positionElement =
        vtkSmartPointer<vtkXMLDataElement>::New();
      positionElement->SetName("PlanePositionOffset");
      positionElement->SetDoubleAttribute("Value",
                                          contourObj->GetPlaneProjectionPosition());
      elem->AddNestedElement(positionElement);

      vtkSmartPointer<vtkXMLDataElement> closedElement =
        vtkSmartPointer<vtkXMLDataElement>::New();
      closedElement->SetName("ContourClosedLoop");
      closedElement->SetIntAttribute("Value",
                                     contourObj->GetClosedLoop());
      elem->AddNestedElement(closedElement);

      vtkDoubleArray* nodesArray = contourInfo->
        GetAllNodesWorldPositions();
      vtkSmartPointer<vtkXMLDataElement> locations =
        vtkSmartPointer<vtkXMLDataElement>::New();
      locations->SetName("NodeWorldPositions");
      locations->SetIntAttribute("NumberOfNodes", nodesArray->GetNumberOfTuples());
      locations->SetIntAttribute("NumberOfValues", nodesArray->GetNumberOfTuples()*3);
      locations->SetAttribute("type", "Float64");
      locations->SetAttribute("Description", "The node location tuples.");

      // write points tuples

      std::stringstream data;
      char str[1024];
      data << "\n";
      for(vtkIdType i=0; i<nodesArray->GetNumberOfTuples(); i++)
        {
        double* pos = nodesArray->GetTuple(i);
        sprintf (str, "%.16lg %.16lg %.16lg\n", pos[0], pos[1], pos[2]);
        data << "          " << str;
        }
      data << "          ";
      locations->AddCharacterData(data.str().c_str(), data.str().length());
      elem->AddNestedElement(locations);

      // Selected Nodes array
      if(contourInfo->GetNumberOfSelectedNodes())
        {
        vtkIdTypeArray* SelNodes = contourInfo->GetSelectedNodes();
        vtkSmartPointer<vtkXMLDataElement> SelNodesElement =
          vtkSmartPointer<vtkXMLDataElement>::New();
        SelNodesElement->SetName("SelectedNodeIndices");
        SelNodesElement->SetIntAttribute("NumberOfValues", SelNodes->GetNumberOfTuples());
        SelNodesElement->SetAttribute("type", "Int32");
        SelNodesElement->SetAttribute("Description", "The selected nodes indices.");

        std::stringstream SelIdsData;
        SelIdsData << "\n" << "          ";
        for(vtkIdType j=0;j<SelNodes->GetNumberOfTuples();j++)
          {
          SelIdsData << " " << SelNodes->GetValue(j);
          }
        SelIdsData << "\n";
        SelNodesElement->SetCharacterData(SelIdsData.str().c_str(),
                                          static_cast<int>(SelIdsData.str().length()));
        elem->AddNestedElement(SelNodesElement);

        }
      }
    }
}

void pqCMBSceneV1Writer::processPlane(SceneNode *node, vtkXMLDataElement *elem)
{
  pqCMBSceneObjectBase *obj = node->getDataObject();
  double p1[3], p2[3];
  vtkXMLDataElement *be;
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

void pqCMBSceneV1Writer::processFileBasedObj(SceneNode *node, vtkXMLDataElement *elem)
{
  pqCMBSceneObjectBase *obj = node->getDataObject();
  std::map<std::string, int>::iterator iter
    = this->DirectoryMap.find(obj->getFileName());
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

void pqCMBSceneV1Writer::processGeometricProperties(SceneNode *node, vtkXMLDataElement *elem)
{
  pqCMBSceneObjectBase *obj = node->getDataObject();
  vtkXMLDataElement *ue = vtkXMLDataElement::New();
  ue->SetName("Units");
  ue->SetAttribute("Value",
                   cmbSceneUnits::convertToString(obj->getUnits()).c_str());
  elem->AddNestedElement(ue);
  ue->Delete();

  vtkXMLDataElement *ce = vtkXMLDataElement::New();
  double color[4];
  ce->SetName("Color");
  obj->getColor(color);
  ce->SetVectorAttribute("Value", 4, color);
  // Is the color inherited?
  if (!node->hasExplicitColor())
    {
    vtkXMLDataElement *ie = vtkXMLDataElement::New();
    ie->SetName("Inherited");
    ce->AddNestedElement(ie);
    ie->Delete();
    }
  elem->AddNestedElement(ce);
  ce->Delete();

  double data[3];
  vtkXMLDataElement *placement = vtkXMLDataElement::New();
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

  if (!node->isVisible())
    {
    vtkXMLDataElement *ve = vtkXMLDataElement::New();
    ve->SetName("Invisible");
    elem->AddNestedElement(ve);
    ve->Delete();
    }

  ce = this->addConstraints(obj);
  elem->AddNestedElement(ce);
  ce->Delete();

  if (obj->hasTexture())
    {
    ce = this->addTextureInfo(obj);
    elem->AddNestedElement(ce);
    ce->Delete();
    }
}


//----------------------------------------------------------------------------
vtkXMLDataElement * pqCMBSceneV1Writer::addConstraints(pqCMBSceneObjectBase *obj)
{
  vtkXMLDataElement *elem = vtkXMLDataElement::New();
    elem->SetName("Constraints");

    if (obj->isXTransConstrain())
    {
    vtkXMLDataElement *ce = vtkXMLDataElement::New();
    ce->SetName("XTransConstraint");
    elem->AddNestedElement(ce);
    ce->Delete();
    }

  if (obj->isYTransConstrain())
    {
    vtkXMLDataElement *ce = vtkXMLDataElement::New();
    ce->SetName("YTransConstraint");
    elem->AddNestedElement(ce);
    ce->Delete();
    }

  if (obj->isZTransConstrain())
    {
    vtkXMLDataElement *ce = vtkXMLDataElement::New();
    ce->SetName("ZTransConstraint");
    elem->AddNestedElement(ce);
    ce->Delete();
    }

  if (obj->isXRotationConstrain())
    {
    vtkXMLDataElement *ce = vtkXMLDataElement::New();
    ce->SetName("XRotationConstraint");
    elem->AddNestedElement(ce);
    ce->Delete();
    }

  if (obj->isYRotationConstrain())
    {
    vtkXMLDataElement *ce = vtkXMLDataElement::New();
    ce->SetName("YRotationConstraint");
    elem->AddNestedElement(ce);
    ce->Delete();
    }

  if (obj->isZRotationConstrain())
    {
    vtkXMLDataElement *ce = vtkXMLDataElement::New();
    ce->SetName("ZRotationConstraint");
    elem->AddNestedElement(ce);
    ce->Delete();
    }

  if (obj->isIsotropicScalingConstrain())
    {
    vtkXMLDataElement *ce = vtkXMLDataElement::New();
    ce->SetName("IsotropicScalingConstraint");
    elem->AddNestedElement(ce);
    ce->Delete();
    }

  if (obj->isXScalingConstrain())
    {
    vtkXMLDataElement *ce = vtkXMLDataElement::New();
    ce->SetName("XScalingConstraint");
    elem->AddNestedElement(ce);
    ce->Delete();
    }

  if (obj->isYScalingConstrain())
    {
    vtkXMLDataElement *ce = vtkXMLDataElement::New();
    ce->SetName("YScalingConstraint");
    elem->AddNestedElement(ce);
    ce->Delete();
    }

  if (obj->isZScalingConstrain())
    {
    vtkXMLDataElement *ce = vtkXMLDataElement::New();
    ce->SetName("ZScalingConstraint");
    elem->AddNestedElement(ce);
    ce->Delete();
    }
  return elem;
}
//----------------------------------------------------------------------------
vtkXMLDataElement * pqCMBSceneV1Writer::addTextureInfo(pqCMBSceneObjectBase *obj)
{
  vtkXMLDataElement *elem = vtkXMLDataElement::New();
  elem->SetName("TextureInfo");

  std::map<std::string, int>::iterator iter
    = this->TextureDirectoryMap.find(obj->getTextureFileName());
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
  for (i = 0, j= 0; i < n; i++, j+=4)
    {
    obj->getRegistrationPointPair(i, &(data[j]), &(data[j+2]));
    }

  elem->SetIntAttribute("NumberOfRegistrationPoints", n);
  elem->SetVectorAttribute("Points", 4*n, data);
  return elem;
}

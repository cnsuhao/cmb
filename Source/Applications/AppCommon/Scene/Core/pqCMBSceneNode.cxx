//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqCMBSceneNode - represents a node in a Scene Tree.
// .SECTION Description
// .SECTION Caveats

#include "pqCMBSceneNode.h"
#include "pqCMBArc.h"
#include "pqCMBBoreHole.h"
#include "pqCMBCrossSection.h"
#include "pqCMBFacetedObject.h"
#include "pqCMBLine.h"
#include "pqCMBPoints.h"
#include "pqCMBSceneNodeIterator.h"
#include "pqCMBSceneTree.h"
#include "pqCMBUniformGrid.h"
#include "pqCMBVOI.h"
#include <QTreeWidgetItem>
#include <vtkMath.h>

#include "vtkDoubleArray.h"
#include "vtkIdTypeArray.h"
#include "vtkPVArcInfo.h"
#include <vtkMatrixToLinearTransform.h>
#include <vtkTransform.h>

pqCMBSceneNode::pqCMBSceneNode(const char* name, pqCMBSceneTree* tree)
{
  this->Object = NULL;
  this->Parent = NULL;
  this->Tree = tree;
  this->Name = name;
  this->MeshOutputIndex = -1;
  this->InfoWidget = NULL;
  this->MarkedForDeletion = false;
  this->Widget = new QTreeWidgetItem(this->Tree->getWidget());
  if (this->Tree->getInfoWidget())
  {
    this->InfoWidget = new QTreeWidgetItem(this->Tree->getInfoWidget());
    this->InfoWidget->setHidden(true);
    this->InfoWidget->setExpanded(true);
  }
  this->init();
}

pqCMBSceneNode::pqCMBSceneNode(const char* name, pqCMBSceneNode* parent, pqCMBSceneObjectBase* obj)
{
  this->Object = obj;
  this->Parent = parent;
  this->Parent->addChild(this);
  this->Tree = parent->getTree();
  this->Name = name;
  this->InfoWidget = NULL;
  this->MarkedForDeletion = false;
  this->Widget = new QTreeWidgetItem(this->Parent->getWidget());
  if (this->Parent->getInfoWidget())
  {
    this->InfoWidget = new QTreeWidgetItem(this->Parent->getInfoWidget());
    this->Tree->getInfoWidget()->blockSignals(true);
    this->InfoWidget->setHidden(true);
    this->InfoWidget->setExpanded(true);
    if (obj)
    {
      this->InfoWidget->setExpanded(false);
      this->populateInfoNodes();
    }
    this->Tree->getInfoWidget()->blockSignals(false);
  }
  this->init();
}

void pqCMBSceneNode::init()
{
  this->Visible = true;
  this->IsLocked = false;
  this->Selected = false;
  this->BeingDeleted = false;
  this->ExplicitColor = false;
  if (this->Parent)
  {
    this->Parent->getColor(this->NodeColor);
  }
  else
  {
    this->NodeColor[0] = this->NodeColor[1] = this->NodeColor[2] = this->NodeColor[3] = 1.0;
  }

  if (this->Object && this->Object->getRepresentation())
  {
    this->Object->setColor(this->NodeColor, false);
  }

  this->Widget->setText(this->getNameColumn(), this->Name.c_str());
  this->Widget->setIcon(this->getVisibilityColumn(), *this->Tree->getIconVisible());

  if (this->InfoWidget)
  {
    this->InfoWidget->setText(this->getNameColumn(), this->Name.c_str());
    //this->InfoWidget->setIcon(this->getVisibilityColumn(),
    //  *this->Tree->getIconVisible());
  }

  QVariant vdata;
  vdata.setValue(static_cast<void*>(this));
  this->Widget->setData(this->getNameColumn(), Qt::UserRole, vdata);
  Qt::ItemFlags flags = this->Widget->flags();

  if (this->InfoWidget)
  {
    this->InfoWidget->setData(this->getNameColumn(), Qt::UserRole, vdata);
    flags = this->InfoWidget->flags();
  }
  if (!this->Object)
  {
    // Lets make sure that theses nodes are not selectable
    // Removed the selectable flag
    //flags &= ~Qt::ItemIsSelectable;
  }
  flags &= ~Qt::ItemIsEditable;
  if (this->InfoWidget)
  {
    this->InfoWidget->setFlags(flags);
  }
  flags |= Qt::ItemIsEditable;
  this->Widget->setFlags(flags);
}

pqCMBSceneNode* pqCMBSceneNode::getNodeFromWidget(QTreeWidgetItem* widget)
{
  return static_cast<pqCMBSceneNode*>(
    widget->data(pqCMBSceneNode::getNameColumn(), Qt::UserRole).value<void*>());
}

pqCMBSceneNode::~pqCMBSceneNode()
{
  this->BeingDeleted = true;
  int i, n = static_cast<int>(this->Children.size());

  //we have to go and remove all the polygons
  //first and they are consumers of other scene node items
  for (i = 0; i < n; i++)
  {
    if (this->Children[i] && this->Children[i]->Object &&
      this->Children[i]->Object->getType() == pqCMBSceneObjectBase::Polygon)
    {
      delete this->Children[i];
      this->Children[i] = NULL;
    }
  }

  for (i = 0; i < n; i++)
  {
    if (this->Children[i])
    {
      delete this->Children[i];
      this->Children[i] = NULL;
    }
  }
  if (this->Object)
  {
    delete this->Object;
  }

  if (this->Parent)
  {
    // if the parent is being deleted then we don't have to remove
    // the node from it nor do we have to delete the Widget
    if (this->Parent->BeingDeleted)
    {
      return;
    }
    this->Parent->removeChild(this);
  }
  if (this->Widget)
  {
    delete this->Widget;
  }
  if (this->InfoWidget)
  {
    delete this->InfoWidget;
  }
}

void pqCMBSceneNode::addChild(pqCMBSceneNode* node)
{
  this->Children.push_back(node);
}

void pqCMBSceneNode::removeChild(pqCMBSceneNode* node, bool deleteNode)
{
  std::vector<pqCMBSceneNode*>::iterator it;
  for (it = this->Children.begin(); it < this->Children.end(); it++)
  {
    if (*it == node)
    {
      this->Children.erase(it);
      if (deleteNode)
      {
        node->Parent = NULL;
        delete node;
      }
      return;
    }
  }
}

void pqCMBSceneNode::changeName(const char* name)
{
  if (this->Name == name)
  {
    return;
  }

  std::string newName = this->Tree->createUniqueName(name);
  this->Widget->setText(this->getNameColumn(), newName.c_str());
  if (this->InfoWidget)
  {
    this->InfoWidget->setText(this->getNameColumn(), newName.c_str());
  }
  this->Name = newName;
}

void pqCMBSceneNode::setParent(pqCMBSceneNode* newParent)
{
  if (this->Parent == newParent)
  {
    return;
  }
  if (newParent)
  {
    newParent->getWidget()->addChild(this->Widget);
    if (this->InfoWidget)
    {
      newParent->getInfoWidget()->addChild(this->InfoWidget);
    }
  }

  this->Parent = newParent;
}

void pqCMBSceneNode::toggleVisibility()
{
  this->setVisibility(!this->Visible);
}

void pqCMBSceneNode::setParentVisibilityOn()
{
  if ((!this->Parent) || this->Parent->Visible)
  {
    return;
  }

  this->Parent->Widget->setIcon(this->getVisibilityColumn(), *this->Tree->getIconVisible());
  this->Parent->Visible = true;
  this->Parent->setParentVisibilityOn();
}
void pqCMBSceneNode::setVisibility(bool mode)
{
  if (this->isMarkedForDeletion() && mode)
  {
    return; // Don't display "deleted" nodes
  }
  if (this->Visible == mode)
  {
    return;
  }

  this->Visible = mode;

  if (mode)
  {
    this->Widget->setIcon(this->getVisibilityColumn(), *this->Tree->getIconVisible());
    this->setParentVisibilityOn();
  }
  else
  {
    this->Widget->setIcon(this->getVisibilityColumn(), *this->Tree->getIconInvisible());
  }
  int n = static_cast<int>(this->Children.size());
  int i;
  for (i = 0; i < n; i++)
  {
    this->Children[i]->setVisibility(mode);
  }

  if (this->Object)
  {
    this->Object->setVisibility(mode);
  }
  // Tell the tree that 3D scene changed
  this->Tree->sceneObjectChanged();
}

void pqCMBSceneNode::toggleLocked()
{
  this->setIsLocked(!this->IsLocked);
}

void pqCMBSceneNode::unlockParent()
{
  if ((!this->Parent) || !this->Parent->IsLocked)
  {
    return;
  }

  this->Parent->Widget->setIcon(this->getLockColumn(), *this->Tree->getIconNull());
  this->Parent->IsLocked = false;
  this->Parent->unlockParent();
}

void pqCMBSceneNode::setIsLocked(bool mode)
{
  if (this->IsLocked == mode)
  {
    return;
  }
  // The Geology Boreholes and Cross Sections should always be locked.
  if (!mode && this->Object &&
    (dynamic_cast<pqCMBBoreHole*>(this->Object) || dynamic_cast<pqCMBCrossSection*>(this->Object)))
  {
    return;
  }
  // update the lock variable
  this->IsLocked = mode;

  if (!mode)
  {
    this->Widget->setIcon(this->getLockColumn(), *this->Tree->getIconNull());
    this->unlockParent();
  }
  else
  {
    this->Widget->setIcon(this->getLockColumn(), *this->Tree->getIconLocked());
  }
  int n = static_cast<int>(this->Children.size());
  int i;
  for (i = 0; i < n; i++)
  {
    this->Children[i]->setIsLocked(mode);
  }

  if (this->Object)
  {
    this->Object->setIsFullyConstrained(mode);
  }
  // Tell the tree that 3D scene changed
  this->Tree->sceneObjectChanged();
}

void pqCMBSceneNode::setSelected(bool mode)
{
  this->Selected = mode;
  if (this->InfoWidget)
  {
    QTreeWidgetItem* parent = this->InfoWidget;
    //Propogate the changes to this node's parents
    while (parent)
    {
      parent->setHidden(!mode);
      parent = parent->parent();
    }
  }
}

void pqCMBSceneNode::setExplicitColor(QColor& qColor)
{
  double color[4];
  qColor.getRgbF(&(color[0]), &(color[1]), &(color[2]), &(color[3]));
  this->setExplicitColor(color);
}

void pqCMBSceneNode::setExplicitColor(double color[4])
{
  if (this->ExplicitColor && (this->NodeColor[0] == color[0]) && (this->NodeColor[1] == color[1]) &&
    (this->NodeColor[2] == color[2]) && (this->NodeColor[3] == color[3]))
  {
    return;
  }

  bool origSetting = this->ExplicitColor;
  this->ExplicitColor = true;
  this->NodeColor[0] = color[0];
  this->NodeColor[1] = color[1];
  this->NodeColor[2] = color[2];
  this->NodeColor[3] = color[3];
  this->pushColor();

  // If this node is currently selected and it
  // did not have exlicit color set then the tree
  // will need to update its color actions
  if (this->isSelected() && (!origSetting))
  {
    this->Tree->updateSelectedColorMode();
  }
}

void pqCMBSceneNode::unsetExplicitColor()
{
  if (!(this->ExplicitColor && this->Parent))
  {
    return;
  }

  this->Parent->getColor(this->NodeColor);
  this->pushColor();

  // If this is a leaf node and is currently selected then
  // the act of changing the color (on the server side) will
  // cause set color to be called and hence will cause this
  // node's explicit color to be reset to true - so lets hold
  // off till the end of setting it to false.

  this->ExplicitColor = false;

  // If this node is currently selected then the tree
  // will need to update its color actions
  if (this->isSelected())
  {
    this->Tree->updateSelectedColorMode();
  }
}

void pqCMBSceneNode::setMeshOutputIndex(int meshIndex)
{
  this->MeshOutputIndex = meshIndex;
}

void pqCMBSceneNode::pushColor()
{
  std::stack<pqCMBSceneNode*> stack;
  pqCMBSceneNode* node = this;

  int i, n;
  stack.push(this);

  while (!stack.empty())
  {
    node = stack.top();
    stack.pop();
    if (node != this)
    {
      this->getColor(node->NodeColor);
    }

    if (node->Object)
    {
      node->Object->setColor(this->NodeColor);
    }
    else
    {
      n = static_cast<int>(node->Children.size());

      for (i = 0; i < n; i++)
      {
        if (!node->Children[i]->ExplicitColor)
        {
          stack.push(node->Children[i]);
        }
      }
    }
  }
}

void pqCMBSceneNode::select()
{
  this->Widget->setSelected(true);
}

void pqCMBSceneNode::unselect()
{
  this->Widget->setSelected(false);
}

void pqCMBSceneNode::zoomOnData()
{
  if (this->Object)
  {
    this->Object->zoomOnObject();
  }
}

bool pqCMBSceneNode::getBounds(vtkBoundingBox* bb) const
{
  if (this->Object)
  {
    this->Object->getBounds(bb);
    return true;
  }

  SceneObjectNodeIterator iter(const_cast<pqCMBSceneNode*>(this));
  pqCMBSceneNode* node;
  while ((node = iter.next()))
  {
    vtkBoundingBox box;
    node->Object->getBounds(&box);
    bb->AddBox(box);
  }

  return bb->IsValid();
}

bool pqCMBSceneNode::isAncestorWidgetSelected() const
{
  pqCMBSceneNode* parent = this->Parent;
  while (parent)
  {
    if (parent->isSelected())
    {
      return true;
    }
    parent = parent->getParent();
  }
  return false;
}

bool pqCMBSceneNode::isLineNode()
{
  return (this->Object && this->Object->getType() == pqCMBSceneObjectBase::Line);
}

bool pqCMBSceneNode::isArcNode()
{
  return (this->Object && this->Object->getType() == pqCMBSceneObjectBase::Arc);
}

void pqCMBSceneNode::collapseAllDataInfo()
{
  if (this->InfoWidget)
  {
    for (unsigned i = 0; i < this->Children.size(); i++)
    {
      this->Children[i]->collapseAllDataInfo();
    }
    if (this->Object)
    {
      this->InfoWidget->setExpanded(false);
    }
  }
}

//Checks the size of trees and expands them to the appropriate size
//by adding nodes
void pqCMBSceneNode::populateInfoNodes()
{
  pqCMBSceneObjectBase::enumObjectType objType = this->Object->getType();
  switch (objType)
  {
    case pqCMBSceneObjectBase::Line:
      while (this->InfoWidget->childCount() < 3)
      {
        this->InfoWidget->addChild(new QTreeWidgetItem);
      }

      while (this->InfoWidget->child(0)->childCount() < 2)
      {
        this->InfoWidget->child(0)->addChild(new QTreeWidgetItem());
      }
      break;
    case pqCMBSceneObjectBase::Arc:
      while (this->InfoWidget->childCount() < 3)
      {
        this->InfoWidget->addChild(new QTreeWidgetItem);
      }
      break;
    case pqCMBSceneObjectBase::VOI:
      while (this->InfoWidget->childCount() < 6)
      {
        this->InfoWidget->addChild(new QTreeWidgetItem);
      }
      while (this->InfoWidget->child(0)->childCount() < 3)
      {
        this->InfoWidget->child(0)->addChild(new QTreeWidgetItem());
      }
      while (this->InfoWidget->child(4)->childCount() < 3)
      {
        this->InfoWidget->child(4)->addChild(new QTreeWidgetItem());
      }
      break;
    //Things that are not lines or VOIs are poly data
    default:
      //If The tree doesn't have necessary widgets for poly data info create them
      int num = 7; //by default all have 7 pieces of information
      if (objType == pqCMBSceneObjectBase::Faceted)
      {
        num = 9;
      }
      else if (objType == pqCMBSceneObjectBase::Points)
      {
        num = 9;
      }
      else if (objType == pqCMBSceneObjectBase::UniformGrid)
      {
        num = 8;
      }
      else if (dynamic_cast<pqCMBTexturedObject*>(this->Object) != NULL)
      {
        num = 8;
      }
      while (this->InfoWidget->childCount() < num)
      {
        this->InfoWidget->addChild(new QTreeWidgetItem);
      }
      while (this->InfoWidget->child(0)->childCount() < 3)
      {
        this->InfoWidget->child(0)->addChild(new QTreeWidgetItem());
      }
      while (this->InfoWidget->child(4)->childCount() < 3)
      {
        this->InfoWidget->child(4)->addChild(new QTreeWidgetItem());
      }
      while (this->InfoWidget->child(5)->childCount() < 2)
      {
        this->InfoWidget->child(5)->addChild(new QTreeWidgetItem());
      }
      break;
  }
}

//Changes the names of the information nodes to reflect new information
void pqCMBSceneNode::renameInfoNodes()
{
  //create nodes if they don't exist
  this->populateInfoNodes();
  QTreeWidgetItem* otype;
  pqCMBSceneObjectBase::enumObjectType objType = this->Object->getType();
  switch (objType)
  {
    //LINE Stats
    case pqCMBSceneObjectBase::Line:
    {
      QTreeWidgetItem* linePoints = this->InfoWidget->child(0);
      linePoints->setText(0, "Points");
      double linePointsArr[6];

      pqCMBLine* lineObj = static_cast<pqCMBLine*>(this->Object);
      lineObj->getPoint1Position(linePointsArr[0], linePointsArr[1], linePointsArr[2]);
      lineObj->getPoint2Position(linePointsArr[3], linePointsArr[4], linePointsArr[5]);
      QString pointType[2] = { "Min: ", "Max: " };
      for (int i = 0; i < 2; ++i)
      {
        QTreeWidgetItem* linePointChild = linePoints->child(i);
        linePointChild->setText(0, pointType[i] + " (" + QString::number(linePointsArr[(i * 3)]) +
            ", " + QString::number(linePointsArr[(i * 3) + 1]) + ", " +
            QString::number(linePointsArr[(i * 3) + 2]) + ")");
      }

      double length = sqrt(vtkMath::Distance2BetweenPoints(linePointsArr, &linePointsArr[3]));
      QTreeWidgetItem* lineLength = this->InfoWidget->child(1);
      lineLength->setText(0, "Length: " + QString::number(length));
      otype = this->InfoWidget->child(2);
      otype->setText(0, "Type: " + QString::fromStdString(this->Object->getUserDefinedType()));
      break;
    }
    // VOI Stats
    case pqCMBSceneObjectBase::VOI:
    {
      vtkBoundingBox bb;
      this->getBounds(&bb);

      double lengths[3];
      double originPoint[3];
      bb.GetLengths(lengths);
      bb.GetMinPoint(originPoint[0], originPoint[1], originPoint[2]);

      double vol = lengths[0] * lengths[1] * lengths[2];
      double sa =
        lengths[0] * lengths[1] * 2 + lengths[1] * lengths[2] * 2 + lengths[0] * lengths[2] * 2;

      vtkTransform* transform = vtkTransform::New();
      this->getDataObject()->getTransform(transform);
      double xaxis[3] = { 1, 0, 0 };
      double yaxis[3] = { 0, 1, 0 };
      double zaxis[3] = { 0, 0, 1 };
      transform->TransformNormal(xaxis, xaxis);
      transform->TransformNormal(yaxis, yaxis);
      transform->TransformNormal(zaxis, zaxis);
      transform->Delete();

      QTreeWidgetItem* axes = this->InfoWidget->child(0);
      axes->setText(0, "Axes");
      axes->child(0)->setText(0, "X: (" + QString::number(xaxis[0]) + ", " +
          QString::number(xaxis[1]) + ", " + QString::number(xaxis[2]) + ")");

      axes->child(1)->setText(0, "Z: (" + QString::number(yaxis[0]) + ", " +
          QString::number(yaxis[1]) + ", " + QString::number(yaxis[2]) + ")");

      axes->child(2)->setText(0, "Y: (" + QString::number(zaxis[0]) + ", " +
          QString::number(zaxis[1]) + ", " + QString::number(zaxis[2]) + ")");

      QTreeWidgetItem* origin = this->InfoWidget->child(1);
      origin->setText(0, "Origin: (" + QString::number(originPoint[0]) + ", " +
          QString::number(originPoint[1]) + ", " + QString::number(originPoint[2]) + ")");

      QTreeWidgetItem* volume = this->InfoWidget->child(2);
      volume->setText(0, "Volume: " + QString::number(vol));

      QTreeWidgetItem* surfaceArea = this->InfoWidget->child(3);
      surfaceArea->setText(0, "Surface Area: " + QString::number(sa));

      QTreeWidgetItem* edgeLength = this->InfoWidget->child(4);
      edgeLength->setText(0, "Edge Length: ");
      edgeLength->child(0)->setText(0, "X: " + QString::number(lengths[0]));
      edgeLength->child(1)->setText(0, "Y: " + QString::number(lengths[1]));
      edgeLength->child(2)->setText(0, "Z: " + QString::number(lengths[2]));
      otype = this->InfoWidget->child(5);
      otype->setText(0, "Type: " + QString::fromStdString(this->Object->getUserDefinedType()));
      break;
    }
    //Contour Stats
    case pqCMBSceneObjectBase::Arc:
    {
      QTreeWidgetItem* contourItem = this->InfoWidget->child(0);
      int totNumber = 0, selNumber = 0;
      QTreeWidgetItem* selNodeItem = this->InfoWidget->child(1);
      pqCMBArc* contourObj = static_cast<pqCMBArc*>(this->Object);
      vtkPVArcInfo* arcInfo = contourObj->getArcInfo();
      if (arcInfo && arcInfo->GetNumberOfPoints())
      {
        totNumber = arcInfo->GetNumberOfPoints();
        selNumber = arcInfo->IsClosedLoop() ? 1 : 2;
        for (vtkIdType idx = 0; idx < selNumber; ++idx)
        {
          double pos[3];
          arcInfo->GetEndNodePos(idx, pos);
          selNodeItem->addChild(new QTreeWidgetItem);
          selNodeItem->child(idx)->setText(0, " (" + QString::number(pos[0]) + ", " +
              QString::number(pos[1]) + ", " + QString::number(pos[2]) + ")");
        }
      }
      contourItem->setText(0, "Total # of Nodes: " + QString::number(totNumber));
      selNodeItem->setText(0, "# of Model Vertex: " + QString::number(selNumber));
      otype = this->InfoWidget->child(2);
      otype->setText(0, "Type: " + QString::fromStdString(this->Object->getUserDefinedType()));
      break;
    }
    //Things that are not lines or VOIs or Contours are poly data
    default:
    {
      QTreeWidgetItem* areaStats = this->InfoWidget->child(0);
      areaStats->setText(0, "Area Stats");
      double areaArr[3];
      this->Object->getAreaStats(areaArr);
      QString areaType[3] = { "Min: ", "Ave: ", "Max: " };
      for (int i = 0; i < 3; ++i)
      {
        QTreeWidgetItem* areaStatsChild = areaStats->child(i);
        areaStatsChild->setText(0, areaType[i] + QString::number(areaArr[i]));
      }

      QTreeWidgetItem* surfaceArea = this->InfoWidget->child(1);
      surfaceArea->setText(0, "Surface Area: " + QString::number(this->Object->getSurfaceArea()));

      QTreeWidgetItem* numPoints = this->InfoWidget->child(2);
      numPoints->setText(
        0, "Number Of Points: " + QString::number(this->Object->getNumberOfPoints()));

      QTreeWidgetItem* numPoly = this->InfoWidget->child(3);
      numPoly->setText(
        0, "Number Of Polygons: " + QString::number(this->Object->getNumberOfPolygons()));

      QTreeWidgetItem* polySides = this->InfoWidget->child(4);
      polySides->setText(0, "Polygonal Sides Stats");
      double polySidesArr[3];
      this->Object->getPolySideStats(polySidesArr);
      QString polyType[3] = { "Min: ", "Ave: ", "Max: " };
      for (int i = 0; i < 3; ++i)
      {
        QTreeWidgetItem* polySidesChild = polySides->child(i);
        polySidesChild->setText(0, polyType[i] + QString::number(polySidesArr[i]));
      }

      QTreeWidgetItem* geoBounds = this->InfoWidget->child(5);
      geoBounds->setText(0, "Geometry Bounds");
      double geoBoundsArr[6];
      this->Object->getGeometryBounds(geoBoundsArr);
      QString geoType[2] = { "Min: ", "Max: " };
      for (int i = 0; i < 2; ++i)
      {
        QTreeWidgetItem* geoBoundsChild = geoBounds->child(i);
        geoBoundsChild->setText(0, geoType[i] + " (" + QString::number(geoBoundsArr[i]) + ", " +
            QString::number(geoBoundsArr[i + 2]) + ", " + QString::number(geoBoundsArr[i + 4]) +
            ")");
      }
      otype = this->InfoWidget->child(6);
      otype->setText(0, "Type: " + QString::fromStdString(this->Object->getUserDefinedType()));
      pqCMBFacetedObject* fobj = dynamic_cast<pqCMBFacetedObject*>(this->Object);
      if (fobj != NULL)
      {
        this->InfoWidget->child(7)->setText(
          0, "FileName: " + QString::fromStdString(fobj->getFileName()));
        if (fobj->hasTexture())
        {
          this->InfoWidget->child(8)->setText(0, "Texture: " + fobj->getTextureFileName());
        }
        else
        {
          this->InfoWidget->child(8)->setText(0, "Texture: None");
        }
      }
      else
      {
        pqCMBPoints* pobj = dynamic_cast<pqCMBPoints*>(this->Object);
        if (pobj != NULL)
        {
          this->InfoWidget->child(7)->setText(
            0, "FileName: " + QString::fromStdString(pobj->getFileName()));
          if (pobj->hasTexture())
          {
            this->InfoWidget->child(8)->setText(0, "Texture: " + pobj->getTextureFileName());
          }
          else
          {
            this->InfoWidget->child(8)->setText(0, "Texture: None");
          }
        }
        else
        {
          pqCMBUniformGrid* uobj = dynamic_cast<pqCMBUniformGrid*>(this->Object);
          if (uobj != NULL)
          {
            this->InfoWidget->child(7)->setText(
              0, "FileName: " + QString::fromStdString(uobj->getFileName()));
          }
          else
          {
            pqCMBTexturedObject* tobj = dynamic_cast<pqCMBTexturedObject*>(this->Object);
            if (tobj != NULL)
            {
              if (tobj->hasTexture())
              {
                this->InfoWidget->child(7)->setText(0, "Texture: " + tobj->getTextureFileName());
              }
              else
              {
                this->InfoWidget->child(7)->setText(0, "Texture: None");
              }
            }
          }
        }
      }
      break;
    }
  }
}

void pqCMBSceneNode::recomputeInfo(QTreeWidgetItem* item)
{
  if (this->InfoWidget)
  {
    for (unsigned i = 0; i < this->Children.size(); i++)
    {
      this->Children[i]->recomputeInfo(item);
    }
    //compare names of the object to see if information needs updating
    if (item->text(0) == this->InfoWidget->text(0) && this->Object)
    {
      this->renameInfoNodes();
    }
  }
}

void pqCMBSceneNode::setMarkedForDeletion()
{
  // Set all children nodes not to have any selection representation
  this->MarkedForDeletion = true;

  if (this->Object)
  {
    this->Object->setMarkedForDeletion();
  }
  else
  {
    SceneObjectNodeIterator iter(this);
    pqCMBSceneNode* node;
    while ((node = iter.next()))
    {
      node->getDataObject()->setMarkedForDeletion();
    }
  }
  this->Widget->setHidden(true);
  this->setVisibility(false);
}

void pqCMBSceneNode::unsetMarkedForDeletion()
{
  this->MarkedForDeletion = false;

  if (this->Object)
  {
    this->Object->unsetMarkedForDeletion();
  }
  else
  {
    SceneObjectNodeIterator iter(this);
    pqCMBSceneNode* node;
    while ((node = iter.next()))
    {
      node->getDataObject()->unsetMarkedForDeletion();
    }
  }
  this->Widget->setHidden(false);
  this->setVisibility(true);
}

void pqCMBSceneNode::replaceObject(pqCMBSceneObjectBase* object)
{
  if (this->Object)
  {
    delete this->Object;
  }
  this->Object = object;
}

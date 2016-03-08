//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtCMBArcWidgetManager
// .SECTION Description
//  Create and controls the arc editing singelton widget
// .SECTION Caveats

#include "qtCMBArcWidgetManager.h"

#include "pqCMBArc.h"
#include "pqCMBSceneTree.h"
#include "pqCMBSceneNode.h"
#include "pqCMBCommonMainWindowCore.h"
#include "qtCMBArcWidget.h"
#include "qtCMBArcEditWidget.h"

#include "pqApplicationCore.h"
#include "pqObjectBuilder.h"
#include "pqDataRepresentation.h"
#include "pqPipelineSource.h"
#include "pqRenderView.h"
#include "pqSMAdaptor.h"
#include "pqServer.h"

#include "vtkCommand.h"
#include "vtkDoubleArray.h"
#include "vtkIdTypeArray.h"
#include "vtkProcessModule.h"
#include "vtkSMNewWidgetRepresentationProxy.h"
#include "vtkSMInputProperty.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxyManager.h"
#include "vtkSMRenderViewProxy.h"
#include "vtkSMSourceProxy.h"
#include "vtkAbstractWidget.h"
#include "vtkNew.h"
#include "vtkCMBSubArcModifyClientOperator.h"

//-----------------------------------------------------------------------------
qtCMBArcWidgetManager::qtCMBArcWidgetManager(pqServer *server, pqRenderView *view)
{
  //server and view need to be set before we call createContourWidget
  this->Server = server;
  this->View = view;
  this->Widget = NULL;
  this->Node = NULL;
  this->Arc = NULL;

  this->EditWidget = NULL;
  this->ActiveWidget = NULL;
}

//-----------------------------------------------------------------------------
qtCMBArcWidgetManager::~qtCMBArcWidgetManager()
{
  this->Server = NULL;
  this->Node = NULL;

  if ( this->Widget )
    {
    //if a widget is deleted without having an active view it throws errors
    if ( this->View && !this->Widget->view() )
      {
      this->Widget->setView(this->View);
      }

    delete this->Widget;
    }
  this->Widget = NULL;
  if ( this->EditWidget )
    {
    delete this->EditWidget;
    }
  this->EditWidget = NULL;
  this->View = NULL;
}

//-----------------------------------------------------------------------------
void qtCMBArcWidgetManager::setActiveNode(pqCMBSceneNode *node)
{
  this->Node = node;
}

void qtCMBArcWidgetManager::setActiveArc(pqCMBArc* arc)
{
  this->Arc = arc;
}

//-----------------------------------------------------------------------------
bool qtCMBArcWidgetManager::hasActiveNode()
{
  return (this->Node != NULL);
}

bool qtCMBArcWidgetManager::hasActiveArc()
{
  return Arc != NULL;
}

bool qtCMBArcWidgetManager::isActive()
{
  return this->hasActiveNode() || this->hasActiveArc();
}

//-----------------------------------------------------------------------------
pqCMBSceneNode * qtCMBArcWidgetManager::getActiveNode()
{
  return this->Node;
}

pqCMBArc* qtCMBArcWidgetManager::getActiveArc()
{
  if(this->Node != NULL)
  {
    return dynamic_cast<pqCMBArc*>( this->Node->getDataObject() );
  }
  return this->Arc;
}

//-----------------------------------------------------------------------------
pqCMBArc* qtCMBArcWidgetManager::createpqCMBArc()
{
  //update the node to have everything but the actual contour data
  //we now create a contour data object
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();

  pqCMBArc* obj = new pqCMBArc();

  //now update the node with the real data object
  return obj;
}

//-----------------------------------------------------------------------------
int qtCMBArcWidgetManager::create()
{
  emit this->Busy();
  if ( !this->Node && !this->Arc )
    {
    emit this->Ready();
    return 0;
    }
  bool created = false;
  int normal;
  double planepos;
  if ( !this->Widget )
    {
    this->Widget = this->createDefaultContourWidget(normal, planepos);
    QObject::connect(this->Widget,SIGNAL(contourDone()),
      this,SLOT(updateArcNode()));
    created = true;
    }

  if ( !created )
    {
    this->Widget->setView(this->View);
    this->getDefaultArcPlane(normal, planepos);
    this->resetArcPlane(normal, planepos);
    this->Widget->setView(this->View);
    this->Widget->setWidgetVisible(true);

    vtkSMPropertyHelper(this->Widget->getWidgetProxy(), "Enabled").Set(1);
    this->Widget->getWidgetProxy()->UpdateVTKObjects();
    this->Widget->showWidget();
    }

  this->Widget->select();
  if(this->Node != NULL)
    {
    pqCMBSceneObjectBase* obj = this->Node->getDataObject();
    if ( obj  && obj->getType()==pqCMBSceneObjectBase::Arc)
      {
      dynamic_cast<pqCMBArc*>(obj)->setPlaneProjectionNormal(normal);
      dynamic_cast<pqCMBArc*>(obj)->setPlaneProjectionPosition(planepos);
      }
    }
  else
    {
    this->Arc->setPlaneProjectionNormal(normal);
    this->Arc->setPlaneProjectionPosition(planepos);
    }
  this->ActiveWidget = this->Widget;
  return 1;
}

//-----------------------------------------------------------------------------
int qtCMBArcWidgetManager::edit()
{
  emit this->Busy();
  if ( !this->Node && !this->Arc )
    {
    emit this->Ready();
    return 0;
    }
  if(!this->EditWidget)
    {
    this->EditWidget = new qtCMBArcEditWidget();
    QObject::connect(this->EditWidget,SIGNAL(
      arcModified(qtCMBArcWidget*, vtkIdType, vtkIdType)),
      this,SLOT(updateModifiedArc(qtCMBArcWidget*, vtkIdType, vtkIdType)));
    QObject::connect(this->EditWidget,SIGNAL(arcModificationfinished()),
      this,SLOT(editingFinished()));
    QObject::connect(this->EditWidget,SIGNAL(startArcEditing()),
      this,SIGNAL(editingStarted()));

    }
  pqCMBArc* arcObj = this->Arc;

  if(this->Node != NULL)
    {
    pqCMBSceneObjectBase* obj = this->Node->getDataObject();
    arcObj = dynamic_cast<pqCMBArc*>(obj);
    }

  this->EditWidget->setView(this->View);

  this->EditWidget->setArc(arcObj);
  this->EditWidget->setArcManager(this);
  this->EditWidget->show();
  this->ActiveWidget = this->EditWidget;

  return 1;
}

//-----------------------------------------------------------------------------
void qtCMBArcWidgetManager::updateArcNode()
{
  if ( !this->Node && !this->Arc )
    {
    return;
    }

  //push the polydata from the widget representation to the poly source

  pqCMBArc* obj = (this->Node != NULL)?dynamic_cast<pqCMBArc*>
    (this->Node->getDataObject()):this->Arc;
  if ( obj )
    {
    vtkSMNewWidgetRepresentationProxy *widget = this->Widget->getWidgetProxy();

    //if the object hasn't been created yet update will call createArc
    //this way we don't have to check here
    QList<vtkIdType> newArcIds;
    vtkIdTypeArray *arcIdsFromSplit = vtkIdTypeArray::New();
    obj->updateArc(widget,arcIdsFromSplit);

    vtkIdType arcIdsSize = arcIdsFromSplit->GetNumberOfTuples();
    if(arcIdsSize > 0)
      {
      //convert this into a QList of vtkIdTypes so we can emit to the tree
      for(vtkIdType idx=0; idx < arcIdsSize; ++idx)
        {
        newArcIds.push_back(arcIdsFromSplit->GetValue(idx));
        }
      }
    arcIdsFromSplit->Delete();

    //make sure the model rep is visible, it would be hidden if we can from edit mode
    pqDataRepresentation *modelRep = obj->getRepresentation();
    if(modelRep)
      {
      modelRep->setVisible(true);
      }

    //pass onto the scene tree that this scene polyline is finished being editing
    //it needs the signal so that the tree can split the arcset into arcs.
    //Also this is need to make all the arc representation rerender to fix
    //any old end nodes hanging around
    if(this->Node != NULL)
      emit this->ArcSplit(this->Node,newArcIds);
    else
      emit this->ArcSplit2(this->Arc, newArcIds);
   }

  //update the object
  this->Widget->setVisible(false);
  this->Widget->reset();
  this->Widget->removeAllNodes();
  this->Widget->setWidgetVisible(false);
  this->Widget->getWidgetProxy()->UpdatePropertyInformation();
  this->Widget->setView(NULL);
  this->ActiveWidget = NULL;

  this->Node = NULL;
  this->Arc = NULL;
  emit this->Ready();
}

//-----------------------------------------------------------------------------
void qtCMBArcWidgetManager::editingFinished()
{
  if(this->EditWidget && this->ActiveWidget == this->EditWidget)
    {
    this->EditWidget->hide();
    }
  this->ActiveWidget = NULL;
  this->Node = NULL;
  this->Arc = NULL;
  emit this->Ready();
  emit this->Finish();
}

//-----------------------------------------------------------------------------
void qtCMBArcWidgetManager::updateModifiedArc(
  qtCMBArcWidget* subArcWidget, vtkIdType startPID, vtkIdType endPID)
{
  if ( (!this->Node && !this->Arc) || this->ActiveWidget != this->EditWidget)
    {
    return;
    }

  //push the polydata from the widget representation to the poly source
  pqCMBArc* obj = (this->Node != NULL)?dynamic_cast<pqCMBArc*>(this->Node->getDataObject()):this->Arc;
  if ( obj )
    {
    vtkSMNewWidgetRepresentationProxy *widget = subArcWidget->getWidgetProxy();

    //if the object hasn't been created yet update will call createArc
    //this way we don't have to check here
    QList<vtkIdType> newArcIds;
    vtkNew<vtkIdTypeArray> arcIdsFromSplit;
    //call the update arc operator
    vtkNew<vtkCMBSubArcModifyClientOperator> updateAndSplitOp;
    updateAndSplitOp->SetStartPointId(startPID);
    updateAndSplitOp->SetEndPointId(endPID);
    bool valid = updateAndSplitOp->Operate(obj->getArcId(),widget,
      vtkSMSourceProxy::SafeDownCast(obj->getSource()->getProxy()));
    if (!valid)
      {
      //we didn't update ourselves, most likely bad widget representation
      //ie. a representation that has 1 or 0 points
      return;
      }

    //copy the arc ids to create new arcs for
    arcIdsFromSplit->DeepCopy(updateAndSplitOp->GetCreatedArcs());

    vtkIdType arcIdsSize = arcIdsFromSplit->GetNumberOfTuples();
    if(arcIdsSize > 0)
      {
      //convert this into a QList of vtkIdTypes so we can emit to the tree
      for(vtkIdType idx=0; idx < arcIdsSize; ++idx)
        {
        newArcIds.push_back(arcIdsFromSplit->GetValue(idx));
        }
      }

    //make sure the model rep is visible, it would be hidden if we can from edit mode
    pqDataRepresentation *modelRep = obj->getRepresentation();
    if(modelRep)
      {
      modelRep->setVisible(true);
      }

    //pass onto the scene tree that this scene polyline is finished being editing
    //it needs the signal so that the tree can split the arcset into arcs.
    //Also this is need to make all the arc representation rerender to fix
    //any old end nodes hanging around
    if(this->Node != NULL) emit this->ArcSplit(this->Node,newArcIds);
    else emit this->ArcSplit2(this->Arc,newArcIds);
    }
}

//-----------------------------------------------------------------------------
qtCMBArcWidget* qtCMBArcWidgetManager::createDefaultContourWidget(
  int& normal, double& planePos)
{
  this->getDefaultArcPlane(normal, planePos);
  return this->createContourWidget(normal, planePos);
}

//-----------------------------------------------------------------------------
qtCMBArcWidget* qtCMBArcWidgetManager::createContourWidget(
   int normal, double position)
{
  vtkSMProxy* pointplacer = vtkSMProxyManager::GetProxyManager()->NewProxy(
    "point_placers", "BoundedPlanePointPlacer");

  qtCMBArcWidget *widget= new qtCMBArcWidget(
    pointplacer, pointplacer, NULL);

  vtkSMProxy* repProxy =
    widget->getWidgetProxy()->GetRepresentationProxy();
  widget->setObjectName("CmbSceneContourWidget");

  vtkSMPropertyHelper(pointplacer, "ProjectionNormal").Set(normal);
  vtkSMPropertyHelper(pointplacer, "ProjectionPosition").Set(position);
  widget->setLineInterpolator(0);
  widget->setPointPlacer(pointplacer);
  pointplacer->UpdateVTKObjects();
  pointplacer->Delete();


  //this block is needed to create the widget in the right order
  //we need to set on the proxy enabled, not the widget
  //than we need to call Initialize
  widget->setView( this->View );
  widget->setWidgetVisible( this->View != NULL );

  vtkSMPropertyHelper(widget->getWidgetProxy(), "AlwaysOnTop").Set(1);
  vtkSMPropertyHelper(widget->getWidgetProxy(), "Enabled").Set(1);
  widget->getWidgetProxy()->UpdateVTKObjects();
  widget->showWidget();

  return widget;
}

//-----------------------------------------------------------------------------
pqCMBArc* qtCMBArcWidgetManager::createLegacyV1Contour(
  const int &normal,const double &position,const int &closedLoop,
  vtkDoubleArray* nodePositions, vtkIdTypeArray* SelIndices)
{

  qtCMBArcWidget* contourWidget =
    this->createContourWidget(normal,position);

  vtkSMNewWidgetRepresentationProxy *widgetProxy =
    contourWidget->getWidgetProxy();

  if(nodePositions && nodePositions->GetNumberOfTuples() > 0)
    {
    QList<QVariant> values;
    double pointPos[3];
    for(vtkIdType i=0; i<nodePositions->GetNumberOfTuples(); i++)
      {
      nodePositions->GetTuple(i,pointPos);
      values << pointPos[0] << pointPos[1] << pointPos[2];
      }
    pqSMAdaptor::setMultipleElementProperty(
      widgetProxy->GetRepresentationProxy()->GetProperty("NodePositions"),
      values);
    }

  if ( SelIndices && SelIndices->GetNumberOfTuples() > 0 )
    {
    QList<QVariant> values;
    for(vtkIdType i=0; i<SelIndices->GetNumberOfTuples(); i++)
      {
      values << SelIndices->GetValue(i);
      }
    pqSMAdaptor::setMultipleElementProperty(
      widgetProxy->GetRepresentationProxy()->GetProperty("SelectNodes"),
      values);
    }

  //push all the node positions down to the server before
  //we call on close loop, or else close loop will fail
  widgetProxy->UpdateVTKObjects();

  if ( closedLoop )
    {
    widgetProxy->InvokeCommand("CloseLoop");
    }

  pqCMBArc *obj = this->createpqCMBArc();
  obj->createArc(widgetProxy);

  //obj->SetPlaneProjectionNormal(normal);
  //obj->SetPlaneProjectionPosition(position);

  //now we need to delete the widget
  delete contourWidget;

  return obj;
}

//-----------------------------------------------------------------------------
void qtCMBArcWidgetManager::getDefaultArcPlane(
  int& orthoPlane, double& projpos)
{
  double focalPt[3], position[3], viewUp[3], viewDirection[3];
  double cameraDistance, parallelScale;
  pqCMBCommonMainWindowCore::getViewCameraInfo(
    this->View, focalPt, position, viewDirection, cameraDistance,
                      viewUp, parallelScale);
  projpos = 0;
  QList<QVariant> values =
    pqSMAdaptor::getMultipleElementProperty(
    this->View->getProxy()->GetProperty("CameraFocalPointInfo"));
  projpos = values[2].toDouble();
  orthoPlane = 2; // z axis
  if (viewDirection[0] < -.99 || viewDirection[0] > .99)
    {
    projpos = values[0].toDouble();
    orthoPlane = 0; // x axis
    }
  else if (viewDirection[1] < -.99 || viewDirection[1] > .99)
    {
    orthoPlane = 1; // y axis;
    projpos = values[1].toDouble();
    }
}

//-----------------------------------------------------------------------------
void qtCMBArcWidgetManager::resetArcPlane(
  int normal, double planePos)
{
  vtkSMProxyProperty* proxyProp =
    vtkSMProxyProperty::SafeDownCast(
    this->Widget->getWidgetProxy()->GetProperty("PointPlacer"));
  if (proxyProp && proxyProp->GetNumberOfProxies())
    {
    vtkSMProxy* pointplacer = proxyProp->GetProxy(0);
    vtkSMPropertyHelper(pointplacer, "ProjectionNormal").Set(normal);
    vtkSMPropertyHelper(pointplacer, "ProjectionPosition").Set(planePos);
    pointplacer->MarkModified(pointplacer);
    pointplacer->UpdateVTKObjects();
    }
}

//-----------------------------------------------------------------------------
void qtCMBArcWidgetManager::modifyArc(
  vtkIdType startPID, vtkIdType endPID, int opType)
{
  if ( (!this->Node && !this->Arc) || this->ActiveWidget != this->EditWidget)
    {
    return;
    }
  pqCMBArc* obj = (this->Node != NULL)?dynamic_cast<pqCMBArc*>(this->Node->getDataObject()):this->Arc;
  if ( obj )
    {
    //call the update arc operator
    vtkNew<vtkCMBSubArcModifyClientOperator> modifyOp;
    modifyOp->SetStartPointId(startPID);
    modifyOp->SetEndPointId(endPID);
    bool valid = modifyOp->Operate(obj->getArcId(),NULL,
      vtkSMSourceProxy::SafeDownCast(obj->getSource()->getProxy()),opType);
    if (!valid)
      {
      return;
      }

    //make sure the model rep is visible, it would be hidden if we can from edit mode
    pqDataRepresentation *modelRep = obj->getRepresentation();
    if(modelRep)
      {
      obj->updateRepresentation();
      modelRep->setVisible(true);
      }
    // if there are new arcs, emit proper signals
    if(opType == vtkCMBSubArcModifyClientOperator::OpMAKEARC)
      {
      QList<vtkIdType> newArcIds;
      vtkNew<vtkIdTypeArray> arcIdsFromSplit;
      //copy the arc ids to create new arcs for
      arcIdsFromSplit->DeepCopy(modifyOp->GetCreatedArcs());

      vtkIdType arcIdsSize = arcIdsFromSplit->GetNumberOfTuples();
      if(arcIdsSize > 0)
        {
        //convert this into a QList of vtkIdTypes so we can emit to the tree
        for(vtkIdType idx=0; idx < arcIdsSize; ++idx)
          {
          newArcIds.push_back(arcIdsFromSplit->GetValue(idx));
          }
        }
      //pass onto the scene tree that this scene polyline is finished being editing
      //it needs the signal so that the tree can split the arcset into arcs.
      //Also this is need to make all the arc representation rerender to fix
      //any old end nodes hanging around
      if(this->Node != NULL) emit this->ArcSplit(this->Node,newArcIds);
      else emit this->ArcSplit2(this->Arc,newArcIds);
      }
    else if(this->Node != NULL)
      {
      //this is required so that the arc can tell polygons that it has changed
      //so that those polygons can remesh
      emit this->ArcModified(this->Node);
      }
    else
      {
      emit this->ArcModified2(this->Arc);
      }
   }
}

//-----------------------------------------------------------------------------
void qtCMBArcWidgetManager::straightenArc(vtkIdType startPID, vtkIdType endPID)
{
  this->modifyArc(startPID, endPID,
    vtkCMBSubArcModifyClientOperator::OpSTRAIGHTEN);
}

//-----------------------------------------------------------------------------
void qtCMBArcWidgetManager::collapseSubArc(vtkIdType startPID, vtkIdType endPID)
{
  this->modifyArc(startPID, endPID,
    vtkCMBSubArcModifyClientOperator::OpCOLLAPSE);
}

//-----------------------------------------------------------------------------
void qtCMBArcWidgetManager::makeArc(vtkIdType startPID, vtkIdType endPID)
{
  this->modifyArc(startPID, endPID,
    vtkCMBSubArcModifyClientOperator::OpMAKEARC);
}

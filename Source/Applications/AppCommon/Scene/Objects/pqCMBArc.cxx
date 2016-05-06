//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "pqCMBArc.h"

#include "pqCMBPolygon.h"

#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqObjectBuilder.h"
#include "pqDataRepresentation.h"
#include "pqPipelineSource.h"
#include "pqRenderView.h"
#include "pqSMAdaptor.h"
#include "pqServer.h"
#include "pqServerManagerModel.h"

#include "vtkCMBArcCreateClientOperator.h"
#include "vtkCMBArcDeleteClientOperator.h"
#include "vtkCMBArcEditClientOperator.h"
#include "vtkCMBArcUpdateAndSplitClientOperator.h"
#include "vtkCMBArcAutoConnectClientOperator.h"
#include "vtkCMBArcFindPickPointOperator.h"

#include "vtkSMPVRepresentationProxy.h"

#include "vtkCommand.h"
#include "vtkIdTypeArray.h"
#include "vtkProcessModule.h"
#include "vtkSMNewWidgetRepresentationProxy.h"
#include "vtkSMOutputPort.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxyProperty.h"
#include "vtkSMProxyManager.h"
#include "vtkSMSourceProxy.h"
#include "vtkPVArcInfo.h"
#include "vtkNew.h"

//-----------------------------------------------------------------------------
pqCMBArc::pqCMBArc()
  :pqCMBSceneObjectBase()
{
  //for an arc the source is actually the arc provider not the arc itself
  this->Source = NULL;
  this->ArcInfo = NULL;
  this->ArcId = -1;
  this->UserDefinedType = "Arc";
  this->PlaneProjectionNormal = 2;
  this->PlaneProjectionPosition = 0;
  this->selColor[0]=this->selColor[2]=this->selColor[3]= 1.0;
  this->selColor[1]=0.0;
  this->origColor[0]=this->origColor[1]=this->origColor[2]=this->origColor[3]= 1.0;
}

//-----------------------------------------------------------------------------
pqCMBArc::pqCMBArc(vtkSMSourceProxy *proxy)
  :pqCMBSceneObjectBase()
{
  //for an arc the source is actually the arc provider not the arc itself
  this->Source = NULL;
  this->ArcInfo = NULL;
  this->ArcId = -1;
  this->UserDefinedType = "Arc";
  this->PlaneProjectionNormal = 2;
  this->PlaneProjectionPosition = 0;
  this->selColor[0]=this->selColor[2]=this->selColor[3]= 1.0;
  this->selColor[1]=0.0;
  this->origColor[0]=this->origColor[1]=this->origColor[2]=this->origColor[3]= 1.0;

  this->createArc(proxy);
}

//-----------------------------------------------------------------------------
pqCMBArc::~pqCMBArc()
{
  if(this->ArcInfo)
    {
    this->ArcInfo->Delete();
    }

  //delete the arc from the server by calling the delete operator
  if (this->ArcId != -1)
    {
    vtkNew<vtkCMBArcDeleteClientOperator> delOp;
    delOp->DeleteArc(this->ArcId);
    }

  //remove ourselves from each polygon
  std::set<pqCMBPolygon*>::iterator it;
  for (it=this->PolygonsUsingArc.begin();
       it!=this->PolygonsUsingArc.end();
       ++it)
    {
    (*it)->removeArc(this);
    }

}

//-----------------------------------------------------------------------------
bool pqCMBArc::createArc(vtkSMNewWidgetRepresentationProxy *widget)
{
  if (this->ArcId == -1)
    {
    vtkNew<vtkCMBArcCreateClientOperator> createOp;
    bool valid = createOp->Create(widget);
    if (!valid)
      {
      return false;
      }

    //update the arc id
    this->ArcId = createOp->GetArcId();

    //update the rep
    this->updateRepresentation();

    //update the plane normal and position
    this->updatePlaneProjectionInfo(widget);
    return true;
    }
  return false;
}

//------------- ----------------------------------------------------------------
bool pqCMBArc::createArc(vtkSMSourceProxy *proxy)
{
  if (this->ArcId == -1)
    {
    vtkNew<vtkCMBArcCreateClientOperator> createOp;
    bool valid = createOp->Create(proxy);
    if (!valid)
      {
      return false;
      }

    this->ArcId = createOp->GetArcId();
    this->updateRepresentation();
    return true;
    }
  return false;
}

//------------------------------------------------------------------------------
bool pqCMBArc::createArc(const vtkIdType& arcId)
{
  if (this->ArcId == -1 && arcId > -1)
    {
    this->ArcId = arcId;
    this->updateRepresentation();
    return true;
    }
  return false;
}

//-----------------------------------------------------------------------------
bool pqCMBArc::editArc(vtkSMNewWidgetRepresentationProxy *widget)
{
  if (this->ArcId == -1)
    {
    return false;
    }

  //for now only works in built in mode
  vtkNew<vtkCMBArcEditClientOperator> editOp;
  editOp->SetArcIsClosed(this->isClosedLoop());
  return editOp->Operate(this->Source->getProxy(),widget);
}

//-----------------------------------------------------------------------------
bool pqCMBArc::findPickPoint(vtkSMOutputPort* port)
{
  if (this->ArcId == -1)
    {
    return false;
    }

  //for now only works in built in mode
  vtkNew<vtkCMBArcFindPickPointOperator> pickOp;
  return pickOp->Operate(this->ArcId,port);

}
//-----------------------------------------------------------------------------
bool pqCMBArc::updateArc(vtkSMNewWidgetRepresentationProxy *widget,
                            vtkIdTypeArray *newlyCreatedArcIds)
{
  if (this->ArcId == -1)
    {
    bool valid = this->createArc(widget);
    if (!valid)
      {
      return false;
      }
    }

  if ( newlyCreatedArcIds == NULL)
    {
    //this needs to be created before being passed in
    return false;
    }

  //call the update arc operator
  vtkNew<vtkCMBArcUpdateAndSplitClientOperator> updateAndSplitOp;
  bool valid = updateAndSplitOp->Operate(this->ArcId,widget);
  if (!valid)
    {
    //we didn't update ourselves, most likely bad widget representation
    //ie. a representation that has 1 or 0 points
    return false;
    }

  //copy the arc ids to create new arcs for
  newlyCreatedArcIds->DeepCopy(updateAndSplitOp->GetCreatedArcs());

  //update the rep
  this->updateRepresentation();

  //update the plane normal and position
  this->updatePlaneProjectionInfo(widget);


  return true;
}

//-----------------------------------------------------------------------------
vtkIdType pqCMBArc::autoConnect(const vtkIdType& secondArcId)
{
  if(this->ArcId == -1 || secondArcId == -1 || this->ArcId == secondArcId)
    {
    return -1;
    }

  vtkNew<vtkCMBArcAutoConnectClientOperator> autoConnectOp;
  bool valid = autoConnectOp->Operate(this->ArcId,secondArcId);
  if(valid)
    {
    return autoConnectOp->GetArcId();
    }
  return -1;
}

//-----------------------------------------------------------------------------
vtkPVArcInfo* pqCMBArc::getArcInfo()
{
  if (!this->Source)
    {
    return NULL;
    }
  if ( !this->ArcInfo )
    {
    this->ArcInfo = vtkPVArcInfo::New();
    }
  this->ArcInfo->SetGatherAllInfo();

  //collect the information from the server poly source into
  //the representation info.
  vtkSMProxy *proxy = this->Source->getProxy();

  proxy->GatherInformation(this->ArcInfo);
  return this->ArcInfo;
}

//-----------------------------------------------------------------------------
bool pqCMBArc::isClosedLoop()
{
  //we need to query the server for this information. The reason is
  //that another arc could have moved an end node causing this to go from
  //unclosed to being closed.
  if ( !this->ArcInfo )
    {
    this->ArcInfo = vtkPVArcInfo::New();
    }

  if (!this->Source)
    {
    //we have no arc this is an invalid call
    return false;
    }

  this->ArcInfo->SetGatherLoopInfoOnly();

   //collect the information from the server poly source into
  //the representation info.
  this->Source->getProxy()->GatherInformation(this->ArcInfo);

  return this->ArcInfo->IsClosedLoop();
}
//-----------------------------------------------------------------------------
int pqCMBArc::getClosedLoop()
{
  //we need to query the server for this information. The reason is
  //that another arc could have moved an end node causing this to go from
  //unclosed to being closed.
  return this->isClosedLoop()? 1 : 0;
}


//-----------------------------------------------------------------------------
void pqCMBArc::inheritPolygonRelationships(pqCMBArc *parent)
{
  //make us have the same polygons
  this->PolygonsUsingArc = parent->PolygonsUsingArc;
  std::set<pqCMBPolygon*>::iterator it;
  for (it=this->PolygonsUsingArc.begin();
       it!=this->PolygonsUsingArc.end();
       ++it)
    {
    (*it)->addArc(this);
    }
}

//-----------------------------------------------------------------------------
pqCMBSceneObjectBase::enumObjectType pqCMBArc::getType() const
{
  return pqCMBSceneObjectBase::Arc;
}

//-----------------------------------------------------------------------------
pqCMBSceneObjectBase *pqCMBArc::duplicate( pqServer * /*server*/,
                                        pqRenderView * /*view*/,
                                        bool /*updateRep*/)
{
  return NULL;
}

//-----------------------------------------------------------------------------
void pqCMBArc::setSelectionInput(vtkSMSourceProxy *selectionInput)
{
  if(!selectionInput)
    {
    this->deselect();
    }
  else
    {
    this->select();
    }
  this->Superclass::setSelectionInput(selectionInput);
}

//-----------------------------------------------------------------------------
void pqCMBArc::select()
{
  this->Superclass::setColor(this->selColor);
}

//-----------------------------------------------------------------------------

void pqCMBArc::deselect()
{
  this->Superclass::setColor(this->origColor);
}

//-----------------------------------------------------------------------------
void pqCMBArc::getColor(double color[4]) const
{
  for(int i=0; i<4; i++)
    {
    color[i] = this->origColor[i];
    }
}

//-----------------------------------------------------------------------------
void pqCMBArc::setColor(double color[4], bool updateRep)
{
  for(int i=0; i<4; i++)
    {
    this->origColor[i] = color[i];
    }
  this->Superclass::setColor(color, updateRep);
}

//-----------------------------------------------------------------------------
void pqCMBArc::setMarkedForDeletion()
{
  if (!this->Source)
    {
    return;
    }

  vtkNew<vtkCMBArcDeleteClientOperator> undoOp;
  undoOp->SetMarkedForDeletion(this->ArcId);

  this->Superclass::setMarkedForDeletion();
}

//-----------------------------------------------------------------------------
void pqCMBArc::unsetMarkedForDeletion()
{
  if (!this->Source)
    {
    return;
    }

  vtkNew<vtkCMBArcDeleteClientOperator> undoOp;
  undoOp->SetUnMarkedForDeletion(this->ArcId);

  this->Superclass::unsetMarkedForDeletion();
}

//-----------------------------------------------------------------------------
void pqCMBArc::arcIsModified()
{
  std::set<pqCMBPolygon*>::iterator it;
  for (it=this->PolygonsUsingArc.begin();
       it!=this->PolygonsUsingArc.end();
       ++it)
    {
    (*it)->arcIsDirty(this);
    }
}

//-----------------------------------------------------------------------------
void pqCMBArc::updateRepresentation()
{
  if ( this->ArcId == -1 )
    {
    return;
    }

  pqApplicationCore* core = pqApplicationCore::instance();
  pqView* view = pqActiveObjects::instance().activeView();
  pqObjectBuilder* builder = core->getObjectBuilder();

  //use case for this is if a person creates an invalid arc
  //we can call this after each edit / update cycle to make
  //sure that we can have a representation.
  //Also this will support if for some odd reason the ArcId changes
  if (!this->Source)
    {
    //create an arc provider for this arc
    this->Source = builder->createSource("CmbArcGroup", "ArcProvider",
                                        core->getActiveServer());
    }

  //tell the provider the arc id it needs to be connected too
  vtkSMProxy *sourceProxy = this->Source->getProxy();
  vtkSMPropertyHelper(sourceProxy,"ArcId").Set(-1);
  sourceProxy->UpdateVTKObjects();
  vtkSMPropertyHelper(sourceProxy,"ArcId").Set(this->ArcId);
  sourceProxy->UpdateVTKObjects();

  //key line to tell the client to re-render the arc
  //this is needed since another arc could have caused this
  //arc to move.
  sourceProxy->MarkModified(NULL);
  vtkSMSourceProxy::SafeDownCast(sourceProxy)->UpdatePipeline();

  if (!this->getRepresentation())
    {
    pqDataRepresentation* repr = builder->createDataRepresentation(
      this->Source->getOutputPort(0), view, "GeometryRepresentation");
    vtkSMPVRepresentationProxy::SetScalarColoring(repr->getProxy(),
                                                  QString("").toLatin1().data(), 0);
    this->setRepresentation(repr);
    }

  this->Superclass::updateRepresentation();
}


//-----------------------------------------------------------------------------
void pqCMBArc::addPolygon(pqCMBPolygon* poly)
{
  this->PolygonsUsingArc.insert(poly);
}

//-----------------------------------------------------------------------------
void pqCMBArc::removePolygon(pqCMBPolygon* poly)
{
  this->PolygonsUsingArc.erase(poly);
}

//-----------------------------------------------------------------------------
void pqCMBArc::setRepresentation(pqDataRepresentation *rep)
{
  pqCMBSceneObjectBase::setRepresentation(rep);

  // For now lets turn off LOD
  this->setLODMode(1);

  //set the original color
  vtkSMProxy* reprProxy = rep->getProxy();
  vtkSMPropertyHelper(reprProxy,"DiffuseColor").Get(this->origColor, 3);
  this->origColor[3] = vtkSMPropertyHelper(reprProxy, "Opacity").GetAsDouble();
}


//-----------------------------------------------------------------------------
void pqCMBArc::updatePlaneProjectionInfo(vtkSMNewWidgetRepresentationProxy *widget)
{
  vtkSMProxy* repProxy = widget->GetRepresentationProxy();
  vtkSMProxyProperty* proxyProp =
    vtkSMProxyProperty::SafeDownCast(
    repProxy->GetProperty("PointPlacer"));
  if (proxyProp && proxyProp->GetNumberOfProxies())
    {
    vtkSMProxy* pointplacer = proxyProp->GetProxy(0);
    this->PlaneProjectionNormal = pqSMAdaptor::getElementProperty(
      pointplacer->GetProperty("ProjectionNormal")).toInt();
    this->PlaneProjectionPosition = pqSMAdaptor::getElementProperty(
      pointplacer->GetProperty("ProjectionPosition")).toDouble();
    }
}

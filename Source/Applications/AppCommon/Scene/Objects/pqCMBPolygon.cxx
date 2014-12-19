/*=========================================================================

  Program:   CMB
  Module:    pqCMBPolygon.cxx

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/
#include "pqCMBPolygon.h"
#include "pqCMBSceneNode.h"
#include "pqCMBSceneObjectBase.h"
#include "pqCMBArc.h"

#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqObjectBuilder.h"
#include "pqDataRepresentation.h"
#include "pqPipelineSource.h"
#include "pqRenderView.h"
#include "pqSMAdaptor.h"
#include "pqServer.h"
#include "pqOutputPort.h"
#include "pqServerManagerModel.h"

#include "vtkCMBArcPolygonCreateClientOperator.h"

#include "vtkCommand.h"
#include "vtkIdTypeArray.h"
#include "vtkProcessModule.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxyProperty.h"
#include "vtkSMProxyManager.h"
#include "vtkSMSourceProxy.h"
#include "vtkNew.h"

#include <algorithm>

//-----------------------------------------------------------------------------
pqCMBPolygon::pqCMBPolygon(double minAngle, double edgeLength,
  std::vector<pqCMBSceneNode*> &inputNodes):
  pqCMBTexturedObject()
{
  this->UserDefinedType = "Polygon";
  this->MinAngle = minAngle;
  this->EdgeLength = edgeLength;
  this->ValidPolygon = false;
  this->FileName = std::string();
  this->InputArcs = std::set<pqCMBArc*>();
  this->NeedsRemesh = true;

  std::vector<pqCMBSceneNode*>::iterator it;
  for (it=inputNodes.begin();it!=inputNodes.end();++it)
    {
    pqCMBArc *arc = dynamic_cast<pqCMBArc*>(
                                              (*it)->getDataObject());
    if(arc)
      {
      this->addArc(arc);
      arc->addPolygon(this);
      }
    }

  //generate our source and make our representation
  pqApplicationCore* core = pqApplicationCore::instance();
  pqView* view = pqActiveObjects::instance().activeView();
  pqRenderView *rview = qobject_cast<pqRenderView*>(view);
  pqObjectBuilder* builder = core->getObjectBuilder();
  this->Source  = builder->createSource("CmbArcGroup",
                                        "PolygonProvider",
                                        core->getActiveServer());

  //make the mesh
  this->ValidPolygon = this->remesh();
  if (this->ValidPolygon)
    {
    //create the representation and texture
    this->prepTexturedObject(core->getActiveServer(), rview);
    rview->forceRender();
    }
  else
    {
    builder->destroy(this->Source);
    this->Source = NULL;
    }
}

//-----------------------------------------------------------------------------
pqCMBPolygon::~pqCMBPolygon()
{
  //remove ourselves from each arc
  std::set<pqCMBArc*>::iterator it;
  for (it=this->InputArcs.begin();
       it!=this->InputArcs.end();
       ++it)
    {
    (*it)->removePolygon(this);
    }
}

//-----------------------------------------------------------------------------
pqCMBSceneObjectBase::enumObjectType pqCMBPolygon::getType() const
{
  return pqCMBSceneObjectBase::Polygon;
}
//-----------------------------------------------------------------------------
pqCMBSceneObjectBase *pqCMBPolygon::duplicate(pqServer * /*server*/,
                                                  pqRenderView * /*view*/,
                                                  bool /*updateRep*/)
{
  return NULL;
}

//-----------------------------------------------------------------------------
std::string pqCMBPolygon::getFileName() const
{
  return this->FileName;
}

//-----------------------------------------------------------------------------
void pqCMBPolygon::setFileName(std::string &filename)
{
  this->FileName=filename;
}

//-----------------------------------------------------------------------------
bool pqCMBPolygon::writeToFile()
{
  if ( this->FileName.size() == 0 )
    {
    return false;
    }

  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();
  pqPipelineSource* writer = builder->createFilter("writers","XMLPolyDataWriter",this->Source);
  pqSMAdaptor::setElementProperty(writer->getProxy()->GetProperty("FileName"),this->FileName.c_str());
  writer->getProxy()->UpdateVTKObjects();
  vtkSMSourceProxy::SafeDownCast(writer->getProxy())->UpdatePipeline();
  builder->destroy(writer);

  return true;
}

//-----------------------------------------------------------------------------
void pqCMBPolygon::updateRepresentation()
{
  if (!this->Source)
    {
    return;
    }

 if(!this->NeedsRemesh)
   {
   //only remesh if we have been marked dirty
   this->Superclass::updateRepresentation();
   return;
   }

  //we are dirty, remesh
  const bool remeshed = this->remesh();
  if(remeshed)
    {
    vtkSMProxy *sourceProxy = this->Source->getProxy();
    //key line to tell the client to re-render the arc
    //this is needed since another arc could have caused this
    //arc to move.
    sourceProxy->MarkModified(NULL);
    vtkSMSourceProxy::SafeDownCast(sourceProxy)->UpdatePipeline();
    }

  this->Superclass::updateRepresentation();
  this->NeedsRemesh = !remeshed;
}

//-----------------------------------------------------------------------------
bool pqCMBPolygon::remesh()
{
  if(!this->NeedsRemesh)
    {
    //only remesh if we have been marked dirty
    return this->ValidPolygon;
    }

  vtkNew<vtkCMBArcPolygonCreateClientOperator> createOp;
  std::set< pqCMBArc* >::const_iterator it;
  for (it=this->InputArcs.begin();
       it!=this->InputArcs.end();
       ++it)
    {
    createOp->AddArc((*it)->getArcId());
    std::cout << (*it)->getArcId() << std::endl;
    (*it)->addPolygon(this);
    }

  this->ValidPolygon = createOp->Create(this->MinAngle,this->EdgeLength,
                                        this->Source->getProxy());

  return this->ValidPolygon;
}


//-----------------------------------------------------------------------------
void pqCMBPolygon::addArc(pqCMBArc* arc)
{
  typedef std::set< pqCMBArc* >::iterator iterator;
  std::pair<iterator,bool> result = this->InputArcs.insert(arc);
  this->NeedsRemesh = result.second;
}

//-----------------------------------------------------------------------------
void pqCMBPolygon::removeArc(pqCMBArc* arc)
{
  std::size_t countRemoved = this->InputArcs.erase(arc);
  this->NeedsRemesh = countRemoved > 0;
}

//-----------------------------------------------------------------------------
void pqCMBPolygon::arcIsDirty( pqCMBArc* )
{
  this->NeedsRemesh = true;
}

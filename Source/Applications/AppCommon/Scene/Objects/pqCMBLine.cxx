/*=========================================================================

  Program:   CMB
  Module:    pqCMBLine.cxx

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
#include "pqCMBLine.h"

#include "pqApplicationCore.h"
#include "pq3DWidgetFactory.h"
#include "pqCMBLineWidget.h"
#include "pqObjectBuilder.h"
#include "pqPipelineSource.h"
#include "pqRenderView.h"
#include "pqSMAdaptor.h"
#include "pqServer.h"
#include <vtkSMPropertyHelper.h>

#include "vtkSMNewWidgetRepresentationProxy.h"
#include "vtkSMInputProperty.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMSourceProxy.h"
#include "vtkSMRepresentationProxy.h"

//-----------------------------------------------------------------------------
pqCMBLine::pqCMBLine() : pqCMBSceneObjectBase()
{
  this->LineWidget = NULL;
}

//-----------------------------------------------------------------------------
pqCMBLine::~pqCMBLine()
{
  if(this->LineWidget)
    {
    delete this->LineWidget;
    }
}

//-----------------------------------------------------------------------------
void pqCMBLine::getDefaultBounds(
  pqRenderView* theView, double bounds[6])
{
  if(!theView)
    {
    return;
    }
  double focal[3];
  pqCMBSceneObjectBase::getCameraFocalPoint(theView, focal);
  bounds[0]=focal[0]-0.5;bounds[2]=focal[1]-0.5;bounds[4]=focal[2]-0.5;
  bounds[1]=focal[0]+0.5;bounds[3]=focal[1]+0.5;bounds[5]=focal[2]+0.5;
}

//-----------------------------------------------------------------------------
pqCMBLine::pqCMBLine(pqCMBSceneObjectBase* refObj,
                           pqServer *server,
                           pqRenderView *view,
                           bool updateRep)
{
  double bounds[6];
  pqCMBLine::getDefaultBounds(view, bounds);
  if (refObj && refObj->getType() != pqCMBSceneObjectBase::Line)
    {
    refObj->getBounds(bounds);
    }

  double pos1[3], pos2[3];
  pos1[0]=bounds[0];pos1[1]=bounds[2];pos1[2]=bounds[4];
  pos2[0]=bounds[1];pos2[1]=bounds[3];pos2[2]=bounds[5];

  this->initialize(pos1, pos2, server, view, updateRep);
}

//-----------------------------------------------------------------------------
pqCMBLine::pqCMBLine(double point1[3],
                           double point2[3],
                           pqServer *server,
                           pqRenderView *view,
                           bool updateRep)
{
  this->initialize(point1, point2, server, view, updateRep);
}
//-----------------------------------------------------------------------------
void pqCMBLine::initialize(double point1[3],
                              double point2[3],
                              pqServer *server,
                              pqRenderView *view,
                              bool /*updateRep*/)
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();

  QPointer<pqPipelineSource> WidgetProxy =
    builder->createSource("sources", "Ruler", server);
  vtkSMProxy* sourceProxy = WidgetProxy->getProxy();
  this->LineWidget = new pqCMBLineWidget(
    sourceProxy, sourceProxy);
  this->LineWidget->setObjectName("pqCMBLineWidget");
  this->LineWidget->setView(view);
  vtkSMProxy* repProxy = this->LineWidget->getWidgetProxy();
  pqSMAdaptor::setElementProperty(repProxy->GetProperty("Visibility"), true);
  QList<QVariant> values;
  values << point1[0] << point1[1] << point1[2];
  pqSMAdaptor::setMultipleElementProperty(
    repProxy->GetProperty("Point1WorldPosition"), values);
  values.clear();
  values << point2[0] << point2[1] << point2[2];
  pqSMAdaptor::setMultipleElementProperty(
    repProxy->GetProperty("Point2WorldPosition"), values);
  repProxy->UpdateVTKObjects();
  this->UserDefinedType = "Line";
}
//-----------------------------------------------------------------------------
void pqCMBLine::setVisibility(bool mode)
{
  if (this->LineWidget && mode != this->LineWidget->widgetVisible())
    {
    this->LineWidget->setProcessEvents(mode);
    this->LineWidget->setWidgetVisible(mode);
    }
}

//-----------------------------------------------------------------------------
void pqCMBLine::setSelectionInput(
  vtkSMSourceProxy *selectionInput)
{
  if(!selectionInput)
    {
    this->deselect();
    }
  else
    {
    this->select();
    }
}

//-----------------------------------------------------------------------------
void pqCMBLine::select()
{
  //this->LineWidget->setProcessEvents(1);
  this->LineWidget->select();
  this->LineWidget->setVisible(true);
}

//-----------------------------------------------------------------------------

void pqCMBLine::deselect()
{
  //this->LineWidget->setProcessEvents(0);
  this->LineWidget->setVisible(false);
  this->LineWidget->deselect();
}

//-----------------------------------------------------------------------------
int pqCMBLine::getPointPosition( int pointIdx,
  double &x, double &y, double &z)
{
  if(!this->LineWidget)
    {
    return 0;
    }
  if(pointIdx != 1 && pointIdx != 2)
    {
    return 0;
    }
  vtkSMNewWidgetRepresentationProxy* repProxy = this->LineWidget->getWidgetProxy();
  repProxy->UpdatePropertyInformation();

  QList<QVariant> position;;
  char strProp[100];
  sprintf(strProp, "Point%dWorldPositionInfo", pointIdx);
  position = pqSMAdaptor::getMultipleElementProperty(
    repProxy->GetProperty(strProp));

  x = position[0].toDouble();
  y = position[1].toDouble();
  z = position[2].toDouble();
  return 1;
}

//-----------------------------------------------------------------------------
void pqCMBLine::updateRepresentation()
{
  vtkSMRepresentationProxy* repProxy = vtkSMRepresentationProxy::SafeDownCast(
    this->LineWidget->getWidgetProxy()->GetRepresentationProxy());
  if(repProxy)
    {
    repProxy->UpdateVTKObjects();
    repProxy->UpdatePipeline();
    }
}

//-----------------------------------------------------------------------------
pqCMBSceneObjectBase *pqCMBLine::duplicate(pqServer *server,
                                                  pqRenderView *view,
                                                  bool updateRep)
{
  double point1[3], point2[3];
  this->getPoint1Position(point1[0], point1[1], point1[2]);
  this->getPoint2Position(point2[0], point2[1], point2[2]);
  pqCMBLine *nobj = new pqCMBLine(point1, point2, server, view, updateRep);
  nobj->UserDefinedType = this->UserDefinedType;
  double color[4];
  this->getColor(color);
  nobj->setColor(color);
  return nobj;
}

//-----------------------------------------------------------------------------
void pqCMBLine::getColor(double color[4]) const
{
//  vtkSMPropertyHelper(this->LineWidget->getWidgetProxy(), "LineColor").Get(color, 3);
  this->LineWidget->getColor(color);
  color[3] = 1.0; // Assume we don't support transparent lines for now
}
//-----------------------------------------------------------------------------
void pqCMBLine::setColor(double color[4], bool /*updateRep*/)
{
//  vtkSMPropertyHelper(this->LineWidget->getWidgetProxy(), "LineColor").Set(color, 3);
  this->LineWidget->setColor(color);
  this->updateRepresentation();
}
//-----------------------------------------------------------------------------
void pqCMBLine::getBounds(double /*bounds*/[6]) const
{
  return;
  //this->LineWidget->getBounds(bounds);
}
//-----------------------------------------------------------------------------
void pqCMBLine::getDataBounds(double /*bounds*/[6]) const
{
  return;
  //this->getBounds(bounds);
}

//-----------------------------------------------------------------------------
pqCMBSceneObjectBase::enumObjectType pqCMBLine::getType() const
{
  return pqCMBSceneObjectBase::Line;
}
//-----------------------------------------------------------------------------

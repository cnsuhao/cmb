/*=========================================================================

  Program:   CMB
  Module:    pqCMBVOI.cxx

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



#include "pqCMBVOI.h"


#include "pqApplicationCore.h"
#include "pqDataRepresentation.h"
#include "pqObjectBuilder.h"
#include "pqDataRepresentation.h"
#include "pqPipelineSource.h"
#include "pqRenderView.h"
#include "pqSMAdaptor.h"
#include "pqServer.h"

#include "vtkBoundingBox.h"
#include <vtkProcessModule.h>
#include "vtkPVDataInformation.h"
#include "vtkPVLASOutputBlockInformation.h"
#include "vtkPVSceneGenObjectInformation.h"
#include <vtkSMDataSourceProxy.h>
#include <vtkSMDoubleVectorProperty.h>
#include <vtkSMIntVectorProperty.h>
#include "vtkSMNewWidgetRepresentationProxy.h"
#include <vtkSMPropertyHelper.h>
#include "vtkSMRepresentationProxy.h"
#include <vtkSMProxyProperty.h>
#include <vtkSMRenderViewProxy.h>
#include <vtkSMRepresentationProxy.h>
#include <vtkSMSourceProxy.h>
#include <vtkTransform.h>
#include "vtkSMProxyManager.h"
#include "vtkImageData.h"
#include <QVariant>

#include "vtkPVXMLElement.h"
#include "vtkSMPropertyIterator.h"
//-----------------------------------------------------------------------------
pqCMBVOI::pqCMBVOI() : pqCMBSceneObjectBase()
{
}
//-----------------------------------------------------------------------------
pqCMBVOI::pqCMBVOI(pqPipelineSource*source,
                         pqRenderView *view, pqServer * /*server*/)
  : pqCMBSceneObjectBase(source)
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();

  this->setRepresentation(
    builder->createDataRepresentation(
    this->Source->getOutputPort(0), view, "GeometryRepresentation"));
}
//-----------------------------------------------------------------------------
pqCMBVOI::pqCMBVOI(double origin[3],
                         double bounds[6],
                         pqServer *server,
                         pqRenderView *view,
                         bool updateRep)
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();

  this->Source =
    builder->createSource("sources", "OutlineSource", server);
  QList<QVariant> values;
  values << bounds[0] << bounds[1] << bounds[2] << bounds[3]
         << bounds[4] << bounds[5];
  pqSMAdaptor::setMultipleElementProperty(this->Source->
                                          getProxy()->GetProperty("Bounds"),
                                          values);
  this->Source->getProxy()->UpdateVTKObjects();

  this->setRepresentation(
    builder->createDataRepresentation(
    this->Source->getOutputPort(0), view, "GeometryRepresentation"));
  values.clear();
  values << origin[0] << origin[1] << origin[2];

  pqSMAdaptor::setMultipleElementProperty(this->getRepresentation()->
                                          getProxy()->GetProperty("Position"),
                                          values);
  if (updateRep)
    {
    this->getRepresentation()->getProxy()->UpdateVTKObjects();
    }

  this->UserDefinedType = "VOI";
}

//-----------------------------------------------------------------------------
pqCMBVOI::~pqCMBVOI()
{
}

//-----------------------------------------------------------------------------
pqCMBSceneObjectBase::enumObjectType pqCMBVOI::getType() const
{
  return pqCMBSceneObjectBase::VOI;
}
//-----------------------------------------------------------------------------
pqCMBSceneObjectBase *pqCMBVOI::duplicate(pqServer *server,
                                    pqRenderView *view,
                                    bool updateRep)
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();

  pqPipelineSource *pdSource =
    builder->createSource("sources", "OutlineSource", server);

  pdSource->getProxy()->Copy(this->Source->getProxy());
  pdSource->getProxy()->UpdateVTKObjects();
  pqCMBVOI *nobj = new pqCMBVOI(pdSource, view, server);
  this->duplicateInternals(nobj);
  if (updateRep)
    {
    nobj->getRepresentation()->getProxy()->UpdateVTKObjects();
    }
  return nobj;
}

//-----------------------------------------------------------------------------
void pqCMBVOI::setVOI(double minPnt[3], double maxPnt[3])
{

  double dx = 0.5 *(maxPnt[0] - minPnt[0]);
  double dy = 0.5 *(maxPnt[1] - minPnt[1]);
  double dz = 0.5 *(maxPnt[2] - minPnt[2]);

  QList<QVariant> values;
  values << -dx << dx << -dy << dy << -dz << dz;
  pqSMAdaptor::setMultipleElementProperty(this->Source->
                                          getProxy()->GetProperty("Bounds"),
                                          values);
  this->Source->getProxy()->UpdateVTKObjects();

  double temp[3];
  temp[0] = minPnt[0] + dx;
  temp[1] = minPnt[1] + dy;
  temp[2] = minPnt[2] + dz;

  this->setPosition(temp, false);
  temp[0] = temp[1] = temp[2] = 0.0;
  this->setOrientation(temp, false);
  temp[0] = temp[1] = temp[2] = 1.0;
  this->setScale(temp, true);
  this->Source->updatePipeline();

 }

//-----------------------------------------------------------------------------
int pqCMBVOI::getVOI(double minPnt[3], double maxPnt[3]) const
{
  double bounds[6];
  this->getBounds(bounds);

  minPnt[0] = bounds[0];
  maxPnt[0] = bounds[1];
  minPnt[1] = bounds[2];
  maxPnt[1] = bounds[3];
  minPnt[2] = bounds[4];
  maxPnt[2] = bounds[5];
  return 0;
}

//-----------------------------------------------------------------------------
bool pqCMBVOI::contains(pqCMBSceneObjectBase *object) const
{
  if (!object)
    {
    return false;
    }
  vtkBoundingBox myBB, objBB;
  this->getBounds(&myBB);
  object->getBounds(&objBB);
  return myBB.Contains(objBB);
}

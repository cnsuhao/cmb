//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "pqCMBVOI.h"

#include "pqApplicationCore.h"
#include "pqDataRepresentation.h"
#include "pqDataRepresentation.h"
#include "pqObjectBuilder.h"
#include "pqPipelineSource.h"
#include "pqRenderView.h"
#include "pqSMAdaptor.h"
#include "pqServer.h"

#include "vtkBoundingBox.h"
#include "vtkImageData.h"
#include "vtkPVDataInformation.h"
#include "vtkPVLASOutputBlockInformation.h"
#include "vtkPVSceneGenObjectInformation.h"
#include "vtkSMNewWidgetRepresentationProxy.h"
#include "vtkSMProxyManager.h"
#include "vtkSMRepresentationProxy.h"
#include <QVariant>
#include <vtkProcessModule.h>
#include <vtkSMDataSourceProxy.h>
#include <vtkSMDoubleVectorProperty.h>
#include <vtkSMIntVectorProperty.h>
#include <vtkSMPropertyHelper.h>
#include <vtkSMProxyProperty.h>
#include <vtkSMRenderViewProxy.h>
#include <vtkSMRepresentationProxy.h>
#include <vtkSMSourceProxy.h>
#include <vtkTransform.h>

#include "vtkPVXMLElement.h"
#include "vtkSMPropertyIterator.h"

pqCMBVOI::pqCMBVOI()
  : pqCMBSceneObjectBase()
{
}

pqCMBVOI::pqCMBVOI(pqPipelineSource* source, pqRenderView* view, pqServer* /*server*/)
  : pqCMBSceneObjectBase(source)
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();

  this->setRepresentation(builder->createDataRepresentation(
    this->Source->getOutputPort(0), view, "GeometryRepresentation"));
}

pqCMBVOI::pqCMBVOI(
  double origin[3], double bounds[6], pqServer* server, pqRenderView* view, bool updateRep)
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();

  this->Source = builder->createSource("sources", "OutlineSource", server);
  QList<QVariant> values;
  values << bounds[0] << bounds[1] << bounds[2] << bounds[3] << bounds[4] << bounds[5];
  pqSMAdaptor::setMultipleElementProperty(this->Source->getProxy()->GetProperty("Bounds"), values);
  this->Source->getProxy()->UpdateVTKObjects();

  this->setRepresentation(builder->createDataRepresentation(
    this->Source->getOutputPort(0), view, "GeometryRepresentation"));
  values.clear();
  values << origin[0] << origin[1] << origin[2];

  pqSMAdaptor::setMultipleElementProperty(
    this->getRepresentation()->getProxy()->GetProperty("Position"), values);
  if (updateRep)
  {
    this->getRepresentation()->getProxy()->UpdateVTKObjects();
  }

  this->UserDefinedType = "VOI";
}

pqCMBVOI::~pqCMBVOI()
{
}

pqCMBSceneObjectBase::enumObjectType pqCMBVOI::getType() const
{
  return pqCMBSceneObjectBase::VOI;
}

pqCMBSceneObjectBase* pqCMBVOI::duplicate(pqServer* server, pqRenderView* view, bool updateRep)
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();

  pqPipelineSource* pdSource = builder->createSource("sources", "OutlineSource", server);

  pdSource->getProxy()->Copy(this->Source->getProxy());
  pdSource->getProxy()->UpdateVTKObjects();
  pqCMBVOI* nobj = new pqCMBVOI(pdSource, view, server);
  this->duplicateInternals(nobj);
  if (updateRep)
  {
    nobj->getRepresentation()->getProxy()->UpdateVTKObjects();
  }
  return nobj;
}

void pqCMBVOI::setVOI(double minPnt[3], double maxPnt[3])
{

  double dx = 0.5 * (maxPnt[0] - minPnt[0]);
  double dy = 0.5 * (maxPnt[1] - minPnt[1]);
  double dz = 0.5 * (maxPnt[2] - minPnt[2]);

  QList<QVariant> values;
  values << -dx << dx << -dy << dy << -dz << dz;
  pqSMAdaptor::setMultipleElementProperty(this->Source->getProxy()->GetProperty("Bounds"), values);
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

bool pqCMBVOI::contains(pqCMBSceneObjectBase* object) const
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

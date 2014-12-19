/*=========================================================================

  Program:   CMB
  Module:    pqCMBConicalRegion.cxx

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



#include "pqCMBConicalRegion.h"

#include "pqActiveServer.h"
#include "pqApplicationCore.h"
#include "pqDataRepresentation.h"
#include "pqObjectBuilder.h"
#include "pqDataRepresentation.h"
#include "pqPipelineSource.h"
#include "pqRenderView.h"
#include "pqSMAdaptor.h"
#include "pqServer.h"
#include "qtCMBApplicationOptions.h"

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
pqCMBConicalRegion::pqCMBConicalRegion() : pqCMBSceneObjectBase()
{
}
//-----------------------------------------------------------------------------
pqCMBConicalRegion::pqCMBConicalRegion(pqPipelineSource*source,
                         pqRenderView *view, pqServer * /*server*/)
  : pqCMBSceneObjectBase(source)
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();

  this->setRepresentation(qobject_cast<pqDataRepresentation*>(
    builder->createDataRepresentation(
    this->Source->getOutputPort(0), view, "GeometryRepresentation")));
  pqSMAdaptor::setEnumerationProperty(
    this->getRepresentation()->getProxy()->GetProperty("Representation"),
    qtCMBApplicationOptions::instance()->defaultRepresentationType().c_str());
}
//-----------------------------------------------------------------------------
pqCMBConicalRegion::pqCMBConicalRegion(double baseCenter[3],
                                             double baseRadius,
                                             double height,
                                             double topRadius,
                                             double direction[3],
                                             int resolution,
                                             pqServer *server,
                                             pqRenderView *view,
                                             bool updateRep)
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();

  this->Source =
    builder->createSource("sources", "CmbConeSource", server);

  vtkSMPropertyHelper(this->Source->getProxy(),
    "BaseRadius").Set(baseRadius);

  vtkSMPropertyHelper(this->Source->getProxy(),
    "TopRadius").Set(topRadius);

  vtkSMPropertyHelper(this->Source->getProxy(),
    "Height").Set(height);

  vtkSMPropertyHelper(this->Source->getProxy(),
    "Resolution").Set(resolution);

  vtkSMPropertyHelper(this->Source->getProxy(),
                      "Direction").Set(direction, 3);

  this->Source->getProxy()->UpdateVTKObjects();

  this->setRepresentation(
    builder->createDataRepresentation(
    this->Source->getOutputPort(0), view, "GeometryRepresentation"));
  pqSMAdaptor::setEnumerationProperty(
    this->getRepresentation()->getProxy()->GetProperty("Representation"),
    qtCMBApplicationOptions::instance()->defaultRepresentationType().c_str());

  vtkSMPropertyHelper(this->getRepresentation()->getProxy(),
    "Position").Set(baseCenter, 3);

  if (updateRep)
    {
    this->getRepresentation()->getProxy()->UpdateVTKObjects();
    }

  this->UserDefinedType = "ConicalRegion";
}

//-----------------------------------------------------------------------------
pqCMBConicalRegion::~pqCMBConicalRegion()
{
}

//-----------------------------------------------------------------------------
pqCMBSceneObjectBase::enumObjectType pqCMBConicalRegion::getType() const
{
  return pqCMBSceneObjectBase::GeneralCone;
}
//-----------------------------------------------------------------------------
pqCMBSceneObjectBase *pqCMBConicalRegion::duplicate(pqServer *server,
                                    pqRenderView *view,
                                    bool updateRep)
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();

  pqPipelineSource *pdSource =
    builder->createSource("sources", "CmbConeSource", server);

  pdSource->getProxy()->Copy(this->Source->getProxy());
  pdSource->getProxy()->UpdateVTKObjects();
  pqCMBConicalRegion *nobj = new pqCMBConicalRegion(pdSource, view, server);
  this->duplicateInternals(nobj);
  if (updateRep)
    {
    nobj->getRepresentation()->getProxy()->UpdateVTKObjects();
    }
  return nobj;
}

//-----------------------------------------------------------------------------
void pqCMBConicalRegion::setBaseCenter(double p[3])
{

  vtkSMPropertyHelper(this->getRepresentation()->getProxy(),
    "Position").Set(p, 3);
  this->getRepresentation()->getProxy()->UpdateVTKObjects();
}

//-----------------------------------------------------------------------------
void pqCMBConicalRegion::getBaseCenter(double p[3]) const
{

  vtkSMPropertyHelper(this->getRepresentation()->getProxy(),
    "Position").Get(p, 3);
}

//-----------------------------------------------------------------------------
void pqCMBConicalRegion::setDirection(double p[3])
{

  vtkSMPropertyHelper(this->Source->getProxy(),
    "Direction").Set(p, 3);
  this->Source->getProxy()->UpdateVTKObjects();
}

//-----------------------------------------------------------------------------
void pqCMBConicalRegion::getDirection(double p[3]) const
{

  vtkSMPropertyHelper(this->Source->getProxy(),
    "Direction").Get(p, 3);
}

//-----------------------------------------------------------------------------
void pqCMBConicalRegion::setHeight(double a)
{

  vtkSMPropertyHelper(this->Source->getProxy(),
    "Height").Set(a);
  this->Source->getProxy()->UpdateVTKObjects();
}

//-----------------------------------------------------------------------------
double pqCMBConicalRegion::getHeight() const
{

  return vtkSMPropertyHelper(this->Source->getProxy(),
                             "Height").GetAsDouble();
}

//-----------------------------------------------------------------------------
void pqCMBConicalRegion::setBaseRadius(double a)
{

  vtkSMPropertyHelper(this->Source->getProxy(),
    "BaseRadius").Set(a);
  this->Source->getProxy()->UpdateVTKObjects();
}

//-----------------------------------------------------------------------------
double pqCMBConicalRegion::getBaseRadius() const
{

  return vtkSMPropertyHelper(this->Source->getProxy(),
                             "BaseRadius").GetAsDouble();
}

//-----------------------------------------------------------------------------
void pqCMBConicalRegion::setTopRadius(double a)
{

  vtkSMPropertyHelper(this->Source->getProxy(),
    "TopRadius").Set(a);
  this->Source->getProxy()->UpdateVTKObjects();
}

//-----------------------------------------------------------------------------
double pqCMBConicalRegion::getTopRadius() const
{

  return vtkSMPropertyHelper(this->Source->getProxy(),
                             "TopRadius").GetAsDouble();
}

//-----------------------------------------------------------------------------
void pqCMBConicalRegion::setResolution(int a)
{

  vtkSMPropertyHelper(this->Source->getProxy(),
    "Resolution").Set(a);
  this->Source->getProxy()->UpdateVTKObjects();
}

//-----------------------------------------------------------------------------
int pqCMBConicalRegion::getResolution() const
{

  return vtkSMPropertyHelper(this->Source->getProxy(),
                             "Resolution").GetAsInt();
}

//-----------------------------------------------------------------------------

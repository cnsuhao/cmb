//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================



#include "pqCMBCrossSection.h"


#include "pqApplicationCore.h"
#include "pqDataRepresentation.h"
#include "pqObjectBuilder.h"
#include "pqDataRepresentation.h"
#include "pqPipelineRepresentation.h"
#include "pqPipelineSource.h"
#include "pqRenderView.h"
#include "pqSMAdaptor.h"
#include "pqServer.h"
#include "qtCMBApplicationOptions.h"

#include <vtkProcessModule.h>
#include <vtkSMDataSourceProxy.h>
#include <vtkSMDoubleVectorProperty.h>
#include <vtkSMIntVectorProperty.h>
#include <vtkSMPropertyHelper.h>
#include "vtkSMRepresentationProxy.h"
#include <vtkSMProxyProperty.h>
#include <vtkSMRenderViewProxy.h>
#include <vtkSMRepresentationProxy.h>
#include <vtkSMSourceProxy.h>
#include "vtkSMProxyManager.h"
#include "vtkGeometryRepresentation.h"
#include <QVariant>

#include "pqRepresentationHelperFunctions.h"
#include "vtkDataObject.h"

//-----------------------------------------------------------------------------
pqCMBCrossSection::pqCMBCrossSection() : pqCMBSceneObjectBase()
{
}
//-----------------------------------------------------------------------------
pqCMBCrossSection::pqCMBCrossSection(pqPipelineSource*source,
                         pqRenderView *view, pqServer * /*server*/)
  : pqCMBSceneObjectBase(source)
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();

  this->setRepresentation(qobject_cast<pqDataRepresentation*>(
    builder->createDataRepresentation(
    this->Source->getOutputPort(0), view, "GeometryRepresentation")));
  //pqSMAdaptor::setElementProperty(
  //  this->getRepresentation()->getProxy()->GetProperty("LineWidth"), 3);
  pqSMAdaptor::setElementProperty(
    this->getRepresentation()->getProxy()->GetProperty("PointSize"), 5);
//  pqSMAdaptor::setEnumerationProperty(
//    this->getRepresentation()->getProxy()->GetProperty("Representation"),
//    qtCMBApplicationOptions::instance()->defaultRepresentationType().c_str());

  RepresentationHelperFunctions::CMB_COLOR_REP_BY_ARRAY(
    this->getRepresentation()->getProxy(), "MATID", vtkDataObject::CELL);
  this->UserDefinedType = "GeoCrossSection";
}

//-----------------------------------------------------------------------------
pqCMBCrossSection::~pqCMBCrossSection()
{
}

//-----------------------------------------------------------------------------
pqCMBSceneObjectBase::enumObjectType pqCMBCrossSection::getType() const
{
  return pqCMBSceneObjectBase::GeoCrossSection;
}
//-----------------------------------------------------------------------------
pqCMBSceneObjectBase *pqCMBCrossSection::duplicate(pqServer *server,
                                    pqRenderView *view,
                                    bool updateRep)
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();

  pqPipelineSource *pdSource =
    builder->createSource("sources", "HydroModelPolySource", server);

  vtkSMDataSourceProxy::SafeDownCast(pdSource->getProxy())->CopyData(
    vtkSMSourceProxy::SafeDownCast(this->Source->getProxy()));
  pdSource->updatePipeline();
  pqCMBCrossSection *nobj = new pqCMBCrossSection(pdSource, view, server);
  this->duplicateInternals(nobj);
  if (updateRep)
    {
    nobj->getRepresentation()->getProxy()->UpdateVTKObjects();
    }
  return nobj;
}

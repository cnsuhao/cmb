//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "pqCMBBoreHole.h"

#include "pqApplicationCore.h"
#include "pqDataRepresentation.h"
#include "pqDataRepresentation.h"
#include "pqObjectBuilder.h"
#include "pqPipelineRepresentation.h"
#include "pqPipelineSource.h"
#include "pqRenderView.h"
#include "pqSMAdaptor.h"
#include "pqServer.h"
#include "qtCMBApplicationOptions.h"

#include "vtkGeometryRepresentation.h"
#include "vtkPVDataInformation.h"
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

#include "pqRepresentationHelperFunctions.h"
#include "vtkDataObject.h"

pqCMBBoreHole::pqCMBBoreHole()
  : pqCMBSceneObjectBase()
{
}

pqCMBBoreHole::pqCMBBoreHole(pqPipelineSource* source, pqRenderView* view, pqServer* /*server*/)
  : pqCMBSceneObjectBase(source)
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();

  //create the filter that extracts each contour
  this->TubeFilter = builder->createFilter("filters", "TubeFilter", source);
  vtkSMSourceProxy::SafeDownCast(this->TubeFilter->getProxy())->UpdatePipeline();
  this->setRepresentation(qobject_cast<pqDataRepresentation*>(builder->createDataRepresentation(
    this->TubeFilter->getOutputPort(0), view, "GeometryRepresentation")));
  pqSMAdaptor::setEnumerationProperty(
    this->getRepresentation()->getProxy()->GetProperty("Representation"),
    qtCMBApplicationOptions::instance()->defaultRepresentationType().c_str());
  RepresentationHelperFunctions::CMB_COLOR_REP_BY_ARRAY(
    this->getRepresentation()->getProxy(), "mat", vtkDataObject::CELL);

  this->UserDefinedType = "GeoBoreHole";
}

pqCMBBoreHole::~pqCMBBoreHole()
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();

  if (this->TubeFilter)
  {
    builder->destroy(this->TubeFilter);
    this->TubeFilter = 0;
  }
}

void pqCMBBoreHole::setTubeRadius(double radius)
{
  vtkSMPropertyHelper(this->TubeFilter->getProxy(), "Radius").Set(radius);
  this->TubeFilter->getProxy()->UpdateVTKObjects();
  this->updateRepresentation();
}

pqCMBSceneObjectBase::enumObjectType pqCMBBoreHole::getType() const
{
  return pqCMBSceneObjectBase::GeoBoreHole;
}

pqCMBSceneObjectBase* pqCMBBoreHole::duplicate(pqServer* server, pqRenderView* view, bool updateRep)
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();

  pqPipelineSource* pdSource = builder->createSource("sources", "HydroModelPolySource", server);

  vtkSMDataSourceProxy::SafeDownCast(pdSource->getProxy())
    ->CopyData(vtkSMSourceProxy::SafeDownCast(this->TubeFilter->getProxy()));
  pdSource->updatePipeline();
  pqCMBBoreHole* nobj = new pqCMBBoreHole(pdSource, view, server);
  this->duplicateInternals(nobj);
  if (updateRep)
  {
    nobj->getRepresentation()->getProxy()->UpdateVTKObjects();
  }
  return nobj;
}

pqPipelineSource* pqCMBBoreHole::getSelectionSource() const
{
  return this->TubeFilter;
}

vtkSMSourceProxy* pqCMBBoreHole::getSelectionInput() const
{
  vtkSMSourceProxy* proxy;
  proxy = vtkSMSourceProxy::SafeDownCast(this->TubeFilter->getProxy());
  return proxy->GetSelectionInput(0);
}

void pqCMBBoreHole::setSelectionInput(vtkSMSourceProxy* selectionInput)
{
  vtkSMSourceProxy* proxy;
  proxy = vtkSMSourceProxy::SafeDownCast(this->TubeFilter->getProxy());
  proxy->SetSelectionInput(0, selectionInput, 0);
}

void pqCMBBoreHole::getDataBounds(double bounds[6]) const
{
  vtkSMSourceProxy::SafeDownCast(this->TubeFilter->getProxy())
    ->GetDataInformation()
    ->GetBounds(bounds);
}

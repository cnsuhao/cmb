/*=========================================================================

  Program:   CMB
  Module:    pqCMBBoreHole.cxx

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



#include "pqCMBBoreHole.h"

#include "pqActiveServer.h"
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
#include "vtkPVDataInformation.h"
#include <QVariant>

#include "pqRepresentationHelperFunctions.h"
#include "vtkDataObject.h"

//-----------------------------------------------------------------------------
pqCMBBoreHole::pqCMBBoreHole() : pqCMBSceneObjectBase()
{
}
//-----------------------------------------------------------------------------
pqCMBBoreHole::pqCMBBoreHole(pqPipelineSource*source,
                         pqRenderView *view, pqServer * /*server*/)
  : pqCMBSceneObjectBase(source)
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();

  //create the filter that extracts each contour
  this->TubeFilter = builder->createFilter("filters",
    "TubeFilter", source);
  vtkSMSourceProxy::SafeDownCast(this->TubeFilter->getProxy())->UpdatePipeline();
  this->setRepresentation(qobject_cast<pqDataRepresentation*>(
    builder->createDataRepresentation(
    this->TubeFilter->getOutputPort(0), view, "GeometryRepresentation")));
  pqSMAdaptor::setEnumerationProperty(
    this->getRepresentation()->getProxy()->GetProperty("Representation"),
    qtCMBApplicationOptions::instance()->defaultRepresentationType().c_str());
  RepresentationHelperFunctions::CMB_COLOR_REP_BY_ARRAY(
    this->getRepresentation()->getProxy(), "mat", vtkDataObject::CELL);

  this->UserDefinedType = "GeoBoreHole";
}

//-----------------------------------------------------------------------------
pqCMBBoreHole::~pqCMBBoreHole()
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();

  if (this->TubeFilter)
    {
    builder->destroy( this->TubeFilter );
    this->TubeFilter = 0;
    }
}
//-----------------------------------------------------------------------------
void pqCMBBoreHole::setTubeRadius(double radius)
{
  vtkSMPropertyHelper(this->TubeFilter->getProxy(),
                      "Radius").Set(radius);
  this->TubeFilter->getProxy()->UpdateVTKObjects();
  this->updateRepresentation();
}
//-----------------------------------------------------------------------------
pqCMBSceneObjectBase::enumObjectType pqCMBBoreHole::getType() const
{
  return pqCMBSceneObjectBase::GeoBoreHole;
}
//-----------------------------------------------------------------------------
pqCMBSceneObjectBase *pqCMBBoreHole::duplicate(pqServer *server,
                                    pqRenderView *view,
                                    bool updateRep)
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();

  pqPipelineSource *pdSource =
    builder->createSource("sources", "HydroModelPolySource", server);

  vtkSMDataSourceProxy::SafeDownCast(pdSource->getProxy())->CopyData(
    vtkSMSourceProxy::SafeDownCast(this->TubeFilter->getProxy()));
  pdSource->updatePipeline();
  pqCMBBoreHole *nobj = new pqCMBBoreHole(pdSource, view, server);
  this->duplicateInternals(nobj);
  if (updateRep)
    {
    nobj->getRepresentation()->getProxy()->UpdateVTKObjects();
    }
  return nobj;
}
//-----------------------------------------------------------------------------
pqPipelineSource * pqCMBBoreHole::getSelectionSource() const
{
  return this->TubeFilter;
}

//-----------------------------------------------------------------------------
vtkSMSourceProxy *pqCMBBoreHole::getSelectionInput() const
{
  vtkSMSourceProxy *proxy;
  proxy = vtkSMSourceProxy::SafeDownCast(
    this->TubeFilter->getProxy() );
  return proxy->GetSelectionInput(0);
  }

//-----------------------------------------------------------------------------
void pqCMBBoreHole::setSelectionInput(vtkSMSourceProxy *selectionInput)
{
  vtkSMSourceProxy *proxy;
  proxy = vtkSMSourceProxy::SafeDownCast(
    this->TubeFilter->getProxy() );
  proxy->SetSelectionInput(0, selectionInput, 0);
}

//-----------------------------------------------------------------------------
void pqCMBBoreHole::getDataBounds(double bounds[6]) const
{
  vtkSMSourceProxy::SafeDownCast(this->TubeFilter->getProxy())->
    GetDataInformation()->GetBounds(bounds);
}

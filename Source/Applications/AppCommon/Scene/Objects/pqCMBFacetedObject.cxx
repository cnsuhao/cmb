/*=========================================================================

  Program:   CMB
  Module:    pqCMBFacetedObject.cxx

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



#include "pqCMBFacetedObject.h"


#include "pqApplicationCore.h"
#include "pqDataRepresentation.h"
#include "pqObjectBuilder.h"
#include "pqDataRepresentation.h"
#include "pqPipelineSource.h"
#include "pqRenderView.h"
#include "pqSMAdaptor.h"
#include "pqServer.h"

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
#include <QFileInfo>
#include <QVariant>

//-----------------------------------------------------------------------------
pqCMBFacetedObject::pqCMBFacetedObject() : pqCMBTexturedObject()
{
  this->SurfaceType = pqCMBSceneObjectBase::Other;
}
//-----------------------------------------------------------------------------
pqCMBFacetedObject::pqCMBFacetedObject(pqPipelineSource *source,
                                               pqRenderView *view,
                                               pqServer *server,
                                               const char *filename)
  : pqCMBTexturedObject(source, view, server)
{
  this->FileName = filename;
  this->SurfaceType = pqCMBSceneObjectBase::Other;
}

//-----------------------------------------------------------------------------
pqCMBFacetedObject::pqCMBFacetedObject(pqPipelineSource* source,
                                            pqServer *server,
                                            pqRenderView *view,
                                            bool updateRep)
  : pqCMBTexturedObject(source, view, server)
{
  if (updateRep)
    {
    this->getRepresentation()->getProxy()->UpdateVTKObjects();
    }
  this->SurfaceType = pqCMBSceneObjectBase::Other;
}

//-----------------------------------------------------------------------------
pqCMBFacetedObject::pqCMBFacetedObject(const char *filename,
                                             pqServer *server,
                                             pqRenderView *view,
                                             bool /*updateRep*/)
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();
  QStringList files;
  files << filename;

  builder->blockSignals(true);
  pqPipelineSource* source;
  QFileInfo finfo(filename);
  source =  builder->createReader("sources", "CMBGeometryReader", files, server);
  builder->blockSignals(false);
  this->Source = source;
  this->prepTexturedObject(server, view);
  this->setFileName(filename);
  this->SurfaceType = pqCMBSceneObjectBase::Other;
}

//-----------------------------------------------------------------------------
pqCMBFacetedObject::~pqCMBFacetedObject()
{
}

//-----------------------------------------------------------------------------
pqCMBFacetedObject::enumObjectType pqCMBFacetedObject::getType() const
{
  return pqCMBSceneObjectBase::Faceted;
}
//-----------------------------------------------------------------------------
pqPipelineSource * pqCMBFacetedObject::getTransformedSource(pqServer *server) const
{
  vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
  this->getTransform(transform);
  vtkMatrix4x4 *matrix = transform->GetMatrix();
  // if non-identity transform... need to transform the data
  if (matrix->Element[0][0] != 1 || matrix->Element[0][1] != 0 ||
    matrix->Element[0][2] != 0 || matrix->Element[0][3] != 0 ||
    matrix->Element[1][0] != 0 || matrix->Element[1][1] != 1 ||
    matrix->Element[1][2] != 0 || matrix->Element[1][3] != 0 ||
    matrix->Element[2][0] != 0 || matrix->Element[2][1] != 0 ||
    matrix->Element[2][2] != 1 || matrix->Element[2][3] != 0 ||
    matrix->Element[3][0] != 0 || matrix->Element[3][1] != 0 ||
    matrix->Element[3][2] != 0 || matrix->Element[3][3] != 1)
    {
    QList<QVariant> values;
    for (int i = 0; i < 4; i++)
      {
      for (int j = 0; j < 4; j++)
        {
        values << matrix->Element[i][j];
        }
      }
    pqObjectBuilder* builder = pqApplicationCore::instance()->getObjectBuilder();

    vtkSMProxy *transformProxy = builder->createProxy("transforms", "Transform",
      server, "transforms");
    pqSMAdaptor::setMultipleElementProperty(
      transformProxy->GetProperty("Matrix"), values);
    transformProxy->UpdateVTKObjects();

    pqPipelineSource *transformFilter = builder->createFilter( "filters",
      "TransformFilter", this->Source);
    pqSMAdaptor::setProxyProperty(
      transformFilter->getProxy()->GetProperty("Transform"), transformProxy);
    transformFilter->getProxy()->UpdateVTKObjects();
    vtkSMSourceProxy::SafeDownCast( transformFilter->getProxy() )->UpdatePipeline();

    pqPipelineSource *pdSource = builder->createSource("sources",
      "HydroModelPolySource", server);
    vtkSMDataSourceProxy::SafeDownCast(pdSource->getProxy())->CopyData(
      vtkSMSourceProxy::SafeDownCast(transformFilter->getProxy()));
    builder->destroy(transformFilter);

    return pdSource;
    }
  return this->Source;
}

//-----------------------------------------------------------------------------
pqPipelineSource *pqCMBFacetedObject::duplicatePipelineSource(pqServer *server)
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();

  pqPipelineSource *pdSource =
    builder->createSource("sources", "HydroModelPolySource", server);

  vtkSMDataSourceProxy::SafeDownCast(pdSource->getProxy())->CopyData(
    vtkSMSourceProxy::SafeDownCast(this->Source->getProxy()));
  return pdSource;
}

//-----------------------------------------------------------------------------
pqCMBSceneObjectBase *pqCMBFacetedObject::duplicate(pqServer *server,
                                                    pqRenderView *view,
                                                    bool updateRep)
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();

  pqPipelineSource *pdSource =
    builder->createSource("sources", "HydroModelPolySource", server);

  vtkSMDataSourceProxy::SafeDownCast(pdSource->getProxy())->CopyData(
    vtkSMSourceProxy::SafeDownCast(this->Source->getProxy()));

  pqCMBFacetedObject *nobj = new pqCMBFacetedObject(pdSource, view, server,
                                                          this->FileName.c_str());
  this->duplicateInternals(nobj);
  nobj->SurfaceType = this->SurfaceType;

  if (updateRep)
    {
    nobj->getRepresentation()->getProxy()->UpdateVTKObjects();
    }

  return nobj;
}


//-----------------------------------------------------------------------------
std::string pqCMBFacetedObject::getSurfaceTypeAsString() const
{
  return this->convertSurfaceTypeToString(this->SurfaceType);
}

//-----------------------------------------------------------------------------

/*=========================================================================

  Program:   CMB
  Module:    pqCMBPlane.cxx

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



#include "pqCMBPlane.h"

#include "pqActiveServer.h"
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
#include <QVariant>

#include "vtkPVXMLElement.h"
#include "vtkSMPropertyIterator.h"
//-----------------------------------------------------------------------------
pqCMBPlane::pqCMBPlane() : pqCMBTexturedObject()
{
}
//-----------------------------------------------------------------------------
pqCMBPlane::pqCMBPlane(pqPipelineSource*source,
                         pqRenderView *view, pqServer *server)
  : pqCMBTexturedObject(source, view, server)
{
}
//-----------------------------------------------------------------------------
pqCMBPlane::pqCMBPlane(double p1[3],
                             double p2[3],
                             pqServer *server,
                             pqRenderView *view,
                             bool updateRep)
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();
  double p[3];
  this->Source =
    builder->createSource("sources", "PlaneSource", server);
  vtkSMPropertyHelper(this->Source->getProxy(), "Origin").Set(p1, 3);
  p[0] = p2[0];
  p[1] = p1[1];
  p[2] = p1[2];

  vtkSMPropertyHelper(this->Source->getProxy(), "Point1").Set(p, 3);
  p[0] = p1[0];
  p[1] = p2[1];
  vtkSMPropertyHelper(this->Source->getProxy(), "Point2").Set(p, 3);
  this->prepTexturedObject(server, view);
  if (updateRep)
    {
    this->getRepresentation()->getProxy()->UpdateVTKObjects();
    }

  this->UserDefinedType = "GroundPlane";
}

//-----------------------------------------------------------------------------
pqCMBPlane::~pqCMBPlane()
{
}


//-----------------------------------------------------------------------------
void pqCMBPlane::setPlaneInfo(double p1[3], double p2[3])
{
  double p[3];
  vtkSMPropertyHelper(this->Source->getProxy(), "Origin").Set(p1, 3);
  p[0] = p2[0];
  p[1] = p1[1];
  p[2] = p1[2];
  vtkSMPropertyHelper(this->Source->getProxy(), "Point1").Set(p, 3);
  p[0] = p1[0];
  p[1] = p2[1];
  vtkSMPropertyHelper(this->Source->getProxy(), "Point2").Set(p, 3);
  this->Source->getProxy()->UpdateVTKObjects();
 }

//-----------------------------------------------------------------------------
int pqCMBPlane::getPlaneInfo(double p1[3], double p2[3]) const
{
  double p[3];
  vtkSMPropertyHelper(this->Source->getProxy(), "Origin").Get(p1, 3);
  p2[2] = p1[2]; // Set Z
  vtkSMPropertyHelper(this->Source->getProxy(), "Point1").Get(p, 3);
  p2[0] = p[0]; // Set X
  vtkSMPropertyHelper(this->Source->getProxy(), "Point2").Get(p, 3);
  p2[1] = p[1]; // Set Y
  return 0;
}

//-----------------------------------------------------------------------------
pqCMBSceneObjectBase::enumObjectType pqCMBPlane::getType() const
{
  return pqCMBSceneObjectBase::GroundPlane;
}
//-----------------------------------------------------------------------------
pqCMBSceneObjectBase *pqCMBPlane::duplicate(pqServer *server,
                                    pqRenderView *view,
                                    bool updateRep)
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();

  pqPipelineSource *pdSource =
    builder->createSource("sources", "PlaneSource", server);

  pdSource->getProxy()->Copy(this->Source->getProxy());
  pdSource->getProxy()->UpdateVTKObjects();
  pqCMBPlane *nobj = new pqCMBPlane(pdSource, view, server);
  this->duplicateInternals(nobj);
  if (updateRep)
    {
    nobj->getRepresentation()->getProxy()->UpdateVTKObjects();
    }
  return nobj;
}

//-----------------------------------------------------------------------------

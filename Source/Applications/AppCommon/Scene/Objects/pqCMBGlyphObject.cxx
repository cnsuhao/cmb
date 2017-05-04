//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "pqCMBGlyphObject.h"

#include "pqApplicationCore.h"
#include "pqDataRepresentation.h"
#include "pqObjectBuilder.h"
#include "pqPipelineRepresentation.h"
#include "pqPipelineSource.h"
#include "pqRenderView.h"
#include "pqSMAdaptor.h"
#include "pqServer.h"

#include "vtkGeometryRepresentation.h"
#include "vtkPVDataInformation.h"
#include "vtkPVLASOutputBlockInformation.h"
#include "vtkPVSceneGenObjectInformation.h"
#include "vtkSMCMBGlyphPointSourceProxy.h"
#include "vtkSMNewWidgetRepresentationProxy.h"
#include "vtkSMProxyManager.h"
#include "vtkSMRepresentationProxy.h"
#include <QFileInfo>
#include <QVariant>
#include <vtkProcessModule.h>
#include <vtkSMDataSourceProxy.h>
#include <vtkSMDoubleVectorProperty.h>
#include <vtkSMIntVectorProperty.h>
#include <vtkSMPropertyHelper.h>
#include <vtkSMProxyProperty.h>
#include <vtkSMRenderViewProxy.h>
#include <vtkSMSourceProxy.h>
#include <vtkTransform.h>

#include "pqRepresentationHelperFunctions.h"
#include "vtkDataObject.h"

//-----------------------------------------------------------------------------
pqCMBGlyphObject::pqCMBGlyphObject()
  : pqCMBSceneObjectBase()
{
  this->SurfaceType = pqCMBSceneObjectBase::Other;
}
//-----------------------------------------------------------------------------
pqCMBGlyphObject::pqCMBGlyphObject(pqPipelineSource* glyphSource, pqRenderView* view,
  pqServer* server, const char* glyphFilename, const char* pointsFilename, bool updateRep)
{
  this->GlyphFileName = glyphFilename;
  this->PointsFileName = pointsFilename;
  this->GlyphSource = glyphSource;
  this->SurfaceType = pqCMBSceneObjectBase::Other;
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();
  builder->blockSignals(true);
  this->Source = builder->createSource("sources", "CmbGlyphPointSource", server);
  builder->blockSignals(false);
  vtkSMCMBGlyphPointSourceProxy::SafeDownCast(this->Source->getProxy())
    ->ReadFromFile(pointsFilename);
  this->initialize(view, server, updateRep);
}

//-----------------------------------------------------------------------------
pqCMBGlyphObject::pqCMBGlyphObject(pqPipelineSource* glyphSource, pqRenderView* view,
  pqServer* server, const char* glyphFilename, bool updateRep)
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();
  this->GlyphSource = glyphSource;
  this->SurfaceType = pqCMBSceneObjectBase::Other;

  builder->blockSignals(true);
  this->Source = builder->createSource("sources", "CmbGlyphPointSource", server);
  builder->blockSignals(false);
  this->GlyphFileName = glyphFilename;
  this->initialize(view, server, updateRep);
}

//-----------------------------------------------------------------------------
pqCMBGlyphObject::pqCMBGlyphObject(
  const char* glyphFilename, pqServer* server, pqRenderView* view, bool updateRep)
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();
  QStringList files;
  files << glyphFilename;

  builder->blockSignals(true);
  this->GlyphSource = builder->createReader("sources", "CMBGeometryReader", files, server);
  this->Source = builder->createSource("sources", "CmbGlyphPointSource", server);
  builder->blockSignals(false);
  this->GlyphFileName = glyphFilename;
  this->SurfaceType = pqCMBSceneObjectBase::Other;
  this->initialize(view, server, updateRep);
}

//-----------------------------------------------------------------------------
pqCMBGlyphObject::pqCMBGlyphObject(const char* glyphFilename, const char* pointsFilename,
  pqServer* server, pqRenderView* view, bool updateRep)
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();
  QStringList files;
  files << glyphFilename;

  builder->blockSignals(true);
  this->GlyphSource = builder->createReader("sources", "CMBGeometryReader", files, server);
  this->Source = builder->createSource("sources", "CmbGlyphPointSource", server);
  builder->blockSignals(false);
  vtkSMCMBGlyphPointSourceProxy::SafeDownCast(this->Source->getProxy())
    ->ReadFromFile(pointsFilename);
  this->GlyphFileName = glyphFilename;
  this->PointsFileName = pointsFilename;
  this->SurfaceType = pqCMBSceneObjectBase::Other;
  this->initialize(view, server, updateRep);
}

//-----------------------------------------------------------------------------
pqCMBGlyphObject::~pqCMBGlyphObject()
{
  if (this->GlyphSource)
  {
    pqApplicationCore* core = pqApplicationCore::instance();
    pqObjectBuilder* builder = core->getObjectBuilder();
    builder->destroy(this->GlyphSource);
    this->GlyphSource = NULL;
  }
}

//-----------------------------------------------------------------------------
void pqCMBGlyphObject::initialize(pqRenderView* view, pqServer* /*server*/, bool updateRep)
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();
  this->SourceProxy = vtkSMCMBGlyphPointSourceProxy::SafeDownCast(this->Source->getProxy());
  this->SourceProxy->UpdateVTKObjects();
  this->SourceProxy->UpdatePipeline();
  this->setRepresentation(builder->createDataRepresentation(
    this->Source->getOutputPort(0), view, "GeometryRepresentation"));
  this->GlyphSource->getProxy()->MarkModified(this->GlyphSource->getProxy());

  vtkSMProxy* repProxy = this->getRepresentation()->getProxy();
  RepresentationHelperFunctions::CMB_COLOR_REP_BY_ARRAY(repProxy, "Color", vtkDataObject::POINT);

  pqSMAdaptor::setEnumerationProperty(repProxy->GetProperty("Representation"), "3D Glyphs");
  //vtkSMPropertyHelper(repProxy, "ImmediateModeRendering").Set(0);
  vtkSMPropertyHelper(repProxy, "GlyphType").Set(this->GlyphSource->getProxy());
  vtkSMPropertyHelper(repProxy, "Scaling").Set(true);
  vtkSMPropertyHelper(repProxy, "ScaleMode").Set(2);
  vtkSMPropertyHelper(repProxy, "Orient").Set(true);
  vtkSMPropertyHelper(repProxy, "OrientationMode").Set(1);
  vtkSMPropertyHelper(repProxy, "MapScalars").Set(0);
  vtkSMPropertyHelper(repProxy, "SelectScaleArray").Set("Scaling");
  vtkSMPropertyHelper(repProxy, "SelectOrientationVectors").Set("Orientation");

  // This is a work arround for a bug in ParaView
  vtkSMPropertyHelper(repProxy, "ScaleFactor").Set(1.0);
  if (updateRep)
  {
    repProxy->UpdateVTKObjects();
  }
}
//-----------------------------------------------------------------------------
void pqCMBGlyphObject::clearSelectedPointsColor()
{
  this->SourceProxy->ResetColorsToDefault();
}
//-----------------------------------------------------------------------------
pqCMBGlyphObject::enumObjectType pqCMBGlyphObject::getType() const
{
  return pqCMBSceneObjectBase::Glyph;
}

//-----------------------------------------------------------------------------
pqPipelineSource* pqCMBGlyphObject::duplicateGlyphPipelineSource(pqServer* server)
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();

  pqPipelineSource* pdSource = builder->createSource("sources", "HydroModelPolySource", server);

  vtkSMDataSourceProxy::SafeDownCast(pdSource->getProxy())
    ->CopyData(vtkSMSourceProxy::SafeDownCast(this->GlyphSource->getProxy()));
  return pdSource;
}
//-----------------------------------------------------------------------------
pqCMBSceneObjectBase* pqCMBGlyphObject::duplicate(
  pqServer* server, pqRenderView* view, bool updateRep)
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();

  pqPipelineSource* pdSource = builder->createSource("sources", "HydroModelPolySource", server);

  vtkSMDataSourceProxy::SafeDownCast(pdSource->getProxy())
    ->CopyData(vtkSMSourceProxy::SafeDownCast(this->GlyphSource->getProxy()));

  pqCMBGlyphObject* nobj =
    new pqCMBGlyphObject(pdSource, view, server, this->GlyphFileName.c_str());
  this->duplicateInternals(nobj);
  nobj->SurfaceType = this->SurfaceType;

  // Duplicate the points and other information
  int i, n = this->getNumberOfPoints();
  double d[3];
  for (i = 0; i < n; i++)
  {
    this->getPoint(i, d);
    nobj->insertNextPoint(d);
    this->getScale(i, d);
    nobj->setScale(i, d);
    this->getOrientation(i, d);
    nobj->setOrientation(i, d);
  }

  if (updateRep)
  {
    nobj->getRepresentation()->getProxy()->UpdateVTKObjects();
  }

  return nobj;
}

//-----------------------------------------------------------------------------
void pqCMBGlyphObject::insertNextPoint(double* p)
{
  this->SourceProxy->InsertNextPoint(p);
}

//-----------------------------------------------------------------------------
vtkIdType pqCMBGlyphObject::getNumberOfPoints()
{
  return const_cast<vtkSMCMBGlyphPointSourceProxy*>(this->SourceProxy)->GetNumberOfPoints();
}

//-----------------------------------------------------------------------------
void pqCMBGlyphObject::getAveragePoint(double* pa)
{
  int i, n = this->getNumberOfPoints();
  pa[0] = pa[1] = pa[2] = 0.0;
  if (n == 0)
  {
    return;
  }

  double p[3];
  for (i = 0; i < n; i++)
  {
    const_cast<vtkSMCMBGlyphPointSourceProxy*>(this->SourceProxy)->GetPoint(i, p);
    pa[0] += p[0];
    pa[1] += p[1];
    pa[2] += p[2];
  }
  pa[0] /= n;
  pa[1] /= n;
  pa[2] /= n;
}

//-----------------------------------------------------------------------------
void pqCMBGlyphObject::getPoint(vtkIdType i, double* p) const
{
  const_cast<vtkSMCMBGlyphPointSourceProxy*>(this->SourceProxy)->GetPoint(i, p);
}

//-----------------------------------------------------------------------------
void pqCMBGlyphObject::setPoint(vtkIdType i, double* p)
{
  this->SourceProxy->SetPoint(i, p);
}

//-----------------------------------------------------------------------------
void pqCMBGlyphObject::getScale(vtkIdType i, double* p) const
{
  const_cast<vtkSMCMBGlyphPointSourceProxy*>(this->SourceProxy)->GetScale(i, p);
}

//-----------------------------------------------------------------------------
void pqCMBGlyphObject::setScale(vtkIdType i, double* p)
{
  const_cast<vtkSMCMBGlyphPointSourceProxy*>(this->SourceProxy)->SetScale(i, p);
}

//-----------------------------------------------------------------------------
void pqCMBGlyphObject::getOrientation(vtkIdType i, double* p) const
{
  const_cast<vtkSMCMBGlyphPointSourceProxy*>(this->SourceProxy)->GetOrientation(i, p);
}

//-----------------------------------------------------------------------------
void pqCMBGlyphObject::setOrientation(vtkIdType i, double* p)
{
  const_cast<vtkSMCMBGlyphPointSourceProxy*>(this->SourceProxy)->SetOrientation(i, p);
}

//-----------------------------------------------------------------------------
void pqCMBGlyphObject::writePointsFile() const
{
  if (this->PointsFileName != "")
  {
    const_cast<vtkSMCMBGlyphPointSourceProxy*>(this->SourceProxy)
      ->WriteToFile(this->PointsFileName.c_str());
  }
}
//-----------------------------------------------------------------------------
std::string pqCMBGlyphObject::getSurfaceTypeAsString() const
{
  return this->convertSurfaceTypeToString(this->SurfaceType);
}

//-----------------------------------------------------------------------------
pqPipelineSource* pqCMBGlyphObject::getGlyphSource() const
{
  return this->GlyphSource;
}
//-----------------------------------------------------------------------------
void pqCMBGlyphObject::getColor(double color[4]) const
{
  this->SourceProxy->GetDefaultColor(color);
}

//-----------------------------------------------------------------------------
void pqCMBGlyphObject::setColor(double color[4], bool updateRep)
{
  this->SourceProxy->SetDefaultColor(color);
  this->SourceProxy->MarkModified(NULL);
  if (updateRep)
  {
    this->getRepresentation()->getProxy()->UpdateVTKObjects();
  }
}
//-----------------------------------------------------------------------------
void pqCMBGlyphObject::applyTransform(
  double scaleDelta[3], double orientationDelta[3], double translationDelta[3])
{
  this->SourceProxy->ApplyTransform(orientationDelta, translationDelta, scaleDelta);
  this->getRepresentation()->getProxy()->UpdateVTKObjects();
}
//-----------------------------------------------------------------------------
void pqCMBGlyphObject::copyAttributes(pqCMBSceneObjectBase* obj)
{
  double ori[3], scale[3], trans[3], color[4];
  obj->getOrientation(ori);
  obj->getScale(scale);
  trans[0] = trans[1] = trans[2] = 0.0;
  obj->getColor(color);
  this->applyTransform(scale, ori, trans);
  this->setColor(color);
  this->getRepresentation()->getProxy()->UpdateVTKObjects();
}
//-----------------------------------------------------------------------------

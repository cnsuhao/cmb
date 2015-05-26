//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================



#include "pqCMBTexturedObject.h"


#include "pqApplicationCore.h"
#include "pqDataRepresentation.h"
#include "vtkGeometryRepresentation.h"
#include "pqObjectBuilder.h"
#include "pqDataRepresentation.h"
#include "pqPipelineSource.h"
#include "pqPipelineRepresentation.h"
#include "pqRenderView.h"
#include "pqSMAdaptor.h"
#include "pqServer.h"

#include <vtkAlgorithm.h>
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
#include "pqWaitCursor.h"

#include "vtkNew.h"

#include "pqRepresentationHelperFunctions.h"
#include "vtkDataObject.h"

//-----------------------------------------------------------------------------
pqCMBTexturedObject::pqCMBTexturedObject() : pqCMBSceneObjectBase()
{
  this->NumberOfRegistrationPoints = 0;
  this->BathymetrySource = NULL;
  this->ShowElevation = false;
}
//-----------------------------------------------------------------------------
pqCMBTexturedObject::pqCMBTexturedObject(pqPipelineSource *source,
                                               pqRenderView *view,
                                               pqServer *server)
  : pqCMBSceneObjectBase(source)
{
  this->prepTexturedObject(server, view);
  this->NumberOfRegistrationPoints = 0;
  this->BathymetrySource = NULL;
  this->ShowElevation = false;
}

//-----------------------------------------------------------------------------
pqCMBTexturedObject::~pqCMBTexturedObject()
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();

  if (this->TexturePointIntensityFilter)
    {
    builder->destroy( this->TexturePointIntensityFilter );
    this->TexturePointIntensityFilter = 0;
    }

  if (this->TextureImageSource)
    {
    builder->destroy( this->TextureImageSource );
    this->TextureImageSource = 0;
    }

  if (this->ElevationFilter)
    {
    builder->destroy(this->ElevationFilter);
    this->ElevationFilter = NULL;
    }

  if (this->RegisterTextureFilter)
    {
    builder->destroy(this->RegisterTextureFilter);
    this->RegisterTextureFilter = NULL;
    }

  if (this->BathymetryFilter)
    {
    builder->destroy(this->BathymetryFilter);
    this->BathymetryFilter = NULL;
    }
  if (this->BathymetrySource)
    {
    this->BathymetrySource = 0;
    }
}
//-----------------------------------------------------------------------------
pqPipelineSource * pqCMBTexturedObject::getSelectionSource() const
{
  return this->ElevationFilter;
}

//-----------------------------------------------------------------------------
vtkSMSourceProxy *pqCMBTexturedObject::getSelectionInput() const
{
  vtkSMSourceProxy *proxy;
  proxy = vtkSMSourceProxy::SafeDownCast(
    this->ElevationFilter->getProxy() );
  return proxy->GetSelectionInput(0);
  }

//-----------------------------------------------------------------------------
void pqCMBTexturedObject::setSelectionInput(vtkSMSourceProxy *selectionInput)
{
  vtkSMSourceProxy *proxy;
  proxy = vtkSMSourceProxy::SafeDownCast(
    this->ElevationFilter->getProxy() );
  proxy->SetSelectionInput(0, selectionInput, 0);
}

//-----------------------------------------------------------------------------
void pqCMBTexturedObject::getDataBounds(double bounds[6]) const
{
  vtkSMSourceProxy::SafeDownCast(this->BathymetryFilter->getProxy())->
    GetDataInformation()->GetBounds(bounds);
}
//-----------------------------------------------------------------------------
void pqCMBTexturedObject::duplicateInternals(pqCMBSceneObjectBase *o)
{
  pqCMBSceneObjectBase::duplicateInternals(o);
  pqCMBTexturedObject *nobj = dynamic_cast<pqCMBTexturedObject*>(o);
  if (this->TextureImageSource)
    {
    nobj->setTextureMap(this->TextureFileName.toStdString().c_str(),
                        this->NumberOfRegistrationPoints,
                        this->RegistrationPoints);
    }
  nobj->showElevation(this->ShowElevation);
}

//-----------------------------------------------------------------------------
void pqCMBTexturedObject::prepTexturedObject(pqServer * /*server*/,
                                                pqRenderView *view)
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();
  this->Source->getProxy()->UpdateVTKObjects();

  // Add in the ApplyBathymetry Filter, by default it is just
  // a ShallowCopy (NoOP)
  this->BathymetryFilter =
    builder->createFilter("filters",
    "CmbApplyBathymetry",  this->Source);
  vtkSMPropertyHelper(this->BathymetryFilter->getProxy(),
    "NoOP").Set(1);
  this->BathymetryFilter->getProxy()->UpdateVTKObjects();
  this->ElevationRadious = 1.0;
  // force pipeline update
  vtkSMSourceProxy::SafeDownCast(
    this->BathymetryFilter->getProxy() )->UpdatePipeline();

  this->RegisterTextureFilter =
    builder->createFilter("filters",
                          "RegisterPlanarTextureMapFilter",
                          this->BathymetryFilter);
  vtkSMPropertyHelper(this->RegisterTextureFilter->getProxy(),
                      "GenerateCoordinates").Set(0);
  this->RegisterTextureFilter->getProxy()->UpdateVTKObjects();

  this->ElevationFilter =
    builder->createFilter("filters",
                          "LIDARElevationFilter",
                          this->RegisterTextureFilter);
  vtkSMPropertyHelper(this->ElevationFilter->getProxy(),
                      "CreateElevation").Set(0);
  this->ElevationFilter->getProxy()->UpdateVTKObjects();
  // force pipeline update
  vtkSMSourceProxy::SafeDownCast(
    this->ElevationFilter->getProxy() )->UpdatePipeline();

  this->setRepresentation(
    builder->createDataRepresentation(
      this->ElevationFilter->getOutputPort(0),
      view, "CmbLargeTextureRepresentation"));//"GeometryRepresentation");//
  if(this->getRepresentation())
    {
    // regardless of object type (although primarily for LIDAR),
    // set initial point size to be 2
    vtkSMPropertyHelper(this->getRepresentation()->getProxy(), "PointSize").Set(2);
    vtkSMPropertyHelper(this->getRepresentation()->getProxy(), "MapScalars").Set(0);
    //vtkSMPropertyHelper(this->getRepresentation()->getProxy(), "StaticMode").Set(1);
    vtkSMPropertyHelper(this->getRepresentation()->getProxy(), "Specular").Set(0.1);

    vtkNew<vtkPVSceneGenObjectInformation> info;
    this->RegisterTextureFilter->getProxy()->GatherInformation(info.GetPointer());

    // if we have "Color" point data, setup to color by it... otherwise clear the
    // "color by" array... telling it to use "Solid Color"


    if (info->GetHasColorPointData())
      {
      RepresentationHelperFunctions::CMB_COLOR_REP_BY_ARRAY(
        this->getRepresentation()->getProxy(), "Color", vtkDataObject::POINT);
//      vtkSMPropertyHelper(this->getRepresentation()->getProxy(), "ColorArrayName").Set("Color");
      }
    else
      {
      RepresentationHelperFunctions::CMB_COLOR_REP_BY_ARRAY(
        this->getRepresentation()->getProxy(), NULL, vtkDataObject::POINT);
//      vtkSMPropertyHelper(this->getRepresentation()->getProxy(), "ColorArrayName").Set("");
      }
    }
}
//-----------------------------------------------------------------------------
void pqCMBTexturedObject::showElevation(bool flag)
{
  if (this->ShowElevation == flag)
    {
    return;
    }

  this->ShowElevation = flag;
  if (this->ShowElevation)
    {
    vtkSMPropertyHelper(this->ElevationFilter->getProxy(),
                        "CreateElevation").Set(1);
    double bounds[6], p[3];
    p[0] = p[1] = 0.0;
    this->getDataBounds(bounds);
    p[2] = bounds[4];
    vtkSMPropertyHelper(this->ElevationFilter->getProxy(),
                        "LowPoint").Set(p, 3);
    p[2] = bounds[5];
    vtkSMPropertyHelper(this->ElevationFilter->getProxy(),
                        "HighPoint").Set(p, 3);
    }
  else
    {
    vtkSMPropertyHelper(this->ElevationFilter->getProxy(),
                        "CreateElevation").Set(0);
    }

  vtkSMPropertyHelper(this->getRepresentation()->getProxy(), "MapScalars").Set(0);
  RepresentationHelperFunctions::CMB_COLOR_REP_BY_ARRAY(
      this->getRepresentation()->getProxy(),
      this->ShowElevation ? "Elevation" : "Color", vtkDataObject::POINT);
  this->ElevationFilter->getProxy()->UpdateVTKObjects();
  // force pipeline update
  vtkSMSourceProxy::SafeDownCast(
                                 this->ElevationFilter->getProxy() )->UpdatePipeline();
}
//-----------------------------------------------------------------------------
void pqCMBTexturedObject::getRegistrationPointPair(int i,
                                                  double xy[2],
                                                  double st[2]) const
{
  int j = 4*i;
  xy[0] = this->RegistrationPoints[j++];
  xy[1] = this->RegistrationPoints[j++];
  st[0] = this->RegistrationPoints[j++];
  st[1] = this->RegistrationPoints[j++];
}
//-----------------------------------------------------------------------------
void pqCMBTexturedObject::unsetTextureMap()
{
  this->NumberOfRegistrationPoints = 0;
  this->TextureFileName = "";
  if (this->TextureImageSource)
    {
    pqApplicationCore* core = pqApplicationCore::instance();
    pqObjectBuilder* builder = core->getObjectBuilder();
    vtkSMPropertyHelper(this->getRepresentation()->getProxy(),
                        "LargeTextureInput").Set(static_cast<vtkSMProxy*>(0));
    this->getRepresentation()->getProxy()->UpdateProperty("LargeTextureInput");
    builder->destroy( this->TextureImageSource );
    this->TextureImageSource = 0;
    }

  vtkSMPropertyHelper(this->RegisterTextureFilter->getProxy(),
                      "GenerateCoordinates").Set(0);
  this->RegisterTextureFilter->getProxy()->UpdateVTKObjects();
  this->getRepresentation()->getProxy()->UpdateVTKObjects();
}
//-----------------------------------------------------------------------------
void pqCMBTexturedObject::setTextureMap(const char *filename, int numberOfRegistrationPoints,
                                       double *points)
{
  if (this->hasTexture() && this->TextureImageSource &&
    this->TextureFileName.compare( filename ))
    {
    this->unsetTextureMap();
    }

  this->NumberOfRegistrationPoints = numberOfRegistrationPoints;
  int i, n = 4*numberOfRegistrationPoints;
  for (i = 0; i < n; i++)
    {
    this->RegistrationPoints[i] = points[i];
    }

  if (!this->TextureImageSource)
    {
    this->TextureImageSource = RepresentationHelperFunctions::ReadTextureImage(
      pqApplicationCore::instance()->getObjectBuilder(),
      this->Source->getServer(), filename);
    this->TextureFileName = filename;
    vtkSMPropertyHelper(this->getRepresentation()->getProxy(), "LargeTextureInput").Set(
      this->TextureImageSource->getProxy() );
    this->getRepresentation()->getProxy()->UpdateProperty("LargeTextureInput");

    vtkAlgorithm* source = vtkAlgorithm::SafeDownCast(
      this->TextureImageSource->getProxy()->GetClientSideObject());
    vtkImageData* image = vtkImageData::SafeDownCast(source->GetOutputDataObject(0));

    int extents[6];
    double ev[2];
    image->GetExtent(extents);
    ev[0] = extents[0];
    ev[1] = extents[1];
    vtkSMPropertyHelper(this->RegisterTextureFilter->getProxy(),
      "SRange").Set(ev, 2);
    ev[0] = extents[2];
    ev[1] = extents[3];
    vtkSMPropertyHelper(this->RegisterTextureFilter->getProxy(),
      "TRange").Set(ev, 2);
    }

  if (numberOfRegistrationPoints == 2)
    {
    vtkSMPropertyHelper(this->RegisterTextureFilter->getProxy(),
                        "TwoPointRegistration").Set(this->RegistrationPoints, 8);
    }
  else
    {
    vtkSMPropertyHelper(this->RegisterTextureFilter->getProxy(),
                        "ThreePointRegistration").Set(this->RegistrationPoints, 12);
    }

  vtkSMPropertyHelper(this->RegisterTextureFilter->getProxy(),
                      "GenerateCoordinates").Set(1);
  this->RegisterTextureFilter->getProxy()->UpdateVTKObjects();
  vtkSMSourceProxy::SafeDownCast(
    this->RegisterTextureFilter->getProxy())->UpdatePipeline();

  this->getRepresentation()->getProxy()->UpdateVTKObjects();

  if (this->TexturePointIntensityFilter)
    {
    pqApplicationCore::instance()->getObjectBuilder()->destroy(
      this->TexturePointIntensityFilter );
    this->TexturePointIntensityFilter = 0;
    }
}

//-----------------------------------------------------------------------------
pqServer* pqCMBTexturedObject::getTextureRegistrationServer(void)
{
  return this->RegisterTextureFilter->getServer();
}

//-----------------------------------------------------------------------------
double pqCMBTexturedObject::getTextureIntensityAtPoint(double pt[3])
{
  if (!this->TexturePointIntensityFilter)
    {
    pqObjectBuilder* builder = pqApplicationCore::instance()->getObjectBuilder();

    QList<pqOutputPort*> registerInput, textureInput;
    registerInput.push_back( this->RegisterTextureFilter->getOutputPort(0) );
    textureInput.push_back( this->TextureImageSource->getOutputPort(0) );
    QMap<QString, QList<pqOutputPort*> > namedInputs;
    namedInputs["Input"] = registerInput;
    namedInputs["TextureData"] = textureInput;

    this->TexturePointIntensityFilter = builder->createFilter("filters",
      "TexturePointIntensityFilter", namedInputs,
      this->RegisterTextureFilter->getServer() );
    }

  vtkSMProxy *rproxy = this->getRepresentation()->getProxy();
  vtkSMProxy *fproxy = this->TexturePointIntensityFilter->getProxy();
  fproxy->GetProperty("Orientation")->
    Copy(rproxy->GetProperty("Orientation"));

  fproxy->GetProperty("Translation")->
    Copy(rproxy->GetProperty("Position"));

  fproxy->GetProperty("Scale")->
    Copy(rproxy->GetProperty("Scale"));


  QList<QVariant> values;
  values << pt[0] << pt[1] << pt[2];
  pqSMAdaptor::setMultipleElementProperty(
    fproxy->GetProperty("TestPoint"), values);

  fproxy->UpdateVTKObjects();
  vtkSMSourceProxy::SafeDownCast(fproxy)->UpdatePipeline();
  fproxy->UpdatePropertyInformation();
  return vtkSMPropertyHelper(fproxy, "Intensity").GetAsDouble();
}
//-----------------------------------------------------------------------------
pqCMBSceneObjectBase* pqCMBTexturedObject::getBathymetrySource()
{
  return this->BathymetrySource;
}

//-----------------------------------------------------------------------------
void pqCMBTexturedObject::unApplyBathymetry()
{
  if (this->BathymetrySource)
    {
    this->BathymetrySource = 0;
    }
  vtkSMProxyProperty* pSource = vtkSMProxyProperty::SafeDownCast(
    this->BathymetryFilter->getProxy()->GetProperty("Source"));
  pSource->RemoveAllProxies();
  vtkSMPropertyHelper(this->BathymetryFilter->getProxy(),
    "NoOP").Set(1);
  this->ElevationRadious = 1.0;
  this->BathymetryFilter->getProxy()->UpdateVTKObjects();
  this->getRepresentation()->getProxy()->UpdateVTKObjects();
}
//-----------------------------------------------------------------------------
void pqCMBTexturedObject::applyBathymetry(
  pqCMBSceneObjectBase* bathymetrySource, double elevationRadious,
  bool useHighLimit, double eleHigh, bool useLowLimit, double eleLow)
{
  if (this->BathymetrySource && this->BathymetrySource != bathymetrySource)
    {
    this->unApplyBathymetry();
    }

  if ((!this->BathymetrySource && bathymetrySource) || this->BathymetrySource)
     /* (this->BathymetrySource && this->ElevationRadious != elevationRadious))*/
    {
    pqWaitCursor cursor;

    this->BathymetrySource = bathymetrySource;
    vtkSMSourceProxy::SafeDownCast(this->BathymetrySource->
    getSource()->getProxy())->UpdatePipeline();
    vtkSMSourceProxy* smFilter = vtkSMSourceProxy::SafeDownCast(
      this->BathymetryFilter->getProxy());

    vtkSMProxyProperty* pSource = vtkSMProxyProperty::SafeDownCast(
      smFilter->GetProperty("Source"));
    pSource->RemoveAllProxies();
    pSource->AddProxy(this->BathymetrySource->getSource()->getProxy());
    vtkSMPropertyHelper(smFilter, "ElevationRadius").Set(elevationRadious);
    this->ElevationRadious = elevationRadious;

    vtkSMPropertyHelper(smFilter, "HighestZValue").Set(eleHigh);
    vtkSMPropertyHelper(smFilter, "UseHighestZValue").Set(useHighLimit);
    vtkSMPropertyHelper(smFilter, "LowestZValue").Set(eleLow);
    vtkSMPropertyHelper(smFilter, "UseLowestZValue").Set(useLowLimit);
    vtkSMPropertyHelper(smFilter, "NoOP").Set(0);
    smFilter->UpdateVTKObjects();
    smFilter->UpdatePipeline();
    this->getRepresentation()->getProxy()->UpdateVTKObjects();
    }
}

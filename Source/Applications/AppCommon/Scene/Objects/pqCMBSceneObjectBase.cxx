//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "pqCMBSceneObjectBase.h"

#include "pqApplicationCore.h"
#include "pqDataRepresentation.h"
#include "pqDataRepresentation.h"
#include "pqObjectBuilder.h"
#include "pqOutputPort.h"
#include "pqPipelineRepresentation.h"
#include "pqPipelineSource.h"
#include "pqRenderView.h"
#include "pqSMAdaptor.h"
#include "pqServer.h"
#include "pqVariableType.h"

#include "vtkBoundingBox.h"
#include "vtkCallbackCommand.h"
#include "vtkGeometryRepresentation.h"
#include "vtkPVDataInformation.h"
#include "vtkPVLASOutputBlockInformation.h"
#include "vtkPVSceneGenObjectInformation.h"
#include "vtkSMNewWidgetRepresentationProxy.h"
#include "vtkSMProxyManager.h"
#include "vtkSMRepresentationProxy.h"
#include <QFileInfo>
#include <QVariant>
#include <vtkProcessModule.h>
#include <vtkSMDataSourceProxy.h>
#include <vtkSMDoubleVectorProperty.h>
#include <vtkSMIdTypeVectorProperty.h>
#include <vtkSMIntVectorProperty.h>
#include <vtkSMPropertyHelper.h>
#include <vtkSMProxyProperty.h>
#include <vtkSMRenderViewProxy.h>
#include <vtkSMRepresentationProxy.h>
#include <vtkSMSourceProxy.h>
#include <vtkTransform.h>

#include "pqRepresentationHelperFunctions.h"
#include "vtkDataObject.h"

#include "pqRepresentationHelperFunctions.h"
using namespace RepresentationHelperFunctions;

//We need a callback to keep track of when the user changes the Representation
//transform matrix, generally done through the display editor panel
vtkCallbackCommand* CallBack;

class pqCMBSceneObjectBase::pqCMBSceneObjectBaseInternal
{
public:
  pqCMBSceneObjectBaseInternal(pqCMBSceneObjectBase* obj)
    : TransformNeedsUpdate(true)
  {
    this->Transform = vtkTransform::New();
    this->CallBack = vtkCallbackCommand::New();
    this->CallBack->SetCallback(pqCMBSceneObjectBase::DoRepresentationCallback);
    this->CallBack->SetClientData(static_cast<void*>(obj));
  }
  ~pqCMBSceneObjectBaseInternal()
  {
    this->Transform->Delete();
    this->CallBack->Delete();
  }
  //the needs to be mutable since we only update it when
  //somebody calls getBounds or getTransform.
  mutable vtkTransform* Transform;
  mutable bool TransformNeedsUpdate;
  vtkCallbackCommand* CallBack;
  unsigned long OrientationTag;
  unsigned long PositionTag;
  unsigned long ScaleTag;
};

pqCMBSceneObjectBase::pqCMBSceneObjectBase()
{
  this->Source = NULL;
  this->Representation = NULL;
  this->setupSceneObject();
}

pqCMBSceneObjectBase::pqCMBSceneObjectBase(pqPipelineSource* source)
{
  this->Source = source;
  this->Representation = NULL;
  this->setupSceneObject();
}

void pqCMBSceneObjectBase::setupSceneObject()
{
  this->clearConstraints();

  this->IsFullyConstrained = false;
  this->IsSnapTarget = false;
  this->Units = cmbSceneUnits::Unknown;
  this->UserDefinedType = "Unknown";

  this->Internal = new pqCMBSceneObjectBaseInternal(this);
}

void pqCMBSceneObjectBase::clearConstraints()
{
  this->Constraints[0] = this->Constraints[1] = this->Constraints[2] = this->Constraints[3] =
    this->Constraints[4] = this->Constraints[5] = this->Constraints[6] = this->Constraints[7] =
      this->Constraints[8] = this->Constraints[9] = false;
}

void pqCMBSceneObjectBase::DoRepresentationCallback(
  vtkObject* /*vtk_obj*/, unsigned long /*event*/, void* client_data, void* /*call_data*/)
{
  pqCMBSceneObjectBase* obj = static_cast<pqCMBSceneObjectBase*>(client_data);
  if (obj)
  {
    obj->Internal->TransformNeedsUpdate = true;
  }
}

pqCMBSceneObjectBase::~pqCMBSceneObjectBase()
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();

  if (this->Representation)
  {
    builder->destroy(this->Representation);
    this->Representation = NULL;
  }

  if (this->ClosestPointFilter)
  {
    builder->destroy(this->ClosestPointFilter);
    this->ClosestPointFilter = NULL;
  }

  if (this->PolyDataStatsFilter)
  {
    builder->destroy(this->PolyDataStatsFilter);
    this->PolyDataStatsFilter = NULL;
  }

  if (this->Source)
  {
    builder->destroy(this->Source);
    this->Source = NULL;
  }

  if (this->Internal)
  {
    delete this->Internal;
  }
}

std::string pqCMBSceneObjectBase::getTypeAsString() const
{
  return this->convertTypeToString(this->getType());
}

void pqCMBSceneObjectBase::setVisibility(bool mode)
{
  if (this->Representation)
  {
    this->Representation->setVisible(mode);
  }
}

int pqCMBSceneObjectBase::getPickable()
{
  if (this->Representation)
  {
    return pqSMAdaptor::getElementProperty(
             this->Representation->getProxy()->GetProperty("Pickable"))
      .toInt();
  }
  return 0;
}

void pqCMBSceneObjectBase::setPickable(bool mode)
{
  if (this->Representation)
  {
    pqSMAdaptor::setElementProperty(
      this->Representation->getProxy()->GetProperty("Pickable"), mode);
  }
}

void pqCMBSceneObjectBase::setRepresentation(pqDataRepresentation* rep)
{
  if (this->Representation)
  {
    vtkSMProxy* oldProxy = this->Representation->getProxy();
    oldProxy->GetProperty("Orientation")->RemoveObservers(this->Internal->OrientationTag);
    oldProxy->GetProperty("Position")->RemoveObservers(this->Internal->PositionTag);
    oldProxy->GetProperty("Scale")->RemoveObservers(this->Internal->ScaleTag);
  }
  this->Representation = rep;

  if (this->Representation)
  {
    vtkSMProxy* srcProxy = this->Representation->getProxy();
    if (srcProxy)
    {
      this->Internal->OrientationTag =
        srcProxy->GetProperty("Orientation")
          ->AddObserver(vtkCommand::ModifiedEvent, this->Internal->CallBack);
      this->Internal->PositionTag =
        srcProxy->GetProperty("Position")
          ->AddObserver(vtkCommand::ModifiedEvent, this->Internal->CallBack);
      this->Internal->ScaleTag = srcProxy->GetProperty("Scale")->AddObserver(
        vtkCommand::ModifiedEvent, this->Internal->CallBack);
    }
  }
}

void pqCMBSceneObjectBase::setSource(pqPipelineSource* source)
{
  this->Source = source;
}

pqPipelineSource* pqCMBSceneObjectBase::getSource() const
{
  return this->Source;
}

pqPipelineSource* pqCMBSceneObjectBase::getSelectionSource() const
{
  return this->Source;
}

vtkSMSourceProxy* pqCMBSceneObjectBase::getSelectionInput() const
{
  if (!this->Source)
  {
    return NULL;
  }
  vtkSMSourceProxy* proxy;
  proxy = vtkSMSourceProxy::SafeDownCast(this->Source->getProxy());
  return proxy->GetSelectionInput(0);
}

void pqCMBSceneObjectBase::setSelectionInput(vtkSMSourceProxy* selectionInput)
{
  if (!this->Source)
  {
    return;
  }
  vtkSMSourceProxy* proxy;
  proxy = vtkSMSourceProxy::SafeDownCast(this->Source->getProxy());
  proxy->SetSelectionInput(0, selectionInput, 0);
}

pqDataRepresentation* pqCMBSceneObjectBase::getRepresentation() const
{
  return this->Representation;
}

void pqCMBSceneObjectBase::duplicateInternals(pqCMBSceneObjectBase* nobj)
{
  nobj->UserDefinedType = this->UserDefinedType;

  vtkSMProxy* srcProxy = this->Representation->getProxy();
  vtkSMProxy* destProxy = nobj->Representation->getProxy();

  vtkSMPropertyHelper(destProxy, "MapScalars").Set(0);
  //vtkSMPropertyHelper(destProxy, "StaticMode").Set(1);

  destProxy->GetProperty("Orientation")->Copy(srcProxy->GetProperty("Orientation"));

  destProxy->GetProperty("Position")->Copy(srcProxy->GetProperty("Position"));

  destProxy->GetProperty("Scale")->Copy(srcProxy->GetProperty("Scale"));

  destProxy->GetProperty("DiffuseColor")->Copy(srcProxy->GetProperty("DiffuseColor"));
}

void pqCMBSceneObjectBase::getCameraFocalPoint(pqRenderView* view, double pos[3])
{
  QList<QVariant> values =
    pqSMAdaptor::getMultipleElementProperty(view->getProxy()->GetProperty("CameraFocalPointInfo"));
  pos[0] = values[0].toDouble();
  pos[1] = values[1].toDouble();
  pos[2] = values[2].toDouble();
}

void pqCMBSceneObjectBase::setUnits(cmbSceneUnits::Enum utype)
{
  this->Units = utype;
}

void pqCMBSceneObjectBase::setConstraint(int i, bool mode)
{
  this->Constraints[i] = mode;
}

std::string pqCMBSceneObjectBase::convertTypeToString(pqCMBSceneObjectBase::enumObjectType t)
{
  //TODO: This should be replaced with getting the string from the object it self
  switch (t)
  {
    case pqCMBSceneObjectBase::Points:
      return "Points";
    case pqCMBSceneObjectBase::VOI:
      return "VOI";
    case pqCMBSceneObjectBase::Line:
      return "Line";
    case pqCMBSceneObjectBase::Arc:
      return "Arc";
    case pqCMBSceneObjectBase::Polygon:
      return "Polygon";
    case pqCMBSceneObjectBase::GroundPlane:
      return "GroundPlane";
    case pqCMBSceneObjectBase::Faceted:
      return "Faceted";
    case pqCMBSceneObjectBase::Glyph:
      return "Glyph";
    case pqCMBSceneObjectBase::UniformGrid:
      return "UniformGrid";
    case pqCMBSceneObjectBase::GeneralCone:
      return "GeneralCone";
    case pqCMBSceneObjectBase::SolidMesh:
      return "SolidMesh";
    case pqCMBSceneObjectBase::GeoBoreHole:
      return "GeoBoreHole";
    case pqCMBSceneObjectBase::GeoCrossSection:
      return "GeoCrossSection";
    default:
      return "Unknown";
  }
  return "Unknown";
}

pqCMBSceneObjectBase::enumObjectType pqCMBSceneObjectBase::convertStringToType(const char* typeName)
{
  std::string name = typeName;

  if (name == "Points")
  {
    return pqCMBSceneObjectBase::Points;
  }

  if (name == "VOI")
  {
    return pqCMBSceneObjectBase::VOI;
  }

  if (name == "Line")
  {
    return pqCMBSceneObjectBase::Line;
  }

  if (name == "Contour")
  {
    //needed for legacy support of V1 where arcs where named contours
    return pqCMBSceneObjectBase::Arc;
  }

  if (name == "Arc")
  {
    return pqCMBSceneObjectBase::Arc;
  }

  if (name == "GroundPlane")
  {
    return pqCMBSceneObjectBase::GroundPlane;
  }

  if (name == "Polygon")
  {
    return pqCMBSceneObjectBase::Polygon;
  }

  if (name == "Faceted")
  {
    return pqCMBSceneObjectBase::Faceted;
  }

  if (name == "Glyph")
  {
    return pqCMBSceneObjectBase::Glyph;
  }

  if (name == "UniformGrid")
  {
    return pqCMBSceneObjectBase::UniformGrid;
  }

  if (name == "GeneralCone")
  {
    return pqCMBSceneObjectBase::GeneralCone;
  }
  if (name == "SolidMesh")
  {
    return pqCMBSceneObjectBase::SolidMesh;
  }
  if (name == "GeoBoreHole")
  {
    return pqCMBSceneObjectBase::GeoBoreHole;
  }
  if (name == "GeoCrossSection")
  {
    return pqCMBSceneObjectBase::GeoCrossSection;
  }

  return pqCMBSceneObjectBase::Unknown;
}

std::string pqCMBSceneObjectBase::convertSurfaceTypeToString(
  pqCMBSceneObjectBase::enumSurfaceType t)
{
  switch (t)
  {
    case pqCMBSceneObjectBase::Solid:
      return "Solid";
    case pqCMBSceneObjectBase::TIN:
      return "TIN";
    case pqCMBSceneObjectBase::Other:
      return "Other";
    default:
      return "Other";
  }
  return "Other";
}

pqCMBSceneObjectBase::enumSurfaceType pqCMBSceneObjectBase::convertStringToSurfaceType(
  const char* typeName)
{
  std::string name = typeName;

  if (name == "Solid")
  {
    return pqCMBSceneObjectBase::Solid;
  }

  if (name == "TIN")
  {
    return pqCMBSceneObjectBase::TIN;
  }

  if (name == "Other")
  {
    return pqCMBSceneObjectBase::Other;
  }

  return pqCMBSceneObjectBase::Other;
}

void pqCMBSceneObjectBase::getColor(double color[4]) const
{
  vtkSMPropertyHelper(this->Representation->getProxy(), "DiffuseColor").Get(color, 3);
  color[3] = vtkSMPropertyHelper(this->Representation->getProxy(), "Opacity").GetAsDouble();
}

void pqCMBSceneObjectBase::setColor(double color[4], bool updateRep)
{
  if (!this->Representation)
  {
    return;
  }
  vtkSMPropertyHelper(this->Representation->getProxy(), "DiffuseColor").Set(color, 3);
  vtkSMPropertyHelper(this->Representation->getProxy(), "AmbientColor").Set(color, 3);
  vtkSMPropertyHelper(this->Representation->getProxy(), "Opacity").Set(color[3]);

  if (updateRep)
  {
    this->Representation->getProxy()->UpdateVTKObjects();
  }
}

void pqCMBSceneObjectBase::getBounds(vtkBoundingBox* inBB) const
{
  GetRepresentationTransformedBounds(this->Internal->Transform, this->Representation, inBB);
}

void pqCMBSceneObjectBase::getBounds(double bounds[6]) const
{
  vtkBoundingBox bb;
  this->getBounds(&bb);
  bb.GetBounds(bounds);
}

void pqCMBSceneObjectBase::getDataBounds(double bounds[6]) const
{
  vtkSMSourceProxy::SafeDownCast(this->Source->getProxy())->GetDataInformation()->GetBounds(bounds);
}

void pqCMBSceneObjectBase::updateVectorProperty(
  const char* name, double* v, const int& size, const bool& updateRep)
{
  vtkSMPropertyHelper(this->Representation->getProxy(), name).Set(v, size);
  if (updateRep)
  {
    this->Representation->getProxy()->UpdateVTKObjects();
  }
  this->Internal->TransformNeedsUpdate = true;
}

void pqCMBSceneObjectBase::getPosition(double pos[3]) const
{
  vtkSMPropertyHelper(this->Representation->getProxy(), "Position").Get(pos, 3);
}

void pqCMBSceneObjectBase::setPosition(double pos[3], bool updateRep)
{
  this->updateVectorProperty("Position", pos, 3, updateRep);
}

void pqCMBSceneObjectBase::getOrientation(double ori[3]) const
{
  vtkSMPropertyHelper(this->Representation->getProxy(), "Orientation").Get(ori, 3);
}

void pqCMBSceneObjectBase::setOrientation(double ori[3], bool updateRep)
{
  this->updateVectorProperty("Orientation", ori, 3, updateRep);
}

void pqCMBSceneObjectBase::getScale(double scale[3]) const
{
  vtkSMPropertyHelper(this->Representation->getProxy(), "Scale").Get(scale, 3);
}

void pqCMBSceneObjectBase::setScale(double scale[3], bool updateRep)
{
  this->updateVectorProperty("Scale", scale, 3, updateRep);
}

void pqCMBSceneObjectBase::getOrigin(double origin[3]) const
{
  vtkSMPropertyHelper(this->Representation->getProxy(), "Origin").Get(origin, 3);
}

void pqCMBSceneObjectBase::setOrigin(double origin[3], bool updateRep)
{
  this->updateVectorProperty("Origin", origin, 3, updateRep);
}

void pqCMBSceneObjectBase::updateRepresentation()
{
  if (!this->Representation)
  {
    return;
  }
  this->Representation->getProxy()->UpdateVTKObjects();
}

void pqCMBSceneObjectBase::updateTransform() const
{
  if (!this->Internal->TransformNeedsUpdate)
  {
    return;
  }

  double origin[3], position[3], orientation[3], scale[3];
  this->getOrigin(origin);
  this->getPosition(position);
  this->getOrientation(orientation);
  this->getScale(scale);

  // build the transformation
  vtkTransform* t = this->Internal->Transform;
  t->Identity();
  t->PreMultiply();
  t->Translate(position[0] + origin[0], position[1] + origin[1], position[2] + origin[2]);
  t->RotateZ(orientation[2]);
  t->RotateX(orientation[0]);
  t->RotateY(orientation[1]);
  t->Scale(scale);
  t->Translate(-origin[0], -origin[1], -origin[2]);

  this->Internal->TransformNeedsUpdate = false;
}

void pqCMBSceneObjectBase::getTransform(vtkTransform* transform) const
{
  if (!transform)
  {
    return;
  }
  this->updateTransform();
  transform->DeepCopy(this->Internal->Transform);
}

void pqCMBSceneObjectBase::setTransform(vtkTransform* transform, bool updateRep)
{
  if (!transform)
  {
    return;
  }

  double orientation[3], translation[3], scale[3], origin[3] = { 0, 0, 0 };
  transform->GetOrientation(orientation);
  transform->GetPosition(translation);
  transform->GetScale(scale);

  this->setPosition(translation, updateRep);
  this->setOrientation(orientation, updateRep);
  this->setScale(scale, updateRep);
  this->setOrigin(origin, updateRep);
  if (updateRep && this->Representation)
  {
    this->Representation->getProxy()->UpdateVTKObjects();
  }

  //copy the transform into the member variable
  this->Internal->Transform->DeepCopy(transform);
  this->Internal->TransformNeedsUpdate = false;
}

void pqCMBSceneObjectBase::getAreaStats(double* areaStats)
{
  this->updatePolyDataStats();
  vtkSMProxy* fproxy = this->PolyDataStatsFilter->getProxy();
  for (int i = 0; i < 3; i++)
  {
    areaStats[i] =
      pqSMAdaptor::getMultipleElementProperty(fproxy->GetProperty("AreaStats"), i).toDouble();
  }
}

void pqCMBSceneObjectBase::getGeometryBounds(double* geoBounds) const
{
  (const_cast<pqCMBSceneObjectBase*>(this))->updatePolyDataStats();
  vtkSMProxy* fproxy = this->PolyDataStatsFilter->getProxy();
  for (int i = 0; i < 6; i++)
  {
    geoBounds[i] =
      pqSMAdaptor::getMultipleElementProperty(fproxy->GetProperty("GeometryBounds"), i).toDouble();
  }
}

void pqCMBSceneObjectBase::getPolySideStats(double* polySide)
{
  this->updatePolyDataStats();
  vtkSMProxy* fproxy = this->PolyDataStatsFilter->getProxy();
  for (int i = 0; i < 3; i++)
  {
    polySide[i] =
      pqSMAdaptor::getMultipleElementProperty(fproxy->GetProperty("PolygonalSideStats"), i)
        .toDouble();
  }
}

double pqCMBSceneObjectBase::getSurfaceArea()
{
  this->updatePolyDataStats();
  vtkSMProxy* fproxy = this->PolyDataStatsFilter->getProxy();
  return pqSMAdaptor::getElementProperty(fproxy->GetProperty("TotalSurfaceArea")).toDouble();
  ;
}

vtkIdType pqCMBSceneObjectBase::getNumberOfPoints()
{
  this->updatePolyDataStats();
  vtkSMProxy* fproxy = this->PolyDataStatsFilter->getProxy();
  return vtkSMIdTypeVectorProperty::SafeDownCast(fproxy->GetProperty("NumberOfPoints"))
    ->GetElement(0);
}

vtkIdType pqCMBSceneObjectBase::getNumberOfPolygons()
{
  this->updatePolyDataStats();
  vtkSMProxy* fproxy = this->PolyDataStatsFilter->getProxy();
  return vtkSMIdTypeVectorProperty::SafeDownCast(fproxy->GetProperty("NumberOfPolygons"))
    ->GetElement(0);
}

void pqCMBSceneObjectBase::updatePolyDataStats()
{
  // Do we already have a poly data stats filter?
  if (!this->PolyDataStatsFilter)
  {
    pqApplicationCore* core = pqApplicationCore::instance();
    pqObjectBuilder* builder = core->getObjectBuilder();
    this->PolyDataStatsFilter =
      builder->createFilter("filters", "PolyDataStatsFilter", this->Source);
  }

  vtkSMProxy* rproxy = this->Representation->getProxy();
  vtkSMProxy* fproxy = this->PolyDataStatsFilter->getProxy();
  fproxy->GetProperty("Orientation")->Copy(rproxy->GetProperty("Orientation"));

  fproxy->GetProperty("Translation")->Copy(rproxy->GetProperty("Position"));

  fproxy->GetProperty("Scale")->Copy(rproxy->GetProperty("Scale"));

  fproxy->UpdateVTKObjects();
  vtkSMSourceProxy::SafeDownCast(fproxy)->UpdatePipeline();
  fproxy->UpdatePropertyInformation();
}

void pqCMBSceneObjectBase::getClosestPoint(const double p[3], double cp[3])
{
  // Do we already have a closest point filter?
  if (!this->ClosestPointFilter)
  {
    pqApplicationCore* core = pqApplicationCore::instance();
    pqObjectBuilder* builder = core->getObjectBuilder();
    this->ClosestPointFilter = builder->createFilter("filters", "ClosestPointFilter", this->Source);
  }

  vtkSMProxy* rproxy = this->Representation->getProxy();
  vtkSMProxy* fproxy = this->ClosestPointFilter->getProxy();
  fproxy->GetProperty("Orientation")->Copy(rproxy->GetProperty("Orientation"));

  fproxy->GetProperty("Translation")->Copy(rproxy->GetProperty("Position"));

  fproxy->GetProperty("Scale")->Copy(rproxy->GetProperty("Scale"));

  QList<QVariant> values;
  values << p[0] << p[1] << p[2];
  pqSMAdaptor::setMultipleElementProperty(fproxy->GetProperty("TestPoint"), values);

  fproxy->UpdateVTKObjects();
  vtkSMSourceProxy::SafeDownCast(fproxy)->UpdatePipeline();
  fproxy->UpdatePropertyInformation();
  double* data =
    vtkSMDoubleVectorProperty::SafeDownCast(fproxy->GetProperty("ClosestPoint"))->GetElements();
  cp[0] = data[0];
  cp[1] = data[1];
  cp[2] = data[2];
}

void pqCMBSceneObjectBase::setLODMode(int mode, bool updateRep)
{
  vtkSMIntVectorProperty::SafeDownCast(this->Representation->getProxy()->GetProperty("SuppressLOD"))
    ->SetElements1(mode);
  if (updateRep)
  {
    this->Representation->getProxy()->UpdateVTKObjects();
  }
}

void pqCMBSceneObjectBase::zoomOnObject()
{
  double bounds[6];
  this->getBounds(bounds);
  pqRenderView* renModule = qobject_cast<pqRenderView*>(this->Representation->getView());
  if (renModule)
  {
    vtkSMRenderViewProxy* rm = renModule->getRenderViewProxy();
    rm->ResetCamera(bounds);
    renModule->render();
  }
}

void pqCMBSceneObjectBase::applyTransform(
  double scaleDelta[3], double orientationDelta[3], double translationDelta[3])
{
  double val[3];

  // Update the Scale
  this->getScale(val);
  val[0] *= scaleDelta[0];
  val[1] *= scaleDelta[1];
  val[2] *= scaleDelta[2];
  this->setScale(val);

  // Update the Orientation
  this->getOrientation(val);
  val[0] += orientationDelta[0];
  val[1] += orientationDelta[1];
  val[2] += orientationDelta[2];
  this->setOrientation(val);

  // Update the Position
  this->getPosition(val);
  val[0] += translationDelta[0];
  val[1] += translationDelta[1];
  val[2] += translationDelta[2];
  this->setPosition(val);
  this->Representation->getProxy()->UpdateVTKObjects();
}

void pqCMBSceneObjectBase::setMarkedForDeletion()
{
  this->setSelectionInput(NULL);
}

void pqCMBSceneObjectBase::unsetMarkedForDeletion()
{
}

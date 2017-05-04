//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "pqCMBLIDARPieceObject.h"

#include "pqApplicationCore.h"
#include "pqObjectBuilder.h"
#include "pqPipelineRepresentation.h"
#include "pqPipelineSource.h"
#include "pqRenderView.h"
#include "pqSMAdaptor.h"
#include "vtkGeometryRepresentation.h"
#include "vtkPVDataInformation.h"
#include "vtkSMPVRepresentationProxy.h"
#include "vtkSMProxyProperty.h"
#include "vtkSMRepresentationProxy.h"
#include "vtkTransform.h"
#include <vtkProcessModule.h>
#include <vtkSMDataSourceProxy.h>
#include <vtkSMIntVectorProperty.h>
#include <vtkSMPropertyHelper.h>
#include <vtkSMProxyManager.h>
#include <vtkSMRenderViewProxy.h>
#include <vtkSMRepresentationProxy.h>
#include <vtkSMSourceProxy.h>

#include "pqRepresentationHelperFunctions.h"
#include "vtkDataObject.h"

#include <QTableWidgetItem>

//-----------------------------------------------------------------------------
pqCMBLIDARPieceObject::pqCMBLIDARPieceObject()
{
  this->init();
}

//-----------------------------------------------------------------------------
void pqCMBLIDARPieceObject::init()
{
  this->Source = NULL;
  this->DiggerSource = NULL;
  this->Representation = NULL;
  this->UseElevationFilter = false;
  this->FileName = "";
  this->PieceName = "";
  this->PieceIndex = 0;
  this->DisplayOnRatio = this->SaveOnRatio = 1;
  this->ReadOnRatio = -1; // haven't read yet!
  this->TotalNumberOfPiecePoints = 0;
  this->NumberOfReadPoints = 0;
  this->NumberOfDisplayPointsEstimate = 0;
  this->NumberOfSavePointsEstimate = 0;
  //  this->FileOffset = 0;
  this->Visibility = 1;
  this->Highlight = 0;
  this->ClipState = false;
  //this->Widget = NULL;
  this->HighlightColor[0] = 1.0;
  this->HighlightColor[1] = 0.0;
  this->HighlightColor[2] = 1.0;
  this->OriginalColor[0] = this->OriginalColor[1] = this->OriginalColor[2] = 1.0;

  this->ThresholdPosition[0] = 0.0;
  this->ThresholdPosition[1] = 0.0;
  this->ThresholdPosition[2] = 0.0;
  this->ThresholdOrientation[0] = 0.0;
  this->ThresholdOrientation[1] = 0.0;
  this->ThresholdOrientation[2] = 0.0;
  this->ThresholdScale[0] = 1.0;
  this->ThresholdScale[1] = 1.0;
  this->ThresholdScale[2] = 1.0;
  this->ThresholdOrigin[0] = 0.0;
  this->ThresholdOrigin[1] = 0.0;
  this->ThresholdOrigin[2] = 0.0;
}

//-----------------------------------------------------------------------------
pqCMBLIDARPieceObject::~pqCMBLIDARPieceObject()
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();

  if (this->Representation)
  {
    builder->destroy(this->Representation);
    this->Representation = NULL;
  }

  if (this->ElevationSource)
  {
    builder->destroy(this->ElevationSource);
    this->ElevationSource = NULL;
  }
  if (this->ThresholdSource)
  {
    builder->destroy(this->ThresholdSource);
    this->ThresholdSource = NULL;
  }
  if (this->DiggerSource)
  {
    builder->destroy(this->DiggerSource);
    this->DiggerSource = NULL;
  }
  if (this->ContourSource)
  {
    builder->destroy(this->ContourSource);
    this->ContourSource = NULL;
  }
  if (this->Source)
  {
    builder->destroy(this->Source);
    this->Source = NULL;
  }
}

//-----------------------------------------------------------------------------
void pqCMBLIDARPieceObject::setDisplayOnRatio(int onRatio)
{
  if (onRatio > 0 && onRatio != this->DisplayOnRatio)
  {
    this->DisplayOnRatio = onRatio;
  }
}

//-----------------------------------------------------------------------------
void pqCMBLIDARPieceObject::setSaveOnRatio(int onRatio)
{
  if (onRatio > 0 && onRatio != this->SaveOnRatio)
  {
    this->SaveOnRatio = onRatio;
  }
}

//-----------------------------------------------------------------------------
void pqCMBLIDARPieceObject::setReadOnRatio(int onRatio)
{
  if (onRatio > 0 && onRatio != this->ReadOnRatio)
  {
    this->ReadOnRatio = onRatio;
  }
}

//-----------------------------------------------------------------------------
void pqCMBLIDARPieceObject::setRepresentation(pqDataRepresentation* rep)
{
  this->Representation = rep;
}

//-----------------------------------------------------------------------------
void pqCMBLIDARPieceObject::setSource(pqPipelineSource* source)
{
  this->Source = source;
}

//-----------------------------------------------------------------------------
pqPipelineSource* pqCMBLIDARPieceObject::getSource() const
{
  return this->Source;
}
//-----------------------------------------------------------------------------
void pqCMBLIDARPieceObject::setThresholdSource(pqPipelineSource* thresholdSource)
{
  this->ThresholdSource = thresholdSource;
}

//-----------------------------------------------------------------------------
pqPipelineSource* pqCMBLIDARPieceObject::getThresholdSource() const
{
  return this->ThresholdSource;
}
//-----------------------------------------------------------------------------
void pqCMBLIDARPieceObject::setContourSource(pqPipelineSource* contourSource)
{
  this->ContourSource = contourSource;
}

//-----------------------------------------------------------------------------
pqPipelineSource* pqCMBLIDARPieceObject::getContourSource() const
{
  return this->ContourSource;
}

//-----------------------------------------------------------------------------
pqPipelineSource* pqCMBLIDARPieceObject::getDiggerSource() const
{
  return this->DiggerSource;
}

//-----------------------------------------------------------------------------
pqPipelineSource* pqCMBLIDARPieceObject::getElevationSource() const
{
  return this->ElevationSource;
}

//-----------------------------------------------------------------------------
pqDataRepresentation* pqCMBLIDARPieceObject::getRepresentation() const
{
  return this->Representation;
}

//-----------------------------------------------------------------------------
void pqCMBLIDARPieceObject::setPieceIndex(int pieceIdx)
{
  this->PieceIndex = pieceIdx;
}

//-----------------------------------------------------------------------------
void pqCMBLIDARPieceObject::setElevationFilterLowPoint(double* lowPoint)
{
  pqSMAdaptor::setMultipleElementProperty(
    this->ElevationSource->getProxy()->GetProperty("LowPoint"), 0, lowPoint[0]);
  pqSMAdaptor::setMultipleElementProperty(
    this->ElevationSource->getProxy()->GetProperty("LowPoint"), 1, lowPoint[1]);
  pqSMAdaptor::setMultipleElementProperty(
    this->ElevationSource->getProxy()->GetProperty("LowPoint"), 2, lowPoint[2]);
  this->ElevationSource->getProxy()->UpdateProperty("LowPoint", true);
}

//-----------------------------------------------------------------------------
void pqCMBLIDARPieceObject::setElevationFilterHighPoint(double* highPoint)
{
  pqSMAdaptor::setMultipleElementProperty(
    this->ElevationSource->getProxy()->GetProperty("HighPoint"), 0, highPoint[0]);
  pqSMAdaptor::setMultipleElementProperty(
    this->ElevationSource->getProxy()->GetProperty("HighPoint"), 1, highPoint[1]);
  pqSMAdaptor::setMultipleElementProperty(
    this->ElevationSource->getProxy()->GetProperty("HighPoint"), 2, highPoint[2]);
  this->ElevationSource->getProxy()->UpdateProperty("HighPoint", true);
}

//-----------------------------------------------------------------------------
void pqCMBLIDARPieceObject::useElevationFilter(bool useElevation)
{
  if (useElevation == this->UseElevationFilter)
  {
    return;
  }

  this->UseElevationFilter = useElevation;
  int mapScalars = useElevation ? 1 : 0;
  if (!this->Highlight)
  {
    vtkSMPropertyHelper(this->Representation->getProxy(), "MapScalars").Set(mapScalars);
    RepresentationHelperFunctions::CMB_COLOR_REP_BY_ARRAY(this->Representation->getProxy(),
      useElevation ? "Elevation" : "Color", vtkDataObject::POINT, false);
  }
}

//-----------------------------------------------------------------------------
void pqCMBLIDARPieceObject::updateRepresentation()
{
  this->getContourSource()->updatePipeline();
  this->getThresholdSource()->updatePipeline();
  if (this->UseElevationFilter)
  {
    this->ElevationSource->updatePipeline();
    if (!this->Highlight)
    {
      RepresentationHelperFunctions::CMB_COLOR_REP_BY_ARRAY(
        this->Representation->getProxy(), "Elevation", vtkDataObject::POINT, false);
    }
  }
  this->Representation->getProxy()->UpdateVTKObjects();
}

//-----------------------------------------------------------------------------
pqCMBLIDARPieceObject* pqCMBLIDARPieceObject::createObject(pqPipelineSource* source, double* bounds,
  pqServer* /*server*/, pqRenderView* view, bool updateRep)
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();
  pqCMBLIDARPieceObject* obj = new pqCMBLIDARPieceObject();
  obj->Source = source;

  obj->ContourSource = builder->createFilter("filters", "ClipPolygons", source);
  obj->DiggerSource = builder->createFilter("filters", "ArcDepressFilter", obj->ContourSource);
  obj->ThresholdSource =
    builder->createFilter("filters", "PointThresholdFilter", obj->DiggerSource);
  obj->ElevationSource =
    builder->createFilter("filters", "LIDARElevationFilter", obj->ThresholdSource);
  // not initially viewing elevation output, but at least have filter set such
  // that sensible values are computed (the filter autmatically executes when
  // doing createDataRepresentation call)
  double minPt[3] = { 0, 0, bounds[4] }, maxPt[3] = { 0, 0, bounds[5] };
  obj->setElevationFilterLowPoint(minPt);
  obj->setElevationFilterHighPoint(maxPt);

  // force read
  vtkSMSourceProxy::SafeDownCast(obj->Source->getProxy())->UpdatePipeline();

  obj->Representation = builder->createDataRepresentation(
    obj->ElevationSource->getOutputPort(0), view, "GeometryRepresentation");
  vtkSMPVRepresentationProxy::SafeDownCast(obj->Representation->getProxy())
    ->SetRepresentationType("Point Gaussian");

  vtkSMPropertyHelper(obj->Representation->getProxy(), "GaussianRadius").Set(0);

  if (updateRep)
  {
    obj->Representation->getProxy()->UpdateVTKObjects();
  }

  vtkSMPropertyHelper(obj->Representation->getProxy(), "AmbientColor").Get(obj->OriginalColor, 3);

  // setup Origin for rotation
  double center[3];
  // only want up to 4 digits after the "."
  // (Note: really doing int(10000 * center) / 10000.0, where
  // center = (bounds[0] + bounds[1]) / 2.0,  but (10000 / 2.0) = 5000)
  center[0] = floor(5000 * (bounds[0] + bounds[1])) / 10000.0;
  center[1] = floor(5000 * (bounds[2] + bounds[3])) / 10000.0;
  center[2] = floor(5000 * (bounds[4] + bounds[5])) / 10000.0;
  //obj->setOrigin(center);
  vtkSMPropertyHelper(obj->Representation->getProxy(), "MapScalars").Set(0);
  RepresentationHelperFunctions::CMB_COLOR_REP_BY_ARRAY(
    obj->Representation->getProxy(), "Color", vtkDataObject::POINT, false);

  return obj;
}

//-----------------------------------------------------------------------------
void pqCMBLIDARPieceObject::setLODMode(int mode, bool updateRep)
{
  vtkSMIntVectorProperty::SafeDownCast(this->Representation->getProxy()->GetProperty("SuppressLOD"))
    ->SetElements1(mode);
  if (updateRep)
  {
    this->Representation->getProxy()->UpdateVTKObjects();
  }
}
//-----------------------------------------------------------------------------
void pqCMBLIDARPieceObject::getBounds(double bounds[6]) const
{
  vtkSMRepresentationProxy::SafeDownCast(this->Representation->getProxy())
    ->GetRepresentedDataInformation()
    ->GetBounds(bounds);
}

//-----------------------------------------------------------------------------
void pqCMBLIDARPieceObject::zoomOnObject()
{
  double bounds[6];
  this->getBounds(bounds);
  if (bounds[0] > bounds[1])
  {
    return; // if the bounds are invalid, don't change the camera
  }
  pqRenderView* renModule = qobject_cast<pqRenderView*>(this->Representation->getView());
  if (renModule)
  {
    vtkSMRenderViewProxy* rm = renModule->getRenderViewProxy();
    rm->ResetCamera(bounds);
    renModule->render();
  }
}

//----------------------------------------------------------------------------
void pqCMBLIDARPieceObject::setVisibility(int visible)
{
  if (this->Visibility != visible)
  {
    this->Visibility = visible;
    pqDataRepresentation* rep = this->Representation;

    rep->setVisible(visible);
  }
}

//----------------------------------------------------------------------------
void pqCMBLIDARPieceObject::setSelectionInput(vtkSMSourceProxy* selInput)
{
  vtkSMSourceProxy::SafeDownCast(this->getElevationSource() ? this->getElevationSource()->getProxy()
                                                            : this->getSource()->getProxy())
    ->SetSelectionInput(0, selInput, 0);
}

//----------------------------------------------------------------------------
void pqCMBLIDARPieceObject::setHighlight(int highlight)
{
  if (this->Highlight != highlight)
  {
    this->Highlight = highlight;
    pqDataRepresentation* rep = this->Representation;

    if (highlight)
    {
      RepresentationHelperFunctions::CMB_COLOR_REP_BY_ARRAY(
        rep->getProxy(), NULL, vtkDataObject::POINT, false);
      vtkSMPropertyHelper(rep->getProxy(), "DiffuseColor").Set(this->HighlightColor, 3);
      vtkSMPropertyHelper(rep->getProxy(), "AmbientColor").Set(this->HighlightColor, 3);
      rep->getProxy()->UpdateVTKObjects();
    }
    else
    {
      vtkSMPropertyHelper(rep->getProxy(), "DiffuseColor").Set(this->OriginalColor, 3);
      vtkSMPropertyHelper(rep->getProxy(), "AmbientColor").Set(this->OriginalColor, 3);
      int mapScalars = this->UseElevationFilter ? 1 : 0;
      vtkSMPropertyHelper(this->Representation->getProxy(), "MapScalars").Set(mapScalars);
      RepresentationHelperFunctions::CMB_COLOR_REP_BY_ARRAY(this->Representation->getProxy(),
        this->UseElevationFilter ? "Elevation" : "Color", vtkDataObject::POINT, false);
    }
  }
}

//----------------------------------------------------------------------------
bool pqCMBLIDARPieceObject::isObjectUpToDate(bool clipEnabled, vtkBoundingBox& clipBBox)
{
  // is our clipping state up-to-date
  if (!this->isClipUpToDate(clipEnabled, clipBBox))
  {
    return false;
  }

  // is the display onRatio up-to-date
  if (this->DisplayOnRatio != this->ReadOnRatio)
  {
    return false;
  }

  return true;
}

//----------------------------------------------------------------------------
bool pqCMBLIDARPieceObject::isClipUpToDate(bool clipEnabled, vtkBoundingBox& clipBBox)
{
  // is our clipping state up-to-date
  if (clipEnabled != this->ClipState || (clipEnabled && (!this->areClippingBoundsEqual(clipBBox) ||
                                                          !this->isClipTransformationUnchanged())))
  {
    return false;
  }
  return true;
}

//-----------------------------------------------------------------------------
bool pqCMBLIDARPieceObject::isClipTransformationUnchanged()
{
  double position[3], scale[3], orientation[3], origin[3];
  this->getPosition(position);
  this->getOrientation(orientation);
  this->getOrigin(origin);
  this->getScale(scale);

  if (position[0] != this->ClipPosition[0] || position[1] != this->ClipPosition[1] ||
    position[2] != this->ClipPosition[2] || scale[0] != this->ClipScale[0] ||
    scale[1] != this->ClipScale[1] || scale[2] != this->ClipScale[2] ||
    orientation[0] != this->ClipOrientation[0] || orientation[1] != this->ClipOrientation[1] ||
    orientation[2] != this->ClipOrientation[2] ||
    ((origin[0] != this->ClipOrigin[0] || origin[1] != this->ClipOrigin[1] ||
       origin[2] != this->ClipOrigin[2]) &&
        (orientation[0] != 0 || orientation[1] != 0 || orientation[2] != 0)))
  {
    return false;
  }
  return true;
}

//-----------------------------------------------------------------------------
void pqCMBLIDARPieceObject::saveClipPosition()
{
  vtkSMPropertyHelper(this->Representation->getProxy(), "Position").Get(this->ClipPosition, 3);
}

//-----------------------------------------------------------------------------
void pqCMBLIDARPieceObject::saveClipOrientation()
{
  vtkSMPropertyHelper(this->Representation->getProxy(), "Orientation")
    .Get(this->ClipOrientation, 3);
}

//-----------------------------------------------------------------------------
void pqCMBLIDARPieceObject::saveClipScale()
{
  vtkSMPropertyHelper(this->Representation->getProxy(), "Scale").Get(this->ClipScale, 3);
}

//-----------------------------------------------------------------------------
void pqCMBLIDARPieceObject::saveClipOrigin()
{
  vtkSMPropertyHelper(this->Representation->getProxy(), "Origin").Get(this->ClipOrigin, 3);
}

//-----------------------------------------------------------------------------
bool pqCMBLIDARPieceObject::isThresholdTransformationUnchanged()
{
  double position[3], scale[3], orientation[3], origin[3];
  this->getPosition(position);
  this->getOrientation(orientation);
  this->getOrigin(origin);
  this->getScale(scale);

  if (position[0] != this->ThresholdPosition[0] || position[1] != this->ThresholdPosition[1] ||
    position[2] != this->ThresholdPosition[2] || scale[0] != this->ThresholdScale[0] ||
    scale[1] != this->ThresholdScale[1] || scale[2] != this->ThresholdScale[2] ||
    orientation[0] != this->ThresholdOrientation[0] ||
    orientation[1] != this->ThresholdOrientation[1] ||
    orientation[2] != this->ThresholdOrientation[2] ||
    ((origin[0] != this->ThresholdOrigin[0] || origin[1] != this->ThresholdOrigin[1] ||
       origin[2] != this->ThresholdOrigin[2]) &&
        (orientation[0] != 0 || orientation[1] != 0 || orientation[2] != 0)))
  {
    return false;
  }
  return true;
}

//-----------------------------------------------------------------------------
void pqCMBLIDARPieceObject::saveThresholdPosition()
{
  vtkSMPropertyHelper(this->Representation->getProxy(), "Position").Get(this->ThresholdPosition, 3);
}

//-----------------------------------------------------------------------------
void pqCMBLIDARPieceObject::saveThresholdOrientation()
{
  vtkSMPropertyHelper(this->Representation->getProxy(), "Orientation")
    .Get(this->ThresholdOrientation, 3);
}

//-----------------------------------------------------------------------------
void pqCMBLIDARPieceObject::saveThresholdScale()
{
  vtkSMPropertyHelper(this->Representation->getProxy(), "Scale").Get(this->ThresholdScale, 3);
}

//-----------------------------------------------------------------------------
void pqCMBLIDARPieceObject::saveThresholdOrigin()
{
  vtkSMPropertyHelper(this->Representation->getProxy(), "Origin").Get(this->ThresholdOrigin, 3);
}

//-----------------------------------------------------------------------------
void pqCMBLIDARPieceObject::getPosition(double pos[3]) const
{
  vtkSMPropertyHelper(this->Representation->getProxy(), "Position").Get(pos, 3);
}

//-----------------------------------------------------------------------------
void pqCMBLIDARPieceObject::setPosition(double pos[3], bool updateRep /*=true*/)
{
  vtkSMPropertyHelper(this->Representation->getProxy(), "Position").Set(pos, 3);
  if (updateRep)
  {
    this->Representation->getProxy()->UpdateVTKObjects();
  }
}

//-----------------------------------------------------------------------------
void pqCMBLIDARPieceObject::getOrientation(double ori[3]) const
{
  vtkSMPropertyHelper(this->Representation->getProxy(), "Orientation").Get(ori, 3);
}

//-----------------------------------------------------------------------------
void pqCMBLIDARPieceObject::setOrientation(double ori[3], bool updateRep /*=true*/)
{
  vtkSMPropertyHelper(this->Representation->getProxy(), "Orientation").Set(ori, 3);
  if (updateRep)
  {
    this->Representation->getProxy()->UpdateVTKObjects();
  }
}

//-----------------------------------------------------------------------------
void pqCMBLIDARPieceObject::getScale(double scale[3]) const
{
  vtkSMPropertyHelper(this->Representation->getProxy(), "Scale").Get(scale, 3);
}

//-----------------------------------------------------------------------------
void pqCMBLIDARPieceObject::setScale(double scale[3], bool updateRep /*=true*/)
{
  vtkSMPropertyHelper(this->Representation->getProxy(), "Scale").Set(scale, 3);
  if (updateRep)
  {
    this->Representation->getProxy()->UpdateVTKObjects();
  }
}

//-----------------------------------------------------------------------------
void pqCMBLIDARPieceObject::getOrigin(double origin[3]) const
{
  vtkSMPropertyHelper(this->Representation->getProxy(), "Origin").Get(origin, 3);
}

//-----------------------------------------------------------------------------
void pqCMBLIDARPieceObject::setOrigin(double origin[3], bool updateRep /*=true*/)
{
  vtkSMPropertyHelper(this->Representation->getProxy(), "Origin").Set(origin, 3);
  if (updateRep)
  {
    this->Representation->getProxy()->UpdateVTKObjects();
  }
}

//-----------------------------------------------------------------------------
bool pqCMBLIDARPieceObject::isPieceTransformed()
{
  double position[3], scale[3], orientation[3];
  this->getPosition(position);
  this->getOrientation(orientation);
  this->getScale(scale);

  if (position[0] != 0 || position[1] != 0 || position[2] != 0 || orientation[0] != 0 ||
    orientation[1] != 0 || orientation[2] != 0 || scale[0] != 1 || scale[1] != 1 || scale[2] != 1)
  {
    return true;
  }
  return false;
}

//-----------------------------------------------------------------------------
void pqCMBLIDARPieceObject::getTransform(vtkTransform* transform) const
{
  if (!transform)
  {
    return;
  }

  double position[3], scale[3], orientation[3], origin[3];
  this->getPosition(position);
  this->getOrientation(orientation);
  this->getOrigin(origin);
  this->getScale(scale);

  // build the transformation
  transform->Identity();
  transform->PreMultiply();
  transform->Translate(position[0] + origin[0], position[1] + origin[1], position[2] + origin[2]);
  transform->RotateZ(orientation[2]);
  transform->RotateX(orientation[0]);
  transform->RotateY(orientation[1]);
  transform->Scale(scale);
  transform->Translate(-origin[0], -origin[1], -origin[2]);
}

//-----------------------------------------------------------------------------
void pqCMBLIDARPieceObject::updateThresholdUseFilter(int idx, int useFilter)
{
  this->updateProxyProperty(this->getThresholdSource(), "SetUseFilter", idx, useFilter);
}

//-----------------------------------------------------------------------------
void pqCMBLIDARPieceObject::updatePolygonUseFilter(int idx, int useFilter)
{
  this->updateProxyProperty(this->getContourSource(), "ClipApplyPolygon", idx, useFilter);
}

//-----------------------------------------------------------------------------
void pqCMBLIDARPieceObject::updatePolygonInvert(int idx, int invert)
{
  this->updateProxyProperty(this->getContourSource(), "InsideOut", idx, invert);
}

//-----------------------------------------------------------------------------
void pqCMBLIDARPieceObject::updateGroupInvert(int groupIdx, int invert)
{
  this->setActiveContourGroup(groupIdx);
  //this->updateProxyProperty(
  //  this->getContourSource(), "GroupInvert", !invert);

  pqSMAdaptor::setElementProperty(
    this->getContourSource()->getProxy()->GetProperty("GroupInvert"), !invert);
  this->getContourSource()->getProxy()->UpdateProperty("GroupInvert", true);
}

//-----------------------------------------------------------------------------
void pqCMBLIDARPieceObject::updatePolygonROI(int idx, int roi)
{
  this->updateProxyProperty(this->getContourSource(), "ClipAsROI", idx, roi);
}

//-----------------------------------------------------------------------------
void pqCMBLIDARPieceObject::updateProxyProperty(
  pqPipelineSource* source, const char* name, int idx, int val)
{
  vtkSMSourceProxy* smSource = vtkSMSourceProxy::SafeDownCast(source->getProxy());
  // only need to do filter if it is "active" ("UseFilter" is true)
  pqSMAdaptor::setMultipleElementProperty(smSource->GetProperty(name), 0, idx);
  pqSMAdaptor::setMultipleElementProperty(smSource->GetProperty(name), 1, val);
  smSource->UpdateProperty(name, true);
  smSource->UpdatePipeline();
}

//-----------------------------------------------------------------------------
void pqCMBLIDARPieceObject::resetWithNoThresholds(bool update)
{
  vtkSMSourceProxy* currentThresholds =
    vtkSMSourceProxy::SafeDownCast(this->getThresholdSource()->getProxy());
  currentThresholds->UpdatePropertyInformation();
  int numFilters =
    pqSMAdaptor::getElementProperty(currentThresholds->GetProperty("NumberOfThresholdSets"))
      .toInt();
  if (numFilters == 0)
  {
    return;
  }
  QList<QVariant> values;
  for (int i = 0; i < numFilters; i++)
  {
    values << 0;
  }
  pqSMAdaptor::setMultipleElementProperty(
    currentThresholds->GetProperty("UseFilterForAll"), values);
  currentThresholds->UpdateVTKObjects();
  if (update)
  {
    this->updateRepresentation();
  }
}
//-----------------------------------------------------------------------------
void pqCMBLIDARPieceObject::resetWithNoContours(bool update)
{
  pqPipelineSource* clipPoly = this->getContourSource();
  vtkSMProxyProperty* clipFunc =
    vtkSMProxyProperty::SafeDownCast(clipPoly->getProxy()->GetProperty("ClipPolygon"));
  int numFilters = clipFunc->GetNumberOfProxies();
  if (numFilters == 0)
  {
    return;
  }
  QList<QVariant> values;
  for (int i = 0; i < numFilters; i++)
  {
    values << 0;
  }
  pqSMAdaptor::setMultipleElementProperty(
    clipPoly->getProxy()->GetProperty("ClipApplyPolygonForAll"), values);
  clipPoly->getProxy()->UpdateVTKObjects();
  if (update)
  {
    this->updateRepresentation();
  }
}

//-----------------------------------------------------------------------------
void pqCMBLIDARPieceObject::addThreshold()
{
  vtkSMSourceProxy* thresholdSourceProxy =
    vtkSMSourceProxy::SafeDownCast(this->getThresholdSource()->getProxy());
  thresholdSourceProxy->Modified();
  thresholdSourceProxy->InvokeCommand("AddFilter");
}
//-----------------------------------------------------------------------------
void pqCMBLIDARPieceObject::removeThreshold()
{
  vtkSMSourceProxy* thresholdSourceProxy =
    vtkSMSourceProxy::SafeDownCast(this->getThresholdSource()->getProxy());

  thresholdSourceProxy->Modified();
  thresholdSourceProxy->InvokeCommand("RemoveFilter");
  //After removing the filter select the 0th element
  pqSMAdaptor::setElementProperty(thresholdSourceProxy->GetProperty("SetActiveFilterIndex"), 0);
  thresholdSourceProxy->UpdateProperty("SetActiveFilterIndex", true);
  thresholdSourceProxy->UpdateVTKObjects();
  thresholdSourceProxy->UpdatePipeline();
}
//-----------------------------------------------------------------------------
void pqCMBLIDARPieceObject::clearThresholds()
{
  vtkSMSourceProxy* thresholdSourceProxy =
    vtkSMSourceProxy::SafeDownCast(this->getThresholdSource()->getProxy());

  thresholdSourceProxy->Modified();
  thresholdSourceProxy->InvokeCommand("RemoveAllFilters");
  thresholdSourceProxy->UpdateVTKObjects();
  thresholdSourceProxy->UpdatePipeline();
}
//-----------------------------------------------------------------------------
void pqCMBLIDARPieceObject::addContour(vtkSMProxy* implicitLoop)
{
  pqPipelineSource* clipPoly = this->getContourSource();
  vtkSMProxyProperty* clipFunc =
    vtkSMProxyProperty::SafeDownCast(clipPoly->getProxy()->GetProperty("ClipPolygon"));
  clipFunc->AddProxy(implicitLoop);
  clipPoly->getProxy()->UpdateVTKObjects();
  if (this->getVisibility())
  {
    clipPoly->updatePipeline();
  }
}
//-----------------------------------------------------------------------------
void pqCMBLIDARPieceObject::removeContour(vtkSMProxy* implicitLoop)
{
  pqPipelineSource* clipPoly = this->getContourSource();
  vtkSMProxyProperty* clipFunc =
    vtkSMProxyProperty::SafeDownCast(clipPoly->getProxy()->GetProperty("ClipPolygon"));
  clipFunc->RemoveProxy(implicitLoop);
  clipPoly->getProxy()->UpdateVTKObjects();
  if (this->getVisibility())
  {
    clipPoly->updatePipeline();
  }
}
//-----------------------------------------------------------------------------
void pqCMBLIDARPieceObject::clearContours()
{

  vtkSMProxy* clipPoly = this->getContourSource()->getProxy();
  //vtkSMProxyProperty* clipFunc = vtkSMProxyProperty::SafeDownCast(
  //  clipPoly->getProxy()->GetProperty("ClipPolygon"));
  //clipFunc->RemoveAllProxies();
  //clipPoly->getProxy()->UpdateVTKObjects();
  clipPoly->InvokeCommand("RemoveAllClipPolygons");
  clipPoly->UpdateVTKObjects();
  if (this->getVisibility())
  {
    this->getContourSource()->updatePipeline();
  }
}
//-----------------------------------------------------------------------------
bool pqCMBLIDARPieceObject::hasActiveFilters()
{
  this->getThresholdSource()->getProxy()->UpdatePropertyInformation();
  this->getContourSource()->getProxy()->UpdatePropertyInformation();
  if (pqSMAdaptor::getElementProperty(
        this->getThresholdSource()->getProxy()->GetProperty("NumberOfActiveThresholdSets"))
        .toInt() ||
    pqSMAdaptor::getElementProperty(
      this->getContourSource()->getProxy()->GetProperty("NumberOfActivePolygons"))
      .toInt())
  {
    return true;
  }
  return false;
}
//-----------------------------------------------------------------------------
void pqCMBLIDARPieceObject::setActiveContourGroup(int groupId)
{
  vtkSMProxy* contourProxy = this->getContourSource()->getProxy();
  pqSMAdaptor::setElementProperty(contourProxy->GetProperty("ActiveGroupIndex"), groupId);
  contourProxy->UpdateProperty("ActiveGroupIndex", true);
}

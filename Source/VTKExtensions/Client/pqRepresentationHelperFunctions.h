/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkCMBReaderHelperFunctions.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME pqRepresentationHelperFunctions -
// .SECTION Description - Helper Functions common to pqDataRepresentation
//

#ifndef __pqRepresentationHelperFunctions_h
#define __pqRepresentationHelperFunctions_h

#include "pqDataRepresentation.h"
#include "pqObjectBuilder.h"
#include "cmbSystemConfig.h"

#include "vtkSMPropertyHelper.h"
#include "vtkSMPVRepresentationProxy.h"
#include "vtkSMSourceProxy.h"
#include "vtkBoundingBox.h"
#include "vtkTransform.h"
#include "vtkNew.h"
#include "vtkPVArrayInformation.h"
#include "vtkPVDataInformation.h"
#include "vtkSMStringVectorProperty.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMTransferFunctionManager.h"
#include "vtkSMTransferFunctionProxy.h"
#include "vtkSMSessionProxyManager.h"
#include "vtkStringList.h"
#include "vtkTuple.h"

//BTX
namespace RepresentationHelperFunctions
{
const int TwoDManipulatorTypes[9] = {3, 1, 2, 1, 2, 2, 2, 3, 1};
const int ThreeDManipulatorTypes[9] = {4, 1, 2, 3, 4, 1, 2, 3, 2};

/*
static pqRenderViewBase::ManipulatorType TwoDManipulatorTypes[9] =
  {
    { 1, 0, 0, "TrackballRoll", QByteArray()},
    { 2, 0, 0, "TrackballPan1", QByteArray()},
    { 3, 0, 0, "TrackballZoom", QByteArray()},
    { 1, 1, 0, "TrackballPan1", QByteArray()},
    { 2, 1, 0, "TrackballZoom", QByteArray()},
    { 3, 1, 0, "TrackballDolly", QByteArray()},
    { 1, 0, 1, "TrackballZoom", QByteArray()},
    { 2, 0, 1, "TrackballRoll", QByteArray()},
    { 3, 0, 1, "TrackballPan1", QByteArray()},
  };

static pqRenderViewBase::ManipulatorType ThreeDManipulatorTypes[9] =
  {
    { 1, 0, 0, "TrackballRotate", QByteArray()},
    { 2, 0, 0, "TrackballPan1", QByteArray()},
    { 3, 0, 0, "TrackballZoom", QByteArray()},
    { 1, 1, 0, "TrackballRoll", QByteArray()},
    { 2, 1, 0, "TrackballRotate", QByteArray()},
    { 3, 1, 0, "TrackballPan1", QByteArray()},
    { 1, 0, 1, "TrackballZoom", QByteArray()},
    { 2, 0, 1, "TrackballRotate", QByteArray()},
    { 3, 0, 1, "TrackballZoom", QByteArray()},
  };
*/
/* ------ Reference from paraview --------
 // Note the 2D is differenct
    <SettingsProxy name="RenderViewInteractionSettings" label="Camera"
      processes="client|dataserver|renderserver">
      <IntVectorProperty name="Camera3DManipulators"
        number_of_elements="9"
        default_values= l m r
                      " 4 1 2
                shift   3 4 1
                ctrl    2 4 2 "
        panel_widget="camera_manipulator">
        <!-- For now, we are marking this property is_internal so that it won't
        get saved in state files. -->
        <Documentation>
          Select how interactions are mapped to camera movements when in 3D interaction mode.
        </Documentation>
        <EnumerationDomain name="enum">
          <Entry text="None" value="0" />
          <Entry text="Pan" value="1" />
          <Entry text="Zoom" value="2" />
          <Entry text="Roll" value="3" />
          <Entry text="Rotate" value="4" />
          <Entry text="Multi-Rotate" value="5" />
        </EnumerationDomain>
      </IntVectorProperty>
      <IntVectorProperty name="Camera2DManipulators"
        number_of_elements="9"
        default_values="1 3 2 2 2 2 3 1 4"
        panel_widget="camera_manipulator">
        <!-- For now, we are marking this property is_internal so that it won't
        get saved in state files. -->
        <Documentation>
          Select how interactions are mapped to camera movements when in 2D interaction mode.
        </Documentation>
        <EnumerationDomain name="enum">
          <Entry text="None" value="0" />
          <Entry text="Pan" value="1" />
          <Entry text="Zoom" value="2" />
          <Entry text="Roll" value="3" />
          <Entry text="Rotate" value="4" />
        </EnumerationDomain>
      </IntVectorProperty>
      <PropertyGroup label="2D Interaction Options">
        <Property name="Camera2DManipulators" />
      </PropertyGroup>
      <PropertyGroup label="3D Interaction Options">
        <Property name="Camera3DManipulators" />
      </PropertyGroup>
      <Hints>
        <UseDocumentationForLabels />
      </Hints>
    </SettingsProxy>
*/

static void GetRepresentationTransform(vtkSMProxy* contourRepProxy, vtkTransform *t)
{
  double position[3], scale[3], orientation[3], origin[3];
  vtkSMPropertyHelper(contourRepProxy, "Position").Get(position, 3);
  vtkSMPropertyHelper(contourRepProxy, "Orientation").Get(orientation, 3);
  vtkSMPropertyHelper(contourRepProxy, "Scale").Get(scale, 3);
  vtkSMPropertyHelper(contourRepProxy, "Origin").Get(origin, 3);
  // build the transformation
  t->Identity();
  t->PreMultiply();
  t->Translate( position[0] + origin[0], position[1] + origin[1],
               position[2] + origin[2] );
  t->RotateZ( orientation[2] );
  t->RotateX( orientation[0] );
  t->RotateY( orientation[1] );
  t->Scale( scale );
  t->Translate( -origin[0], -origin[1], -origin[2] );
}

//-----------------------------------------------------------------------------
static void GetRepresentationTransformedBounds(vtkTransform *t,
  pqDataRepresentation* rep, vtkBoundingBox *inBB)
{
  double transformedBounds[6];
  GetRepresentationTransform(rep->getProxy(), t);
  rep->getDataBounds(transformedBounds);
  vtkBoundingBox bb;
  //take the transform matrix
  double p[3], tp[3];
  for (int i=0; i < 2; ++i)
    {
    p[0] = transformedBounds[i];
    for (int j=0; j < 2; ++j)
      {
      p[1] = transformedBounds[2 + j];
      for (int k=0; k < 2; ++k)
        {
        p[2] = transformedBounds[4 + k];
        t->TransformPoint(p,tp);
        bb.AddPoint(tp);
        }
      }
    }
  *inBB = bb;
}

//-----------------------------------------------------------------------------
static void GetRepresentationTransformedBounds(
  pqDataRepresentation* rep, double bounds[6])
{
  vtkBoundingBox bb;
  vtkNew<vtkTransform> t;
  GetRepresentationTransformedBounds(t.GetPointer(), rep, &bb);
  bb.GetBounds(bounds);
}

//-----------------------------------------------------------------------------
static pqPipelineSource* ReadTextureImage(
  pqObjectBuilder* builder, pqServer *server, const char *filename,
  const char* xmlgroup="sources", const char* xmlname="CMBNetworkImageSource")
{
  QStringList fileList;
  fileList << filename;
  builder->blockSignals(true);
  pqPipelineSource* textureImageSource = builder->createReader(
    xmlgroup, xmlname, fileList, server);
  builder->blockSignals(false);
  vtkSMSourceProxy::SafeDownCast(textureImageSource->getProxy())->UpdatePipeline();
  return textureImageSource;
}

static bool CMB_COLOR_REP_BY_ARRAY(
    vtkSMProxy* reproxy, const char* arrayname, int attribute_type,
    bool rescale = true )
{
  bool res = vtkSMPVRepresentationProxy::SetScalarColoring(
    reproxy, arrayname, attribute_type);
  if(rescale && res && vtkSMPVRepresentationProxy::GetUsingScalarColoring(reproxy))
    {
    vtkSMPropertyHelper inputHelper(reproxy->GetProperty("Input"));
    vtkSMSourceProxy* inputProxy =
      vtkSMSourceProxy::SafeDownCast(inputHelper.GetAsProxy());
    int port = inputHelper.GetOutputPort();
    if (inputProxy)
      {
      vtkPVDataInformation* dataInfo = inputProxy->GetDataInformation(port);
      vtkPVArrayInformation* info = dataInfo->GetArrayInformation(
        arrayname, attribute_type);
      vtkSMPVRepresentationProxy* pvRepProxy =
        vtkSMPVRepresentationProxy::SafeDownCast(reproxy);
      if (!info && pvRepProxy)
        {
        vtkPVDataInformation* representedDataInfo =
          pvRepProxy->GetRepresentedDataInformation();
        info = representedDataInfo->GetArrayInformation(arrayname, attribute_type);
        }
      // make sure we have the requested array before calling rescale TF
      if(info)
        {
        res = vtkSMPVRepresentationProxy::RescaleTransferFunctionToDataRange(
          reproxy, arrayname, attribute_type);
        }
      }
    }
  return res;
}

static bool CMB_COLOR_REP_BY_INDEXED_LUT(
    vtkSMProxy* reproxy, const char* arrayname, vtkSMProxy* lutProxy, int attribute_type
    /*, vtkSMProxy* view */ )
{

  vtkSMProperty* colorArray = reproxy->GetProperty("ColorArrayName");
  if (!colorArray)
    {
    return false;
    }

  vtkSMPropertyHelper colorArrayHelper(colorArray);
  colorArrayHelper.SetInputArrayToProcess(attribute_type, arrayname);

  if (arrayname == NULL || arrayname[0] == '\0' || !lutProxy)
    {
    vtkSMPropertyHelper(reproxy, "LookupTable", true).RemoveAllValues();
    vtkSMPropertyHelper(reproxy, "ScalarOpacityFunction", true).RemoveAllValues();
    reproxy->UpdateVTKObjects();
    return true;
    }

  // Now, setup transfer functions.
  if (vtkSMProperty* lutProperty = reproxy->GetProperty("LookupTable"))
    {
//    vtkSMPropertyHelper(lutProperty).RemoveAllValues();
//    reproxy->UpdateVTKObjects();
    vtkSMPropertyHelper(lutProperty).Set(lutProxy);
    reproxy->UpdateVTKObjects();

/*
    vtkSMPropertyHelper lutPropertyHelper(lutProperty);
      if (lutPropertyHelper.GetNumberOfElements() > 0 &&
        lutPropertyHelper.GetAsProxy(0) != NULL)
      {
      if (vtkSMProxy* sbProxy = vtkSMTransferFunctionProxy::FindScalarBarRepresentation(
          lutPropertyHelper.GetAsProxy(), view))
        {
        vtkSMPropertyHelper(sbProxy, "LookupTable", true).RemoveAllValues();
        sbProxy->UpdateVTKObjects();
        vtkSMPropertyHelper(sbProxy, "LookupTable", true).Set(lutProxy);
        sbProxy->UpdateVTKObjects();
        }

      }
*/
    }
  return true;
}


static void MODELBUILDER_SETUP_CATEGORICAL_CTF(
    vtkSMProxy* reproxy, vtkSMProxy* lutProxy,
    const std::vector<vtkTuple<double, 3> > &rgbColors,
    vtkStringList* new_annotations)
//    const std::vector<vtkTuple<const char*, 2> >& new_annotations)
{
  // Now, setup transfer functions.
  // vtkNew<vtkSMTransferFunctionManager> mgr;
  if (lutProxy && reproxy->GetProperty("LookupTable"))
    {
//    vtkSMProxy* lutProxy =
//      mgr->GetColorTransferFunction(arrayname, reproxy->GetSessionProxyManager());
    vtkSMPropertyHelper(lutProxy, "IndexedLookup", true).Set(1);

    if (new_annotations->GetLength() > 0)
      {
      vtkSMStringVectorProperty* svp = vtkSMStringVectorProperty::SafeDownCast(
        lutProxy->GetProperty("Annotations"));
      if (svp)
        {
        svp->SetElements(new_annotations);
//        svp->SetElements(new_annotations[0].GetData(),
//          static_cast<unsigned int>(new_annotations.size()*2));
        }

      }
    if (rgbColors.size() > 0)
      {
      vtkSMPropertyHelper indexedColors(lutProxy->GetProperty("IndexedColors"));
      indexedColors.Set(rgbColors[0].GetData(),
        static_cast<unsigned int>(rgbColors.size() * 3));
      }

    lutProxy->UpdateVTKObjects();
    }

}


static void MODELBUILDER_SYNCUP_DISPLAY_LUT(
    const char* arrayname, vtkSMProxy* lutProxy)
{
    // we also want to update the IndexedColor of LUT registered with transferfunction manager.
    vtkSMPropertyHelper indexedColors(lutProxy->GetProperty("IndexedColors"));
    std::vector<double> rgbColors = indexedColors.GetDoubleArray();
    vtkNew<vtkSMTransferFunctionManager> mgr;
    vtkSMProxy* displayLUTProxy =
      mgr->GetColorTransferFunction(arrayname, lutProxy->GetSessionProxyManager());
    if(displayLUTProxy)
      {
      vtkSMPropertyHelper(displayLUTProxy, "IndexedColors").Set(&rgbColors[0],
        static_cast<unsigned int>(rgbColors.size()));
      displayLUTProxy->UpdateVTKObjects();
      }
}

}
//ETX
#endif

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqRepresentationHelperFunctions -
// .SECTION Description - Helper Functions common to pqDataRepresentation
//

#ifndef __pqRepresentationHelperFunctions_h
#define __pqRepresentationHelperFunctions_h

#include "cmbSystemConfig.h"
#include "pqDataRepresentation.h"
#include "pqObjectBuilder.h"
#include "pqPipelineSource.h"

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

#include <algorithm>

//BTX
namespace RepresentationHelperFunctions
{

inline void GetRepresentationTransform(vtkSMProxy* contourRepProxy, vtkTransform *t)
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
inline void GetRepresentationTransformedBounds(vtkTransform *t,
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
inline void GetRepresentationTransformedBounds(
  pqDataRepresentation* rep, double bounds[6])
{
  vtkBoundingBox bb;
  vtkNew<vtkTransform> t;
  GetRepresentationTransformedBounds(t.GetPointer(), rep, &bb);
  bb.GetBounds(bounds);
}

//-----------------------------------------------------------------------------
inline pqPipelineSource* ReadTextureImage(
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

inline bool CMB_COLOR_REP_BY_ARRAY(
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

inline bool CMB_COLOR_REP_BY_INDEXED_LUT(
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
    // NOTE: We have to turn this off for openGL2, because there is a bug in paraview/vtk
    // (Oct 14, 2015) that if this is on, color-by-field-data string array will not work.
    // Once the bug is fixed in paraview/vtk, we can turn this back on again.
    vtkSMPropertyHelper(reproxy, "InterpolateScalarsBeforeMapping", true).Set(0);
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

inline const char *helper_internal_convert(const std::string & s)
{
   return s.c_str();
}

inline void MODELBUILDER_SETUP_CATEGORICAL_CTF(
    vtkSMProxy* reproxy, vtkSMProxy* lutProxy,
    std::vector<vtkTuple<double, 3> > &rgbColors,
    const std::vector<std::string> & in_annotations)
//    const std::vector<vtkTuple<const char*, 2> >& new_annotations)
{
  // Now, setup transfer functions.
  // vtkNew<vtkSMTransferFunctionManager> mgr;
  if (lutProxy && reproxy->GetProperty("LookupTable"))
    {
//    vtkSMProxy* lutProxy =
//      mgr->GetColorTransferFunction(arrayname, reproxy->GetSessionProxyManager());
    vtkSMPropertyHelper(lutProxy, "IndexedLookup", true).Set(1);

    if (in_annotations.size() > 0)
      {

//      std::vector<vtkTuple<const char*, 2> > new_annotations;
      std::vector<const char*> new_annotations;
      std::transform(in_annotations.begin(), in_annotations.end(),
        std::back_inserter(new_annotations), helper_internal_convert); 

      vtkSMStringVectorProperty* svp = vtkSMStringVectorProperty::SafeDownCast(
        lutProxy->GetProperty("Annotations"));
      if (svp)
        {
//        svp->SetElements(new_annotations);
        svp->SetElements((const char**)(&new_annotations[0]),
          static_cast<unsigned int>(new_annotations.size()));
        }
      }

    if (rgbColors.size() > 0)
      {
      // Note: this is a work-around for a bug in paraview that color-by-partial
      // field data array of a multiblockdataset will leak color with blocks that
      // do not have the field data array. Therefore, we are adding "no group"
      // and "no attribute" for the color lookup table, and we want to set those
      // color to white be default. Later on, we may get these "no ..." colors from
      // application settings.
      std::vector<std::string>::const_iterator git =
        std::find(in_annotations.begin(), in_annotations.end(), "no group");
      std::vector<std::string>::const_iterator ait =
        std::find(in_annotations.begin(), in_annotations.end(), "no attribute");
      int idx = (git != in_annotations.end()) ? (git - in_annotations.begin()) :
        ((ait != in_annotations.end()) ? (ait - in_annotations.begin()) : -1);
      int numColors = (int)rgbColors.size();
      if(idx >= 0 && (idx%2) == 0 && (idx/2) < numColors)
        {
        idx = idx/2;
        rgbColors[idx][0] = rgbColors[idx][1] = rgbColors[idx][2] = 1.0;
        }

      vtkSMPropertyHelper indexedColors(lutProxy->GetProperty("IndexedColors"));
      indexedColors.Set(rgbColors[0].GetData(),
        static_cast<unsigned int>(rgbColors.size() * 3));
      }
    lutProxy->UpdateVTKObjects();
    }

}

inline void MODELBUILDER_SYNCUP_DISPLAY_LUT(
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

/*=========================================================================

  Program:   ParaView
  Module:    vtkSMAnimationSceneKMLWriter.cxx

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkSMAnimationSceneKMLWriter.h"

#include "vtkDateTime.h"
#include "vtkErrorCode.h"
#include "vtkImageData.h"

#include "vtkObjectFactory.h"
#include "vtkRenderWindow.h"
#include "vtkSMAnimationScene.h"
#include "vtkSMAnimationSceneProxy.h"
//#include "vtkSMMultiProcessRenderView.h"
#include "vtkSmartPointer.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMRenderViewProxy.h"
#include "vtkTIFFWriter.h"
#include "vtkToolkits.h"

#include <algorithm>
#include <string>
#include <vtksys/SystemTools.hxx>
#include "vtkSMPropertyHelper.h"

#include <sstream>

vtkStandardNewMacro(vtkSMAnimationSceneKMLWriter);

//-----------------------------------------------------------------------------
vtkSMAnimationSceneKMLWriter::vtkSMAnimationSceneKMLWriter()
{
  this->Magnification = 1;
  this->ErrorCode = 0;

  this->ActualSize[0] = this->ActualSize[1] = 0;

  this->FileCount = 0;

  this->Prefix = 0;
  this->Suffix = 0;

  this->BackgroundColor[0] = this->BackgroundColor[1] =
    this->BackgroundColor[2] = 0.0;

  this->KMLExporter = vtkKMLExporter::New();
}

//-----------------------------------------------------------------------------
vtkSMAnimationSceneKMLWriter::~vtkSMAnimationSceneKMLWriter()
{
  this->SetPrefix(0);
  this->SetSuffix(0);

  this->KMLExporter->Delete();
}

//-----------------------------------------------------------------------------
bool vtkSMAnimationSceneKMLWriter::SaveInitialize(int fcount)
{
  // Create writers.
  if (!this->CreateWriter())
    {
    return false;
    }

  this->UpdateImageSize();

//  this->AnimationScene->SetOverrideStillRender(1);

  this->FileCount = fcount;

#if !defined(__APPLE__)
  // Iterate over all views and enable offscreen rendering. This avoid toggling
  // of the offscreen rendering flag on every frame.
  unsigned int num_modules = this->AnimationScene->GetNumberOfViewProxies();
  for (unsigned int cc=0; cc < num_modules; cc++)
    {
    vtkSMRenderViewProxy* rmview = vtkSMRenderViewProxy::SafeDownCast(
      this->AnimationScene->GetViewProxy(cc));
    if (rmview)
      {
      if (vtkSMPropertyHelper(rmview,
        "UseOffscreenRenderingForScreenshots").GetAsInt() == 1)
        {
        vtkSMPropertyHelper(rmview, "UseOffscreenRendering").Set(1);
          rmview->UpdateProperty("UseOffscreenRendering");
        }
      }
    }
#endif

  return true;
}

//-----------------------------------------------------------------------------
void vtkSMAnimationSceneKMLWriter::CaptureViewImage(
  vtkSMViewProxy* view, int magnification)
{
  vtkSMRenderViewProxy* rmview = vtkSMRenderViewProxy::SafeDownCast(view);
  if (rmview)
    {
    int old_threshold = -1;
    if (rmview->GetProperty("RemoteRenderThreshold"))
      {
      vtkSMPropertyHelper helper(rmview, "RemoteRenderThreshold");
      old_threshold = helper.GetAsInt();
      helper.Set(VTK_INT_MAX);
      rmview->StillRender();
    }

    std::ostringstream ostr;
    ostr << this->FileCount;
    std::string filename = this->Prefix;
    filename = filename + ostr.str() + this->Suffix;

    vtkRenderWindow* renWin = rmview->GetRenderWindow();
    this->KMLExporter->SetRenderWindow(renWin);
    this->KMLExporter->SetFileName(filename.c_str());
    this->KMLExporter->SetWriteTimeSeriesData(true);
    this->KMLExporter->SetMagnification(magnification);
    this->KMLExporter->Write();

    if (rmview->GetProperty("RemoteRenderThreshold"))
      {
      vtkSMPropertyHelper(rmview, "RemoteRenderThreshold").Set(old_threshold);
      }
    }
}

//-----------------------------------------------------------------------------
bool vtkSMAnimationSceneKMLWriter::SaveFrame(double time)
{
  vtkDateTime* dt(0);
  switch(this->GetKMLExporter()->GetDeltaDateTimeUnit())
    {
    case 0:
      {
      dt = this->KMLExporter->GetStartDateTime()->CreateNewAddYears(
        static_cast<int>(time));
      this->KMLExporter->AddStartDateTime(dt);
      break;
      }
    case 1:
      {
      vtkErrorMacro("Cannot create new vtkDateTime by adding months.");
      return false;
      }
    case 2:
      {
      dt = this->KMLExporter->GetStartDateTime()->CreateNewAddDays(
        static_cast<int>(time));
      this->KMLExporter->AddStartDateTime(dt);
      break;
      }
    case 3:
      {
      dt = this->KMLExporter->GetStartDateTime()->CreateNewAddHours(
        static_cast<int>(time));
      this->KMLExporter->AddStartDateTime(dt);
      break;
      }
    case 4:
      {
      dt = this->KMLExporter->GetStartDateTime()->CreateNewAddMinutes(
        static_cast<int>(time));
      this->KMLExporter->AddStartDateTime(dt);
      break;
      }
    case 5:
      {
      dt = this->KMLExporter->GetStartDateTime()->CreateNewAddSeconds(
        static_cast<int>(time));
      this->KMLExporter->AddStartDateTime(dt);
      break;
      }
    default:
      {
      vtkErrorMacro("Date time unit "
                    << this->GetKMLExporter()->GetDeltaDateTimeUnit()
                    << " is not valid.");
      return false;
      }
    }

  unsigned int num_modules = this->AnimationScene->GetNumberOfViewProxies();
  if (num_modules > 1)
    {
    for (unsigned int cc=0; cc < num_modules; cc++)
      {
      vtkSMViewProxy* view = this->AnimationScene->GetViewProxy(cc);
      this->CaptureViewImage(view, this->Magnification);
      }
    }
  else if (num_modules == 1)
    {
    // If only one view, we speed things up slightly by using the
    // captured image directly.
    vtkSMViewProxy* view = this->AnimationScene->GetViewProxy(0);
    this->CaptureViewImage(view, this->Magnification);
    }

  this->FileCount++;

  return true;
}

//-----------------------------------------------------------------------------
bool vtkSMAnimationSceneKMLWriter::SaveFinalize()
{
//  this->AnimationScene->SetOverrideStillRender(0);

  // restore offscreen rendering state.
  unsigned int num_modules = this->AnimationScene->GetNumberOfViewProxies();
  for (unsigned int cc=0; cc < num_modules; cc++)
    {
    vtkSMRenderViewProxy* rmview = vtkSMRenderViewProxy::SafeDownCast(
      this->AnimationScene->GetViewProxy(cc));
    if (rmview)
      {
      vtkSMPropertyHelper(rmview, "UseOffscreenRendering").Set(0);
      rmview->UpdateProperty("UseOffscreenRendering");
      }
    }

  // Write SuperKML now.
  if(!this->KMLExporter->WriteAnimation())
    {
    vtkErrorMacro("Failed to produce super KML for animations.");
    return false;
    }

  return true;
}

//-----------------------------------------------------------------------------
bool vtkSMAnimationSceneKMLWriter::CreateWriter()
{
  std::string filename = this->FileName;
  std::string::size_type dot_pos = filename.rfind(".");
  if(dot_pos != std::string::npos)
    {
    this->SetPrefix(filename.substr(0, dot_pos).c_str());
    this->SetSuffix(filename.substr(dot_pos).c_str());
    }
  else
    {
    this->SetPrefix(this->FileName);
    this->SetSuffix("");
    }

  return true;
}

//-----------------------------------------------------------------------------
void vtkSMAnimationSceneKMLWriter::UpdateImageSize()
{
  int gui_size[2] = {1, 1};
  vtkSMViewProxy* view = this->AnimationScene->GetViewProxy(0);
  if (view)
    {
    vtkSMPropertyHelper size(view, "ViewSize");
    vtkSMPropertyHelper position(view, "ViewPosition");
    if (gui_size[0] < size.GetAsInt(0) + position.GetAsInt(0))
      {
      gui_size[0] = size.GetAsInt(0) + position.GetAsInt(0);
      }
    if (gui_size[1] < size.GetAsInt(1) + position.GetAsInt(1))
      {
      gui_size[1] = size.GetAsInt(1) + position.GetAsInt(1);
      }
    }
  else
    {
    vtkErrorMacro("AnimationScene has no view modules added to it.");
    }
  gui_size[0] *= this->Magnification;
  gui_size[1] *= this->Magnification;
  this->SetActualSize(gui_size);
}

//-----------------------------------------------------------------------------
void vtkSMAnimationSceneKMLWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Magnification: " << this->Magnification << endl;
  os << indent << "ErrorCode: " << this->ErrorCode << endl;
  os << indent << "BackgroundColor: " << this->BackgroundColor[0]
    << ", " << this->BackgroundColor[1] << ", " << this->BackgroundColor[2]
    << endl;
}


/*=========================================================================

  Program:   ParaView
  Module:    vtkCMBPolylineActor.cxx

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkCMBPolylineActor.h"
#include "vtkObjectFactory.h"
#include "vtkMapper.h"
#include "vtkDataSet.h"
#include "vtkIntArray.h"
#include "vtkFieldData.h"
#include "vtkProperty.h"
#include "vtkOpenGL.h"

//-----------------------------------------------------------------------------
vtkStandardNewMacro(vtkCMBPolylineActor);

//----------------------------------------------------------------------------
vtkCMBPolylineActor::vtkCMBPolylineActor()
{
  this->DisableGLDEPTH = 1;
  this->EnableLOD = 0;
}

//----------------------------------------------------------------------------
vtkCMBPolylineActor::~vtkCMBPolylineActor()
{
}
/*
//----------------------------------------------------------------------------
void vtkCMBPolylineActor::Render(vtkRenderer *ren, vtkMapper *vtkNotUsed(m))
{
  if(this->DisableGLDEPTH)
    {
    glDisable( GL_DEPTH_TEST );
    }

  Superclass::Render(ren,0);

  if(this->DisableGLDEPTH)
    {
    glEnable( GL_DEPTH_TEST );
    }
}
*/
//----------------------------------------------------------------------------
void vtkCMBPolylineActor::ShallowCopy(vtkProp *prop)
{
  vtkCMBPolylineActor *a = vtkCMBPolylineActor::SafeDownCast(prop);
  if ( a != NULL )
    {
    this->DisableGLDEPTH = a->DisableGLDEPTH;
    }

  // Now do superclass
  this->vtkPVLODActor::ShallowCopy(prop);
}
//----------------------------------------------------------------------------
int vtkCMBPolylineActor::RenderOpaqueGeometry(vtkViewport *vp)
{
  int renderedSomething = 0;
  // the opaque render pass has to be here for selection to work
//  if(!this->DisableGLDEPTH)
    {
    renderedSomething = this->Superclass::RenderOpaqueGeometry(vp);
    }
  return renderedSomething;
}

//-----------------------------------------------------------------------------
int vtkCMBPolylineActor::RenderOverlay(vtkViewport *vp)
{
  int renderedSomething = 0;
  // always on top.
  if(this->DisableGLDEPTH)
    {
    glDisable( GL_DEPTH_TEST );
    renderedSomething = this->Superclass::RenderOpaqueGeometry(vp);
    glEnable( GL_DEPTH_TEST );
    }
  else
    {
    renderedSomething = this->Superclass::RenderOverlay(vp);
    }
  return renderedSomething;
}

//----------------------------------------------------------------------------
void vtkCMBPolylineActor::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "DisableGLDEPTH: " << this->DisableGLDEPTH << endl;
}

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkSMMeshSourceProxy.h"

#include "vtkClientServerMoveData.h"
#include "vtkClientServerStream.h"
#include "vtkDoubleArray.h"
#include "vtkIdTypeArray.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkProcessModule.h"
#include "vtkSceneContourSource.h"

vtkStandardNewMacro(vtkSMMeshSourceProxy);

//---------------------------------------------------------------------------
vtkSMMeshSourceProxy::vtkSMMeshSourceProxy()
{
}

//---------------------------------------------------------------------------
vtkSMMeshSourceProxy::~vtkSMMeshSourceProxy()
{
}
//----------------------------------------------------------------------------
bool vtkSMMeshSourceProxy::MovePoints(vtkSMProxy* movedProxy, vtkSMProxy* transformProxy)
{
  if (!movedProxy)
  {
    return false;
  }

  vtkClientServerStream stream;
  stream << vtkClientServerStream::Invoke << VTKOBJECT(movedProxy) << "GetOutput"
         << vtkClientServerStream::End;

  this->ExecuteStream(stream);
  if (transformProxy)
  {
    stream << vtkClientServerStream::Invoke << VTKOBJECT(this) << "MoveTransformPoints"
           << this->GetLastResult(vtkProcessModule::DATA_SERVER_ROOT).GetArgument(0, 0)
           << VTKOBJECT(transformProxy) << vtkClientServerStream::End;
  }
  else
  {
    stream << vtkClientServerStream::Invoke << VTKOBJECT(this) << "MovePoints"
           << this->GetLastResult(vtkProcessModule::DATA_SERVER_ROOT).GetArgument(0, 0)
           << vtkClientServerStream::End;
  }

  this->ExecuteStream(stream);
  bool moved = false;
  this->GetLastResult(vtkProcessModule::DATA_SERVER_ROOT).GetArgument(0, 0, &moved);
  return moved;
}
//----------------------------------------------------------------------------
void vtkSMMeshSourceProxy::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

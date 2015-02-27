/*=========================================================================

  Program:   ParaView
  Module:    vtkSMCMBGlyphPointSourceProxy.cxx

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkSMCMBGlyphPointSourceProxy.h"

#include "pqSMAdaptor.h"
#include "vtkClientServerStream.h"
#include "vtkDoubleArray.h"
#include "vtkIdTypeArray.h"
#include "vtkObjectFactory.h"
#include "vtkProcessModule.h"
#include "vtkPVXMLElement.h"
#include "vtkSMNewWidgetRepresentationProxy.h"
#include "vtkSMSession.h"

vtkStandardNewMacro(vtkSMCMBGlyphPointSourceProxy);

//---------------------------------------------------------------------------
vtkSMCMBGlyphPointSourceProxy::vtkSMCMBGlyphPointSourceProxy()
{
}

//---------------------------------------------------------------------------
vtkSMCMBGlyphPointSourceProxy::~vtkSMCMBGlyphPointSourceProxy()
{
  this->SetVTKClassName(0);
}

//----------------------------------------------------------------------------
void vtkSMCMBGlyphPointSourceProxy::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
//----------------------------------------------------------------------------
vtkIdType vtkSMCMBGlyphPointSourceProxy::InsertNextPoint(double *p)
{
  vtkClientServerStream stream;
  stream << vtkClientServerStream::Invoke
         << VTKOBJECT(this) << "InsertNextPoint"
         << p[0] << p[1] << p[2]
         << vtkClientServerStream::End;
  this->ExecuteStream(stream);
  vtkIdType id;
  int retVal = this->GetSession()->GetLastResult(vtkPVSession::DATA_SERVER_ROOT).GetArgument(0, 0, &id);

  if (!retVal)
    {
    vtkErrorMacro("Error getting array from server.");
    return -1;
    }
  this->MarkModified(this);
  return id;
}
//----------------------------------------------------------------------------

// Description:
// Insert the next point and its properties into the object
vtkIdType vtkSMCMBGlyphPointSourceProxy::InsertNextPoint(double *point,
                                                         double *color,
                                                         double *scale,
                                                         double *orientation,
                                                         int visibility)
{
  vtkClientServerStream stream;
  stream << vtkClientServerStream::Invoke
         << VTKOBJECT(this) << "InsertNextPoint"
         << point[0] << point[1] << point[2]
         << color[0] << color[1] << color[2] << color[3]
         << scale[0] << scale[1] << scale[2]
         << orientation[0] << orientation[1] << orientation[2]
         << visibility
         << vtkClientServerStream::End;
  this->ExecuteStream(stream);
  vtkClientServerStream values;
  int retVal = this->GetSession()->GetLastResult(this->Location).GetArgument(0, 0, &values);

  if (!retVal)
    {
    vtkErrorMacro("Error getting array from server.");
    return -1;
    }

  vtkIdType id;
  int status = values.GetArgument(0, 0, &id);
  if (!status)
    {
    vtkErrorMacro("Error getting id.");
    }
  this->MarkModified(this);
  return id;
}
//----------------------------------------------------------------------------

void vtkSMCMBGlyphPointSourceProxy::SetScale(vtkIdType index, double *scale)
{
  this->SendDouble3Vector("SetScale", index, scale);
}
//----------------------------------------------------------------------------
void vtkSMCMBGlyphPointSourceProxy::SetOrientation(vtkIdType index, double *orientation)
{
  this->SendDouble3Vector("SetOrientation", index, orientation);
}
//----------------------------------------------------------------------------
void vtkSMCMBGlyphPointSourceProxy::SetVisibility(vtkIdType index, int visibility)
{
  vtkClientServerStream stream;
  stream << vtkClientServerStream::Invoke
         << VTKOBJECT(this) << "SetVisibility"
         << index << visibility
         << vtkClientServerStream::End;
  this->ExecuteStream(stream);
  this->MarkModified(this);
}
//----------------------------------------------------------------------------
void vtkSMCMBGlyphPointSourceProxy::SetColor(vtkIdType index, double *color)
{
  vtkClientServerStream stream;
  stream << vtkClientServerStream::Invoke
         << VTKOBJECT(this) << "SetColor"
         << index
         << color[0] << color[1] << color[2] << color[3]
         << vtkClientServerStream::End;
  this->ExecuteStream(stream);
  this->MarkModified(this);
}
//----------------------------------------------------------------------------
void vtkSMCMBGlyphPointSourceProxy::UnsetColor(vtkIdType index)
{
  vtkClientServerStream stream;
  stream << vtkClientServerStream::Invoke
         << VTKOBJECT(this) << "UnsetColor"
         << index
         << vtkClientServerStream::End;
  this->ExecuteStream(stream);
  this->MarkModified(this);
}
//----------------------------------------------------------------------------
void vtkSMCMBGlyphPointSourceProxy::ResetColorsToDefault()
{
  vtkClientServerStream stream;
  stream << vtkClientServerStream::Invoke
         << VTKOBJECT(this) << "ResetColorsToDefault"
         << vtkClientServerStream::End;
  this->ExecuteStream(stream);
  this->MarkModified(this);
}
//----------------------------------------------------------------------------
void vtkSMCMBGlyphPointSourceProxy::SetDefaultColor(double *color)
{
  vtkClientServerStream stream;
  stream << vtkClientServerStream::Invoke
         << VTKOBJECT(this) << "SetDefaultColor"
         << color[0] << color[1] << color[2] << color[3]
         << vtkClientServerStream::End;
  this->ExecuteStream(stream);
  this->MarkModified(this);
}
//----------------------------------------------------------------------------
void vtkSMCMBGlyphPointSourceProxy::SetGlyphSourceBounds(double bounds[6])
{
  vtkClientServerStream stream;
  stream << vtkClientServerStream::Invoke
    << VTKOBJECT(this) << "SetGlyphSourceBounds"
    << vtkClientServerStream::InsertArray(bounds, 6)
  << vtkClientServerStream::End;
  this->ExecuteStream(stream);
  this->MarkModified(this);
}

//----------------------------------------------------------------------------
void vtkSMCMBGlyphPointSourceProxy::ApplyTransform(vtkIdType index, double *o,
                                                   double *p, double *s)
{
  vtkClientServerStream stream;
  stream << vtkClientServerStream::Invoke
         << VTKOBJECT(this) << "ApplyTransform"
         << index
         << vtkClientServerStream::InsertArray(o, 3)
         << vtkClientServerStream::InsertArray(p, 3)
         << vtkClientServerStream::InsertArray(s, 3)
         << vtkClientServerStream::End;
  this->ExecuteStream(stream);
  this->MarkModified(this);
}
//----------------------------------------------------------------------------
void vtkSMCMBGlyphPointSourceProxy::ApplyTransform(double *o,
                                                   double *p, double *s)
{
  vtkClientServerStream stream;
  stream << vtkClientServerStream::Invoke
         << VTKOBJECT(this) << "ApplyTransform"
         << vtkClientServerStream::InsertArray(o, 3)
         << vtkClientServerStream::InsertArray(p, 3)
         << vtkClientServerStream::InsertArray(s, 3)
         << vtkClientServerStream::End;
  this->ExecuteStream(stream);
  this->MarkModified(this);
}
//----------------------------------------------------------------------------
void vtkSMCMBGlyphPointSourceProxy::ReadFromFile(const char *fname)
{
  this->SendString("ReadFromFile", fname);
}
//----------------------------------------------------------------------------
void vtkSMCMBGlyphPointSourceProxy::WriteToFile(const char *fname)
{
  this->SendString("WriteToFile", fname);
}
//----------------------------------------------------------------------------
void vtkSMCMBGlyphPointSourceProxy::SetPoint(vtkIdType index, double *point)
{
  this->SendDouble3Vector("SetPoint", index, point);
}
//----------------------------------------------------------------------------
void vtkSMCMBGlyphPointSourceProxy::GetPoint(vtkIdType index, double *p)
{
  this->ReceiveDouble3Vector("GetPoint", index, p);
}
//----------------------------------------------------------------------------
void vtkSMCMBGlyphPointSourceProxy::GetScale(vtkIdType index, double *s)
{
  this->ReceiveDouble3Vector("GetScale", index, s);
}
//----------------------------------------------------------------------------
void vtkSMCMBGlyphPointSourceProxy::GetOrientation(vtkIdType index, double *o)
{
  this->ReceiveDouble3Vector("GetOrientation", index, o);
}
//----------------------------------------------------------------------------
int vtkSMCMBGlyphPointSourceProxy::GetVisibility(vtkIdType index)
{
  int val;

  vtkClientServerStream stream;

  stream << vtkClientServerStream::Invoke
         << VTKOBJECT(this) << "GetVisibility"  << index
         << vtkClientServerStream::End;

  this->ExecuteStream(stream);

  vtkClientServerStream values =
          this->GetSession()->GetLastResult(this->Location);

  if (values.GetNumberOfArguments(0) < 0)
    {
    vtkErrorMacro("Error getting data from server.");
    return -1;
    }

  int status = values.GetArgument(0, 0, &val);
  if (!status)
    {
    vtkErrorMacro("Error getting visibility.");
    }
  return val;
}
//----------------------------------------------------------------------------
vtkIdType vtkSMCMBGlyphPointSourceProxy::GetNumberOfPoints()
{
  vtkIdType val;

  vtkClientServerStream stream;

  stream << vtkClientServerStream::Invoke
         << VTKOBJECT(this) << "GetNumberOfPoints"
         << vtkClientServerStream::End;

  this->ExecuteStream(stream);

  vtkClientServerStream values = this->GetSession()->GetLastResult(this->Location);

  if (values.GetNumberOfArguments(0) < 0)
    {
    vtkErrorMacro("Error getting data from server.");
    return -1;
    }

  int status = values.GetArgument(0, 0, &val);
  if (!status)
    {
    vtkErrorMacro("Error getting number of points.");
    }
  return val;
}
//----------------------------------------------------------------------------
void vtkSMCMBGlyphPointSourceProxy::GetColor(vtkIdType index, double *color)
{

  vtkClientServerStream stream;

  stream << vtkClientServerStream::Invoke
         << VTKOBJECT(this) << "GetColor"  << index
         << vtkClientServerStream::End;

  this->ExecuteStream(stream);

  vtkClientServerStream values = this->GetSession()->GetLastResult(this->Location);

  if (values.GetNumberOfArguments(0) < 0)
    {
    vtkErrorMacro("Error getting array from server.");
    return;
    }

  vtkTypeUInt32 length;
  values.GetArgumentLength(0, 0, &length);
  if (length != 4)
    {
    vtkErrorMacro("Error getting array from server - incorrect size returned.");
    return;
    }
  int status = values.GetArgument(0, 0, color, 4);
  if (!status)
    {
    vtkErrorMacro("Error getting color.");
    }
}
//----------------------------------------------------------------------------
void vtkSMCMBGlyphPointSourceProxy::GetDefaultColor(double *color)
{

  vtkClientServerStream stream;

  stream << vtkClientServerStream::Invoke
         << VTKOBJECT(this) << "GetDefaultColor"
         << vtkClientServerStream::End;

  this->ExecuteStream(stream);

  vtkClientServerStream values = this->GetSession()->GetLastResult(this->Location);

  if (values.GetNumberOfArguments(0) < 0)
    {
    vtkErrorMacro("Error getting array from server.");
    return;
    }

  vtkTypeUInt32 length;
  values.GetArgumentLength(0, 0, &length);
  if (length != 4)
    {
    vtkErrorMacro("Error getting array from server - incorrect size returned.");
    return;
    }
  int status = values.GetArgument(0, 0, color, 4);
  if (!status)
    {
    vtkErrorMacro("Error getting  default color.");
    }
}
//----------------------------------------------------------------------------
void vtkSMCMBGlyphPointSourceProxy::GetBounds(vtkIdType index, double *b)
{

  vtkClientServerStream stream;

  stream << vtkClientServerStream::Invoke
         << VTKOBJECT(this) << "GetBounds"  << index
         << vtkClientServerStream::End;

  this->ExecuteStream(stream);
  const vtkClientServerStream& res = this->GetLastResult(this->Location);
  int retVal = res.GetArgument(0, 0, b, 6);
  if (!retVal)
    {
    vtkErrorMacro("Error getting bounds from server.");
    }
}
//----------------------------------------------------------------------------
void vtkSMCMBGlyphPointSourceProxy::SendString(const char *func,
                                               const char  *data)
{
  vtkClientServerStream stream;
  stream << vtkClientServerStream::Invoke
         << VTKOBJECT(this) << func
         << data
         << vtkClientServerStream::End;
  this->ExecuteStream(stream);
}
//----------------------------------------------------------------------------
void vtkSMCMBGlyphPointSourceProxy::SendDouble3Vector(const char *func,
                                                     vtkIdType index,
                                                     double *data)
{
  vtkClientServerStream stream;
  stream << vtkClientServerStream::Invoke
         << VTKOBJECT(this) << func
         << index
         << data[0] << data[1] << data[2]
         << vtkClientServerStream::End;
  this->ExecuteStream(stream);
  this->MarkModified(this);
}
//----------------------------------------------------------------------------
void vtkSMCMBGlyphPointSourceProxy::ReceiveDouble3Vector(const char *func,
                                                         vtkIdType index,
                                                         double *data)
{

  vtkClientServerStream stream;

  stream << vtkClientServerStream::Invoke
         << VTKOBJECT(this) << func  << index
         << vtkClientServerStream::End;

  this->ExecuteStream(stream);

  vtkClientServerStream values =
    this->GetSession()->GetLastResult(this->Location);

  if (values.GetNumberOfArguments(0) < 0)
    {
    vtkErrorMacro("Error getting array from server.");
    return;
    }

  vtkTypeUInt32 length;
  values.GetArgumentLength(0, 0, &length);
  if (length != 3)
    {
    vtkErrorMacro("Error getting array from server - incorrect size returned.");
    return;
    }
  int status = values.GetArgument(0, 0, data, 3);
  if (!status)
    {
    vtkErrorMacro("Error getting double vector 3.");
    }
}
//----------------------------------------------------------------------------

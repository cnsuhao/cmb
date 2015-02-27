/*=========================================================================

Copyright (c) 1998-2010 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
=========================================================================*/
#include "vtkSMLIDARMultiFilesReaderProxy.h"

#include "vtkClientServerStream.h"
#include "vtkObjectFactory.h"
#include "vtkPVXMLElement.h"
#include "vtkProcessModule.h"
#include "vtkSMProxyProperty.h"
#include "vtkSMSession.h"
#include "vtkTransform.h"
#include "vtkMatrix4x4.h"

vtkStandardNewMacro(vtkSMLIDARMultiFilesReaderProxy);

//-----------------------------------------------------------------------------
template <class T>
void SMGetReaderMethodResult(T& res, vtkSMLIDARMultiFilesReaderProxy* proxy,
  const char* methodName, const char* filename)
{
  vtkClientServerStream stream;
  stream << vtkClientServerStream::Invoke
    << VTKOBJECT(proxy) << methodName  << filename
    << vtkClientServerStream::End;
  proxy->GetSession()->ExecuteStream(proxy->GetLocation(), stream);
  vtkIdType id;
  vtkClientServerStream values;
  int retVal = proxy->GetSession()->GetLastResult(proxy->GetLocation()).GetArgument(0, 0, &id);

  if (values.GetNumberOfArguments(0) < 0)
    {
    res = -1;
    }

  int status = values.GetArgument(0, 0, &res);
  if (!status)
    {
    res = -1;
    }
}

//-----------------------------------------------------------------------------
vtkSMLIDARMultiFilesReaderProxy::vtkSMLIDARMultiFilesReaderProxy()
{
}

//-----------------------------------------------------------------------------
vtkSMLIDARMultiFilesReaderProxy::~vtkSMLIDARMultiFilesReaderProxy()
{
}

//-----------------------------------------------------------------------------
int vtkSMLIDARMultiFilesReaderProxy::GetKnownNumberOfPieces(
  const char* filename)
{
  int val=-1;
  SMGetReaderMethodResult(val, this,
    "GetKnownNumberOfPieces", filename);
  if(val<0)
    {
    vtkErrorMacro("Error running GetKnownNumberOfPieces from server.");
    }

  return val;
}

//-----------------------------------------------------------------------------
vtkIdType vtkSMLIDARMultiFilesReaderProxy::GetTotalNumberOfPoints(
  const char* filename)
{
  vtkIdType val=-1;
  SMGetReaderMethodResult(val, this,
    "GetTotalNumberOfPoints", filename);
  if(val<0)
    {
    vtkErrorMacro("Error running GetTotalNumberOfPoints from server.");
    }
  return val;
}

//-----------------------------------------------------------------------------
vtkIdType vtkSMLIDARMultiFilesReaderProxy::GetRealNumberOfOutputPoints(
  const char* filename)
{
  vtkIdType val;
  SMGetReaderMethodResult(val, this,
    "GetRealNumberOfOutputPoints", filename);
  if(val<0)
    {
    vtkErrorMacro("Error running GetRealNumberOfOutputPoints from server.");
    }
  return val;
}

//-----------------------------------------------------------------------------
vtkIdType vtkSMLIDARMultiFilesReaderProxy::GetNumberOfPointsInPiece(
  const char* filename, int pieceIndex)
{
  vtkIdType val;
  vtkProcessModule* pm = vtkProcessModule::GetProcessModule();

  vtkClientServerStream stream;
  stream << vtkClientServerStream::Invoke
  << VTKOBJECT(this) << "GetNumberOfPointsInPiece"
  << filename << pieceIndex
  << vtkClientServerStream::End;
  this->ExecuteStream(stream);
  vtkIdType id;
  vtkClientServerStream values;
  int retVal = this->GetLastResult(this->GetLocation()).GetArgument(0, 0, &id);

  if (values.GetNumberOfArguments(0) < 0)
    {
    vtkErrorMacro("Error GetNumberOfPointsInPiece from server.");
    return -1;
    }

  int status = values.GetArgument(0, 0, &val);
  if (!status)
    {
    vtkErrorMacro("Error GetNumberOfPointsInPiece.");
    }
  return val;
}

//-----------------------------------------------------------------------------
void vtkSMLIDARMultiFilesReaderProxy::RemoveAllRequestedReadPieces()
{
  vtkProcessModule* pm = vtkProcessModule::GetProcessModule();

  vtkClientServerStream stream;
  stream << vtkClientServerStream::Invoke
  << VTKOBJECT(this) << "RemoveAllRequestedReadPieces"
  << vtkClientServerStream::End;
  this->ExecuteStream(stream);
}

//-----------------------------------------------------------------------------
void vtkSMLIDARMultiFilesReaderProxy::AddRequestedPieceForRead(
  const char* filename, int pieceIndex, int OnRatio)
{
  vtkProcessModule* pm = vtkProcessModule::GetProcessModule();

  vtkClientServerStream stream;
  stream << vtkClientServerStream::Invoke
  << VTKOBJECT(this) << "AddRequestedPieceForRead"
  << filename << pieceIndex << OnRatio
  << vtkClientServerStream::End;
  this->ExecuteStream(stream);
}

//-----------------------------------------------------------------------------
void vtkSMLIDARMultiFilesReaderProxy::SetConvertFromLatLongToXYZ(
  const char* filename, bool mode)
{
  vtkProcessModule* pm = vtkProcessModule::GetProcessModule();

  vtkClientServerStream stream;
  stream << vtkClientServerStream::Invoke
  << VTKOBJECT(this) << "SetConvertFromLatLongToXYZ"
  << filename << mode
  << vtkClientServerStream::End;
  this->ExecuteStream(stream);
}

//-----------------------------------------------------------------------------
void vtkSMLIDARMultiFilesReaderProxy::ClearTransform(
  const char* filename)
{
  vtkProcessModule* pm = vtkProcessModule::GetProcessModule();

  vtkClientServerStream stream;
  stream << vtkClientServerStream::Invoke
  << VTKOBJECT(this) << "ClearTransform"
  << filename
  << vtkClientServerStream::End;
  this->ExecuteStream(stream);
}

//-----------------------------------------------------------------------------
void vtkSMLIDARMultiFilesReaderProxy::SetTransform(
  const char* filename,vtkTransform *transform)
{
  if(transform)
    {
    this->SetTransform(filename, *transform->GetMatrix()->Element);
    }
  else
    {
    this->ClearTransform(filename);
    }
}

//-----------------------------------------------------------------------------
void vtkSMLIDARMultiFilesReaderProxy::SetTransform(
  const char* filename, double* elements)
{
  vtkProcessModule* pm = vtkProcessModule::GetProcessModule();

  vtkClientServerStream stream;
  stream << vtkClientServerStream::Invoke
  << VTKOBJECT(this) << "SetTransform"
  << filename << vtkClientServerStream::InsertArray(
  &(elements[0]), 16)
  << vtkClientServerStream::End;
  this->ExecuteStream(stream);
}

//-----------------------------------------------------------------------------
void vtkSMLIDARMultiFilesReaderProxy::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

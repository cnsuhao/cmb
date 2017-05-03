//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkSMCMBConeCellClassifierProxy.h"

#include "pqSMAdaptor.h"
#include "vtkClientServerStream.h"
#include "vtkDoubleArray.h"
#include "vtkIdTypeArray.h"
#include "vtkObjectFactory.h"
#include "vtkPVXMLElement.h"
#include "vtkProcessModule.h"
#include "vtkSMNewWidgetRepresentationProxy.h"

vtkStandardNewMacro(vtkSMCMBConeCellClassifierProxy);

vtkSMCMBConeCellClassifierProxy::vtkSMCMBConeCellClassifierProxy()
{
}

vtkSMCMBConeCellClassifierProxy::~vtkSMCMBConeCellClassifierProxy()
{
  this->SetVTKClassName(0);
}

void vtkSMCMBConeCellClassifierProxy::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

// Description:
// Insert the next cone
void vtkSMCMBConeCellClassifierProxy::InsertNextCone(const double baseCenter[3],
  const double axisDir[3], double height, double r0, double r1, int oldVal, int newVal)
{
  vtkClientServerStream stream;
  stream << vtkClientServerStream::Invoke << VTKOBJECT(this) << "InsertNextCone"
         << vtkClientServerStream::InsertArray(baseCenter, 3)
         << vtkClientServerStream::InsertArray(axisDir, 3) << height << r0 << r1 << oldVal << newVal
         << vtkClientServerStream::End;
  this->ExecuteStream(stream);
  this->MarkModified(this);
}

// Description:
// Insert the next cone
void vtkSMCMBConeCellClassifierProxy::InsertNextCone(const double baseCenter[3],
  const double axisDir[3], double height, double r0, double r1, const double translate[3],
  const double orientation[3], const double scaling[3], int oldVal, int newVal)
{
  vtkClientServerStream stream;
  stream << vtkClientServerStream::Invoke << VTKOBJECT(this) << "InsertNextCone"
         << vtkClientServerStream::InsertArray(baseCenter, 3)
         << vtkClientServerStream::InsertArray(axisDir, 3) << height << r0 << r1
         << vtkClientServerStream::InsertArray(translate, 3)
         << vtkClientServerStream::InsertArray(orientation, 3)
         << vtkClientServerStream::InsertArray(scaling, 3) << oldVal << newVal
         << vtkClientServerStream::End;
  this->ExecuteStream(stream);
  this->MarkModified(this);
}

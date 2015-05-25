//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkCMBArcDeleteClientOperator.h"


#include "vtkClientServerStream.h"
#include "vtkClientServerMoveData.h"
#include "vtkObjectFactory.h"
#include "vtkSMArcOperatorProxy.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxyManager.h"
#include "vtkSMNewWidgetRepresentationProxy.h"

vtkStandardNewMacro(vtkCMBArcDeleteClientOperator);

//---------------------------------------------------------------------------
vtkCMBArcDeleteClientOperator::vtkCMBArcDeleteClientOperator()
{
}

//---------------------------------------------------------------------------
vtkCMBArcDeleteClientOperator::~vtkCMBArcDeleteClientOperator()
{
}

//----------------------------------------------------------------------------
bool vtkCMBArcDeleteClientOperator::SetMarkedForDeletion(const vtkIdType& arcId)
{
  return this->Operate(arcId,Mark_Mode);
}
//----------------------------------------------------------------------------
bool vtkCMBArcDeleteClientOperator::SetUnMarkedForDeletion(const vtkIdType& arcId)
{
  return this->Operate(arcId,UnMark_Mode);
}

//----------------------------------------------------------------------------
bool vtkCMBArcDeleteClientOperator::DeleteArc(const vtkIdType& arcId)
{
  return this->Operate(arcId,Delete_Mode);
}

//----------------------------------------------------------------------------
bool vtkCMBArcDeleteClientOperator:: Operate(const vtkIdType& arcId,
      vtkCMBArcDeleteClientOperator::Mode mode)
{
  if (!this)
    {
    return false;
    }
  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSMArcOperatorProxy *proxy = vtkSMArcOperatorProxy::SafeDownCast(
        manager->NewProxy("CmbArcGroup","DeleteOperator"));

  //pass down the mode to the delete operator
  vtkSMPropertyHelper arcIdHelper(proxy,"DeleteMode");
  arcIdHelper.Set(mode);

  bool valid = proxy->Operate(arcId);

  proxy->Delete();
  return valid;
}

//----------------------------------------------------------------------------
void vtkCMBArcDeleteClientOperator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

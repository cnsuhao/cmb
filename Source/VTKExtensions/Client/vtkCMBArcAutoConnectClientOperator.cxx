/*=========================================================================

  Program:   ParaView
  Module:    vtkCMBArcAutoConnectClientOperator.cxx

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkCMBArcAutoConnectClientOperator.h"


#include "vtkObjectFactory.h"
#include "vtkSMArcOperatorProxy.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxyManager.h"

vtkStandardNewMacro(vtkCMBArcAutoConnectClientOperator);

//---------------------------------------------------------------------------
vtkCMBArcAutoConnectClientOperator::vtkCMBArcAutoConnectClientOperator():
  ArcId(-1)
{
}

//---------------------------------------------------------------------------
vtkCMBArcAutoConnectClientOperator::~vtkCMBArcAutoConnectClientOperator()
{
}

//----------------------------------------------------------------------------
bool vtkCMBArcAutoConnectClientOperator::Operate(
  const vtkIdType& firstArcId, const vtkIdType& secondArcId)
{
  if (!this)
    {
    return false;
    }

  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSMArcOperatorProxy *proxy = vtkSMArcOperatorProxy::SafeDownCast(
        manager->NewProxy("CmbArcGroup","AutoConnectOperator"));
  bool valid = proxy->Operate(firstArcId,secondArcId);
  if (valid)
    {
    vtkSMPropertyHelper helper(proxy,"CreatedArcId");
    helper.UpdateValueFromServer();
    this->ArcId = helper.GetAsIdType();
    }
  proxy->Delete();
  return true;
}

//----------------------------------------------------------------------------
void vtkCMBArcAutoConnectClientOperator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
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

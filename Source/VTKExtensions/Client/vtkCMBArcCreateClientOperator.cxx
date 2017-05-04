//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkCMBArcCreateClientOperator.h"

#include "vtkClientServerMoveData.h"
#include "vtkClientServerStream.h"
#include "vtkObjectFactory.h"
#include "vtkSMArcOperatorProxy.h"
#include "vtkSMNewWidgetRepresentationProxy.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxyManager.h"

vtkStandardNewMacro(vtkCMBArcCreateClientOperator);

vtkCMBArcCreateClientOperator::vtkCMBArcCreateClientOperator()
  : ArcId(-1)
{
}

vtkCMBArcCreateClientOperator::~vtkCMBArcCreateClientOperator()
{
}

bool vtkCMBArcCreateClientOperator::Create(vtkSMNewWidgetRepresentationProxy* widgetProxy)
{
  this->ArcId = -1;
  if (!widgetProxy)
  {
    return false;
  }

  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSMArcOperatorProxy* proxy =
    vtkSMArcOperatorProxy::SafeDownCast(manager->NewProxy("CmbArcGroup", "CreateOperator"));
  bool valid = proxy->Operate(widgetProxy);
  if (valid)
  {
    vtkSMPropertyHelper helper(proxy, "CreatedArcId");
    helper.UpdateValueFromServer();
    this->ArcId = helper.GetAsIdType();
  }
  proxy->Delete();
  return valid;
}

bool vtkCMBArcCreateClientOperator::Create(vtkSMSourceProxy* sourceProxy)
{
  this->ArcId = -1;
  if (!sourceProxy)
  {
    return false;
  }

  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSMArcOperatorProxy* proxy =
    vtkSMArcOperatorProxy::SafeDownCast(manager->NewProxy("CmbArcGroup", "CreateOperator"));
  bool valid = proxy->Operate(sourceProxy);
  if (valid)
  {
    vtkSMPropertyHelper helper(proxy, "CreatedArcId");
    helper.UpdateValueFromServer();
    this->ArcId = helper.GetAsIdType();
  }
  proxy->Delete();
  return valid;
}

void vtkCMBArcCreateClientOperator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

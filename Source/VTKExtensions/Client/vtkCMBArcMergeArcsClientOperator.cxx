//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkCMBArcMergeArcsClientOperator.h"


#include "vtkClientServerStream.h"
#include "vtkClientServerMoveData.h"
#include "vtkObjectFactory.h"
#include "vtkSMArcOperatorProxy.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxyManager.h"
#include "vtkSMNewWidgetRepresentationProxy.h"

vtkStandardNewMacro(vtkCMBArcMergeArcsClientOperator);

//---------------------------------------------------------------------------
vtkCMBArcMergeArcsClientOperator::vtkCMBArcMergeArcsClientOperator():
  ArcId(-1),ArcIdToDelete(-1)
{
}

//---------------------------------------------------------------------------
vtkCMBArcMergeArcsClientOperator::~vtkCMBArcMergeArcsClientOperator()
{
}

//----------------------------------------------------------------------------
bool vtkCMBArcMergeArcsClientOperator::Operate(
  const vtkIdType& firstArcId, const vtkIdType& secondArcId)
{
  if (!this)
    {
    return false;
    }

  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSMArcOperatorProxy *proxy = vtkSMArcOperatorProxy::SafeDownCast(
        manager->NewProxy("CmbArcGroup","MergeArcsOperator"));
  bool valid = proxy->Operate(firstArcId,secondArcId);
  if (!valid)
    {
    proxy->Delete();
    return false;
    }

  vtkSMPropertyHelper helper(proxy,"CreatedArcId");
  helper.UpdateValueFromServer();
  this->ArcId = helper.GetAsIdType();

  vtkSMPropertyHelper helper2(proxy,"ArcIdToDelete");
  helper2.UpdateValueFromServer();
  this->ArcIdToDelete = helper2.GetAsIdType();

  proxy->Delete();
  return true;
}

//----------------------------------------------------------------------------
void vtkCMBArcMergeArcsClientOperator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

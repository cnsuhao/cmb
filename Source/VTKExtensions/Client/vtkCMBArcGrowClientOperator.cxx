//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkCMBArcGrowClientOperator.h"

#include "vtkClientServerMoveData.h"
#include "vtkClientServerStream.h"
#include "vtkIdTypeArray.h"
#include "vtkObjectFactory.h"
#include "vtkSMArcOperatorProxy.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxyManager.h"

vtkStandardNewMacro(vtkCMBArcGrowClientOperator);

//---------------------------------------------------------------------------
vtkCMBArcGrowClientOperator::vtkCMBArcGrowClientOperator()
{
  this->GrownArcIds = NULL;
}

//---------------------------------------------------------------------------
vtkCMBArcGrowClientOperator::~vtkCMBArcGrowClientOperator()
{
  if (this->GrownArcIds)
  {
    this->GrownArcIds->Delete();
  }
}

//----------------------------------------------------------------------------
bool vtkCMBArcGrowClientOperator::Operate(std::list<vtkIdType> arcIds)
{
  if (arcIds.size() == 0)
  {
    return false;
  }

  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSMArcOperatorProxy* proxy =
    vtkSMArcOperatorProxy::SafeDownCast(manager->NewProxy("CmbArcGroup", "GrowOperator"));

  //send all the arc ids down to the server
  std::list<vtkIdType>::iterator arcIt;
  for (arcIt = arcIds.begin(); arcIt != arcIds.end(); ++arcIt)
  {
    vtkSMPropertyHelper addId(proxy, "AddArc");
    addId.Set(*arcIt);
    proxy->UpdateVTKObjects();
  }

  bool valid = proxy->Operate();
  if (!valid)
  {
    proxy->Delete();
    return false;
  }

  if (this->GrownArcIds)
  {
    this->GrownArcIds->Delete();
    this->GrownArcIds = NULL;
  }
  this->GrownArcIds = vtkIdTypeArray::New();

  vtkSMPropertyHelper helper(proxy, "GrownArcSetIds");
  helper.UpdateValueFromServer();
  for (unsigned int i = 0; i < helper.GetNumberOfElements(); ++i)
  {
    this->GrownArcIds->InsertNextValue(helper.GetAsIdType(i));
  }

  proxy->Delete();
  return true;
}

//----------------------------------------------------------------------------
void vtkCMBArcGrowClientOperator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

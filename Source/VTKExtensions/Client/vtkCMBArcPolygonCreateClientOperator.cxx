//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkCMBArcPolygonCreateClientOperator.h"

#include "vtkSMArcOperatorProxy.h"
#include "vtkSMProxy.h"
#include "vtkSMProxyManager.h"
#include "vtkObjectFactory.h"

#include "vtkIdTypeArray.h"
#include "vtkCMBArcPolygonProvider.h"
#include "vtkCMBPolygonFromArcsOperator.h"

vtkStandardNewMacro(vtkCMBArcPolygonCreateClientOperator);

//---------------------------------------------------------------------------
vtkCMBArcPolygonCreateClientOperator::vtkCMBArcPolygonCreateClientOperator()
{
  this->ArcIds = NULL;
}

//---------------------------------------------------------------------------
void vtkCMBArcPolygonCreateClientOperator::AddArc(vtkIdType arcId)
{
  this->InputArcIds.push_back(arcId);
}

//---------------------------------------------------------------------------
vtkCMBArcPolygonCreateClientOperator::~vtkCMBArcPolygonCreateClientOperator()
{
  if(this->ArcIds)
    {
    this->ArcIds->Delete();
    }
}

//----------------------------------------------------------------------------
bool vtkCMBArcPolygonCreateClientOperator::Create(double minAngle,
  double edgeLength,vtkSMProxy *providerProxy)
{
  if (this->InputArcIds.size() == 0)
    {
    return false;
    }

  vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();
  vtkSMArcOperatorProxy *proxy = vtkSMArcOperatorProxy::SafeDownCast(
        manager->NewProxy("CmbArcGroup","PolygonFromArcsOperator"));


  //send all the arc ids down to the server
  vtkCMBPolygonFromArcsOperator *polyOperator = vtkCMBPolygonFromArcsOperator::SafeDownCast(
      proxy->GetClientSideObject());

  std::list<vtkIdType>::iterator arcIt;
  for (arcIt = this->InputArcIds.begin(); arcIt!=this->InputArcIds.end();++arcIt)
    {
    polyOperator->AddArcId(*arcIt);
    }
  this->InputArcIds.clear();
  bool valid = proxy->Operate( );

  if ( valid )
    {
    //if it is valid we need to create the representation from it
    vtkCMBArcPolygonProvider *provider = vtkCMBArcPolygonProvider::SafeDownCast(
      providerProxy->GetClientSideObject());

    provider->SetOuterLoopArcIds( polyOperator->GetOuterLoop() );

    vtkIdType size = polyOperator->GetNumberOfInnerLoops();
    for(vtkIdType i=0; i < size; ++i)
      {
      provider->AddInnerLoopArcIds(polyOperator->GetInnerLoop(i));
      }

    provider->SetMinAngle(minAngle);
    provider->SetEdgeLength(edgeLength);
    }


  proxy->Delete();
  return valid;
}

//----------------------------------------------------------------------------
void vtkCMBArcPolygonCreateClientOperator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

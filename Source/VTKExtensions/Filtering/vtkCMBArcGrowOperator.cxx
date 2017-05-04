//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBArcGrowOperator.h"

#include "vtkCMBArc.h"
#include "vtkCMBArcEndNode.h"
#include "vtkCMBArcManager.h"

#include "vtkIdTypeArray.h"
#include "vtkObjectFactory.h"

#include <list>
#include <set>

vtkStandardNewMacro(vtkCMBArcGrowOperator);

//----------------------------------------------------------------------------
vtkCMBArcGrowOperator::vtkCMBArcGrowOperator()
{
  this->GrownArcSetIds = NULL;
}

//----------------------------------------------------------------------------
vtkCMBArcGrowOperator::~vtkCMBArcGrowOperator()
{
  if (this->GrownArcSetIds)
  {
    this->GrownArcSetIds->Delete();
  }
}

//----------------------------------------------------------------------------
void vtkCMBArcGrowOperator::AddArc(vtkIdType arcId)
{
  this->InputArcs.insert(arcId);
}

//----------------------------------------------------------------------------
void vtkCMBArcGrowOperator::ClearInputArcs()
{
  this->InputArcs.clear();
}

//----------------------------------------------------------------------------
bool vtkCMBArcGrowOperator::Operate()
{
  if (this->InputArcs.size() == 0)
  {
    return false;
  }

  if (this->GrownArcSetIds)
  {
    this->GrownArcSetIds->Delete();
  }

  vtkCMBArcManager* manager = vtkCMBArcManager::GetInstance();
  ArcSet grownArcIds;
  ArcSet::iterator it;
  for (it = this->InputArcs.begin(); it != this->InputArcs.end(); ++it)
  {
    grownArcIds.insert((*it));

    std::set<vtkCMBArc*> arcs = manager->GetConnectedArcs(manager->GetArc((*it)));
    std::set<vtkCMBArc*>::iterator arcIt;
    for (arcIt = arcs.begin(); arcIt != arcs.end(); ++arcIt)
    {
      grownArcIds.insert((*arcIt)->GetId());
    }
  }

  //convert grownArcIds to idtype array
  this->GrownArcSetIds = vtkIdTypeArray::New();
  this->GrownArcSetIds->SetName("GrownArcSetIds");
  vtkIdType size = static_cast<vtkIdType>(grownArcIds.size());
  this->GrownArcSetIds->SetNumberOfValues(size);

  vtkIdType index = 0;
  for (it = grownArcIds.begin(); it != grownArcIds.end(); it++)
  {
    this->GrownArcSetIds->SetValue(index++, *it);
  }

  this->ClearInputArcs();
  return true;
}

//----------------------------------------------------------------------------
void vtkCMBArcGrowOperator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

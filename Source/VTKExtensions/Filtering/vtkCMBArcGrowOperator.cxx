/*=========================================================================

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
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

#include "vtkCMBArcGrowOperator.h"

#include "vtkCMBArcManager.h"
#include "vtkCMBArcEndNode.h"
#include "vtkCMBArc.h"

#include "vtkIdTypeArray.h"
#include "vtkObjectFactory.h"

#include <set>
#include <list>

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
  if ( this->InputArcs.size() == 0)
    {
    return false;
    }

  if (this->GrownArcSetIds)
    {
    this->GrownArcSetIds->Delete();
    }

  vtkCMBArcManager *manager = vtkCMBArcManager::GetInstance();
  ArcSet grownArcIds;
  ArcSet::iterator it;
  for (it=this->InputArcs.begin();it!=this->InputArcs.end();++it)
    {
    grownArcIds.insert((*it));

    std::set<vtkCMBArc*> arcs = manager->GetConnectedArcs(manager->GetArc((*it)));
    std::set<vtkCMBArc*>::iterator arcIt;
    for(arcIt=arcs.begin();arcIt!=arcs.end();++arcIt)
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
    this->GrownArcSetIds->SetValue(index++,*it);
    }

  this->ClearInputArcs();
  return true;
  }

//----------------------------------------------------------------------------
void vtkCMBArcGrowOperator::PrintSelf(ostream& os, vtkIndent indent)
  {
  this->Superclass::PrintSelf(os,indent);
}

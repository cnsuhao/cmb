//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBArcDeleteOperator.h"

#include "vtkCMBArcManager.h"
#include "vtkCMBArc.h"

#include "vtkCellArray.h"
#include "vtkIdTypeArray.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include "vtkObjectFactory.h"

vtkStandardNewMacro(vtkCMBArcDeleteOperator);

//----------------------------------------------------------------------------
vtkCMBArcDeleteOperator::vtkCMBArcDeleteOperator()
{
  this->DeleteMode = 0; //default is real delete
}

//----------------------------------------------------------------------------
vtkCMBArcDeleteOperator::~vtkCMBArcDeleteOperator()
{

}

//----------------------------------------------------------------------------
bool vtkCMBArcDeleteOperator::Operate(vtkIdType arcId)
{
  int deleteMode = this->DeleteMode;
  this->DeleteMode = 0; //set to default

  vtkCMBArc *toDelete = vtkCMBArcManager::GetInstance()->GetArc(arcId);
  if (!toDelete)
    {
    //use case is the arc is marked for deletion and we now need to really
    //delete or unmark for deletion
    toDelete = vtkCMBArcManager::GetInstance()->GetArcReadyForDeletion(arcId);
    if (!toDelete)
      {
      return false;
      }
    }

  if (deleteMode == 0 || deleteMode > 2)
    {
    toDelete->Delete();
    }
  else if(deleteMode == 1)
    {
    toDelete->MarkedForDeletion();
    }
  else if(deleteMode == 2)
    {
    toDelete->UnMarkedForDeletion();
    }
  return true;
}

//----------------------------------------------------------------------------
void vtkCMBArcDeleteOperator::PrintSelf(ostream& os, vtkIndent indent)
  {
  this->Superclass::PrintSelf(os,indent);
}

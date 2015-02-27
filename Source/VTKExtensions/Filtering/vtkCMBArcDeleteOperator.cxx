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

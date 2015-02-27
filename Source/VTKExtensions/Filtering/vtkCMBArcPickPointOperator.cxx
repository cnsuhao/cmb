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

#include "vtkCMBArcPickPointOperator.h"

#include "vtkCMBArcManager.h"
#include "vtkCMBArcEndNode.h"
#include "vtkCMBArc.h"

#include "vtkCellArray.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkObjectFactory.h"
#include "vtkSelection.h"
#include "vtkSelectionNode.h"
#include "vtkPVExtractSelection.h"
#include "vtkSmartPointer.h"

vtkStandardNewMacro(vtkCMBArcPickPointOperator);

//----------------------------------------------------------------------------
vtkCMBArcPickPointOperator::vtkCMBArcPickPointOperator()
{
  this->ArcId = -1;
  this->PickedPointId = -1;
}

//----------------------------------------------------------------------------
vtkCMBArcPickPointOperator::~vtkCMBArcPickPointOperator()
{

}

//----------------------------------------------------------------------------
bool vtkCMBArcPickPointOperator::Operate(vtkPVExtractSelection *source)
{
  if (source == NULL || this->ArcId < 0)
    {
    return false;
    }

  this->PickedPointId = -1;

  //we want the second input object as that holds the actual selection.
  vtkDataObject *dataObj = source->GetOutputDataObject(1);
  vtkSelection *selection = vtkSelection::SafeDownCast(dataObj);
  if(!selection)
    {
    vtkErrorMacro("Unable to get the selection from the vtkPVExtractSelection.")
    return false;
    }

  //currently can only handle a single selection node
  unsigned int numSelections = selection->GetNumberOfNodes();
  if(numSelections!=2)
    {
    return false;
    }

  //we are going to presume that the selection data coming in, is related
  //to the arc impilicitly. We should add some form of verification by adding
  //come field data flags to the arc providers output.

  vtkSelectionNode *node = selection->GetNode(1);
  int fieldType = node->GetFieldType();
  int contentType = node->GetContentType();

  if(fieldType != vtkSelectionNode::POINT ||
     contentType != vtkSelectionNode::INDICES)
    {
    vtkErrorMacro("Unable to handle the selection type, please make sure selection type is point indices.")
    return false;
    }


  vtkIdTypeArray* pointIds = vtkIdTypeArray::SafeDownCast(
                               node->GetSelectionList());

  const vtkIdType size(pointIds->GetNumberOfComponents());
  this->PickedPointId = size/2;

  return true;
}

//----------------------------------------------------------------------------
void vtkCMBArcPickPointOperator::PrintSelf(ostream& os, vtkIndent indent)
  {
  this->Superclass::PrintSelf(os,indent);
}

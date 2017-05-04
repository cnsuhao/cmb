//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBArcPickPointOperator.h"

#include "vtkCMBArc.h"
#include "vtkCMBArcEndNode.h"
#include "vtkCMBArcManager.h"

#include "vtkCellArray.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkObjectFactory.h"
#include "vtkPVExtractSelection.h"
#include "vtkSelection.h"
#include "vtkSelectionNode.h"
#include "vtkSmartPointer.h"

vtkStandardNewMacro(vtkCMBArcPickPointOperator);

vtkCMBArcPickPointOperator::vtkCMBArcPickPointOperator()
{
  this->ArcId = -1;
  this->PickedPointId = -1;
}

vtkCMBArcPickPointOperator::~vtkCMBArcPickPointOperator()
{
}

bool vtkCMBArcPickPointOperator::Operate(vtkPVExtractSelection* source)
{
  if (source == NULL || this->ArcId < 0)
  {
    return false;
  }

  this->PickedPointId = -1;

  //we want the second input object as that holds the actual selection.
  vtkDataObject* dataObj = source->GetOutputDataObject(1);
  vtkSelection* selection = vtkSelection::SafeDownCast(dataObj);
  if (!selection)
  {
    vtkErrorMacro("Unable to get the selection from the vtkPVExtractSelection.") return false;
  }

  //currently can only handle a single selection node
  unsigned int numSelections = selection->GetNumberOfNodes();
  if (numSelections != 2)
  {
    return false;
  }

  //we are going to presume that the selection data coming in, is related
  //to the arc impilicitly. We should add some form of verification by adding
  //come field data flags to the arc providers output.

  vtkSelectionNode* node = selection->GetNode(1);
  int fieldType = node->GetFieldType();
  int contentType = node->GetContentType();

  if (fieldType != vtkSelectionNode::POINT || contentType != vtkSelectionNode::INDICES)
  {
    vtkErrorMacro("Unable to handle the selection type, please make sure selection type is point "
                  "indices.") return false;
  }

  vtkIdTypeArray* pointIds = vtkIdTypeArray::SafeDownCast(node->GetSelectionList());

  const vtkIdType size(pointIds->GetNumberOfComponents());
  this->PickedPointId = size / 2;

  return true;
}

void vtkCMBArcPickPointOperator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBSpacingFlip.h"

#include "vtkObjectFactory.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkImageData.h"
#include "vtkDataObject.h"
#include "vtkImageFlip.h"

vtkStandardNewMacro(vtkCMBSpacingFlip);

vtkCMBSpacingFlip::vtkCMBSpacingFlip()
{

}

int vtkCMBSpacingFlip
::RequestData(vtkInformation * vtkNotUsed(request),
              vtkInformationVector ** inputVector,
              vtkInformationVector *  outputVector)
{
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  vtkImageData *input = vtkImageData::SafeDownCast(
                                inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkImageData *output = vtkImageData::SafeDownCast(
                               outInfo->Get(vtkDataObject::DATA_OBJECT()));

  double * spacing = input->GetSpacing();

  if(spacing[0] >= 0 && spacing[1] >= 0 && spacing[2] >= 0)
  {
    output->ShallowCopy(input);
  }
  else
  {
    output->DeepCopy(input);
    vtkImageFlip * flipper = vtkImageFlip::New();
    if(spacing[0] < 0 && spacing[1] >= 0 && spacing[2] >= 0)
    {
      flipper->SetFilteredAxis(0);
    }
    else if(spacing[1] < 0 && spacing[0] >= 0 && spacing[2] >= 0)
    {
      flipper->SetFilteredAxis(1);
    }
    else if(spacing[2] < 0 && spacing[0] >= 0 && spacing[1] >= 0)
    {
      flipper->SetFilteredAxis(2);
    }
    else
    {
      //TODO error message for now
    }
    flipper->SetInputData(input);
    flipper->Update();
    output->DeepCopy(flipper->GetOutput());
    flipper->Delete();
  }

  return 1;

}

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkCMBInteractorStyleRubberBand.h"

#include "vtkCallbackCommand.h"
#include "vtkCommand.h"
#include "vtkObjectFactory.h"
#include "vtkRenderWindowInteractor.h"

vtkStandardNewMacro(vtkCMBInteractorStyleRubberBand);

//--------------------------------------------------------------------------
vtkCMBInteractorStyleRubberBand::vtkCMBInteractorStyleRubberBand()
{
}

//--------------------------------------------------------------------------
vtkCMBInteractorStyleRubberBand::~vtkCMBInteractorStyleRubberBand()
{
}

//--------------------------------------------------------------------------
void vtkCMBInteractorStyleRubberBand::OnLeftButtonDown()
{
  // If shift key is down, just rotate
  if (this->CurrentMode && this->Interactor && this->Interactor->GetShiftKey() &&
    this->State != VTKIS_ROTATE)
  {
    this->FindPokedRenderer(
      this->Interactor->GetEventPosition()[0], this->Interactor->GetEventPosition()[1]);
    if (this->CurrentRenderer == NULL)
    {
      return;
    }

    this->GrabFocus(this->EventCallbackCommand);
    this->StartRotate();
    return;
  }

  this->Superclass::OnLeftButtonDown();
}

//--------------------------------------------------------------------------
void vtkCMBInteractorStyleRubberBand::OnMouseMove()
{
  // If shift key is down, just rotate
  if (this->CurrentMode && this->Interactor && this->Interactor->GetShiftKey() &&
    this->State == VTKIS_ROTATE)
  {
    int x = this->Interactor->GetEventPosition()[0];
    int y = this->Interactor->GetEventPosition()[1];

    this->FindPokedRenderer(x, y);
    this->Rotate();
    this->InvokeEvent(vtkCommand::InteractionEvent, NULL);
    return;
  }
  this->Superclass::OnMouseMove();
}

//--------------------------------------------------------------------------
void vtkCMBInteractorStyleRubberBand::OnLeftButtonUp()
{
  // If shift key is down, just rotate
  if (this->CurrentMode && this->Interactor && this->Interactor->GetShiftKey() &&
    this->State == VTKIS_ROTATE)
  {
    this->EndRotate();
    this->ReleaseFocus();
    return;
  }

  this->Superclass::OnLeftButtonUp();
}

//--------------------------------------------------------------------------
void vtkCMBInteractorStyleRubberBand::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

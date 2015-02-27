/*=========================================================================

  Program:   CMB
  Module:    vtkCMBInteractorStyleRubberBand.cxx

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
#include "vtkCMBInteractorStyleRubberBand.h"

#include "vtkObjectFactory.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkCallbackCommand.h"
#include "vtkCommand.h"

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
  if(this->CurrentMode && this->Interactor &&
    this->Interactor->GetShiftKey() && this->State != VTKIS_ROTATE)
    {
    this->FindPokedRenderer(this->Interactor->GetEventPosition()[0],
                            this->Interactor->GetEventPosition()[1]);
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
  if(this->CurrentMode && this->Interactor &&
    this->Interactor->GetShiftKey() && this->State == VTKIS_ROTATE)
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
  if(this->CurrentMode && this->Interactor &&
    this->Interactor->GetShiftKey() && this->State == VTKIS_ROTATE)
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

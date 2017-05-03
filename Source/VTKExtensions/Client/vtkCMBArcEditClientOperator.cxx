//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkCMBArcEditClientOperator.h"

#include "vtkClientServerStream.h"
#include "vtkObjectFactory.h"
#include "vtkSMNewWidgetRepresentationProxy.h"
#include "vtkSMProxy.h"

#include "smtk/extension/vtk/widgets/vtkSMTKArcRepresentation.h"
#include "vtkCMBArc.h"
#include "vtkCMBArcManager.h"
#include "vtkCMBArcProvider.h"
#include "vtkContourWidget.h"

vtkStandardNewMacro(vtkCMBArcEditClientOperator);

//---------------------------------------------------------------------------
vtkCMBArcEditClientOperator::vtkCMBArcEditClientOperator()
  : ArcIsClosed(false)
{
}

//---------------------------------------------------------------------------
vtkCMBArcEditClientOperator::~vtkCMBArcEditClientOperator()
{
}

//----------------------------------------------------------------------------
bool vtkCMBArcEditClientOperator::Operate(
  vtkSMProxy* sourceProxy, vtkSMNewWidgetRepresentationProxy* widgetProxy)
{
  if (!sourceProxy || !widgetProxy)
  {
    return false;
  }

  bool closed = this->ArcIsClosed;
  this->ArcIsClosed = false;

  vtkCMBArcProvider* provider = vtkCMBArcProvider::SafeDownCast(sourceProxy->GetClientSideObject());
  if (!provider)
  {
    return false;
  }

  vtkContourWidget* widget = vtkContourWidget::SafeDownCast(widgetProxy->GetWidget());
  vtkSMTKArcRepresentation* widgetRep =
    vtkSMTKArcRepresentation::SafeDownCast(widget->GetRepresentation());

  //reset the widget
  widgetRep->SetLoggingEnabled(0);
  widget->SetEnabled(true);
  widgetRep->ClearAllNodes(); //Clear nodes clears out the enable
  widget->SetEnabled(true);
  widget->Initialize();
  // enable it again because Initialize() may have disabled it
  widget->SetEnabled(true);
  widgetRep->VisibilityOn();
  //set the arc to have the shape of the provider
  widget->Initialize(provider->GetOutput(), 1);
  widgetRep->SetClosedLoop(closed);

  //we now have to set the end nodes on the widget
  //this will always be the first and last
  widgetRep->SetNthNodeSelected(0);
  if (!closed)
  {
    widgetRep->SetNthNodeSelected(widgetRep->GetNumberOfNodes() - 1);
  }

  int canEdit = 1;
  if (!closed)
  {
    //now set if the arc can only have modify mode
    //the is done by checking if the seond end node is used
    //by anything, if so it can't be edited or points can be inserted
    //ignore arcs that are closed
    vtkCMBArc* arc = vtkCMBArcManager::GetInstance()->GetArc(provider->GetArcId());
    canEdit = arc->GetNumberOfConnectedArcs(1) == 0 ? 1 : 0;
  }
  widgetRep->SetCanEdit(canEdit);

  //reenable logging so that we can find out if the end nodes have changed
  widgetRep->SetLoggingEnabled(1);

  return true;
}
//----------------------------------------------------------------------------
void vtkCMBArcEditClientOperator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkQtCMBConnectComboBox.h"
#include "vtkEventQtSlotConnect.h"

//-----------------------------------------------------------------------------
vtkQtCMBConnectComboBox::vtkQtCMBConnectComboBox(QWidget* p)
  : QComboBox(p)
{
  this->VTKUserEventConnect = vtkSmartPointer<vtkEventQtSlotConnect>::New();
}

//-----------------------------------------------------------------------------
vtkQtCMBConnectComboBox::~vtkQtCMBConnectComboBox()
{
}

//----------------------------------------------------------------------------
void vtkQtCMBConnectComboBox::showPopup()
{
  emit this->startingPopup();

  this->Superclass::showPopup();
}

//----------------------------------------------------------------------------
void vtkQtCMBConnectComboBox::setVTKConnectObject(vtkObject* obj, unsigned long evt)
{
  if (obj)
  {
    this->VTKUserEventConnect->Disconnect();
    this->VTKUserEventConnect->Connect(obj, evt, this, SLOT(onVTKEventInvoked()));
  }
}

//----------------------------------------------------------------------------
void vtkQtCMBConnectComboBox::onVTKEventInvoked()
{
  emit this->vtkObjectEventInvoked();
}

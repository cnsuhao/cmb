/*=========================================================================

  Module:    SimBuilderUIPanel.h,v

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME SimBuilderUIPanel - a user interface panel.
// .SECTION Description
// .SECTION See Also

#ifndef __SimBuilderUIPanel_h
#define __SimBuilderUIPanel_h

#include <QDockWidget>
#include "cmbSystemConfig.h"

class smtkUIManager;
class vtkDiscreteModel;
class vtkSMProxy;

class SimBuilderUIPanel : public QDockWidget
{
  Q_OBJECT

public:         
  SimBuilderUIPanel(QWidget* pW=NULL);
  virtual ~SimBuilderUIPanel();  

  QWidget* panelWidget();
  void initialize();

private:

  QWidget* ContainerWidget;
};

#endif


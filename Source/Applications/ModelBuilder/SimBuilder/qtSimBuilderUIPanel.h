/*=========================================================================

  Module:    qtSimBuilderUIPanel.h,v

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME qtSimBuilderUIPanel - a user interface panel.
// .SECTION Description
// .SECTION See Also

#ifndef __qtSimBuilderUIPanel_h
#define __qtSimBuilderUIPanel_h

#include <QDockWidget>
#include "cmbSystemConfig.h"

class qtSimBuilderUIPanel : public QDockWidget
{
  Q_OBJECT

public:         
  qtSimBuilderUIPanel(QWidget* pW=NULL);
  virtual ~qtSimBuilderUIPanel();  

  QWidget* panelWidget();
  void initialize();

private:

  QWidget* ContainerWidget;
};

#endif


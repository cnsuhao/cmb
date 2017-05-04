//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include <QDockWidget>

class ExampleDockPanel : public QDockWidget
{
  Q_OBJECT
  typedef QDockWidget Superclass;

public:
  ExampleDockPanel(const QString& t, QWidget* p = 0, Qt::WindowFlags f = 0)
    : Superclass(t, p, f)
  {
    this->constructor();
  }
  ExampleDockPanel(QWidget* p = 0, Qt::WindowFlags f = 0)
    : Superclass(p, f)
  {
    this->constructor();
  }

private:
  void constructor();
};

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include <QActionGroup>

class MyToolBarActions : public QActionGroup
{
  Q_OBJECT
public:
  MyToolBarActions(QObject* p);
  ~MyToolBarActions();

public slots:
  void onAction();

};

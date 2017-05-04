//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __pqCMBPluginStarter_h
#define __pqCMBPluginStarter_h

#include "cmbSystemConfig.h"
#include <QObject>

class pqCMBPluginStarter : public QObject
{
  Q_OBJECT
  typedef QObject Superclass;

public:
  pqCMBPluginStarter(QObject* p = 0);
  ~pqCMBPluginStarter() override;

  // Callback for shutdown.
  void onShutdown();

  // Callback for startup.
  void onStartup();

private:
  pqCMBPluginStarter(const pqCMBPluginStarter&); // Not implemented.
  void operator=(const pqCMBPluginStarter&);     // Not implemented.
};

#endif

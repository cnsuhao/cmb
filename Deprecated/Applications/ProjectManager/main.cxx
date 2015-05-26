//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include <QApplication>
#include "cmbProjectManagerConfig.h"
#include "pqApplicationCore.h"
#include "qtCMBProjectManager.h"

int main(int argc, char* argv[])
{
  QApplication qtapp(argc, argv);
  pqApplicationCore appCore(argc, argv);
  qtapp.setApplicationVersion(ProjectManager_VERSION_FULL);
  qtapp.setOrganizationName("Kitware");
  qtapp.setApplicationName("CMB Project Manager");

  qtCMBProjectManager pm;
  pm.show();

  return qtapp.exec();

}

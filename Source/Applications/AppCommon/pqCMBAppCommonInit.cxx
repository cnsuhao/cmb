//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "pqCMBAppCommonInit.h"
#include "pqComponentsInit.h"
#include <QObject> // for Q_INIT_RESOURCE

void pqCMBAppCommonInit()
{
#ifndef BUILD_SHARED_LIBS
  // init dependents
  // pqCoreInit();
  pqComponentsInit();

  // init resources
  Q_INIT_RESOURCE(cmbAppCommon);

#endif
}

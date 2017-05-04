//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "pqCMBPluginStarter.h"

// Server Manager Includes.
#include "vtkClientServerInterpreter.h"
#include "vtkProcessModule.h"

// Qt Includes.
//#include <QtDebug>

// ParaView Includes.

//----------------------------------------------------------------------------
// ClientServer wrapper initialization functions.
//extern "C" void vtkCMBVTKExtensions_Initialize(vtkClientServerInterpreter*);
//----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
pqCMBPluginStarter::pqCMBPluginStarter(QObject* p /*=0*/)
  : QObject(p)
{
}

//-----------------------------------------------------------------------------
pqCMBPluginStarter::~pqCMBPluginStarter()
{
}

//-----------------------------------------------------------------------------
void pqCMBPluginStarter::onStartup()
{
  // FIXME SEB Don't need that anymore // vtkCMBVTKExtensionsCS_Initialize(vtkProcessModule::GetProcessModule()->GetInterpreter());
}

//-----------------------------------------------------------------------------
void pqCMBPluginStarter::onShutdown()
{
  //qWarning() << "Message from pqCMBPluginStarter: Application Shutting down";
}

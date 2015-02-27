/*=========================================================================

Program:   CMB
Module:    pqCMBPluginStarter.cxx

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
=========================================================================*/
#include "pqCMBPluginStarter.h"

// Server Manager Includes.
#include "vtkProcessModule.h"
#include "vtkClientServerInterpreter.h"

// Qt Includes.
//#include <QtDebug>

// ParaView Includes.

//----------------------------------------------------------------------------
// ClientServer wrapper initialization functions.
//extern "C" void vtkCMBVTKExtensions_Initialize(vtkClientServerInterpreter*);
//----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
pqCMBPluginStarter::pqCMBPluginStarter(QObject* p/*=0*/)
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

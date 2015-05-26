//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "pqCMBServerResource.h"

#include "pqServerResource.h"
#include <QFileInfo>

//-----------------------------------------------------------------------------
pqCMBServerResource::pqCMBServerResource(QObject* p) :
  pqServerResources(p)
{
}

//-----------------------------------------------------------------------------
pqCMBServerResource::~pqCMBServerResource()
{
}

//-----------------------------------------------------------------------------
void pqCMBServerResource::initResourceData(
  pqServerResource& resource)
{
  resource.addData("cmbmodelgroup", "CMBModelGroup");
  resource.addData("readeroperator", "CMBModelReader");
}

//-----------------------------------------------------------------------------
void pqCMBServerResource::open(
  pqServer* server, const pqServerResource& resource)
{
  if (!resource.path().isEmpty())
    {
    QFileInfo fInfo(resource.path());

    QString readerGroup = resource.data("cmbmodelgroup");
    QString readerName = resource.data("readeroperator");

    if ((!readerName.isEmpty() && !readerGroup.isEmpty()) ||
      fInfo.suffix().toLower() == "cmb")
      {
      emit this->openCMBResource(server, resource);
      return;
      }
    }

  this->Superclass::open(server, resource);
}

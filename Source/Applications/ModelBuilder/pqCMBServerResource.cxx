/*=========================================================================

  Program:   CMB
  Module:    pqCMBServerResource.cxx

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

/*=========================================================================

  Program:   CMB
  Module:    pqCMBServerResource.h

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
// .NAME pqCMBServerResource - a CMB server resource object.
// .SECTION Description
// This class is based on Paraview's pqServerResources class
// .SECTION Caveats

#ifndef _pqCMBServerResource_h
#define _pqCMBServerResource_h

#include "pqServerResources.h"
#include "cmbSystemConfig.h"

class pqServer;
class pqServerResource;

class pqCMBServerResource :  public pqServerResources
{
  typedef pqServerResources Superclass;
  Q_OBJECT

public:
  pqCMBServerResource(QObject* p);
  ~pqCMBServerResource();

  // Description:
  // Add a resource to the collection
  virtual void initResourceData(pqServerResource& resource);

  // Description:
  // Open a resource on the given server
  virtual void open(pqServer* server, const pqServerResource& resource);

signals:
  // Description:
  // Signal emitted whenever trying to open a new server resource
  void openCMBResource(pqServer* server, const pqServerResource& resource);

private:
  pqCMBServerResource(const pqCMBServerResource&);
  pqCMBServerResource& operator=(const pqCMBServerResource&);

};

#endif

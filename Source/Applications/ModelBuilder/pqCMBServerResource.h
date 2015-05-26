//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
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

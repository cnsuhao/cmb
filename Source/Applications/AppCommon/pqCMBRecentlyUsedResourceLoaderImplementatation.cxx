//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "pqCMBRecentlyUsedResourceLoaderImplementatation.h"

#include "pqCMBCommonMainWindowCore.h"

#include "pqInterfaceTracker.h"
#include <pqApplicationCore.h>
#include <pqRecentlyUsedResourcesList.h>
#include <pqServer.h>
#include <pqServerResource.h>
#include <pqStandardRecentlyUsedResourceLoaderImplementation.h>

//-----------------------------------------------------------------------------
pqCMBRecentlyUsedResourceLoaderImplementatation::pqCMBRecentlyUsedResourceLoaderImplementatation(
  pqCMBCommonMainWindowCore* parentObject)
  : Superclass(pqApplicationCore::instance()->interfaceTracker())
  , Impl(new pqStandardRecentlyUsedResourceLoaderImplementation())
  , Core(parentObject)
{
}

//-----------------------------------------------------------------------------
pqCMBRecentlyUsedResourceLoaderImplementatation::~pqCMBRecentlyUsedResourceLoaderImplementatation()
{
  delete this->Impl;
}

//-----------------------------------------------------------------------------
bool pqCMBRecentlyUsedResourceLoaderImplementatation::canLoad(const pqServerResource& resource)
{
  if (resource.hasData("CMB_MODEL_FILE"))
  {
    return true;
  }
  return this->Impl->canLoad(resource);
}

//-----------------------------------------------------------------------------
bool pqCMBRecentlyUsedResourceLoaderImplementatation::load(
  const pqServerResource& resource, pqServer* server)
{
  if (resource.hasData("CMB_MODEL_FILE"))
  {
    return this->loadModel(resource, server);
  }

  return this->Impl->load(resource, server);
}

//-----------------------------------------------------------------------------
QIcon pqCMBRecentlyUsedResourceLoaderImplementatation::icon(const pqServerResource& resource)
{
  if (resource.hasData("CMB_MODEL_FILE"))
  {
    // FIXME: use a better icon.
    return QIcon(":/pqWidgets/Icons/pqMultiBlockData16.png");
  }

  return this->Impl->icon(resource);
}

//-----------------------------------------------------------------------------
QString pqCMBRecentlyUsedResourceLoaderImplementatation::label(const pqServerResource& resource)
{
  if (resource.hasData("CMB_MODEL_FILE"))
  {
    return resource.path();
  }
  return this->Impl->label(resource);
}

//-----------------------------------------------------------------------------
bool pqCMBRecentlyUsedResourceLoaderImplementatation::addDataFilesToRecentResources(
  pqServer* server, const QStringList& files, const QString& smgroup, const QString& smname)
{
  return pqStandardRecentlyUsedResourceLoaderImplementation::addDataFilesToRecentResources(
    server, files, smgroup, smname);
}

//-----------------------------------------------------------------------------
bool pqCMBRecentlyUsedResourceLoaderImplementatation::addDataFileToRecentResources(
  pqServer* server, const QString& file, const QString& smgroup, const QString& smname)
{
  QStringList files;
  files << file;
  return pqCMBRecentlyUsedResourceLoaderImplementatation::addDataFilesToRecentResources(
    server, files, smgroup, smname);
}

//-----------------------------------------------------------------------------
bool pqCMBRecentlyUsedResourceLoaderImplementatation::addModelFileToRecentResources(
  pqServer* server, const QString& filename)
{
  pqServerResource resource = server->getResource();
  resource.setPath(filename);
  resource.addData("CMB_MODEL_FILE", "1");
  resource.setPath(filename);
  resource.addData("modelmanager", "pqCMBModelManager");
  resource.addData("readoperator", "read");

  pqApplicationCore* core = pqApplicationCore::instance();
  core->recentlyUsedResources().add(resource);
  core->recentlyUsedResources().save(*core->settings());
  return true;
}

//-----------------------------------------------------------------------------
bool pqCMBRecentlyUsedResourceLoaderImplementatation::loadModel(
  const pqServerResource& resource, pqServer* server)
{
  (void)server;
  QString readerGroup = resource.data("modelmanager");
  QString readerName = resource.data("readoperator");
  if ((!readerName.isEmpty() && !readerGroup.isEmpty()) && this->Core)
  {
    QStringList files;
    files << resource.path();
    this->Core->onFileOpen(files);
    return true;
  }
  return false;
}

/*=========================================================================

   Program: ParaView
   Module:  pqCMBRecentlyUsedResourceLoaderImplementatation.cxx

   Copyright (c) 2005,2006 Sandia Corporation, Kitware Inc.
   All rights reserved.

   ParaView is a free software; you can redistribute it and/or modify it
   under the terms of the ParaView license version 1.2.

   See License_v1.2.txt for the full ParaView license.
   A copy of this license can be obtained by contacting
   Kitware Inc.
   28 Corporate Drive
   Clifton Park, NY 12065
   USA

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

========================================================================*/
#include "pqCMBRecentlyUsedResourceLoaderImplementatation.h"

#include "pqCMBCommonMainWindowCore.h"

#include <pqApplicationCore.h>
#include <pqRecentlyUsedResourcesList.h>
#include <pqServer.h>
#include <pqServerResource.h>
#include <pqStandardRecentlyUsedResourceLoaderImplementation.h>
#include "pqInterfaceTracker.h"

//-----------------------------------------------------------------------------
pqCMBRecentlyUsedResourceLoaderImplementatation::
    pqCMBRecentlyUsedResourceLoaderImplementatation(
        pqCMBCommonMainWindowCore *parentObject)
    : Superclass(pqApplicationCore::instance()->interfaceTracker()),
      Impl(new pqStandardRecentlyUsedResourceLoaderImplementation()),
      Core(parentObject) {}

//-----------------------------------------------------------------------------
pqCMBRecentlyUsedResourceLoaderImplementatation::
    ~pqCMBRecentlyUsedResourceLoaderImplementatation() {
  delete this->Impl;
}

//-----------------------------------------------------------------------------
bool pqCMBRecentlyUsedResourceLoaderImplementatation::canLoad(
    const pqServerResource &resource) {
  if (resource.hasData("CMB_MODEL_FILE")) {
    return true;
  }
  return this->Impl->canLoad(resource);
}

//-----------------------------------------------------------------------------
bool pqCMBRecentlyUsedResourceLoaderImplementatation::load(
    const pqServerResource &resource, pqServer *server) {
  if (resource.hasData("CMB_MODEL_FILE")) {
    return this->loadModel(resource, server);
  }

  return this->Impl->load(resource, server);
}

//-----------------------------------------------------------------------------
QIcon pqCMBRecentlyUsedResourceLoaderImplementatation::icon(
    const pqServerResource &resource) {
  if (resource.hasData("CMB_MODEL_FILE")) {
    // FIXME: use a better icon.
    return QIcon(":/pqWidgets/Icons/pqMultiBlockData16.png");
  }

  return this->Impl->icon(resource);
}

//-----------------------------------------------------------------------------
QString pqCMBRecentlyUsedResourceLoaderImplementatation::label(
    const pqServerResource &resource) {
  if (resource.hasData("CMB_MODEL_FILE")) {
    return resource.path();
  }
  return this->Impl->label(resource);
}

//-----------------------------------------------------------------------------
bool pqCMBRecentlyUsedResourceLoaderImplementatation::
    addDataFilesToRecentResources(pqServer *server, const QStringList &files,
                                  const QString &smgroup,
                                  const QString &smname) {
  return pqStandardRecentlyUsedResourceLoaderImplementation::
      addDataFilesToRecentResources(server, files, smgroup, smname);
}

//-----------------------------------------------------------------------------
bool pqCMBRecentlyUsedResourceLoaderImplementatation::
    addDataFileToRecentResources(pqServer *server, const QString &file,
                                 const QString &smgroup,
                                 const QString &smname) {
  QStringList files;
  files << file;
  return pqCMBRecentlyUsedResourceLoaderImplementatation::
      addDataFilesToRecentResources(server, files, smgroup, smname);
}

//-----------------------------------------------------------------------------
bool pqCMBRecentlyUsedResourceLoaderImplementatation::
    addModelFileToRecentResources(pqServer *server, const QString &filename) {
  pqServerResource resource = server->getResource();
  resource.setPath(filename);
  resource.addData("CMB_MODEL_FILE", "1");
  resource.setPath(filename);
  resource.addData("modelmanager", "pqCMBModelManager");
  resource.addData("readoperator", "read");

  pqApplicationCore *core = pqApplicationCore::instance();
  core->recentlyUsedResources().add(resource);
  core->recentlyUsedResources().save(*core->settings());
  return true;
}

//-----------------------------------------------------------------------------
bool pqCMBRecentlyUsedResourceLoaderImplementatation::loadModel(
    const pqServerResource &resource, pqServer *server) {
  QString readerGroup = resource.data("modelmanager");
  QString readerName = resource.data("readoperator");
  if ((!readerName.isEmpty() && !readerGroup.isEmpty()) && this->Core) {
    QStringList files;
    files << resource.path();
    this->Core->onFileOpen(files);
    return true;
  }
  return false;
}

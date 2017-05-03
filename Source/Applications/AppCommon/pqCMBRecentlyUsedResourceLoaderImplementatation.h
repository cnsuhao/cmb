//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef pqCMBRecentlyUsedResourceLoaderImplementatation_h
#define pqCMBRecentlyUsedResourceLoaderImplementatation_h

#include "cmbAppCommonExport.h"
#include <QObject>
#include <QPointer>
#include <pqRecentlyUsedResourceLoaderInterface.h>

class pqServer;
class pqStandardRecentlyUsedResourceLoaderImplementation;
class pqCMBCommonMainWindowCore;

/// pqCMBRecentlyUsedResourceLoaderImplementatation adds support for
/// resources that can be added **Recent Files** menu.
///
/// Current implementation simply forwards to
/// pqStandardRecentlyUsedResourceLoaderImplementation used by ParaView. As
/// needed CMB should update this class.
class CMBAPPCOMMON_EXPORT pqCMBRecentlyUsedResourceLoaderImplementatation
  : public QObject,
    public pqRecentlyUsedResourceLoaderInterface
{
  Q_OBJECT
  Q_INTERFACES(pqRecentlyUsedResourceLoaderInterface)
  typedef QObject Superclass;

public:
  pqCMBRecentlyUsedResourceLoaderImplementatation(pqCMBCommonMainWindowCore* parent = 0);
  virtual ~pqCMBRecentlyUsedResourceLoaderImplementatation();

  virtual bool canLoad(const pqServerResource& resource);
  virtual bool load(const pqServerResource& resource, pqServer* server);
  virtual QIcon icon(const pqServerResource& resource);
  virtual QString label(const pqServerResource& resource);

  /// Please clean up these names, current names are just placeholders to
  /// illustrate the concept. One should have a new `add-` method for every
  /// category of item.
  static bool addDataFilesToRecentResources(
    pqServer* server, const QStringList& files, const QString& smgroup, const QString& smname);
  static bool addDataFileToRecentResources(
    pqServer* server, const QString& file, const QString& smgroup, const QString& smname);
  static bool addModelFileToRecentResources(pqServer* server, const QString& filename);

protected:
  bool loadModel(const pqServerResource& resource, pqServer* server);

private:
  Q_DISABLE_COPY(pqCMBRecentlyUsedResourceLoaderImplementatation);
  pqStandardRecentlyUsedResourceLoaderImplementation* Impl;
  QPointer<pqCMBCommonMainWindowCore> Core;
};

#endif

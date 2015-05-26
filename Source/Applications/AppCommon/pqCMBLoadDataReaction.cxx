//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "pqCMBLoadDataReaction.h"

#include "pqActiveObjects.h"
#include "pqApplicationCore.h"
#include "pqCoreUtilities.h"
#include "pqFileDialog.h"
#include "pqLoadDataReaction.h"
#include "pqObjectBuilder.h"
#include "pqPipelineSource.h"
#include "pqRecentlyUsedResourcesList.h"
#include "pqSelectReaderDialog.h"
#include "pqServer.h"
#include "pqServerResource.h"
#include "pqUndoStack.h"
#include "vtkSMProxy.h"
#include "vtkSMProxyManager.h"
#include "vtkSMReaderFactory.h"

#include <QDebug>
#include <QMapIterator>

#include "vtkStringList.h"
class pqCMBLoadDataReaction::cmbInternals
{
public:
  // These are handled by CMB Applications, not paraview readers.
  QStringList CMBSpecialExtensions;
};

//-----------------------------------------------------------------------------
pqCMBLoadDataReaction::pqCMBLoadDataReaction(QAction* parentObject,
  bool multiFiles) : Superclass(parentObject), m_MultiFiles(multiFiles)
{
  this->Internals = new cmbInternals;
  pqActiveObjects* activeObjects = &pqActiveObjects::instance();
  QObject::connect(activeObjects, SIGNAL(serverChanged(pqServer*)),
    this, SLOT(updateEnableState()));
  this->updateEnableState();
}

//-----------------------------------------------------------------------------
pqCMBLoadDataReaction::~pqCMBLoadDataReaction()
{
  delete this->Internals;
}

//-----------------------------------------------------------------------------
void pqCMBLoadDataReaction::addSpecialExtensions(const QStringList& exts)
{
  foreach(QString ext, exts)
    {
    this->Internals->CMBSpecialExtensions << ext;
    }
}

//-----------------------------------------------------------------------------
void pqCMBLoadDataReaction::updateEnableState()
{
  pqActiveObjects& activeObjects = pqActiveObjects::instance();
  bool enable_state = (activeObjects.activeServer() != NULL);
  this->parentAction()->setEnabled(enable_state);
}

//-----------------------------------------------------------------------------
void pqCMBLoadDataReaction::setPluginIOBehavior(pqPluginIOBehavior* pluginBhv)
{
  this->m_pluginBehavior = pluginBhv;
}
//-----------------------------------------------------------------------------
QList<pqPipelineSource*> pqCMBLoadDataReaction::loadData(
  bool& cancelled, QStringList& files)
{
  return pqCMBLoadDataReaction::loadData(cancelled, files,
    this->m_fileTypes, this->m_programDir,
    this->m_pluginBehavior, this->Internals->CMBSpecialExtensions,
    this->m_MultiFiles, this->m_ReaderExtensionMap);
}

//-----------------------------------------------------------------------------
void pqCMBLoadDataReaction::onTriggered()
{
  bool cancelled = true;
  QStringList files;
  if(this->loadData(cancelled, files).size() == 0 && !cancelled
     && files.size() > 0)
    {
    // if the files are not handled here does not have proper readers,
    // fire a signal, so that applications have a chance to handle it.
    emit this->filesSelected(files);
    }
}

//-----------------------------------------------------------------------------
bool pqCMBLoadDataReaction::isSpecialExtension(const QStringList& files)
{
  return this->isSpecialExtension(
    files, this->Internals->CMBSpecialExtensions);
}

//-----------------------------------------------------------------------------
bool pqCMBLoadDataReaction::isSpecialExtension(const QStringList& files,
    const QStringList& specialExts)
{
  if(files.size() == 0 || specialExts.size() == 0)
    {
    return false;
    }
  foreach(QString file, files)
    {
    QFileInfo finfo(file);
    QString s = finfo.completeSuffix().toLower();
    foreach(QString filetype, specialExts)
      {
      int idx = filetype.indexOf('(');
      QString specialExt = filetype.mid(idx + 1, filetype.indexOf(')') - idx -1);
      if(specialExt.contains(s, Qt::CaseInsensitive))
        {
        return true;
        }
      }
    }
  return false;
}

//-----------------------------------------------------------------------------
QList<pqPipelineSource*> pqCMBLoadDataReaction::loadData( bool& cancelled,
  QStringList& selfiles, const QString& fileTypes,
  const QString& pgmDir, pqPluginIOBehavior* pluginBhv,
  const QStringList& specialExts, bool multiFiles,
  const pqCMBLoadDataReaction::FileExtMap& readerExtensionMap)
{
  pqServer* server = pqActiveObjects::instance().activeServer();
  QString filters = pluginBhv ?
    pluginBhv->supportedFileTypes(server->session()) : "";
  filters = filters.isEmpty() ? fileTypes : (filters + ";;" + fileTypes);
  // Added in special extensions that come in form bridge plugins, etc
  foreach(QString filetype, specialExts)
    {
    filters += ";;";
    filters += filetype;
    }
  pqFileDialog fileDialog(server,
    pqCoreUtilities::mainWidget(),
    tr("Open File:"), pgmDir, filters);
  fileDialog.setObjectName("FileOpenDialog");
  fileDialog.setFileMode(multiFiles ?
    pqFileDialog::ExistingFiles : pqFileDialog::ExistingFile);
  cancelled = true;

  QList<pqPipelineSource*> sources;
  if (fileDialog.exec() == QDialog::Accepted)
    {
    cancelled = false;
    QList<QStringList> files = fileDialog.getAllSelectedFiles();
    QStringList file;
    foreach(file,files)
      {
      selfiles = file; // only get the last file
      pqPipelineSource* source = pqCMBLoadDataReaction::openFiles(
        file, specialExts, readerExtensionMap);
      if(source)
        {
        sources << source;
        }
      }
    }
  return sources;
}

//-----------------------------------------------------------------------------
pqPipelineSource* pqCMBLoadDataReaction::openFiles(const QStringList& files,
  const QStringList& specialExts,
  const pqCMBLoadDataReaction::FileExtMap &readerExtensionMap)
{
  if(pqCMBLoadDataReaction::isSpecialExtension(files, specialExts))
    {
//    emit pqApplicationCore::instance()->getObjectBuilder()->readerCreated(
//      NULL, files[0]);
    return NULL;
    }
  bool cmbReaders = false;
  foreach(QString file, files)
    {
    QFileInfo finfo(file);
    QString s = finfo.completeSuffix().toLower();
    if(readerExtensionMap.keys().contains(s))
      {
      return pqCMBLoadDataReaction::openFile(files,
        readerExtensionMap[s].first,
        readerExtensionMap[s].second);
      }
    }

  pqServer* server = pqActiveObjects::instance().activeServer();
  if(files.size() > 0 && server)
    {
    return pqLoadDataReaction::loadData(files);
    }
  return NULL;
}

//-----------------------------------------------------------------------------
pqPipelineSource* pqCMBLoadDataReaction::openFile(
  const QStringList& files, const QString& group, const QString& readername)
{
  pqObjectBuilder* builder =
    pqApplicationCore::instance()->getObjectBuilder();
  pqServer* server = pqActiveObjects::instance().activeServer();
  pqPipelineSource* reader = builder->createReader(group,
    readername, files, server);
  if (reader)
    {
    pqApplicationCore* core = pqApplicationCore::instance();

    // Add this to the list of recent server resources ...
    pqServerResource resource = server->getResource();
    resource.setPath(files[0]);
    resource.addData("readergroup", reader->getProxy()->GetXMLGroup());
    resource.addData("reader", reader->getProxy()->GetXMLName());
    resource.addData("extrafilesCount", QString("%1").arg(files.size()-1));
    for (int cc=1; cc < files.size(); cc++)
      {
      resource.addData(QString("file.%1").arg(cc-1), files[cc]);
      }
    core->recentlyUsedResources().add(resource);
    core->recentlyUsedResources().save(*core->settings());
    }

  return reader;
}

//-----------------------------------------------------------------------------
void pqCMBLoadDataReaction::addReaderExtensionMap(
  const pqCMBLoadDataReaction::FileExtMap &readerMap)
{
  QMapIterator<QString, QPair<QString, QString> > it(readerMap);
  while(it.hasNext())
    {
    it.next();
    this->addReaderExtensionMap(
      it.key(), it.value().first, it.value().second);
    }
}

//-----------------------------------------------------------------------------
void pqCMBLoadDataReaction::addReaderExtensionMap(const QString &fileext,
  const QString &readergroup, const QString &readername)
{
  m_ReaderExtensionMap.insert(fileext, QPair<QString, QString>(
                              readergroup, readername));
}

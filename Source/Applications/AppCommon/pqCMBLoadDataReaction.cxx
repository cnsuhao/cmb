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
#include "pqCMBFileDialog.h"
#include "pqCMBRecentlyUsedResourceLoaderImplementatation.h"
#include "pqCoreUtilities.h"
#include "pqLoadDataReaction.h"
#include "pqObjectBuilder.h"
#include "pqPipelineSource.h"
#include "pqSelectReaderDialog.h"
#include "pqServer.h"
#include "pqUndoStack.h"
#include "vtkSMProxy.h"
#include "vtkSMProxyManager.h"
#include "vtkSMReaderFactory.h"

#include <smtk/extension/vtk/reader/vtkLASReader.h>

#include <QDebug>
#include <QMapIterator>

#include "vtkStringList.h"
class pqCMBLoadDataReaction::cmbInternals
{
public:
  // These are handled by CMB Applications, not paraview readers.
  QStringList CMBSpecialExtensions;
};

pqCMBLoadDataReaction::pqCMBLoadDataReaction(QAction* parentObject, bool multiFiles)
  : Superclass(parentObject)
  , m_MultiFiles(multiFiles)
{
  this->Internals = new cmbInternals;
  pqActiveObjects* activeObjects = &pqActiveObjects::instance();
  QObject::connect(
    activeObjects, SIGNAL(serverChanged(pqServer*)), this, SLOT(updateEnableState()));
  this->updateEnableState();
}

pqCMBLoadDataReaction::~pqCMBLoadDataReaction()
{
  delete this->Internals;
}

void pqCMBLoadDataReaction::addSpecialExtensions(const QStringList& exts)
{
  foreach (QString ext, exts)
  {
    this->Internals->CMBSpecialExtensions << ext;
  }
}

void pqCMBLoadDataReaction::updateEnableState()
{
  pqActiveObjects& activeObjects = pqActiveObjects::instance();
  bool enable_state = (activeObjects.activeServer() != NULL);
  this->parentAction()->setEnabled(enable_state);
}

void pqCMBLoadDataReaction::setPluginIOBehavior(pqPluginIOBehavior* pluginBhv)
{
  this->m_pluginBehavior = pluginBhv;
}

QList<pqPipelineSource*> pqCMBLoadDataReaction::loadData(bool& cancelled, QStringList& files)
{
  return pqCMBLoadDataReaction::loadData(cancelled, files, this->m_fileTypes, this->m_programDir,
    this->m_pluginBehavior, this->Internals->CMBSpecialExtensions, this->m_MultiFiles,
    this->m_ReaderExtensionMap, this);
}

void pqCMBLoadDataReaction::onTriggered()
{
  bool cancelled = true;
  QStringList files;
  if (this->loadData(cancelled, files).size() == 0 && !cancelled && files.size() > 0)
  {
    // if the files are not handled here does not have proper readers,
    // fire a signal, so that applications have a chance to handle it.
    emit this->filesSelected(files);
  }
}

bool pqCMBLoadDataReaction::isSpecialExtension(const QStringList& files)
{
  return this->isSpecialExtension(files, this->Internals->CMBSpecialExtensions);
}

bool pqCMBLoadDataReaction::isSpecialExtension(
  const QStringList& files, const QStringList& specialExts)
{
  if (files.size() == 0 || specialExts.size() == 0)
  {
    return false;
  }
  foreach (QString file, files)
  {
    QFileInfo finfo(file);
    QString s = finfo.completeSuffix().toLower();
    foreach (QString filetype, specialExts)
    {
      int idx = filetype.indexOf('(');
      QString specialExt = filetype.mid(idx + 1, filetype.indexOf(')') - idx - 1);
      if (specialExt.contains(s, Qt::CaseInsensitive))
      {
        return true;
      }
    }
  }
  return false;
}

QList<pqPipelineSource*> pqCMBLoadDataReaction::loadData(bool& cancelled, QStringList& selfiles,
  const QString& fileTypes, const QString& pgmDir, pqPluginIOBehavior* pluginBhv,
  const QStringList& specialExts, bool multiFiles,
  const pqCMBLoadDataReaction::FileExtMap& readerExtensionMap, pqCMBLoadDataReaction* reaction)
{
  pqServer* server = pqActiveObjects::instance().activeServer();
  QString filters = pluginBhv ? pluginBhv->supportedFileTypes(server->session()) : "";
  filters = filters.isEmpty() ? fileTypes : (filters + ";;" + fileTypes);
  // Added in special extensions that come in form bridge plugins, etc
  foreach (QString filetype, specialExts)
  {
    filters += ";;";
    filters += filetype;
  }
  pqCMBFileDialog fileDialog(
    server, pqCoreUtilities::mainWidget(), tr("Open File:"), pgmDir, filters);
  if (reaction != NULL)
  {
    connect(&fileDialog, SIGNAL(currentSelectedFilesChanged(QStringList const&)), reaction,
      SLOT(getFileHeader(QStringList const&)));
    connect(reaction, SIGNAL(fileheaders(QStringList const&)), &fileDialog,
      SLOT(setMetadata(QStringList const&)));
  }
  fileDialog.setObjectName("FileOpenDialog");
  fileDialog.setFileMode(
    multiFiles ? pqCMBFileDialog::ExistingFiles : pqCMBFileDialog::ExistingFile);
  cancelled = true;

  QList<pqPipelineSource*> sources;
  if (fileDialog.exec() == QDialog::Accepted)
  {
    cancelled = false;
    QList<QStringList> files = fileDialog.getAllSelectedFiles();
    if (reaction && files.size() != 0)
    {
      reaction->sendMultiFileLoad();
    }
    QStringList file;
    foreach (file, files)
    {
      selfiles = file; // only get the last file
      pqPipelineSource* source =
        pqCMBLoadDataReaction::openFiles(file, specialExts, readerExtensionMap);
      if (source)
      {
        sources << source;
      }
    }
    if (reaction && files.size() != 0)
    {
      reaction->sendDoneMultiFileLoad();
    }
  }
  return sources;
}

void pqCMBLoadDataReaction::getFileHeader(QStringList const& files)
{
  QStringList headers;
  foreach (QString f, files)
  {
    QFileInfo fi(f);
    std::string fs = f.toStdString();
    if (fi.isFile())
    {
      if (fi.suffix().toLower() == "las")
      {
        vtkLASReader* l = vtkLASReader::New();
        l->SetFileName(fs.c_str());
        std::string s = l->GetHeaderInfo();
        headers.append(s.c_str());
        l->Delete();
      }
      else
      {
        headers.append("");
      }
    }
  }
  emit fileheaders(headers);
}

pqPipelineSource* pqCMBLoadDataReaction::openFiles(const QStringList& files,
  const QStringList& specialExts, const pqCMBLoadDataReaction::FileExtMap& readerExtensionMap)
{
  if (pqCMBLoadDataReaction::isSpecialExtension(files, specialExts))
  {
    //    emit pqApplicationCore::instance()->getObjectBuilder()->readerCreated(
    //      NULL, files[0]);
    return NULL;
  }
  foreach (QString file, files)
  {
    QFileInfo finfo(file);
    QString s = finfo.completeSuffix().toLower();
    if (readerExtensionMap.keys().contains(s))
    {
      return pqCMBLoadDataReaction::openFile(
        files, readerExtensionMap[s].first, readerExtensionMap[s].second);
    }
  }

  pqServer* server = pqActiveObjects::instance().activeServer();
  if (files.size() > 0 && server)
  {
    return pqLoadDataReaction::loadData(files);
  }
  return NULL;
}

pqPipelineSource* pqCMBLoadDataReaction::openFile(
  const QStringList& files, const QString& group, const QString& readername)
{
  pqObjectBuilder* builder = pqApplicationCore::instance()->getObjectBuilder();
  pqServer* server = pqActiveObjects::instance().activeServer();
  pqPipelineSource* reader = builder->createReader(group, readername, files, server);
  if (reader)
  {
    // For now, just using ParaView's code. One can add support for new types
    // of recent-files items as needed, by creating a subclass of
    // pqRecentlyUsedResourceLoaderInterface, and registering that with the
    // pqInterfaceTracker.
    pqCMBRecentlyUsedResourceLoaderImplementatation::addDataFilesToRecentResources(
      server, files, reader->getProxy()->GetXMLGroup(), reader->getProxy()->GetXMLName());
  }
  return reader;
}

void pqCMBLoadDataReaction::addReaderExtensionMap(
  const pqCMBLoadDataReaction::FileExtMap& readerMap)
{
  QMapIterator<QString, QPair<QString, QString> > it(readerMap);
  while (it.hasNext())
  {
    it.next();
    this->addReaderExtensionMap(it.key(), it.value().first, it.value().second);
  }
}

void pqCMBLoadDataReaction::addReaderExtensionMap(
  const QString& fileext, const QString& readergroup, const QString& readername)
{
  m_ReaderExtensionMap.insert(fileext, QPair<QString, QString>(readergroup, readername));
}

void pqCMBLoadDataReaction::sendMultiFileLoad()
{
  emit(multiFileLoad());
}

void pqCMBLoadDataReaction::sendDoneMultiFileLoad()
{
  emit(doneMultiFileLoad());
}

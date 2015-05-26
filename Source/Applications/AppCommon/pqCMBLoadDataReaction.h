//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __pqLoadModelReaction_h
#define __pqLoadModelReaction_h

#include "pqReaction.h"
#include "cmbAppCommonExport.h"
#include "cmbSystemConfig.h"
#include "pqPluginIOBehavior.h"
#include <QStringList>
#include <QPointer>
#include <QMap>
#include <QPair>

class pqPipelineSource;

/// @ingroup Reactions
/// Reaction for open data files.
class CMBAPPCOMMON_EXPORT pqCMBLoadDataReaction : public pqReaction
{
  Q_OBJECT
  typedef pqReaction Superclass;
  typedef QMap<QString, QPair<QString, QString> > FileExtMap;
public:
  /// Constructor. Parent cannot be NULL.
  pqCMBLoadDataReaction(QAction* parent, bool multiFiles = false);
  ~pqCMBLoadDataReaction();

  /// Set the filters for supported file types, which will be used in the
  /// file open dialog as file extensions. The format should be like this
  /// "LIDAR (*.pts *.bin *.bin.pts);;LAS (*.las);;All files (*)"
  virtual void setSupportedFileTypes(const QString& fileTypes)
  { this->m_fileTypes = fileTypes; }

  /// Set the default directory for the file open dialog.
  virtual void setProgramDirectory(const QString& pgmDir)
  { this->m_programDir = pgmDir; }

  /// Set the default directory for the file open dialog.
  virtual void setPluginIOBehavior(pqPluginIOBehavior*);

  /// Set option whether the file open dialog can have multiple selections
  virtual void setMultiFiles(bool val)
  { this->m_MultiFiles = val; }

  /// Launch the file open dialog, which will also include
  /// file extensions from plugins if available
  /// NOTE: if files are part of cmb special extensions, this reaction
  /// will not try to open the files. It assumes the application should
  /// handle these special extensions.
  /// The arguments are for firing a signal if selected files are not handled
  /// so that the application could handle them.
  QList<pqPipelineSource*> loadData(bool& cancelled, QStringList& files);
  static QList<pqPipelineSource*> loadData(bool& cancelled, QStringList& selfiles,
    const QString& fileTypes,
    const QString& pgmDir = QString(),
    pqPluginIOBehavior* pluginBhv = NULL,
    const QStringList& specialExtensions = QStringList(),
    bool multiFiles = false,
    const FileExtMap& readerExtensionMap = FileExtMap());

  /// Loads data files. Uses reader factory to determine what reader are
  /// supported. Returns the reader is creation successful, otherwise returns
  /// NULL.
  /// NOTE: if files are part of cmb special extensions, this reaction
  /// will not try to open the files. It assumes the application should
  /// handle these special extensions.
  static pqPipelineSource* openFiles(const QStringList& files,
    const QStringList& specialExts = QStringList(),
    const FileExtMap& readerExtensionMap = FileExtMap());
  static pqPipelineSource* openFile(
  const QStringList& files, const QString& group, const QString& readername);

  /// Check if the files is part of the special extensions.
  bool isSpecialExtension(const QStringList& files);
  static bool isSpecialExtension(const QStringList& files,
    const QStringList& specialExts);

  /// Add reader extensions that will be used before those coming from
  /// paraview SM configure xml
  /// <file_extension, <reader_group, reader_name> >
  void addReaderExtensionMap(const FileExtMap &readerMap);
  void addReaderExtensionMap(const QString &fileext,
    const QString &readergroup, const QString &readername);

public slots:
  /// Updates the enabled state. Applications need not explicitly call
  /// this.
  virtual void updateEnableState();

  /// These extensions are handled by CMB Applications
  /// or bridge-plugins, not traditional SM vtk readers.
  /// For Example:
  /// [cmb] is an extension from vtk discrete model bridge loaded
  /// in ModelBuilder, and there is no paraview reader for that.
  /// it is read in by an model operator.
  virtual void addSpecialExtensions(const QStringList& exts);

signals:
  /// fire a signal when the files selected can not be loaded here,
  /// so that applications may have some special readers to handle.
  void filesSelected(const QStringList& files);

protected:
  /// Called when the action is triggered.
  virtual void onTriggered();

  /// Internal parameters
  QPointer<pqPluginIOBehavior> m_pluginBehavior;
  QString m_fileTypes;
  QString m_programDir;
  bool m_MultiFiles;
  FileExtMap m_ReaderExtensionMap;

private:
  Q_DISABLE_COPY(pqCMBLoadDataReaction)
  class cmbInternals;
  cmbInternals* Internals;

};

#endif

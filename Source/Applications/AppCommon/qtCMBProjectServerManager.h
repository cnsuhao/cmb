//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __qtCMBProjectServerManager_h
#define __qtCMBProjectServerManager_h

#include "cmbAppCommonExport.h"
#include <QObject>
#include <QScopedPointer>
#include <QString>
#include "cmbSystemConfig.h"

class vtkSMProxy;

class CMBAPPCOMMON_EXPORT qtCMBProjectServerManager : public QObject
{
  friend class qtCMBProjectManager;
  Q_OBJECT
public:
  qtCMBProjectServerManager( );
  qtCMBProjectServerManager(QString const& path);
  ~qtCMBProjectServerManager();

  enum PROGRAM
  {
  PointsBuilder = 0,
  SceneBuilder,
  ModelBuilder,
  SimulationBuilder,
  GeologyBuilder,
  NUM_PROGRAMS
  };

  bool isOpen() const;
  bool isModified() const;
  QString const& filePath() const;
  bool haveProgramDirectory(qtCMBProjectServerManager::PROGRAM const& program) const;
  QString const& programDirectory(qtCMBProjectServerManager::PROGRAM const& program) const;
  QString const& programName(qtCMBProjectServerManager::PROGRAM const& program) const;
  QRegExp const& programFileRegExp(qtCMBProjectServerManager::PROGRAM const& program) const;


  //ToDo: This needs to be done on the server, instead of the client
  bool programDirectoryHasFiles(qtCMBProjectServerManager::PROGRAM const& program) const;

signals:
  void projectModified(bool);
  void projectLoaded();
  void projectSaved();
  void projectClosed();
  void updateUI();
  void resetUI();

protected:
  void initSignalsSlots();

  QString getDefaultDirectoryForProgram(qtCMBProjectServerManager::PROGRAM const &program);
  void SetProgramDirectory(qtCMBProjectServerManager::PROGRAM const &program, QString const& directory);

  void serverOpenProject(QString const& project);
  void serverSaveProjectAs(QString const& project);
  void serverResetProject();

  void newProject();
  void closeProject();

  void ProjectModified(bool mod);
  void ProjectLoaded();
  void ProjectSaved();
  void ProjectClosed();

  void pullFromServer();


private:
  //active project info
  struct ProjectDetails;
  QScopedPointer<ProjectDetails> ActiveProject;

  //active server project manager
  struct ScopedPointerVTKDeleter;
  QScopedPointer<vtkSMProxy, ScopedPointerVTKDeleter > ProjectManagerProxy;

  Q_DISABLE_COPY(qtCMBProjectServerManager)
};
#endif

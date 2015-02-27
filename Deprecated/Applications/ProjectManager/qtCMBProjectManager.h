/*=========================================================================

  Program:   Visualization Toolkit
  Module:    qtCMBProjectManager.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __qtCMBProjectManager_h
#define __qtCMBProjectManager_h

#include "cmbProjectManagerConfig.h"
#include "qtCMBProjectServerManager.h"
#include <QMainWindow>
#include <QString>
#include "cmbSystemConfig.h"

class vtkSMProxy;

namespace Ui { class qtCMBProjectManagerMainWindow; }

class qtCMBProjectManager : public QMainWindow
{
  Q_OBJECT
public:
  qtCMBProjectManager();
  ~qtCMBProjectManager();

signals:
  void updateUI();

protected slots:
  void newProject();
  void openProject();
  void saveProject();
  void saveProjectAs();
  void closeProject();

  void spawnPointsBuilder();
  void spawnSceneBuilder();
  void spawnMeshBuilder();
  void spawnSimBuilder();
  void spawnProgram(qtCMBProjectServerManager::PROGRAM const& program);

  void onProjectModified(bool const& mod);
  void onProjectLoaded();
  void onProjectSaved();
  void onProjectClosed();
  void onUpdateUI();
  void onResetUI();

protected:
  void closeEvent(QCloseEvent *event);

  //method that spawns the dialog to choose where want projects
  //to be saved if they don't already have a folder
  bool launchNewProjectDialog(qtCMBProjectServerManager::PROGRAM const &program);

  //method allows the user to select a file and load it
  bool launchSelectFileToLoad(qtCMBProjectServerManager::PROGRAM const &program
    , QString &fileToLoad);

private:
  //active server project manager
  qtCMBProjectServerManager ProjectManager;

  Q_DISABLE_COPY(qtCMBProjectManager)
  Ui::qtCMBProjectManagerMainWindow* const Ui;
};
#endif

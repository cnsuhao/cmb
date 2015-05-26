//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "qtCMBProjectManager.h"
#include <QtGui>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QProcessEnvironment>

#include "pqAlwaysConnectedBehavior.h"
#include "pqApplicationCore.h"
#include "pqFileDialog.h"
#include "pqFileDialogModel.h"
#include "pqServer.h"
#include "pqServerConnectReaction.h"
#include "pqServerDisconnectReaction.h"
#include "qtCMBProjectDirectoryDialog.h"
#include "qtCMBProjectFileToLoadDialog.h"
#include "vtkPVPlugin.h"

#include "ui_qtProjectManager.h"
PV_PLUGIN_IMPORT_INIT(CMB_Plugin);

//-----------------------------------------------------------------------------
qtCMBProjectManager::qtCMBProjectManager():
  QMainWindow(0),
  ProjectManager(),
  Ui(new Ui::qtCMBProjectManagerMainWindow())
{
  new pqAlwaysConnectedBehavior(this); //this must be done first, so that we have a builtin pv server
  PV_PLUGIN_IMPORT(CMB_Plugin);

  this->Ui->setupUi(this);
  QObject::connect(this->Ui->PointsButton,
            SIGNAL(clicked(bool)),
            this,
            SLOT(spawnPointsBuilder()));

  QObject::connect(this->Ui->SceneButton,
            SIGNAL(clicked(bool)),
            this,
            SLOT(spawnSceneBuilder()));

  QObject::connect(this->Ui->ModelButton,
            SIGNAL(clicked(bool)),
            this,
            SLOT(spawnMeshBuilder()));

  QObject::connect(this->Ui->SimButton,
            SIGNAL(clicked(bool)),
            this,
            SLOT(spawnSimBuilder()));

  //setup the file menu connections
  QObject::connect(this->Ui->actionNewProject,
    SIGNAL(triggered()), this, SLOT(newProject()));

  QObject::connect(this->Ui->actionOpen,
    SIGNAL(triggered()), this, SLOT(openProject()));

  QObject::connect(this->Ui->actionSave,
    SIGNAL(triggered()), this, SLOT(saveProject()));

  QObject::connect(this->Ui->actionSaveAs,
    SIGNAL(triggered()), this, SLOT(saveProjectAs()));

  QObject::connect(this->Ui->actionClose,
    SIGNAL(triggered()), this, SLOT(closeProject()));

  QObject::connect(this->Ui->actionQuit,
    SIGNAL(triggered()), this, SLOT(close()));

  //setup the connect and disconnect actions
  new pqServerConnectReaction(this->Ui->actionServerConnect);
  new pqServerDisconnectReaction(this->Ui->actionServerDisconnect);

  //setup syncing the User Interface signals /slots
  QObject::connect(&this->ProjectManager, SIGNAL(projectModified(bool)),this,SLOT(onProjectModified(bool)));
  QObject::connect(&this->ProjectManager, SIGNAL(projectLoaded()),this,SLOT(onProjectLoaded()));
  QObject::connect(&this->ProjectManager, SIGNAL(projectSaved()),this,SLOT(onProjectSaved()));
  QObject::connect(&this->ProjectManager, SIGNAL(projectClosed()),this,SLOT(onProjectClosed()));
  QObject::connect(&this->ProjectManager, SIGNAL(updateUI()),this,SLOT(onUpdateUI()));
  QObject::connect(&this->ProjectManager, SIGNAL(resetUI()),this,SLOT(onResetUI()));

}

//-----------------------------------------------------------------------------
qtCMBProjectManager::~qtCMBProjectManager()
{
}


//-----------------------------------------------------------------------------
void qtCMBProjectManager::newProject()
{
  if (this->ProjectManager.isOpen())
    {
    this->closeProject();
    }
  this->ProjectManager.newProject();
}

//-----------------------------------------------------------------------------
void qtCMBProjectManager::openProject()
{
  if (this->ProjectManager.isOpen())
    {
    this->closeProject();
    }

  //spawn a pqFileDialog
  pqFileDialog dialog(pqApplicationCore::instance()->getActiveServer(),
                    this,"Open CMB Project File","",
                    "*.cpf");
  dialog.setFileMode(pqFileDialog::ExistingFile);
  if (dialog.exec() == QDialog::Accepted)
    {
    QString selectedFile = dialog.getSelectedFiles()[0];
    this->ProjectManager.serverOpenProject(selectedFile);
    }
}

//-----------------------------------------------------------------------------
void qtCMBProjectManager::saveProject()
{
  if ( this->ProjectManager.isOpen() &&
       this->ProjectManager.isModified() )
    {
    //we pass the current file path to the server so it will overwrite it
    //with the modifications
    this->ProjectManager.serverSaveProjectAs(this->ProjectManager.filePath());
    }
  else if ( !this->ProjectManager.isOpen() && this->ProjectManager.isModified() )
    {
    this->saveProjectAs();
    }
}

  //-----------------------------------------------------------------------------
void qtCMBProjectManager::saveProjectAs()
{
  //ask the user to save the project to a file
  pqFileDialog dialog(pqApplicationCore::instance()->getActiveServer(),
      this,"Save As CMB Project File","",
      "*.cpf");
    dialog.setFileMode(pqFileDialog::AnyFile);
  if (dialog.exec() == QDialog::Accepted)
    {
    QString selectedFile = dialog.getSelectedFiles()[0];
    this->ProjectManager.serverSaveProjectAs(selectedFile);
    }
}

//-----------------------------------------------------------------------------
void qtCMBProjectManager::closeProject()
{
  this->saveProject();
  this->ProjectManager.closeProject();
}

//-----------------------------------------------------------------------------
void qtCMBProjectManager::closeEvent(QCloseEvent *event)
 {
   event->accept();
 }

//-----------------------------------------------------------------------------
void qtCMBProjectManager::spawnPointsBuilder()
{
  this->spawnProgram(qtCMBProjectServerManager::PointsBuilder);
}

//-----------------------------------------------------------------------------
void qtCMBProjectManager::spawnSceneBuilder()
{
  this->spawnProgram(qtCMBProjectServerManager::SceneBuilder);
}

//-----------------------------------------------------------------------------
void qtCMBProjectManager::spawnMeshBuilder()
{
  this->spawnProgram(qtCMBProjectServerManager::ModelBuilder);
}

//-----------------------------------------------------------------------------
void qtCMBProjectManager::spawnSimBuilder()
{
  this->spawnProgram(qtCMBProjectServerManager::SimulationBuilder);
}

//-----------------------------------------------------------------------------
void qtCMBProjectManager::spawnProgram(
                            qtCMBProjectServerManager::PROGRAM const& program)
{

  //check and see if we have a new project on our hands
  this->saveProject();

  bool canLaunch = true;
  bool newProjectFolder = false;
  //we need to be able to generalize this whole infastructure
  if ( !this->ProjectManager.haveProgramDirectory(program) )
    {
    //we need to ask the user if they want to use the default
    //directory or specify the directory
    canLaunch = this->launchNewProjectDialog(program);
    newProjectFolder = true;
    }

  if (!canLaunch)
    {
    return;
    }

  //call save so that the project file on the server is up to date
  //need to be done before we get the list of files in the program folder
  this->saveProject();

  QString fileToLoad;
  if (!newProjectFolder)
    {
    canLaunch = this->launchSelectFileToLoad(program,fileToLoad);
    }

  if (!canLaunch)
    {
    return;
    }

  //determine the program executable path
  QDir currentDir = QCoreApplication::applicationDirPath();
  QString programName;
  switch(program)
    {
    case qtCMBProjectServerManager::PointsBuilder:
      programName = "PointsBuilder";
      break;
    case qtCMBProjectServerManager::SceneBuilder:
      programName ="SceneBuilder";
      break;
    case qtCMBProjectServerManager::ModelBuilder:
    case qtCMBProjectServerManager::SimulationBuilder:
     programName = "ModelBuilder";
      break;
    case qtCMBProjectServerManager::NUM_PROGRAMS:
    default:
      programName = "";
      break;
    }

#if defined(Q_WS_MAC)
  //need the app suffix for it work properly on MAC OSX
  programName += ".app";
#elif defined(Q_WS_WIN)
  programName += ".exe";
#endif

  bool canCdUp = true;
  while(!currentDir.exists(programName) && canCdUp)
    {
    //walk up the directory tree intill we find the program
    canCdUp = currentDir.cdUp();
    }
  if (!canCdUp)
    {
    //todo: push a message that we couldn't find the program
    return;
    }

  //TODO: Currently ParaView branding does not support
  //a custom pqOptions class so we have to use system env property
  //to pass the name of the project to the launched program.

  //The program will than reparse the xml file and use the information
  //in it for itself, so we have to make sure the project file is fully
  //updated, and not open on the current server
  QProcessEnvironment pEnv;
  pEnv.insert("CMB_PROJECT_FILE_PATH",this->ProjectManager.filePath());
  if ( fileToLoad.size() != 0 )
    {
    pEnv.insert("CMB_FILE_TO_LOAD",fileToLoad);
    }

  //we need to be able to track if
  //a program has been launched, and how it returns ( crash / etc )
  QProcess *process = new QProcess();
  process->setProcessEnvironment(pEnv);
  // We need to put quotes around the program path, otherwise QProcess::start
  // won't work if there are spaces in the path.
  QString progPath = "\"" + currentDir.absoluteFilePath(programName) + "\"";
  process->start(progPath);
}

//-----------------------------------------------------------------------------
bool qtCMBProjectManager::launchNewProjectDialog(
  qtCMBProjectServerManager::PROGRAM const &program)
{

  //get from the server the default directory for this program
  QString defaultDir = this->ProjectManager.getDefaultDirectoryForProgram(
    program);
  QString programName = this->ProjectManager.programName(program);

  qtCMBProjectDirectoryDialog dialog(programName,defaultDir,this);
  bool launch = (dialog.exec() == QDialog::Accepted );
  if ( launch )
    {
    //now update the server with the selected directory as that programs directory
    this->ProjectManager.SetProgramDirectory(
      program, dialog.getSelectedDirectory());
    }
  return launch;
}

//-----------------------------------------------------------------------------
bool qtCMBProjectManager::launchSelectFileToLoad(
  qtCMBProjectServerManager::PROGRAM const &program, QString &fileToLoad)
{

  //get from the server the default directory for this program
  QString directory = this->ProjectManager.programDirectory(program);
  QString programName = this->ProjectManager.programName(program);
  QRegExp programReg = this->ProjectManager.programFileRegExp(program);

  //need to launch the dialog and query the file system for all the files that
  //match the programs list of supported file types
  pqFileDialogModel *model = new pqFileDialogModel(
    pqApplicationCore::instance()->getActiveServer());
  model->setCurrentPath(directory);

  qtCMBProjectFileToLoadDialog dialog(programName,programReg,model,this);
  bool launch = (dialog.exec() == QDialog::Accepted ) && dialog.fileChosen();
  if (launch)
    {
    fileToLoad = dialog.getFileToLoad();
    }

  delete model;
  return launch;
}


//-----------------------------------------------------------------------------
void qtCMBProjectManager::onProjectModified(bool const& mod)
{
  this->Ui->actionSave->setEnabled(mod);
}

//-----------------------------------------------------------------------------
void qtCMBProjectManager::onProjectLoaded( )
{
  //update the file actions
  this->Ui->actionClose->setEnabled(true);
  this->Ui->actionSaveAs->setEnabled(true);
}

//-----------------------------------------------------------------------------
void qtCMBProjectManager::onProjectSaved()
{


}

//-----------------------------------------------------------------------------
void qtCMBProjectManager::onProjectClosed()
{
  this->Ui->actionClose->setEnabled(false);
  this->Ui->actionSaveAs->setEnabled(false);
}

//-----------------------------------------------------------------------------
void qtCMBProjectManager::onUpdateUI()
{
  QString shownName = QFileInfo(this->ProjectManager.filePath()).fileName();
  bool modified = this->ProjectManager.isModified();

  this->setWindowFilePath(shownName);
  this->setWindowModified(modified);

  if ( this->ProjectManager.isOpen() )
    {
    bool pb = this->ProjectManager.programDirectoryHasFiles(
      qtCMBProjectServerManager::PointsBuilder);
    bool sb = this->ProjectManager.programDirectoryHasFiles(
      qtCMBProjectServerManager::SceneBuilder);
    bool mb = this->ProjectManager.programDirectoryHasFiles(
      qtCMBProjectServerManager::ModelBuilder);
    bool ib = this->ProjectManager.programDirectoryHasFiles(
      qtCMBProjectServerManager::SimulationBuilder);

    this->Ui->SimButton->setEnabled((mb || ib));

    this->Ui->PointsArrow->setEnabled(pb);
    this->Ui->SceneArrow->setEnabled(sb);
    this->Ui->ModelArrow->setEnabled(mb);
    }
  else
    {
    //default layout of the buttons and arrows
    this->Ui->SimButton->setEnabled(false);

    this->Ui->PointsArrow->setEnabled(false);
    this->Ui->SceneArrow->setEnabled(false);
    this->Ui->ModelArrow->setEnabled(false);
    }

  this->Ui->PointsButton->setEnabled(true);
  this->Ui->SceneButton->setEnabled(true);
  this->Ui->ModelButton->setEnabled(true);
}

//-----------------------------------------------------------------------------
void qtCMBProjectManager::onResetUI()
{
  this->Ui->SimButton->setEnabled(false);
  this->Ui->PointsButton->setEnabled(false);
  this->Ui->SceneButton->setEnabled(false);
  this->Ui->ModelButton->setEnabled(false);

  this->Ui->PointsArrow->setEnabled(false);
  this->Ui->SceneArrow->setEnabled(false);
  this->Ui->ModelArrow->setEnabled(false);
}

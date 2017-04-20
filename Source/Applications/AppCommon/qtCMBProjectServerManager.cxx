//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "qtCMBProjectServerManager.h"
#include <QDir>
#include <QProcess>
#include <QProcessEnvironment>

#include "pqApplicationCore.h"
#include "pqServer.h"
#include "pqSMAdaptor.h"
#include "vtkProcessModule.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxy.h"
#include "vtkSMProxyManager.h"
#include "vtkSMSessionProxyManager.h"

//-----------------------------------------------------------------------------
struct qtCMBProjectServerManager::ScopedPointerVTKDeleter
{
 template<typename T>
 static inline void cleanup(T *pointer)
  {
  if(pointer)
    { pointer->Delete(); }

  }
};

//-----------------------------------------------------------------------------
struct qtCMBProjectServerManager::ProjectDetails
{
  ProjectDetails( )
    : Open(false),
    Modified(false),
    ProjectFilePath("New Project")
    {
    init();
    }
  ProjectDetails( QString const& fp )
    : Open(true),
    Modified(false),
    ProjectFilePath(fp)
    {
    init();
    }

  void init()
    {
    ProgramNames[0]="Points Builder";
    ProgramNames[1]="Scene Builder";
    ProgramNames[2]="Model Builder";
    ProgramNames[3]="Simulation Builder";

    ProgramFileRegExps[0]=QRegExp(".*.(?:pts|bin|pts.bin|las)$",
      Qt::CaseInsensitive,QRegExp::RegExp2);
    ProgramFileRegExps[1]=QRegExp(".*.(?:sg|osd.txt|map)$",
      Qt::CaseInsensitive,QRegExp::RegExp2);
    ProgramFileRegExps[2]=QRegExp(".*.(?:cmb|vtk|vtu|2dm|3dm|sol|stl|map)$",
      Qt::CaseInsensitive,QRegExp::RegExp2);
    ProgramFileRegExps[3]=QRegExp(".*.(?:simb.xml)$",
      Qt::CaseInsensitive,QRegExp::RegExp2);
    }

  bool Open;
  bool Modified;
  QString ProjectFilePath;
  QString ProgramNames[qtCMBProjectServerManager::NUM_PROGRAMS];
  QString ProgramDirs[qtCMBProjectServerManager::NUM_PROGRAMS];
  QRegExp ProgramFileRegExps[qtCMBProjectServerManager::NUM_PROGRAMS];
};


//-----------------------------------------------------------------------------
qtCMBProjectServerManager::qtCMBProjectServerManager():
  ActiveProject(new ProjectDetails()),
  ProjectManagerProxy()
{
}

//-----------------------------------------------------------------------------
qtCMBProjectServerManager::qtCMBProjectServerManager(QString const& path):
  ActiveProject(new ProjectDetails(path)),
  ProjectManagerProxy()
{
}

//-----------------------------------------------------------------------------
qtCMBProjectServerManager::~qtCMBProjectServerManager()
{
}


//-----------------------------------------------------------------------------
bool qtCMBProjectServerManager::isOpen() const
{
  return this->ActiveProject->Open;
}

//-----------------------------------------------------------------------------
bool qtCMBProjectServerManager::isModified() const
{
  return this->ActiveProject->Modified;
}

//-----------------------------------------------------------------------------
QString const& qtCMBProjectServerManager::filePath() const
{
  return this->ActiveProject->ProjectFilePath;
}

//-----------------------------------------------------------------------------
bool qtCMBProjectServerManager::haveProgramDirectory(
  qtCMBProjectServerManager::PROGRAM const& program) const
{
  return (this->ActiveProject->ProgramDirs[program].size() != 0);
}

//-----------------------------------------------------------------------------
QString const& qtCMBProjectServerManager::programDirectory(
  qtCMBProjectServerManager::PROGRAM const& program) const
{
  return this->ActiveProject->ProgramDirs[program];
}

//-----------------------------------------------------------------------------
QString const& qtCMBProjectServerManager::programName(
  qtCMBProjectServerManager::PROGRAM const& program) const
{
  return this->ActiveProject->ProgramNames[program];
}

//-----------------------------------------------------------------------------
QRegExp const& qtCMBProjectServerManager::programFileRegExp(
  qtCMBProjectServerManager::PROGRAM const& program) const
{
  return this->ActiveProject->ProgramFileRegExps[program];
}

//-----------------------------------------------------------------------------
bool qtCMBProjectServerManager::programDirectoryHasFiles(
  qtCMBProjectServerManager::PROGRAM const& program) const
{
  //ToDo: This needs to be done on the server, instead of the client
  // look at the directory and see if any file matches the regexp
  // we have for that folder
  if (!this->haveProgramDirectory(program))
    {
    return false;
    }

  QDir dir(this->programDirectory(program));
  dir.setFilter(QDir::Files | QDir::Readable); //only check files that we can read

  QString file;
  QStringList files = dir.entryList();
  foreach(file,files)
    {
    if (this->programFileRegExp(program).exactMatch(file))
      {
      return true;
      }
    }
  return false;
}

//-----------------------------------------------------------------------------
void qtCMBProjectServerManager::newProject( )
{
  bool didExist = !this->ActiveProject.isNull();
  this->ActiveProject.reset( new ProjectDetails() );
  if(didExist)
    {
    this->serverResetProject();
    }

  this->ProjectModified(true);
  emit this->updateUI();
}

//-----------------------------------------------------------------------------
void qtCMBProjectServerManager::closeProject( )
{
  if ( this->ActiveProject )
    {
    this->ActiveProject.reset( new ProjectDetails() );
    this->serverResetProject();
    emit this->resetUI();
    }
}

//-----------------------------------------------------------------------------
void qtCMBProjectServerManager::ProjectModified(bool mod)
{
  if ( mod != this->ActiveProject->Modified )
    {
    this->ActiveProject->Modified = mod;
    emit this->projectModified(mod);
    emit this->updateUI();
    }
}

//-----------------------------------------------------------------------------
void qtCMBProjectServerManager::ProjectLoaded( )
{
  //update the file actions
  this->ActiveProject->Open = true;
  this->ProjectModified(false);
  emit this->projectLoaded();
}

//-----------------------------------------------------------------------------
void qtCMBProjectServerManager::ProjectSaved()
{
  this->ActiveProject->Open = true;
  this->ProjectModified(false);
  emit this->projectSaved();
}

//-----------------------------------------------------------------------------
void qtCMBProjectServerManager::ProjectClosed()
{
  this->ActiveProject->Open = false;
  this->ProjectModified(false);
  emit this->projectClosed();
}

//-----------------------------------------------------------------------------
void qtCMBProjectServerManager::serverOpenProject(QString const& project)
{
  //create the reader on the server, which uses the singelton project manager
  vtkSMProxyManager* pm = vtkSMProxyManager::GetProxyManager();
  vtkSMSessionProxyManager* pxm = pm->GetActiveSessionProxyManager();
  if(pxm)
    {
    vtkSMProxy *readerProxy = pxm->NewProxy("utilities","ProjectManagerReader");
    readerProxy->SetLocation(vtkProcessModule::DATA_SERVER);
    pqSMAdaptor::setElementProperty(readerProxy->GetProperty("ProjectFile"),
      project.toStdString().c_str() );
    readerProxy->UpdateProperty("ProjectFile");
    readerProxy->InvokeCommand("ReadProjectFile");
    readerProxy->Delete();

    this->pullFromServer();
    }

  this->ProjectLoaded();
}

//-----------------------------------------------------------------------------
void qtCMBProjectServerManager::serverSaveProjectAs(QString const& project)
{
  //than we save the project file out
  vtkSMProxyManager* pm = vtkSMProxyManager::GetProxyManager();
  vtkSMSessionProxyManager* pxm = pm->GetActiveSessionProxyManager();
  if(pxm)
    {
    vtkSMProxy *readerProxy = pxm->NewProxy("utilities", "ProjectManagerWriter");
    readerProxy->SetLocation(vtkProcessModule::DATA_SERVER);
    pqSMAdaptor::setElementProperty(readerProxy->GetProperty("ProjectFile"),
      project.toStdString().c_str() );
    readerProxy->UpdateProperty("ProjectFile");
    readerProxy->InvokeCommand("WriteProjectFile");
    readerProxy->Delete();
    }

  if ( this->ActiveProject->ProjectFilePath != project )
    {
    //update the information from the server
    this->serverOpenProject(project);
    this->pullFromServer();
    }

  this->ProjectSaved();
}

//-----------------------------------------------------------------------------
void qtCMBProjectServerManager::serverResetProject( )
{
  if(this->ProjectManagerProxy)
    {
    this->ProjectManagerProxy->InvokeCommand("ResetProject");
    this->ProjectManagerProxy->UpdateVTKObjects();
    this->pullFromServer();
    }

  this->ProjectClosed();
}


//-----------------------------------------------------------------------------
QString qtCMBProjectServerManager::getDefaultDirectoryForProgram(
  qtCMBProjectServerManager::PROGRAM const &program)
{
  //create the project manager on the data server
  vtkSMProxyManager* pm = vtkSMProxyManager::GetProxyManager();
  vtkSMSessionProxyManager* pxm = pm->GetActiveSessionProxyManager();
  if(pxm && this->ProjectManagerProxy.isNull())
    {
    this->ProjectManagerProxy.reset(
                              pxm->NewProxy("utilities", "ProjectManager"));
    this->ProjectManagerProxy->SetLocation(vtkProcessModule::DATA_SERVER);
    this->ProjectManagerProxy->UpdateVTKObjects();
    }

  //todo: We still need a better way to do this
  vtkSMPropertyHelper helper(this->ProjectManagerProxy.data(), "ActiveProgram");
  helper.Set(program);

  //fetch from the project manager if it is manaing any programs
  vtkSMPropertyHelper helperPD(this->ProjectManagerProxy.data(),
    "ActiveDefaultProgramDirectory");
  helperPD.UpdateValueFromServer();
  QString directory = helperPD.GetAsString();
  return directory;
}

//-----------------------------------------------------------------------------
void qtCMBProjectServerManager::SetProgramDirectory(
  qtCMBProjectServerManager::PROGRAM const &program, QString const& directory)
{
  //create the project manager on the data server
  vtkSMProxyManager* pm = vtkSMProxyManager::GetProxyManager();
  vtkSMSessionProxyManager* pxm = pm->GetActiveSessionProxyManager();
  if(pxm && this->ProjectManagerProxy.isNull())
    {
    this->ProjectManagerProxy.reset(
                              pxm->NewProxy("utilities", "ProjectManager"));
    this->ProjectManagerProxy->SetLocation(vtkProcessModule::DATA_SERVER);
    this->ProjectManagerProxy->UpdateVTKObjects();
    }

  //todo: We still need a better way to do this
  vtkSMPropertyHelper helper(this->ProjectManagerProxy.data(), "ActiveProgram");
  helper.Set(program);

  vtkSMPropertyHelper helper2(this->ProjectManagerProxy.data(),
    "SetActiveProgramDirectory");
  helper2.Set(directory.toStdString().c_str());
  this->ProjectManagerProxy->UpdateProperty("SetActiveProgramDirectory");
  this->ProjectModified(true);
  this->pullFromServer();
}


//-----------------------------------------------------------------------------
void qtCMBProjectServerManager::pullFromServer()
{
  //create the project manager on the data server
  vtkSMProxyManager* pm = vtkSMProxyManager::GetProxyManager();
  vtkSMSessionProxyManager* pxm = pm->GetActiveSessionProxyManager();
  if(pxm && this->ProjectManagerProxy.isNull())
    {
    this->ProjectManagerProxy.reset(
                              pxm->NewProxy("utilities", "ProjectManager"));
    this->ProjectManagerProxy->SetLocation(vtkProcessModule::DATA_SERVER);
    this->ProjectManagerProxy->UpdateVTKObjects();
    }

  //fetch from the project manager if it is manaing any programs
  vtkSMPropertyHelper helper(this->ProjectManagerProxy.data(), "Active");
  helper.UpdateValueFromServer();
  this->ActiveProject->Open = helper.GetAsInt();
  if ( this->ActiveProject->Open == 0 )
    {
    emit this->updateUI();
    return;
    }

  //fetch from the project manager the file it is based on
  vtkSMPropertyHelper helper2(this->ProjectManagerProxy.data(), "ProjectFile");
  helper2.UpdateValueFromServer();
  this->ActiveProject->ProjectFilePath = QString(helper.GetAsString());


  //fetch from the project manager the current state of all the programs
  for ( int i=0; i < NUM_PROGRAMS; ++i)
    {
    vtkSMPropertyHelper helper3(this->ProjectManagerProxy.data(), "ActiveProgram");
    helper3.Set(i);

    vtkSMPropertyHelper helper4(this->ProjectManagerProxy.data(),
      "ActiveProgramDirectory");
    helper4.UpdateValueFromServer();
    QString pd = helper4.GetAsString();
    this->ActiveProject->ProgramDirs[i] = QString(pd);

    }
  emit this->updateUI();
}

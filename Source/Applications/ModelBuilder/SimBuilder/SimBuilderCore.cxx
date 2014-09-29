/*=========================================================================

Copyright (c) 1998-2003 Kitware Inc. 469 Clifton Corporate Parkway,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced, distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/

#include "SimBuilderCore.h"

#include "SimBuilderUIPanel.h"
#include "SimBuilderCustomExportDialog.h"
#include "DefaultExportTemplate.h"

#include "pqApplicationCore.h"
#include "pqObjectBuilder.h"
#include "pqPipelineSource.h"
#include "pqRenderView.h"
#include "pqServer.h"
#include "pqServerResource.h"
#include "pqRecentlyUsedResourcesList.h"
#include "pqFileDialog.h"
#include "pqFileDialogModel.h"
#include "pqSMAdaptor.h"

#include "vtkSMSourceProxy.h"
#include "vtkPVSceneGenFileInformation.h"
#include "vtkProcessModule.h"

#include "vtkNew.h"
#include "vtkXMLDataElement.h"
#include "vtkXMLDataParser.h"

#include "../pqCMBModelBuilderOptions.h"

#include "smtk/common/ResourceSet.h"
#include "smtk/io/Logger.h"
#include "smtk/io/AttributeReader.h"
#include "smtk/io/AttributeWriter.h"
#include "smtk/io/ResourceSetReader.h"
#include "smtk/io/ResourceSetWriter.h"
#include "smtk/view/Root.h"
#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/extension/qt/qtRootView.h"

#include "smtkUIManager.h"

#include "vtkSMProxyManager.h"
#include "vtkClientServerStream.h"
#include "vtkSMSession.h"
#include "vtkSMIntVectorProperty.h"

#include <QDir>
#include <QFileInfo>
#include <QLayout>
#include <QMessageBox>
#include <QApplication>
#include <QAbstractButton>
#include <QPushButton>

//----------------------------------------------------------------------------
SimBuilderCore::SimBuilderCore(pqServer* pvServer, pqRenderView* view)
{
  this->UIPanel = NULL;

  this->SimFileVersion = "1.0";

  this->Initialize();
  this->setServer(pvServer);
  this->setRenderView(view);
  this->ExportDialog = new SimBuilderCustomExportDialog();
  this->m_attUIManager = NULL;
}

//----------------------------------------------------------------------------
SimBuilderCore::~SimBuilderCore()
{
  this->clearSimulationModel();
  delete this->ExportDialog;
  if(this->m_attUIManager)
    {
    delete this->m_attUIManager;
    }
}

//----------------------------------------------------------------------------
void SimBuilderCore::Initialize()
{
  this->IsSimModelLoaded = false;
  this->LoadTemplateOnly = false;
  this->LoadingScenario = false;
  this->ScenarioEntitiesCreated = false;
  this->CurrentSimFile = "";
  if(this->m_attUIManager)
    {
    delete this->m_attUIManager;
    }
  this->m_attUIManager = NULL;
}

//----------------------------------------------------------------------------
int SimBuilderCore::LoadSimulation(bool templateOnly, bool isScenario)
{
  QString filters;
  if(isScenario)
    {
    filters= "SimBuilder Scenario Files (*sbs);;";
    }
  else if(templateOnly)
    {
    filters= "SimBuilder Template Files (*crf *sbt);;"
             "SimBuilder Resource Files (*crf);;"
             "SimBuilder Legacy Files (*sbt);;";
    }
  else
    {
    filters= "SimBuilder Instance Files (*crf *sbi);;"
             "SimBuilder Resource Files (*crf);;"
             "SimBuilder Legacy Files (*sbi);;";
    }
  filters +=  "All Files (*)";
  QString startDir = pqCMBModelBuilderOptions::instance()->
    defaultSimBuilderTemplateDirectory().c_str();
  pqFileDialog file_dialog(this->ActiveServer,
    NULL, tr("Open File:"), startDir, filters);

  //file_dialog.setAttribute(Qt::WA_DeleteOnClose);
  file_dialog.setObjectName("FileOpenDialog");
  file_dialog.setFileMode(pqFileDialog::ExistingFile);
  if (file_dialog.exec() == QDialog::Accepted)
    {
    QStringList files = file_dialog.getSelectedFiles();
    if (files.size() > 0)
      {
      return this->LoadSimulation(files[0].toAscii().constData());
      }
    }
  return 0;
}

//----------------------------------------------------------------------------
int SimBuilderCore::LoadSimulation(const char *filename)
{
  if (!filename)
    {
    return 0;
    }

  QFileInfo finfo(filename);
  pqFileDialogModel model(this->ActiveServer);
  QString fullpath;
  pqPipelineSource* reader = NULL;
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();
  QString lastExt = finfo.suffix().toLower();

  if(lastExt== "crf")
    {
    return this->LoadResources(filename);
    }

  if(model.fileExists(filename, fullpath) &&
    ( lastExt== "sbt" || lastExt== "sbs" || lastExt== "sbi" ||
    finfo.completeSuffix().toLower() == "simb.xml")) // for backward compatibility
    {
//    this->LoadTemplateOnly = templateOnly;
//    this->LoadingScenario = isScenario;
    // Load the file and set up the pipeline to display the dataset.
    QStringList files;
    files << filename;
    reader = builder->createReader("sources", "StringReader",
      files, this->ActiveServer);
    }
  if(!reader)
    {
    return 0;
    }
  else
    {
    pqServer* server = reader->getServer();

    // Add this to the list of recent server resources ...
    pqServerResource resource = server->getResource();
    resource.setPath(filename);
    resource.addData("readergroup", reader->getProxy()->GetXMLGroup());
    resource.addData("reader", reader->getProxy()->GetXMLName());
    core->recentlyUsedResources().add(resource);
    core->recentlyUsedResources().save(*core->settings());
    }

  return 1;
}

//----------------------------------------------------------------------------
int SimBuilderCore::LoadSimulation(pqPipelineSource* reader,
                                   pqCMBSceneTree* /*sceneTree*/)
{
  // force read
  vtkSMSourceProxy::SafeDownCast( reader->getProxy() )->UpdatePipeline();
  vtkNew<vtkPVSceneGenFileInformation> info;
  reader->getProxy()->GatherInformation(info.GetPointer());
  QFileInfo finfo(info->GetFileName());
  QString lastExt = finfo.suffix().toLower();

  // Resource file handled separately
  if (lastExt == "crf")
    {
    return this->LoadResources(reader, NULL);
    }

  if(lastExt!= "sbt" && lastExt!= "sbs" && lastExt!= "sbi" &&
    finfo.completeSuffix().toLower() != "simb.xml")
    {
    return 0;
    }
  if(finfo.completeSuffix().toLower() == "simb.xml") // for backward compatibility
    {
    QMessageBox::warning(NULL, "SimBuilder Open File!",
      tr("This is an older version of SimBuilder file.\n") +
      tr("Please rename the file properly with one of the following extensions:\n") +
      tr(".sbt (for Template), .sbi (for Instance), .sbs (for Scenario)"));
    return 0;
    }

  if(this->CurrentSimFile == info->GetFileName() || this->isSimModelLoaded())
    {
    // we should not come in here, since application level logic have checked this.
    if(QMessageBox::question(this->GetUIPanel(),
    "Load Simulation?",
    tr("A SimBuilder file is already loaded. Do you want to close current SimBuilder file?\n") +
    tr("NOTE: all unsaved changes to simulation database will be lost if answering Yes."),
    QMessageBox::Yes|QMessageBox::No, QMessageBox::No) == QMessageBox::No)
      {
      return 0;
      }
    else
      {
      QApplication::processEvents();
      this->clearSimulationModel();
      this->Initialize();
      }
    }

  if(finfo.suffix().toLower() == "sbt")
    {
    this->LoadTemplateOnly = true;
    }
  else if(finfo.suffix().toLower() == "sbs")
    {
    this->LoadingScenario = true;
    }
  else if(finfo.suffix().toLower() == "sbi")
    {
    // loading an instance file.
    }
  else
    {
    return 0;
    }

  // We need to get the file contents from server, then load it into attributes
  smtk::io::AttributeReader xmlr;
  std::string dirPath = finfo.dir().path().toStdString();
  std::vector<std::string> searchPaths;
  searchPaths.push_back(dirPath);
  xmlr.setSearchPaths(searchPaths);

  smtk::io::Logger logger;
  bool errStatus = xmlr.readContents(*(this->attributeUIManager()->attManager()),
                                     info->GetFileContents(), logger);

  if(errStatus)
    {
    vtkGenericWarningMacro("Problems reading " <<  info->GetFileName()
                           << ". The information is: " << logger.convertToString());
    }

  //parse element, create GUI components
  this->attributeUIManager()->initializeUI(
    this->GetUIPanel()->panelWidget(), this);
/*
  vtkDiscreteModel* model = this->CMBModel ? this->CMBModel->getModel() : NULL;
  bool ignoreModel = false;
  if(this->LoadingScenario || (this->CMBModel && !this->CMBModel->hasGeometryEntity()))
    {
    ignoreModel = true;
    }
  this->updateCMBModelWithScenario(false);

  this->updateCMBModelItems(model,
    this->CMBModel ? this->CMBModel->getModelWrapper() : NULL);


  int ok = xmlr.process(this->RootDataContainer, info->GetFileContents(),
    model, sceneTree, this->LoadTemplateOnly, ignoreModel);

  if (errStatus)
    {
    QMessageBox::warning(this->GetUIPanel(),
     "Problem loading SimBuilder file!",
      xmlr.errorMessages().c_str());
    this->Initialize();
    return 0;
    }
*/

  // Update export dialog
  this->ExportDialog->setSimAttManager(
    this->attributeUIManager()->attManager());
  this->setDefaultExportTemplate();

  this->IsSimModelLoaded = true;
  this->CurrentSimFile = info->GetFileName();
/*
  this->CurrentSimFile = xmlr.GetFileName();
  this->CurrentTemplateFile = xmlr.GetTemplateFileName();
  this->SimFileVersion = xmlr.GetVersionNumber();
*/
  emit this->newSimFileLoaded(info->GetFileName());
  return 1;
}

//----------------------------------------------------------------------------
int SimBuilderCore::SaveSimulation(bool writeScenario)
{
  QString filters;
  if(writeScenario)
    {
    filters = "SimBuilder Scenario Files (*.sbs);;";
    }
  else
    {
    filters = "SimBuilder Instance Files (*.crf *.sbi);;"
              "SimBuilder Resource Files (*.crf);;"
              "SimBuilder Legacy Files (*.sbi);;";
    }
  filters += "All files (*)";
  pqFileDialog file_dialog(
                           this->ActiveServer,
                           NULL, tr("Save Simulation:"), QString(), filters);
  file_dialog.setObjectName("FileSaveDialog");
  file_dialog.setFileMode(pqFileDialog::AnyFile);
  if (file_dialog.exec() == QDialog::Accepted)
    {
    QStringList files = file_dialog.getSelectedFiles();
    if (files.size() > 0)
      {
      return this->SaveSimulation(files[0].toAscii().constData(), writeScenario);
      }
    }
  return 0;
}

//----------------------------------------------------------------------------
int SimBuilderCore::SaveSimulation(const char *filename, bool /*writeScenario*/)
{
  // Currently, we can't write out the Template File only, and the writing
  // for Instance File is the same as writing for Scenario file.

  if (!filename)
    {
    return 0;
    }

  std::string filecontents;
  smtk::io::Logger logger;
  bool errStatus;

  QFileInfo finfo(filename);
  if ("crf" == finfo.suffix().toLower())
    {
    // Initialize ResourceSet
    smtk::common::ResourceSet resources;
    smtk::attribute::ManagerPtr simManager =
      this->attributeUIManager()->attManager();
    resources.addResource(simManager, "simbuilder", "",
      smtk::common::ResourceSet::INSTANCE);
    smtk::attribute::ManagerPtr expManager =
      this->ExportDialog->exportAttManager(true);  // use baseline atts
    resources.addResource(expManager, "export", "",
      smtk::common::ResourceSet::TEMPLATE);

    // Serialize ResourceSet
    smtk::io::ResourceSetWriter resourceWriter;
    errStatus = resourceWriter.writeString(filecontents, resources, logger);
    }
  else
    {
    // SimBuilderWriter xmlw;
    smtk::io::AttributeWriter xmlw;
    errStatus = xmlw.writeContents( *(this->attributeUIManager()->attManager()),
                                         filecontents, logger);
    }

  if(errStatus)
    {
    QMessageBox::warning(this->GetUIPanel(),
                         "Problem saving SimBuilder file!",
                         logger.convertToString().c_str());
    std::cerr << logger.convertToString() << std::endl;
    return 0;
    }

  //push the data to the server
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();
  pqPipelineSource *serverWriter = builder->createSource("sources", "StringWriter",
    core->getActiveServer());

  if (!serverWriter)
    {
    QMessageBox::warning(this->GetUIPanel(),
      "Problem saving SimBuilder file!",
      "Unable to create a vtkStringWriter");
    std::cerr << "Unable to create a vtkStringWriter" << std::endl;
    return 0;
    }
  vtkSMSourceProxy *proxy = vtkSMSourceProxy::SafeDownCast(serverWriter->getProxy());
  pqSMAdaptor::setElementProperty(proxy->GetProperty("FileName"), filename);
  pqSMAdaptor::setElementProperty(proxy->GetProperty("Text"), filecontents.c_str());
  proxy->UpdateVTKObjects();
  proxy->UpdatePipeline();

  //now that we are done writing, destroy the proxy
  builder->destroy( serverWriter );
  proxy = NULL;

  return 1;
}

//----------------------------------------------------------------------------
int SimBuilderCore::LoadResources(const char *filename)
{
  if (!filename)
    {
    return 0;
    }

  QFileInfo finfo(filename);
  pqFileDialogModel model(this->ActiveServer);
  QString fullpath;
  pqPipelineSource* reader = NULL;
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();
  QString lastExt = finfo.suffix().toLower();
  if(model.fileExists(filename, fullpath) && (lastExt == "crf"))
    {
    QStringList files;
    files << filename;
    reader = builder->createReader("sources", "StringReader",
      files, this->ActiveServer);
    }
  if(!reader)
    {
    return 0;
    }
  else
    {
    pqServer* server = reader->getServer();

    // Add this to the list of recent server resources ...
    pqServerResource resource = server->getResource();
    resource.setPath(filename);
    resource.addData("readergroup", reader->getProxy()->GetXMLGroup());
    resource.addData("reader", reader->getProxy()->GetXMLName());
    core->recentlyUsedResources().add(resource);
    core->recentlyUsedResources().save(*core->settings());
    }

  return 1;
}

//----------------------------------------------------------------------------
int SimBuilderCore::LoadResources(pqPipelineSource* reader,
                                  pqCMBSceneTree* /*sceneTree*/)
{
  // force read
  vtkSMSourceProxy::SafeDownCast( reader->getProxy() )->UpdatePipeline();
  vtkNew<vtkPVSceneGenFileInformation> info;
  reader->getProxy()->GatherInformation(info.GetPointer());
  QFileInfo finfo(info->GetFileName());
  QString lastExt = finfo.suffix().toLower();

  if(lastExt != "crf")
    {
    return 0;
    }

  if(this->CurrentSimFile == info->GetFileName() || this->isSimModelLoaded())
    {
    // Should not come in here, since application level logic checked this.
    if(QMessageBox::question(this->GetUIPanel(),
    "Load Resources?",
    tr("A SimBuilder file is already loaded. Do you want to close current SimBuilder file?\n") +
    tr("NOTE: all unsaved changes to simulation database will be lost if answering Yes."),
    QMessageBox::Yes|QMessageBox::No, QMessageBox::No) == QMessageBox::No)
      {
      return 0;
      }
    else
      {
      QApplication::processEvents();
      this->clearSimulationModel();
      this->Initialize();
      }
    }

  // Instantiate and initialize ResourceSet
  smtk::common::ResourceSet resources;
  std::string linkStartDir = finfo.absoluteDir().path().toStdString();
  resources.setLinkStartPath(linkStartDir);

  // Instantiate reader and std::map for current attribute managers
  smtk::io::ResourceSetReader resourceReader;
  std::map<std::string, smtk::common::ResourcePtr> resourceMap;
  resourceMap["simbuilder"] = this->attributeUIManager()->attManager();
  //resourceMap["export"] = this->GetExportPanel()->uiManager()->attManager();

  // Read input
  smtk::io::Logger logger;
  bool hasErrors = resourceReader.readString(info->GetFileContents(), resources,
                                             logger, true, &resourceMap);
  if (hasErrors)
    {
    std::cerr << "Resource Reader error\n"
              << logger.convertToString()
              << std::endl;

    qDebug("Error loading resource file.");
    return 0;
    }

  //unsigned numResources =  resources.numberOfResources();
  //std::cout << "Number of resources loaded: " << numResources << std::endl;

  // If simulation attributes loaded, update ExportDialog
  smtk::common::Resource::Type simType;
  smtk::common::ResourceSet::ResourceRole simRole;
  smtk::common::ResourceSet::ResourceState simState;
  std::string simLink;
  if (resources.resourceInfo("simbuilder", simType, simRole,
                             simState, simLink))
    {
    this->ExportDialog->setSimAttManager(
      this->attributeUIManager()->attManager());
    this->IsSimModelLoaded = true;
    this->CurrentSimFile = info->GetFileName();
    }

  // Check if export resource loaded, and if not, load default template
  smtk::common::Resource::Type exportType;
  smtk::common::ResourceSet::ResourceRole exportRole;
  smtk::common::ResourceSet::ResourceState exportState;
  std::string exportLink;
  if (resources.resourceInfo("export", exportType, exportRole,
                             exportState, exportLink))
    {
    smtk::common::ResourcePtr expResource;
    resources.get("export", expResource);
    smtk::attribute::ManagerPtr expManager =
      smtk::dynamic_pointer_cast<smtk::attribute::Manager>(expResource);
    this->ExportDialog->setExportAttManager(expManager);
    }
  else
    {
    hasErrors = this->setDefaultExportTemplate();
    if (hasErrors)
      {
      return 0;
      }
    }

  // Create GUI components
  this->attributeUIManager()->initializeUI(
    this->GetUIPanel()->panelWidget(), this);
/*
  vtkDiscreteModel* model = this->CMBModel ? this->CMBModel->getModel() : NULL;
  bool ignoreModel = false;
  if(this->LoadingScenario || (this->CMBModel && !this->CMBModel->hasGeometryEntity()))
    {
    ignoreModel = true;
    }
  this->updateCMBModelWithScenario(false);
  this->updateCMBModelItems(model,
    this->CMBModel ? this->CMBModel->getModelWrapper() : NULL);
*/
  if (this->IsSimModelLoaded)
    {
    emit this->newSimFileLoaded(info->GetFileName());
    }
  return 1;
}

//----------------------------------------------------------------------------
// Loads hard-coded default export template (att manager)
// Returns true if logger has errors
bool SimBuilderCore::setDefaultExportTemplate()
{
  smtk::io::AttributeReader attributeReader;
  smtk::io::Logger logger;
  smtk::attribute::ManagerPtr manager =
    smtk::attribute::ManagerPtr(new smtk::attribute::Manager());
  bool hasErrors = attributeReader.readContents(*manager,
    defaultExportTemplateString, logger);
  if (hasErrors)
    {
    std::cerr << "Error loading default export resource:\n"
              << logger.convertToString()
              << std::endl;
    QMessageBox::warning(this->GetUIPanel(),
                         "ERROR", "Error loading default export template");
    }
  this->ExportDialog->setExportAttManager(manager);
  smtkDebugMacro(logger, "Initialized ExportPanel to default template");
  return hasErrors;
}

//----------------------------------------------------------------------------
SimBuilderUIPanel* SimBuilderCore::GetUIPanel()
{
  if (!this->UIPanel)
    {
    this->UIPanel = new SimBuilderUIPanel();
    this->UIPanel->initialize();
    }
  return this->UIPanel;
}

//----------------------------------------------------------------------------
smtkUIManager*  SimBuilderCore::attributeUIManager()
{
  if(!this->m_attUIManager)
    {
    this->m_attUIManager = new smtkUIManager();
    }

  return this->m_attUIManager;;
}

//----------------------------------------------------------------------------
void SimBuilderCore::updateSimBuilder(pqCMBSceneTree* /*sceneTree*/)
{
  //vtkGenericWarningMacro("Not implemented");
}

//----------------------------------------------------------------------------
void SimBuilderCore::updateSimulationModel()
{
  if(this->attributeUIManager()->rootView() && this->isSimModelLoaded())// && this->CMBModel)
    {
    if(this->isLoadingScenario())
      {
      this->updateCMBModelWithScenario();
      }
    }
}

//----------------------------------------------------------------------------
void SimBuilderCore::clearCMBModel()
{
  if(this->isSimModelLoaded() && this->attributeUIManager()->rootView())
    {
//    this->attributeUIManager()->clearModelItems();
    this->ScenarioEntitiesCreated = false;
    }
}

//----------------------------------------------------------------------------
void SimBuilderCore::updateCMBModelWithScenario(bool emitSignal)
{
}
//----------------------------------------------------------------------------
void SimBuilderCore::clearSimulationModel()
{
  if(this->m_attUIManager)
    {
    delete this->m_attUIManager;
    this->m_attUIManager = NULL;
    }
  if(this->UIPanel)
    {
    this->UIPanel->initialize();
//    delete this->UIPanel;
//    this->UIPanel = NULL;
    }
}

//----------------------------------------------------------------------------
void SimBuilderCore::ExportSimFile()
{
/*
  // Check if there is an unsaved mesh
  bool hasMesh = this->MeshManager->hasMesh();
  bool isCurrent = this->MeshManager->analysisMeshIsCurrent();
  //std::cout << "hasMesh: " << hasMesh << ", isCurrent: " << isCurrent << std::endl;
  if (hasMesh && !isCurrent)
    {
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText("Mesh was generated but not saved.");
    const char *info = "A mesh has been generated but not saved to file."
      " You can still export, however, the export processing will use the *previous* mesh (if any).";
    msgBox.setInformativeText(info);
    msgBox.addButton("Continue to export dialog", QMessageBox::AcceptRole);
    QPushButton *cancelButton = msgBox.addButton(QMessageBox::Cancel);
    msgBox.exec();
    if (msgBox.clickedButton() == cancelButton)
      {
      return;
      }
    }
*/
  smtk::attribute::ManagerPtr attManager = this->attributeUIManager()->attManager();
  if (!attManager)
    {
    QMessageBox::warning(NULL, "Export Warning!",
      "Cannot export - no attribute manager available!");
    return;
    }

  smtk::attribute::ManagerPtr expManager =
    this->ExportDialog->exportAttManager();
  if (!expManager)
    {
    QMessageBox::warning(NULL, "Export Warning!",
      "Cannot export - no *export* manager available!");
    return;
    }

  int status = this->ExportDialog->exec();
  //std::cout << "status " << status << std::endl;

  if (status)
    {
    std::string script = this->ExportDialog->getPythonScript();
    if (script == "")
      {
      QMessageBox::warning(NULL, "Export Error",
        "No python script specified.");
      return;
      }
/*
    // Set up proxy to server
    vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();

    vtkSmartPointer<vtkSMOperatorProxy> operatorProxy(
      vtkSMOperatorProxy::SafeDownCast(
        manager->NewProxy("CMBModelGroup", "PythonExporter")));
    if(!operatorProxy)
      {
      QMessageBox::warning(NULL, "Failure to create operator",
        "Unable to create PythonExporter proxy.");
      return;
      }

    pqCMBModel* model = this->getCMBModel();
    if(!model)
      {
      QMessageBox::warning(NULL, "Export Warning!",
                           "No model available!");
      return;
      }
*/
    std::string simContents;
    smtk::io::AttributeWriter xmlw;
    smtk::io::Logger logger;
    bool errStatus = xmlw.writeContents( *(this->attributeUIManager()->attManager()),
                                         simContents, logger);
    if(errStatus)
      {
      QMessageBox::warning(this->GetUIPanel(),
                           "Problem saving SimBuilder file!",
                           logger.convertToString().c_str());
      std::cerr << logger.convertToString() << std::endl;
      return;
      }

    std::string exportContents;
    errStatus = xmlw.writeContents(*(this->ExportDialog->exportAttManager()),
                                   exportContents, logger);
    if(errStatus)
      {
      QMessageBox::warning(this->GetUIPanel(),
                           "Problem saving SimBuilder file!",
                           logger.convertToString().c_str());
      std::cerr << logger.convertToString() << std::endl;
      return;
      }
/*
    pqSMAdaptor::setElementProperty(operatorProxy->GetProperty("Script"),
                                    script.c_str());
    //pqSMAdaptor::setElementProperty(operatorProxy->GetProperty("OutputFilename"),
    //                                exportDlg.getFileName());
    //pqSMAdaptor::setElementProperty(operatorProxy->GetProperty("AnalysisName"),
    //                                exportDlg.getAnalysisName());

    operatorProxy->UpdateVTKObjects();
    vtkClientServerStream stream;
    stream  << vtkClientServerStream::Invoke
            << VTKOBJECT(operatorProxy) << "Operate"
            << VTKOBJECT(model->getModelWrapper())
            << simContents
            << exportContents
            << vtkClientServerStream::End;

//    model->getModelWrapper()->GetSession()->ExecuteStream(model->getModelWrapper()->GetLocation(), stream);

    // check to see if the operation succeeded on the server
    vtkSMIntVectorProperty* operateSucceeded =
      vtkSMIntVectorProperty::SafeDownCast(
        operatorProxy->GetProperty("OperateSucceeded"));
    operatorProxy->UpdatePropertyInformation();

    if(operateSucceeded->GetElement(0))
      {
      operatorProxy->UpdatePropertyInformation();
      }
    else
      {
      QMessageBox::warning(NULL, "Failure of server operator",
        "Server side create rectangle model failed.");
      }

    operatorProxy->Delete();
    operatorProxy = 0;
*/
    }

  return;
}

//----------------------------------------------------------------------------
void SimBuilderCore::setServer(pqServer* server)
{
  this->ActiveServer = server;
  this->attributeUIManager()->setServer(server);
}
//----------------------------------------------------------------------------
void SimBuilderCore::setRenderView(pqRenderView* view)
{
  this->RenderView = view;
  this->attributeUIManager()->setRenderView(view);
}

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "SimBuilderCore.h"

#include "qtSimBuilderUIPanel.h"
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
#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/extension/qt/qtRootView.h"

#include "pqSimBuilderUIManager.h"

#include "vtkSMProxyManager.h"
#include "vtkClientServerStream.h"
#include "vtkSMSession.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMProxyProperty.h"
#include "vtkSMModelManagerProxy.h"

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
  delete this->m_attUIManager;
}

//----------------------------------------------------------------------------
void SimBuilderCore::Initialize()
{
  this->IsSimModelLoaded = false;
  this->LoadTemplateOnly = false;
  this->LoadingScenario = false;
  this->ScenarioEntitiesCreated = false;
  this->CurrentSimFile = "";
  delete this->m_attUIManager;
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
#include "smtk/io/Logger.h"
#include "smtk/io/AttributeReader.h"

  // We need to get the file contents from server, then load it into attributes
  smtk::io::AttributeReader xmlr;
  std::string dirPath = finfo.dir().path().toStdString();
  std::vector<std::string> searchPaths;
  searchPaths.push_back(dirPath);
  xmlr.setSearchPaths(searchPaths);

  smtk::io::Logger logger;
  bool errStatus = xmlr.readContents(*(this->uiManager()->attributeSystem()),
                                     info->GetFileContents(), logger);

  if(errStatus)
    {
    vtkGenericWarningMacro("Problems reading " <<  info->GetFileName()
                           << ". The information is: " << logger.convertToString());
    }

  // Lets get the toplevel view
  smtk::common::ViewPtr topView = this->uiManager()->attributeSystem()->findTopLevelView();
  if (!topView)
    {
    vtkGenericWarningMacro("There is no TopLevel View in  " <<  info->GetFileName());
    return 0;
    }
  
  //parse element, create GUI components
  this->uiManager()->setSMTKView(topView,this->GetUIPanel()->panelWidget(), this);
  if(!this->uiManager()->topView())
    {
    return 0;
    }
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
  this->ExportDialog->setSimAttSystem(
    this->uiManager()->attributeSystem());
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
    smtk::attribute::SystemPtr simSystem =
      this->uiManager()->attributeSystem();
    resources.addResource(simSystem, "simbuilder", "",
      smtk::common::ResourceSet::INSTANCE);
    smtk::attribute::SystemPtr expSystem =
      this->ExportDialog->exportAttSystem(true);  // use baseline atts
    resources.addResource(expSystem, "export", "",
      smtk::common::ResourceSet::TEMPLATE);

    // Serialize ResourceSet
    smtk::io::ResourceSetWriter resourceWriter;
    errStatus = resourceWriter.writeString(filecontents, resources, logger);
    }
  else
    {
    // SimBuilderWriter xmlw;
    smtk::io::AttributeWriter xmlw;
    errStatus = xmlw.writeContents( *(this->uiManager()->attributeSystem()),
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

  // Instantiate reader and std::map for current attribute systems
  smtk::io::ResourceSetReader resourceReader;
  std::map<std::string, smtk::common::ResourcePtr> resourceMap;
  resourceMap["simbuilder"] = this->uiManager()->attributeSystem();
  //resourceMap["export"] = this->GetExportPanel()->uiManager()->attributeSystem();

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

  // Lets get the toplevel view
  smtk::common::ViewPtr topView = this->uiManager()->attributeSystem()->findTopLevelView();
  if (!topView)
    {
    vtkGenericWarningMacro("There is no TopLevel View in  " <<  info->GetFileName());
    return 0;
    }
  
  // Create GUI components
  this->uiManager()->setSMTKView(topView,this->GetUIPanel()->panelWidget(), this);
  if(!this->uiManager()->topView())
    {
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
    this->ExportDialog->setSimAttSystem(
      this->uiManager()->attributeSystem());
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
    smtk::attribute::SystemPtr expSystem =
      smtk::dynamic_pointer_cast<smtk::attribute::System>(expResource);
    this->ExportDialog->setExportAttSystem(expSystem);
    }
  else
    {
    hasErrors = this->setDefaultExportTemplate();
    if (hasErrors)
      {
      return 0;
      }
    }
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
// Loads hard-coded default export template (att system)
// Returns true if logger has errors
bool SimBuilderCore::setDefaultExportTemplate()
{
  smtk::io::AttributeReader attributeReader;
  smtk::io::Logger logger;
  smtk::attribute::SystemPtr system =
    smtk::attribute::SystemPtr(new smtk::attribute::System());
  bool hasErrors = attributeReader.readContents(*system,
    defaultExportTemplateString, logger);
  if (hasErrors)
    {
    std::cerr << "Error loading default export resource:\n"
              << logger.convertToString()
              << std::endl;
    QMessageBox::warning(this->GetUIPanel(),
                         "ERROR", "Error loading default export template");
    }
  this->ExportDialog->setExportAttSystem(system);
  smtkDebugMacro(logger, "Initialized ExportPanel to default template");
  return hasErrors;
}

//----------------------------------------------------------------------------
qtSimBuilderUIPanel* SimBuilderCore::GetUIPanel()
{
  if (!this->UIPanel)
    {
    this->UIPanel = new qtSimBuilderUIPanel();
    this->UIPanel->initialize();
    }
  return this->UIPanel;
}

//----------------------------------------------------------------------------
pqSimBuilderUIManager*  SimBuilderCore::uiManager()
{
  if(!this->m_attUIManager)
    {
    this->m_attUIManager = new pqSimBuilderUIManager();
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
  if(this->uiManager()->topView() && this->isSimModelLoaded())// && this->CMBModel)
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
  if(this->isSimModelLoaded() && this->uiManager()->topView())
    {
//    this->uiManager()->clearModelItems();
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
  delete this->m_attUIManager;
  this->m_attUIManager = NULL;
  if(this->UIPanel)
    {
    this->UIPanel->initialize();
//    delete this->UIPanel;
//    this->UIPanel = NULL;
    }
}

//----------------------------------------------------------------------------
void SimBuilderCore::ExportSimFile(vtkSMModelManagerProxy* mmproxy)
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
  smtk::attribute::SystemPtr attSystem = this->uiManager()->attributeSystem();
  if (!attSystem)
    {
    QMessageBox::warning(NULL, "Export Warning!",
      "Cannot export - no attribute system available!");
    return;
    }

  smtk::attribute::SystemPtr expSystem =
    this->ExportDialog->exportAttSystem();
  if (!expSystem)
    {
    QMessageBox::warning(NULL, "Export Warning!",
      "Cannot export - no *export* system available!");
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

    // Set up proxy to server
    vtkSMProxyManager* manager = vtkSMProxyManager::GetProxyManager();

    vtkSmartPointer<vtkSMProxy> exportProxy(
      manager->NewProxy("ModelBridge", "PythonExporter"));
    if(!exportProxy)
      {
      QMessageBox::warning(NULL, "Failure to create PythonExporter",
        "Unable to create PythonExporter proxy.");
      return;
      }

    std::string simContents;
    smtk::io::AttributeWriter xmlw;
    smtk::io::Logger logger;
    bool errStatus = xmlw.writeContents( *(this->uiManager()->attributeSystem()),
                                         simContents, logger);
    if(errStatus)
      {
      QMessageBox::warning(NULL,
                           "Problem saving SimBuilder file!",
                           logger.convertToString().c_str());
      std::cerr << logger.convertToString() << std::endl;
      return;
      }

    std::string exportContents;
    errStatus = xmlw.writeContents(*(this->ExportDialog->exportAttSystem()),
                                   exportContents, logger);
    if(errStatus)
      {
      QMessageBox::warning(NULL,
                           "Problem saving SimBuilder file!",
                           logger.convertToString().c_str());
      std::cerr << logger.convertToString() << std::endl;
      return;
      }

    pqSMAdaptor::setElementProperty(exportProxy->GetProperty("Script"),
                                    script.c_str());
    vtkSMProxyProperty* smwrapper =
      vtkSMProxyProperty::SafeDownCast(
      exportProxy->GetProperty("ModelManagerWrapper"));
    smwrapper->RemoveAllProxies();
    smwrapper->AddProxy(mmproxy);
//    vtkSMPropertyHelper(exportProxy, "ModelEntityID").Set(
//      model.entity().toString().c_str());

    exportProxy->UpdateVTKObjects();
    vtkClientServerStream stream;
    stream  << vtkClientServerStream::Invoke
            << VTKOBJECT(exportProxy) << "Operate"
            << VTKOBJECT(mmproxy)
            << simContents
            << exportContents
            << vtkClientServerStream::End;

    mmproxy->GetSession()->ExecuteStream(mmproxy->GetLocation(), stream);

    // check to see if the operation succeeded on the server
    vtkSMIntVectorProperty* operateSucceeded =
      vtkSMIntVectorProperty::SafeDownCast(
        exportProxy->GetProperty("OperateSucceeded"));
    exportProxy->UpdatePropertyInformation();

    if(operateSucceeded->GetElement(0))
      {
      exportProxy->UpdatePropertyInformation();
      }
    else
      {
      QMessageBox::warning(NULL, "Failure of server operator",
        "Server side Export failed.");
      }

    exportProxy->Delete();
    exportProxy = 0;

    }

  return;
}

//----------------------------------------------------------------------------
void SimBuilderCore::setServer(pqServer* server)
{
  this->ActiveServer = server;
  this->uiManager()->setServer(server);
}
//----------------------------------------------------------------------------
void SimBuilderCore::setRenderView(pqRenderView* view)
{
  this->RenderView = view;
  this->uiManager()->setRenderView(view);
}

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME A dialog for simulation exporting.
// .SECTION Description
// .SECTION Caveats

#include "SimBuilderExportDialog.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/FileItemDefinition.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/ItemDefinition.h"
#include "smtk/attribute/System.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/extension/qt/qtBaseView.h"
#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/io/AttributeReader.h"
#include "smtk/io/AttributeWriter.h"
#include "smtk/io/Logger.h"

#include "pqApplicationCore.h"
#include "pqServer.h"

#include <QBoxLayout>
#include <QButtonGroup>
#include <QCheckBox>
#include <QCoreApplication>
#include <QDebug>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QFrame>
#include <QLabel>
#include <QMessageBox>
#include <QRadioButton>
#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QVBoxLayout>
#include <QWidget>

#include <vtksys/SystemTools.hxx>

#include <sstream>

//-----------------------------------------------------------------------------
SimBuilderExportDialog::SimBuilderExportDialog() :
  Status(-1), ActiveServer(0), IsPanelSet(false), IsMultipleSelect(false)
{
  // Instantiate dialog
  this->MainDialog = new QDialog();
  this->MainDialog->setObjectName("SimulationExportDialog");
  this->MainDialog->setWindowTitle("Simulation Export Dialog");
  QVBoxLayout *layout = new QVBoxLayout();
  this->MainDialog->setLayout(layout);

  // Instantiate container for analysis-selection widget
  this->AnalysisTypesContainer = new QFrame;
  this->AnalysisTypesContainer->setContentsMargins(0, 0, 0, 0);
  QVBoxLayout *containerLayout = new QVBoxLayout();
  this->AnalysisTypesContainer->setLayout(containerLayout);
  layout->addWidget(this->AnalysisTypesContainer);

  this->AnalysisTypesWidget = NULL;

  // Instantiate container for smtk (export) panel
  this->ContentWidget = new QWidget;
  this->ContentWidget->setObjectName("ExportContentWidget");
  QVBoxLayout *widgetLayout = new QVBoxLayout();
  this->ContentWidget->setLayout(widgetLayout);
  layout->addWidget(this->ContentWidget);

  this->ExportUIManager = new pqSimBuilderUIManager();

  QDialogButtonBox *buttonBox =
    new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  buttonBox->setObjectName("ExportButtonBox");
  layout->addWidget(buttonBox);

  QObject::connect(buttonBox, SIGNAL(accepted()), this->MainDialog, SLOT(accept()));
  QObject::connect(buttonBox, SIGNAL(rejected()), this->MainDialog, SLOT(reject()));
}

//-----------------------------------------------------------------------------
SimBuilderExportDialog::~SimBuilderExportDialog()
{
  delete this->ExportUIManager;
  delete this->MainDialog;
}

//-----------------------------------------------------------------------------
int SimBuilderExportDialog::exec()
{
  if (!this->SimAttSystem)
    {
    QMessageBox::warning(
      NULL,
      "Export Error",
      "Cannot export becauuse no simulation template or attributes have been loaded",
      QMessageBox::Ok);
    return 0;
    }

  if (!this->IsPanelSet)
    {
    this->updatePanel();
    }

  if (!this->ExportUIManager->topView())
    {
    QMessageBox::warning(NULL,
      tr("No Top Level View Warning"),
      tr("There is no top level view created in the UI."),
      QMessageBox::Ok);
    return 0;
    }

  // Need to ping export panel (top level view) to hide advanced items at start
  this->ExportUIManager->topView()->showAdvanceLevel(0);

  this->MainDialog->setModal(true);
  this->MainDialog->show();

  // Show dialog and wait
  QEventLoop loop;
  QObject::connect(this->MainDialog, SIGNAL(finished(int)), &loop, SLOT(quit()));
  loop.exec();

  int status = this->MainDialog->result() == QDialog::Accepted ? 1 : 0;

  // If accepted, update AnalysisTypes item
  if (status)
    {
    smtk::attribute::ItemPtr item = this->getExportSpecItem("AnalysisTypes");
    smtk::attribute::StringItemPtr typesItem =
      smtk::dynamic_pointer_cast<smtk::attribute::StringItem>(item);
    if (typesItem)
      {
      bool ok = typesItem->setNumberOfValues(this->SelectedAnalyses.size());
      if (!ok)
        {
        QString message;
        QTextStream(&message) << "Error setting AnalysisTypes attribute to "
                              << this->SelectedAnalyses.size() << " items";
        QMessageBox::critical(NULL, "Export Error", message);
        status = 0;
        }
      std::set<std::string>::const_iterator iter =
        this->SelectedAnalyses.begin();
      for (int i=0; iter != this->SelectedAnalyses.end(); i++, iter++)
        {
        bool ok = typesItem->setValue(i, *iter);
        }
      }
    }

  return status;
}

//-----------------------------------------------------------------------------
// Returns python script "value" as stored in the export attribute system
std::string SimBuilderExportDialog::getPythonScript() const
{
  smtk::attribute::FileItemPtr fileItem = this->getPythonScriptItem();
  if (!fileItem || !fileItem->isSet(0))
    {
    return "";
    }

  std::string script = fileItem->value(0);
  return script;
}

//-----------------------------------------------------------------------------
// [Slot] Handles analysis selection from radio buttons
void SimBuilderExportDialog::analysisSelected()
{
  QRadioButton *button = dynamic_cast<QRadioButton *>(QObject::sender());
  if (button)
    {
    std::string text = button->text().toStdString();
    if (this->IsMultipleSelect)
      {
      // In multiple-selection mode, either add or remove item
      // depending on button state
      if (button->isChecked())
        {
        this->SelectedAnalyses.insert(text);
        }
      else
        {
        this->SelectedAnalyses.erase(text);
        }
      }
    else
      {
      // In single-selection mode, only one item can be selected
      this->SelectedAnalyses.clear();
      this->SelectedAnalyses.insert(text);
      }
    }
}

//-----------------------------------------------------------------------------
// [Slot] Handles multiple-selection checkbox changing state
void SimBuilderExportDialog::multipleSelectChanged(int state)
{
  bool boolState = static_cast<bool>(state);
  this->IsMultipleSelect = boolState;
  if (!boolState)
    {
    // Clear all buttons
    QList<QAbstractButton *> buttonList = this->AnalysisButtonGroup->buttons();
    for (size_t i; i<buttonList.size(); ++i)
      {
      buttonList.value(i)->setChecked(false);
      }
    }
  this->AnalysisButtonGroup->setExclusive(!boolState);
}

//-----------------------------------------------------------------------------
// Returns the export attribute system
// When the baseline flag is true, return the original/initial attributes
// loaded in the dialog, otherwise return the copy used in the export panel
// (which might be empty, or might have been edited).
smtk::attribute::SystemPtr
SimBuilderExportDialog::
exportAttSystem(bool baseline) const
{
  if (baseline)
    {
    return this->ExportAttSystem;
    }
  return this->ExportUIManager->attributeSystem();
}

//-----------------------------------------------------------------------------
void SimBuilderExportDialog::setExportAttSystem(smtk::attribute::SystemPtr system)
{
  this->ExportAttSystem = system;
  this->IsPanelSet = false;
}

//-----------------------------------------------------------------------------
void SimBuilderExportDialog::setSimAttSystem(smtk::attribute::SystemPtr system)
{
  this->SimAttSystem = system;
  this->IsPanelSet = false;
}

//-----------------------------------------------------------------------------
void SimBuilderExportDialog::setActiveServer(pqServer* server)
{
  this->ActiveServer = server;
}

//-----------------------------------------------------------------------------
// Rebuilds ExportSystem with copy of ExportAttSystem,
// For now, does brute-force copy
void SimBuilderExportDialog::updatePanel()
{
  // Blow away current export system
  if(this->ExportUIManager)
    {
    delete this->ExportUIManager;
    }
  this->ExportUIManager = new pqSimBuilderUIManager();
  this->ExportUIManager->setModelManager(this->SimAttSystem->refModelManager());

  // Serialize export system
  smtk::io::Logger logger;
  smtk::io::AttributeWriter attWriter;
  std::string serializedSystem;
  bool hasError;
  hasError =
    attWriter.writeContents(*this->ExportAttSystem, serializedSystem, logger);

  // std::string filename("export.sbt");
  // hasError =
  //   attWriter.write(*this->ExportAttSystem, filename, logger);
  // std::cout << "Wrote " << filename  << std::endl;

  // Reload into export panel
  smtk::io::AttributeReader attReader;
  hasError = attReader.readContents(*(this->ExportUIManager->attributeSystem()),
    serializedSystem, logger);
  if (hasError)
    {
    QMessageBox::critical(NULL, "Export Error",
      QString::fromStdString(logger.convertToString()));
    return;
    }

  // If python script def has default value, look for script on local filesystem
  std::string scriptPath;
  smtk::attribute::FileItemDefinitionPtr fileDef = this->getPythonScriptDef(
    this->ExportUIManager->attributeSystem());
  if (fileDef && fileDef->hasDefault())
    {
    scriptPath = this->findPythonScriptPath(fileDef->defaultValue());
    if (scriptPath.empty())
      {
      // Not found: unset the default value, so that UI shows it missing
      fileDef->setDefaultValue("");
      fileDef->unsetDefaultValue();
      }
    }

  // Get toplevel view
  smtk::common::ViewPtr topView =
    this->ExportUIManager->attributeSystem()->findTopLevelView();
  if (!topView)
    {
    QMessageBox::critical(
      NULL, "Export Error", "There is no TopLevel View in Export Script!");
    return;
    }

  this->ExportUIManager->setSMTKView(topView, this->ContentWidget, NULL);

  // Initialize script path
  if (!scriptPath.empty())
    {
      smtk::attribute::FileItemPtr fileItem = this->getPythonScriptItem();
      fileItem->setValue(0, scriptPath);
    }

  // Update selection widget for analysis types
  this->SelectedAnalyses.clear();
  this->updateAnalysisTypesWidget();

  this->IsPanelSet = true;
}

//-----------------------------------------------------------------------------
// Rebuilds selection list of analyis types
void SimBuilderExportDialog::updateAnalysisTypesWidget()
{
  if (!this->SimAttSystem)
    {
    // No attributes - should we warn the user?
    return;
    }

  // Get AnalysisTypes item
  smtk::attribute::ItemPtr item = this->getExportSpecItem("AnalysisTypes");
  smtk::attribute::StringItemPtr typesItem =
    smtk::dynamic_pointer_cast<smtk::attribute::StringItem>(item);
  if (!typesItem)
    {
    this->AnalysisTypesContainer->hide();
    return;
    }
  typesItem->setToDefault();
  if (typesItem->numberOfValues() > 0 && typesItem->value(0) == "")
    {
    typesItem->setValue(0, "(Use selection buttons above)");
    }
  for (size_t i=1; i<typesItem->numberOfValues(); ++i)
    {
    typesItem->setValue(i, "");
    }

  // Rebuild the widget
  delete this->AnalysisTypesWidget;
  this->AnalysisTypesWidget = new QFrame(this->AnalysisTypesContainer);
  this->AnalysisTypesWidget->setObjectName("AnalysisTypesWidget");
  this->AnalysisTypesWidget->setFrameStyle(QFrame::StyledPanel);
  QVBoxLayout *frameLayout = new QVBoxLayout();
  this->AnalysisTypesWidget->setLayout(frameLayout);

  QBoxLayout *box = static_cast<QBoxLayout *>(this->AnalysisTypesContainer->layout());
  box->addWidget(this->AnalysisTypesWidget);

  // Add label
  QLabel *label = new QLabel("<strong>Select Analysis Type</strong>",
    this->AnalysisTypesWidget);
  frameLayout->addWidget(label);

  // Construct list of analysis types from simulation attribute system
  std::map<std::string, std::set<std::string> > analysisMap =
    this->SimAttSystem->analyses();
  if (analysisMap.empty())
    {
    return;
    }

  std::list<std::string> analysisTypes;
  std::map<std::string, std::set<std::string> >::const_iterator mapIter;
  for (mapIter = analysisMap.begin(); mapIter != analysisMap.end(); mapIter++)
    {
    analysisTypes.push_back(mapIter->first);
    }
  analysisTypes.sort();

  // Create radio button for each analysis type
  this->AnalysisButtonGroup = new QButtonGroup();
  std::list<std::string>::const_iterator typeIter;
  for (typeIter = analysisTypes.begin(); typeIter != analysisTypes.end(); typeIter++)
    {
    QRadioButton *button = new QRadioButton(this->AnalysisTypesWidget);
    button->setText(QString(typeIter->c_str()));
    frameLayout->addWidget(button);
    this->AnalysisButtonGroup->addButton(button);

    QObject::connect(button, SIGNAL(clicked()),
      this, SLOT(analysisSelected()));

    // If there is only one analysis, or this one was last used, set checked
    if (1 == analysisTypes.size() ||
        this->SelectedAnalyses.find(*typeIter) != this->SelectedAnalyses.end())
      {
      button->click();
      }
    }
  this->AnalysisButtonGroup->setExclusive(!this->IsMultipleSelect);

  // Horizontal rule
  QWidget *horizontalLine = new QWidget(this->AnalysisTypesWidget);
  horizontalLine->setFixedHeight(2);
  horizontalLine->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  horizontalLine->setStyleSheet(QString("background-color: #c0c0c0;"));
  frameLayout->addWidget(horizontalLine);

  // Checkbox to enable-disable multiple selection
  QCheckBox *checkbox = new QCheckBox(this->AnalysisTypesWidget);
  frameLayout->addWidget(checkbox);
  checkbox->setText("Enable multiple selection");
  QObject::connect(checkbox, SIGNAL(stateChanged(int)),
                   this, SLOT(multipleSelectChanged(int)));

  this->AnalysisTypesWidget->show();
  this->AnalysisTypesContainer->show();
}

//-----------------------------------------------------------------------------
// Retrieves PythonScript item definition from export attributes
smtk::attribute::FileItemDefinitionPtr
SimBuilderExportDialog::getPythonScriptDef(
  const smtk::attribute::SystemPtr attributeSystem,
  bool warnIfMissing) const
{
  smtk::attribute::DefinitionPtr exportDef = attributeSystem->findDefinition(
    "ExportSpec");
  if (!exportDef)
    {
    if (warnIfMissing)
      {
      QMessageBox::critical(
        NULL, "Export Error", "ExportSpec definition not found");
      return smtk::attribute::FileItemDefinitionPtr();
      }
    }

  // Traverse item definitions looking for "PythonScript" item def
  for (std::size_t i=0; i < exportDef->numberOfItemDefinitions(); ++i)
    {
    smtk::attribute::ItemDefinitionPtr itemDef = exportDef->itemDefinition(i);
    if (itemDef->name() == "PythonScript")
      {
      smtk::attribute::FileItemDefinitionPtr fileDef =
        smtk::dynamic_pointer_cast<smtk::attribute::FileItemDefinition>(itemDef);
      if (!fileDef && warnIfMissing)
        {
        QMessageBox::critical(
          NULL,
          "Export Error",
          "PythonScript item definition is *not* a FileItemDefinition type");
        return smtk::attribute::FileItemDefinitionPtr();
        }

      // (else)
      return fileDef;
      }
    }

  // (else) not found
  QMessageBox::critical(
    NULL, "Export Error", "PythonScript item not found");
  return smtk::attribute::FileItemDefinitionPtr();
}

//-----------------------------------------------------------------------------
// Retrieves PythonScript item from export attributes
smtk::attribute::FileItemPtr
SimBuilderExportDialog::
getPythonScriptItem(bool warnIfMissing) const
{
  smtk::attribute::ItemPtr item =
    this->getExportSpecItem("PythonScript", warnIfMissing);

  smtk::attribute::FileItemPtr fileItem =
    smtk::dynamic_pointer_cast<smtk::attribute::FileItem>(item);
  if (!fileItem && warnIfMissing)
    {
    QMessageBox::critical(NULL, "Export Error",
      "PythonScript item is *not* a FileItem type");
    }

  return fileItem;
}

//-----------------------------------------------------------------------------
// Retrieves item with specified name from ExportSpec attribute
smtk::attribute::ItemPtr
SimBuilderExportDialog::
getExportSpecItem(const std::string& name, bool warnIfMissing) const
{
  smtk::attribute::SystemPtr system =
    this->ExportUIManager->attributeSystem();
  if (!system)
    {
    if (warnIfMissing)
      {
      QMessageBox::critical(NULL, "Export Error",
        "Export attributes not set");
      }
    return smtk::attribute::ItemPtr();
    }

  std::vector<smtk::attribute::AttributePtr> attList;
  system->findAttributes("ExportSpec", attList);
  if (attList.size() < 1)
    {
    if (warnIfMissing)
      {
      QMessageBox::critical(NULL, "Export Error",
        "No ExportSpec attribute in export attributes");
      }
    return smtk::attribute::ItemPtr();
    }
  if (attList.size() > 1)
    {
    if (warnIfMissing)
      {
      QMessageBox::critical(NULL, "Export Error",
        "Invalid export attributes: more than one ExportSpec");
      }
    return smtk::attribute::ItemPtr();
    }
  smtk::attribute::AttributePtr specAtt = attList[0];

  smtk::attribute::ItemPtr  item = specAtt->find(name);
  if (!item && warnIfMissing)
    {
    std::stringstream ss;
    ss << "Could not file attribute item with name \"" << name << "\"";
    QMessageBox::critical(NULL, "Export Error",
      QString::fromStdString(ss.str()));
    }

  return item;
}

//-----------------------------------------------------------------------------
// Finds absolute path to script file, by checking "standard" locations
std::string SimBuilderExportDialog::findPythonScriptPath(
  smtk::attribute::FileItemPtr fileItem,
  bool warnIfMissing) const
{
  if (fileItem->numberOfValues() == 0)
    {
    if (warnIfMissing)
      {
      QMessageBox::critical(
        NULL, "Export Error", "Attribute item for python script not set");
      }
    return std::string();
    }

  std::string scriptName = fileItem->valueAsString(0);
  return this->findPythonScriptPath(scriptName, warnIfMissing);
}

//-----------------------------------------------------------------------------
// Finds absolute path to script file, by checking "standard" locations
std::string SimBuilderExportDialog::findPythonScriptPath(
  const std::string& scriptName,
  bool warnIfMissing) const
{
  // If empty string
  if (scriptName.empty())
    {
    if (warnIfMissing)
      {
      QMessageBox::critical(
        NULL, "Export Error", "Python script name is empty string");
      }
    return scriptName;
    }

  // If script path is absolute, pass it on
  if (vtksys::SystemTools::FileIsFullPath(scriptName.c_str()))
    {
    return scriptName;
    }

  // If path is relative, can search if the server is running locally
  pqApplicationCore* core = pqApplicationCore::instance();
  pqServer *server = core->getActiveServer();
  if (server->isRemote())
    {
    // Currently we punt if the server is remote
    return std::string();  // return empty string
    }

  // Look for installed workflow directory
  QString appDirPath = QCoreApplication::applicationDirPath();
#ifdef __APPLE__
  QString workflowsPath = appDirPath + "/../../../../../Workflows";
#else
  QString workflowsPath = appDirPath + "/../share/cmb/workflows";
#endif
  if (!QFile::exists(workflowsPath))
    {
    return std::string();
    }
  QDir workflowsDir(workflowsPath);

  // Traverse subfolders to look for script file.
  // Note that this presumes each solver's script file has a unique name.
  QString fragment = QString("/") + QString::fromStdString(scriptName);
  QStringList subfolderList = workflowsDir.entryList(
    QDir::Dirs | QDir::NoDotAndDotDot);
  foreach(QString subfolder, subfolderList)
    {
    QString path = workflowsPath + "/" + subfolder + fragment;
    //qDebug() << "Checking" << path;
    if (QFile::exists(path))
      {
      // Get the canonical path
      QFileInfo fileInfo(path);
      QString canonicalPath = fileInfo.canonicalFilePath();
      return canonicalPath.toStdString();
      }
    }

  // If we reach this point, code did *not* find python script
  if (warnIfMissing)
    {
    std::stringstream ss;
    ss << "Unable to find python export script \"" << scriptName << "\"";
    QMessageBox::critical(NULL, "Export Error", QString::fromStdString(ss.str()));
    }
  return std::string();
}

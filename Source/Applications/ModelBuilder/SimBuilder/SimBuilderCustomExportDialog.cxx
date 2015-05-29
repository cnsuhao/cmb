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

#include "SimBuilderCustomExportDialog.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/System.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/extension/qt/qtRootView.h"
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
#include <QDialog>
#include <QDialogButtonBox>
#include <QEventLoop>
#include <QFrame>
#include <QLabel>
#include <QMessageBox>
#include <QRadioButton>
#include <QString>
#include <QTextStream>
#include <QVBoxLayout>
#include <QWidget>

#include <vtksys/SystemTools.hxx>

#include <sstream>

//-----------------------------------------------------------------------------
SimBuilderCustomExportDialog::SimBuilderCustomExportDialog() :
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
SimBuilderCustomExportDialog::~SimBuilderCustomExportDialog()
{
  delete this->ExportUIManager;
  delete this->MainDialog;
}

//-----------------------------------------------------------------------------
int SimBuilderCustomExportDialog::exec()
{
  if (!this->IsPanelSet)
    {
    this->updatePanel();
    }

  // Need to ping export panel (root view) to hide advanced items at start
  this->ExportUIManager->rootView()->showAdvanceLevel(0);

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
// Returns absolute path to python script,
// Obtained from export attribute system
std::string
SimBuilderCustomExportDialog::
getPythonScript(bool warnIfMissing) const
{
  smtk::attribute::FileItemPtr
    fileItem = this->getPythonScriptItem(warnIfMissing);
  if (!fileItem)
    {
    return "";
    }

  std::string script = this->getPythonScriptPath(fileItem, warnIfMissing);
  return script;
}

//-----------------------------------------------------------------------------
// [Slot] Handles analysis selection from radio buttons
void SimBuilderCustomExportDialog::analysisSelected()
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
void SimBuilderCustomExportDialog::multipleSelectChanged(int state)
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
SimBuilderCustomExportDialog::
exportAttSystem(bool baseline) const
{
  if (baseline)
    {
    return this->ExportAttSystem;
    }
  return this->ExportUIManager->attSystem();
}

//-----------------------------------------------------------------------------
void SimBuilderCustomExportDialog::setExportAttSystem(smtk::attribute::SystemPtr system)
{
  this->ExportAttSystem = system;
  this->IsPanelSet = false;
}

//-----------------------------------------------------------------------------
void SimBuilderCustomExportDialog::setSimAttSystem(smtk::attribute::SystemPtr system)
{
  this->SimAttSystem = system;
  this->IsPanelSet = false;
}

//-----------------------------------------------------------------------------
void SimBuilderCustomExportDialog::setActiveServer(pqServer* server)
{
  this->ActiveServer = server;
}

//-----------------------------------------------------------------------------
// Rebuilds ExportSystem with copy of ExportAttSystem,
// For now, does brute-force copy
void SimBuilderCustomExportDialog::updatePanel()
{
  // Blow away current export system
  if(this->ExportUIManager)
    {
    delete this->ExportUIManager;
    }
  this->ExportUIManager = new pqSimBuilderUIManager();

  // Serialize export system
  smtk::io::Logger logger;
  smtk::io::AttributeWriter attWriter;
  std::string serializedSystem;
  bool hasError;
  hasError =
    attWriter.writeContents(*this->ExportAttSystem, serializedSystem, logger);

  // Reload into export panel
  smtk::io::AttributeReader attReader;
  hasError = attReader.readContents(*(this->ExportUIManager->attSystem()),
    serializedSystem, logger);
  if (hasError)
    {
    QMessageBox::critical(NULL, "Export Error",
      QString::fromStdString(logger.convertToString()));
    return;
    }

  // Update python script item
  smtk::attribute::FileItemPtr fileItem = this->getPythonScriptItem();
  if (fileItem)
    {
    std::string script = this->getPythonScriptPath(fileItem);
    if (script != "")
      {
      fileItem->setValue(0, script);
      }
    }

  // Update selection widget for analysis types
  this->SelectedAnalyses.clear();
  this->updateAnalysisTypesWidget();

  this->ExportUIManager->initializeUI(this->ContentWidget, NULL);
  this->IsPanelSet = true;
}

//-----------------------------------------------------------------------------
// Rebuilds selection list of analyis types
void SimBuilderCustomExportDialog::updateAnalysisTypesWidget()
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

    // If there is only one analysis, or this one was last used, set checked
    if (1 == analysisTypes.size() ||
        this->SelectedAnalyses.find(*typeIter) != this->SelectedAnalyses.end())
      {
      button->setChecked(true);
      }

    QObject::connect(button, SIGNAL(clicked()),
      this, SLOT(analysisSelected()));
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
// Retrieves PythonScript item from export attributes
smtk::attribute::FileItemPtr
SimBuilderCustomExportDialog::
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
SimBuilderCustomExportDialog::
getExportSpecItem(const std::string& name, bool warnIfMissing) const
{
  smtk::attribute::SystemPtr system =
    this->ExportUIManager->attSystem();
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
std::string
SimBuilderCustomExportDialog::
getPythonScriptPath(smtk::attribute::FileItemPtr fileItem,
                    bool warnIfMissing) const
{
  std::string script = fileItem->valueAsString(0);

  // If path is empty, return it
  if ("" == script)
    {
    return script;
    }

  // If script path is absolute, pass it on
  if (vtksys::SystemTools::FileIsFullPath(script.c_str()))
    {
    return script;
    }

  // If path is relative, can search if the server is running locally
  pqApplicationCore* core = pqApplicationCore::instance();
  pqServer *server = core->getActiveServer();
  if (server->isRemote())
    {
    // Currently we punt if the server is remote
    return script;
    }

  // Make list of directories to look for python script
  std::vector<std::string> testDirList;
  std::string appDir = QCoreApplication::applicationDirPath().toStdString();

  // Add path for standard CMB install
  std::string installPath = appDir;
#ifdef __APPLE__
  //installPath += "/ModelBuilder.app/Contents";
  installPath += "/..";
#endif
  installPath += "/Resources/PythonExporters/";
  //std::cout << "installPath: " << installPath << std::endl;
  testDirList.push_back(installPath);

  // Add path for CMB dev/testing (CMB_TEST_DIR)
  std::string testPath = appDir + "/../Source/Testing/Temporary/";
#ifdef __APPLE__
  testPath = appDir + "/../../../../Source/Testing/Temporary/";
#endif
  testDirList.push_back(testPath);

  size_t i;
  for (i=0; i<testDirList.size(); ++i)
    {
    std::string path = testDirList[i] + script;
    if (vtksys::SystemTools::FileExists(path.c_str(), true))
      {
      return path;
      }
    }

  // List out paths that were unsuccessful
  std::cerr << "Unable to find python script \"" << script << "\".\n"
            << "Looked in these directories:\n";
  for (i=0; i<testDirList.size(); ++i)
    {
    std::cerr << "  " << testDirList[i] << "\n";
    }
  std::cerr << std::endl;

  // If we reach this point, code did *not* find python script
  if (warnIfMissing)
    {
    std::stringstream ss;
    ss << "Unable to find python export script \"" << script << "\"";
    QMessageBox::critical(NULL, "Export Error", QString::fromStdString(ss.str()));
    }
  return "";
}

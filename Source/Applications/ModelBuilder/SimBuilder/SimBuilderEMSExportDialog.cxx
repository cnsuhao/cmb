/*=========================================================================

  Program:   CMB
  Module:    SimBuilderEMSExportDialog.cxx

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
=========================================================================*/
// .NAME Represents a dialog for EMS Simulation exporting.
// .SECTION Description
// .SECTION Caveats

#include "SimBuilderEMSExportDialog.h"

#include "ui_qtSimBuilderEMSExportDialog.h"
#include "smtk/attribute/Manager.h"
#include "smtk/model/Model.h"

#include <QDialogButtonBox>
#include <QEventLoop>
#include <QFileInfo>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>

#include <pqFileDialog.h>
#include "pqServer.h"

QString SimBuilderEMSExportDialog::LastPythonScriptParsed = "";
QString SimBuilderEMSExportDialog::SelectedAnalysis = "";
//-----------------------------------------------------------------------------
SimBuilderEMSExportDialog::SimBuilderEMSExportDialog() :
  Status(-1), ActiveServer(0)
{

  this->MainDialog = new QDialog();
  this->Dialog = new Ui::qtSimBuilderEMSExportDialog;
  this->Dialog->setupUi(MainDialog);

  // Turn off Accept
  this->setAcceptable(false);

  QObject::connect(this->Dialog->FileNameText, SIGNAL(textChanged(const QString &)),
    this, SLOT(validate()));

  QObject::connect(this->Dialog->PythonScriptNameText, SIGNAL(editingFinished()),
    this, SLOT(pythonScriptChanged()));

  QObject::connect(this->Dialog->FileBrowserButton, SIGNAL(clicked()),
    this, SLOT(displayFileBrowser()));

  QObject::connect(this->Dialog->PythonScriptBrowserButton, SIGNAL(clicked()),
    this, SLOT(displayPythonScriptBrowser()));

  QObject::connect(this->MainDialog, SIGNAL(accepted()), this, SLOT(accept()));
  QObject::connect(this->MainDialog, SIGNAL(rejected()), this, SLOT(cancel()));

  // Lets restore the last format file we used
  this->Dialog->PythonScriptNameText->setText(this->LastPythonScriptParsed);
  this->LastPythonScriptParsed = "";
  this->pythonScriptChanged();
}

//-----------------------------------------------------------------------------
SimBuilderEMSExportDialog::~SimBuilderEMSExportDialog()
{
  if (this->Dialog)
    {
    delete Dialog;
    }
  if (this->MainDialog)
    {
    delete MainDialog;
    }
}

//-----------------------------------------------------------------------------
int SimBuilderEMSExportDialog::exec()
{
  this->MainDialog->setModal(true);
  this->MainDialog->show();

  QEventLoop loop;
  QObject::connect(this->MainDialog, SIGNAL(finished(int)), &loop, SLOT(quit()));
  loop.exec();

  return this->Status;
}
///-----------------------------------------------------------------------------
void SimBuilderEMSExportDialog::pythonScriptChanged()
{
  QString pythonScript = this->getPythonScriptName();
  // If the format name is "" make sure we deactivate the accept button

  if (pythonScript == "")
    {
    this->setAcceptable(false);
    return;
    }
  // if the format file name has not changed then nothing needs to be done
 if (pythonScript == this->LastPythonScriptParsed)
    {
    return;
    }
  this->LastPythonScriptParsed = pythonScript;
  // See if we are in a valid state
  this->validate();
}
//-----------------------------------------------------------------------------
void SimBuilderEMSExportDialog::accept()
{
  this->Status = 1;
}
//-----------------------------------------------------------------------------
void SimBuilderEMSExportDialog::cancel()
{
  this->Status = 0;
}

//-----------------------------------------------------------------------------
void SimBuilderEMSExportDialog::setModel(smtk::model::Model *model)
{
  this->Model = model;
}
//-----------------------------------------------------------------------------
void SimBuilderEMSExportDialog::setAttManager(smtk::attribute::ManagerPtr manager)
{
  this->AttManager = manager;

  // Rebuild Analysis list
  std::map<std::string, std::set<std::string> > analysisMap =
    this->AttManager->analyses();

  if (analysisMap.empty())
    {
    this->Dialog->groupBox_AnalysisType->hide();
    }
  else
    {
    // Extract list of analysis names
    std::list<std::string> analysisNames;
    std::map<std::string, std::set<std::string> >::const_iterator mapIter;
    for(mapIter = analysisMap.begin(); mapIter != analysisMap.end(); mapIter++)
      {
      analysisNames.push_back(mapIter->first);
      }
    analysisNames.sort();

    // Create radio button for each analysis name
    QVBoxLayout* frameLayout = new QVBoxLayout(this->Dialog->groupBox_AnalysisType);
    std::list<std::string>::const_iterator nameIter;
    for(nameIter = analysisNames.begin(); nameIter != analysisNames.end(); nameIter++)
      {
      QRadioButton *button = new QRadioButton(this->Dialog->groupBox_AnalysisType);
      button->setText(QString(nameIter->c_str()));
      frameLayout->addWidget(button);

      QObject::connect(button, SIGNAL(clicked()),
        this, SLOT(analysisSelected()));

      // If there is only one analysis, or this one was last used, set checked
      if ((1 == analysisNames.size()) || (nameIter->c_str() == SelectedAnalysis))
        {
        button->setChecked(true);
        this->SelectedAnalysis = button->text();
        }
      }

    this->Dialog->groupBox_AnalysisType->show();
    }
}

//-----------------------------------------------------------------------------
QString SimBuilderEMSExportDialog::getFileName() const
{
  return  this->Dialog->FileNameText->text();
}

//-----------------------------------------------------------------------------
QString SimBuilderEMSExportDialog::getPythonScriptName() const
{
  return  this->Dialog->PythonScriptNameText->text();
}
//-----------------------------------------------------------------------------
QString SimBuilderEMSExportDialog::getAnalysisName() const
{
  return  this->SelectedAnalysis;
}
//-----------------------------------------------------------------------------
void SimBuilderEMSExportDialog::setActiveServer(pqServer* server)
{
  this->ActiveServer = server;
}

//-----------------------------------------------------------------------------
void SimBuilderEMSExportDialog::displayPythonScriptBrowser()
{
  pqFileDialog file_dialog(
    this->ActiveServer,
    this->MainDialog, tr("Open Python Script:"), QString(), QString("Python script files (*.py)"));
  file_dialog.setObjectName("InputDialog");
  file_dialog.setWindowModality(Qt::WindowModal);
  file_dialog.setFileMode(pqFileDialog::ExistingFile); //open
  if (file_dialog.exec() == QDialog::Accepted)
    {
    QStringList files = file_dialog.getSelectedFiles();
    if (files.size() > 0)
      {
      this->Dialog->PythonScriptNameText->setText(files[0]);
      this->pythonScriptChanged();
      }
    }
}
//-----------------------------------------------------------------------------
void SimBuilderEMSExportDialog::displayFileBrowser()
{
  pqFileDialog file_dialog(
    this->ActiveServer,
    this->MainDialog, tr("Save Simulation Output:"), QString(), QString("EMS simulation files (*.ems)"));
  file_dialog.setObjectName("OutputDialog");
  file_dialog.setWindowModality(Qt::WindowModal);
  file_dialog.setFileMode(pqFileDialog::AnyFile);
  if (file_dialog.exec() == QDialog::Accepted)
    {
    QStringList files = file_dialog.getSelectedFiles();
    if (files.size() > 0)
      {
      this->Dialog->FileNameText->setText(files[0]);
      this->validate();
      }
    }
}
//-----------------------------------------------------------------------------
void SimBuilderEMSExportDialog::analysisSelected()
{
  QRadioButton *button = dynamic_cast<QRadioButton *>(QObject::sender());
  if (button)
    {
    this->SelectedAnalysis = button->text();
    this->validate();
    }
}

//-----------------------------------------------------------------------------
void SimBuilderEMSExportDialog::validate()
{
  bool valid;
  // In order to be valid we need both a valid format and output file names
  valid = (this->getPythonScriptName() != "") &&
    (this->getFileName() != "") &&
    (this->SelectedAnalysis != "");
  this->setAcceptable(valid);
}
//-----------------------------------------------------------------------------
void SimBuilderEMSExportDialog::setAcceptable(bool state)
{
  this->Dialog->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(!state);
}

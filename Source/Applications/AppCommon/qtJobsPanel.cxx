//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "qtJobsPanel.h"

#include "smtk/extension/cumulus/cumuluswidget.h"

#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSslSocket>
#include <QString>
#include <QVBoxLayout>

//-----------------------------------------------------------------------------
class qtJobsPanel::qtJobsPanelInternal
{
 public:
  qtJobsPanelInternal();

  QWidget *MainWidget;
  QVBoxLayout *MainLayout;

  QWidget *FirstWidget;
  QLineEdit *CumulusUrlEdit;

  cumulus::CumulusWidget *CumulusWidget;
};

//-----------------------------------------------------------------------------
qtJobsPanel::qtJobsPanelInternal::qtJobsPanelInternal()
  : MainWidget(0),
    MainLayout(0),
    FirstWidget(0),
    CumulusUrlEdit(0),
    CumulusWidget(0)
{
}

//-----------------------------------------------------------------------------
qtJobsPanel::qtJobsPanel(QWidget *parent)
  : QDockWidget(parent)
{
  this->Internal = new qtJobsPanelInternal;

  // Initialize main widget
  this->Internal->MainWidget = new QWidget(parent);
  this->Internal->MainLayout = new QVBoxLayout;

  // Instantiate first page (displayed at startup)
  this->Internal->FirstWidget = new QWidget(this->Internal->MainWidget);
  QFormLayout *firstLayout = new QFormLayout;
  this->Internal->CumulusUrlEdit = new QLineEdit(this->Internal->FirstWidget);
  this->Internal->CumulusUrlEdit->setText("http://localhost:8080/api/v1");
  firstLayout->addRow("Cumulus URL", this->Internal->CumulusUrlEdit);
  QPushButton *authenticateButton = new QPushButton(
    "Authenticate", this->Internal->FirstWidget);
  firstLayout->addRow("NERSC Logon", authenticateButton);
  this->Internal->FirstWidget->setLayout(firstLayout);
  this->Internal->MainLayout->addWidget(this->Internal->FirstWidget);

  // Instantiate cumulus-monitoring objects
  this->Internal->CumulusWidget = new cumulus::CumulusWidget(this);
  this->Internal->MainLayout->addWidget(this->Internal->CumulusWidget);
  QObject::connect(
    authenticateButton, SIGNAL(clicked()), this, SLOT(authenticateHPC()));
  connect(this->Internal->CumulusWidget, SIGNAL(resultDownloaded(const QString &)),
          this, SIGNAL(resultDownloaded(const QString &)));

  // Finish main widget
  this->Internal->MainWidget->setLayout(this->Internal->MainLayout);
  this->setWidget(this->Internal->MainWidget);

  this->setObjectName("Jobs Panel");
}

//-----------------------------------------------------------------------------
qtJobsPanel::~qtJobsPanel()
{
  delete this->Internal;
}

//-----------------------------------------------------------------------------
void qtJobsPanel::authenticateHPC()
{
  // Check for SSL
  if (!QSslSocket::supportsSsl())
    {
    QMessageBox::critical(
      NULL,
      QObject::tr("SSL support"),
      QObject::tr("SSL support is required, you must rebuild Qt with SSL support."));
    return;
    }

  // Get cumulus/girder url
  QString cumulusUrl = this->Internal->CumulusUrlEdit->text();
  this->Internal->CumulusWidget->girderUrl(cumulusUrl);

  // Check for cumulus server
  if (!this->Internal->CumulusWidget->isGirderRunning())
    {
    QString msg = QString("Cumulus server NOT FOUND at %1").arg(cumulusUrl);
    QMessageBox::critical(
      NULL,
      QObject::tr("Cumulus Server Not Found"), msg);
    return;
    }

  // Open NERSC login dialog
  this->Internal->CumulusWidget->showLoginDialog();
}

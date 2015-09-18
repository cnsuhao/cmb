//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "qtCMBMeshViewerPanelWidget.h"
#include "ui_qtMeshViewerPanel.h"
#include <QMenu>
#include <QButtonGroup>

class qtCMBMeshViewerPanelWidgetInternal :
  public Ui::qtMeshViewerPanel
{
public:
  QIcon* IconVisible;
  QIcon* IconInvisible;
  QIcon* IconActive;
  QIcon* IconInactive;
  QMenu *ContextMenu;
  QAction *DeleteAction;
  QAction *ActiveInputAction;
  QAction *ExportSubsetAction;
  QButtonGroup* RadioGroup;
};

//-----------------------------------------------------------------------------
qtCMBMeshViewerPanelWidget::qtCMBMeshViewerPanelWidget(
  QWidget* _p): QWidget(_p)
{
  this->Internal = new qtCMBMeshViewerPanelWidgetInternal;
  this->Internal->setupUi(this);
  QPixmap pix(":/cmb/pqEyeball16.png");
  QPixmap pixd(":/cmb/pqEyeballd16.png");
  QPixmap pixa(":/cmb/GrowAccept.png");

  this->Internal->IconVisible = new QIcon(pix);
  this->Internal->IconInvisible = new QIcon(pixd);
  this->Internal->IconActive = new QIcon(pixa);
  this->Internal->IconInactive = new QIcon();
  this->Internal->treeInputs->setContextMenuPolicy(Qt::CustomContextMenu);
  QObject::connect(this->Internal->treeInputs,
    SIGNAL(customContextMenuRequested(const QPoint &)),
    this, SLOT(showContextMenu(const QPoint &)));

  // Set up Context Menu Structure
  this->Internal->ContextMenu = new QMenu(this->Internal->treeInputs);
  this->Internal->ContextMenu->setTitle("Filters Input");
  this->Internal->ActiveInputAction = new QAction(this->Internal->ContextMenu);
  this->Internal->ActiveInputAction->setObjectName(
    QString::fromUtf8("action_activeinput"));
  this->Internal->ActiveInputAction->setText(
    QString::fromUtf8("Set As Active"));
  this->Internal->ActiveInputAction->setEnabled(0);
  this->Internal->DeleteAction = new QAction(this->Internal->ContextMenu);
  this->Internal->DeleteAction->setObjectName(
    QString::fromUtf8("action_deleteinput"));
  this->Internal->DeleteAction->setText(QString::fromUtf8("Delete"));
  this->Internal->DeleteAction->setEnabled(0);
  this->Internal->ExportSubsetAction = new QAction(this->Internal->ContextMenu);
  this->Internal->ExportSubsetAction->setObjectName(
    QString::fromUtf8("action_exportsubset"));
  this->Internal->ExportSubsetAction->setText(QString::fromUtf8("Save Selected As..."));
  this->Internal->ExportSubsetAction->setEnabled(0);
  this->Internal->ContextMenu->addAction(this->Internal->ActiveInputAction);
  this->Internal->ContextMenu->addAction(this->Internal->ExportSubsetAction);
  this->Internal->ContextMenu->addAction(this->Internal->DeleteAction);
  this->Internal->ContextMenu->insertSeparator(this->Internal->DeleteAction);

  this->Internal->RadioGroup = new QButtonGroup(this->Internal->tab_Selection);
  this->Internal->RadioGroup->addButton(
    this->Internal->radioButtonShowHistogram);
  this->Internal->RadioGroup->addButton(
    this->Internal->radioButtonShowSpreadsheet);

  // Setup the material id line edit widget to use a validator
  this->Internal->MaterialIdEntry->
    setValidator(new QIntValidator(this->Internal->MaterialIdEntry));
}

//-----------------------------------------------------------------------------
qtCMBMeshViewerPanelWidget::~qtCMBMeshViewerPanelWidget()
{
  delete this->Internal->IconVisible;
  delete this->Internal->IconInvisible;
  delete this->Internal->IconActive;
  delete this->Internal->IconInactive;
  delete this->Internal->ActiveInputAction;
  delete this->Internal->DeleteAction;
  delete this->Internal->ExportSubsetAction;
  delete this->Internal->ContextMenu;
  delete this->Internal->RadioGroup;
  delete this->Internal;
}
//-----------------------------------------------------------------------------
QIcon* qtCMBMeshViewerPanelWidget::iconVisible()
{
return this->Internal->IconVisible;
}
//-----------------------------------------------------------------------------
QIcon* qtCMBMeshViewerPanelWidget::iconInvisible()
{
  return this->Internal->IconInvisible;
}
//-----------------------------------------------------------------------------
QIcon* qtCMBMeshViewerPanelWidget::iconActive()
{
  return this->Internal->IconActive;
}
//-----------------------------------------------------------------------------
QIcon* qtCMBMeshViewerPanelWidget::iconInactive()
{
  return this->Internal->IconInactive;
}
//-----------------------------------------------------------------------------
Ui::qtMeshViewerPanel* qtCMBMeshViewerPanelWidget::getGUIPanel()
{
  return this->Internal;
}
//-----------------------------------------------------------------------------
QAction* qtCMBMeshViewerPanelWidget::deleteInputAction()
{
  return this->Internal->DeleteAction;
}
//-----------------------------------------------------------------------------
QAction* qtCMBMeshViewerPanelWidget::activeInputAction()
{
 return this->Internal->ActiveInputAction;
}
//-----------------------------------------------------------------------------
QAction* qtCMBMeshViewerPanelWidget::exportSubsetAction()
{
  return this->Internal->ExportSubsetAction;
}
//-----------------------------------------------------------------------------
void qtCMBMeshViewerPanelWidget::showContextMenu(const QPoint &p)
{
  this->Internal->ContextMenu->popup(
    this->Internal->treeInputs->mapToGlobal(p));
}

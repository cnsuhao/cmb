//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "qtCMBPlacementConstraintWidget.h"
#include "ui_qtObjectPlacementConstraint.h"

#include "pqCMBSceneNode.h"
#include "pqFileDialog.h"
#include "pqCMBSceneTree.h"
#include <QMessageBox>
#include <QTableWidget>
#include <QTableWidgetItem>

#include <fstream>
#include <sys/stat.h>

// enum for different column types
enum ContraintTableCol
  {
  UseCol        = 0,
  NameCol       = 1,
  InvertCol     = 2
  };

class qtCMBPlacementConstraintWidgetInternal :
  public Ui::qtObjectPlacementConstraint
{
public:
  std::vector<pqCMBSceneNode *> VOIs;
  std::vector<pqCMBSceneNode *> Contours;
  QMap< pqCMBSceneNode*, int> SelectedContraints;
};

//-----------------------------------------------------------------------------
qtCMBPlacementConstraintWidget::qtCMBPlacementConstraintWidget(pqCMBSceneNode* node,
  QWidget* _p): QWidget(_p), parentNode(node)
{
  this->Internal = new qtCMBPlacementConstraintWidgetInternal;
  this->Internal->setupUi(this);

  this->setupConstraintTable();
  this->updateConstraintTable();
  QObject::connect(this->Internal->GlyphPointsFileBrowserButton, SIGNAL(clicked()),
                   this, SLOT(displayGlyphPlaybackFileBrowser()));
  QObject::connect(this->Internal->GlyphPlaybackOption, SIGNAL(currentIndexChanged(int)),
                   this, SLOT(checkGlyphPlaybackFile(int)));
}

//-----------------------------------------------------------------------------
qtCMBPlacementConstraintWidget::~qtCMBPlacementConstraintWidget()
{
  delete this->Internal;
}

//-----------------------------------------------------------------------------
Ui::qtObjectPlacementConstraint* qtCMBPlacementConstraintWidget::getWidget()
{
  return this->Internal;
}

//-----------------------------------------------------------------------------
void qtCMBPlacementConstraintWidget::setupConstraintTable()
{
  if(!this->parentNode)
    {
    return;
    }
  //Add the new filter to the table
  QTableWidget* table = this->Internal->tableWidget;

  table->setColumnCount(3);
  table->setHorizontalHeaderLabels(
    QStringList() << tr("Use") << tr("Constraint") << tr("Outside") );

  table->setSelectionMode(QAbstractItemView::SingleSelection);
  table->setSelectionBehavior(QAbstractItemView::SelectRows);
  table->verticalHeader()->hide();

  QObject::connect(table, SIGNAL(itemSelectionChanged()),
    this, SLOT(onTableSelectionChanged()), Qt::QueuedConnection);
  QObject::connect(table, SIGNAL(itemChanged(QTableWidgetItem*)),
    this, SLOT(onItemChanged(QTableWidgetItem*))/*, Qt::QueuedConnection*/);
}

//-----------------------------------------------------------------------------
void qtCMBPlacementConstraintWidget::updateConstraintTable()
{
  QTableWidget* table = this->Internal->tableWidget;
  table->clearContents();
  table->setRowCount(0);
  this->parentNode->getTree()->getVOIs(&(this->Internal->VOIs));
  int n = static_cast<int>(this->Internal->VOIs.size());
  int i;
  for (i = 0; i < n; i++)
    {
    this->addTableRow(this->Internal->VOIs[i]);
    }

  this->parentNode->getTree()->getArcs(&(this->Internal->Contours));
  n = static_cast<int>(this->Internal->Contours.size());
  for (i = 0; i < n; i++)
    {
    this->addTableRow(this->Internal->Contours[i]);
    }
}
//-----------------------------------------------------------------------------
void qtCMBPlacementConstraintWidget::addTableRow(pqCMBSceneNode* scenenode)
{
  //Add the new filter to the table
  QTableWidget* table = this->Internal->tableWidget;
  int row_count = table->rowCount();
  table->setRowCount(row_count+1);

  Qt::ItemFlags commFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
  QTableWidgetItem* objItem = new QTableWidgetItem();
  QVariant vdata;
  vdata.setValue(static_cast<void*>(scenenode));
  objItem->setData(Qt::UserRole, vdata);
  table->setItem(row_count, UseCol, objItem); // column 0
  objItem->setFlags(commFlags | Qt::ItemIsUserCheckable);
  objItem->setCheckState(Qt::Unchecked);

  QTableWidgetItem *nameItem = new QTableWidgetItem();
  nameItem->setText(scenenode->getName());
  table->setItem(row_count,NameCol,nameItem);

  QTableWidgetItem* invertItem = new QTableWidgetItem();
  table->setItem(row_count, InvertCol, invertItem);
  invertItem->setFlags(commFlags | Qt::ItemIsUserCheckable);
  invertItem->setCheckState(Qt::Unchecked);
}

//-----------------------------------------------------------------------------
void qtCMBPlacementConstraintWidget:: onTableSelectionChanged()
{

}
//-----------------------------------------------------------------------------
void qtCMBPlacementConstraintWidget::onItemChanged(QTableWidgetItem* /*item*/)
{

}
//-----------------------------------------------------------------------------
const QMap <pqCMBSceneNode*, int>& qtCMBPlacementConstraintWidget::getSelectedConstraints() const
{
  this->Internal->SelectedContraints.clear();

  QTableWidget* table = this->Internal->tableWidget;
  for(int row=0; row<table->rowCount(); row++)
    {
    pqCMBSceneNode* node = static_cast<pqCMBSceneNode*>(
      this->Internal->tableWidget->item(row, UseCol)
      ->data(Qt::UserRole).value<void *>());
    if( node && table->item(row, UseCol)->checkState() == Qt::Checked)
      {
      int invert = (table->item(row, InvertCol)->checkState() == Qt::Checked) ? 1 : 0;
      this->Internal->SelectedContraints[node] = invert;
      }
    }
  return this->Internal->SelectedContraints;
}
//-----------------------------------------------------------------------------
int qtCMBPlacementConstraintWidget::getPlacementCount()
{
  return this->Internal->RandomCount->value();
}
//-----------------------------------------------------------------------------
void qtCMBPlacementConstraintWidget::enableGlyphOption(bool mode)
{
  this->Internal->glyphOption->setEnabled(mode);
  if (!mode)
    {
    this->Internal->glyphOption->setChecked(false);
    }
}
//-----------------------------------------------------------------------------
void qtCMBPlacementConstraintWidget::enableTextureConstraintOption(bool mode)
{
  this->Internal->useTextureAsConstraint->setEnabled(mode);
  if (!mode)
    {
    this->Internal->useTextureAsConstraint->setChecked(false);
    }
}
//-----------------------------------------------------------------------------
bool qtCMBPlacementConstraintWidget::useGlyphs() const
{
  return (this->Internal->glyphOption->isEnabled()) &&
    (this->Internal->glyphOption->isChecked());
}
//-----------------------------------------------------------------------------
bool qtCMBPlacementConstraintWidget::useTextureConstraint() const
{
  return (this->Internal->useTextureAsConstraint->isEnabled()) &&
    (this->Internal->useTextureAsConstraint->isChecked());
}
//-----------------------------------------------------------------------------
void qtCMBPlacementConstraintWidget::displayGlyphPlaybackFileBrowser()
{
  QString filters = "Glyph Playback Files (*.gp);;All files (*)";

  pqCMBSceneTree *tree = this->parentNode->getTree();
  pqFileDialog file_dialog(tree->getCurrentServer(),
                                                     this,
                                                     tr("Open/Save As File:"),
                                                     QString(),
                                                     filters);

  file_dialog.setObjectName("GlyphPlaybackFileImportDialog");
  if(this->Internal->GlyphPlaybackOption->currentIndex() == 0)
    {
    file_dialog.setFileMode(pqFileDialog::AnyFile);
    }
  else if(this->Internal->GlyphPlaybackOption->currentIndex() == 1)
    {
    file_dialog.setFileMode(pqFileDialog::ExistingFile);
    }
  else
    {
    return;
    }
  QObject::connect(&file_dialog, SIGNAL(filesSelected(const QList<QStringList>&)),
    this, SLOT(filesSelected(const QList<QStringList>&)));
  file_dialog.setWindowModality(Qt::WindowModal);
  file_dialog.exec();
}
//-----------------------------------------------------------------------------
void qtCMBPlacementConstraintWidget::filesSelected(const QList<QStringList> &files)
{
  if (files.size() == 0)
    {
    this->Internal->GlyphPointsFileNameText->setText("");
    }

  this->Internal->GlyphPointsFileNameText->setText(files[0][0]);
}
//-----------------------------------------------------------------------------
void qtCMBPlacementConstraintWidget::showGlyphPlaybackGroupBox(bool show) const
{
  if(show)
    {
    this->Internal->GlyphPlaybackGroupBox->show();
    this->Internal->GlyphPlaybackGroupBox->setEnabled(true);
    }
  else
    {
    this->Internal->GlyphPlaybackGroupBox->setEnabled(false);
    this->Internal->GlyphPlaybackGroupBox->hide();
    }
}
//-----------------------------------------------------------------------------
int qtCMBPlacementConstraintWidget::getGlyphPlaybackOption() const
{
  return this->Internal->GlyphPlaybackOption->currentIndex();
}
//-----------------------------------------------------------------------------
QString qtCMBPlacementConstraintWidget::getGlyphPlaybackFilename() const
{
 return this->Internal->GlyphPointsFileNameText->text();
}

//-----------------------------------------------------------------------------
void qtCMBPlacementConstraintWidget::checkGlyphPlaybackFile(int option)
{
  if(option == 0)
    {
    if(this->fileExists(this->Internal->GlyphPointsFileNameText->text()))
      {
      if(QMessageBox::No == QMessageBox::warning(
            this,
            this->windowTitle(),
            QString(tr("%1 already exists. \nDo you want to replace it?")).arg(this->Internal->GlyphPointsFileNameText->text()),
            QMessageBox::Yes,
            QMessageBox::No))
        {
        displayGlyphPlaybackFileBrowser();
        }
      }
    }
  else if(option == 1)
    {
    if(QString::compare(this->Internal->GlyphPointsFileNameText->text(),"") == 0)
      {
      return;
      }
    if(!this->fileExists(this->Internal->GlyphPointsFileNameText->text()))
      {
      QMessageBox::warning(
                  this,
                  this->windowTitle(),
                  QString(tr("%1 does not exist. \nSelect a valid glyph playback file.")).arg(this->Internal->GlyphPointsFileNameText->text()));
      displayGlyphPlaybackFileBrowser();
      }
    }
}

//-----------------------------------------------------------------------------
bool qtCMBPlacementConstraintWidget::fileExists(QString fileName) const
{
  struct stat buf;
  if( stat(fileName.toLatin1().data(), &buf) != -1)
    {
    return true;
    }
  return false;
}

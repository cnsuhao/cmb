//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "pqCMBLIDARPieceTable.h"

#include "pqCMBLIDARPieceObject.h"

#include "pqCMBLIDARPieceObject.h"
#include "pqDataRepresentation.h"
#include "pqPipelineSource.h"
#include "pqRenderView.h"
#include "pqSMAdaptor.h"
#include "vtkSMRepresentationProxy.h"
#include <QHeaderView>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <vtksys/SystemTools.hxx>

// enum for different column types
enum DataTableCol
{
  VisibilityCol = 0,
  FileNameCol,
  PieceIndexCol,
  DisplayOnRatioCol,
  NumDisplayPtsCol,
  SaveOnRatioCol,
  NumSavePtsCol
};

pqCMBLIDARPieceTable::pqCMBLIDARPieceTable(QTableWidget* tableWidget, bool advancedTable /*=false*/)
{
  this->TableWidget = tableWidget;
  this->clear();
  this->initialize(advancedTable);
}

pqCMBLIDARPieceTable::~pqCMBLIDARPieceTable()
{
}

QList<pqCMBLIDARPieceObject*> pqCMBLIDARPieceTable::getAllPieceObjects()
{
  QList<pqCMBLIDARPieceObject*> selObjects;
  for (int i = 0; i < this->TableWidget->rowCount(); i++)
  {
    pqCMBLIDARPieceObject* dataObj = this->getRowObject(i);
    if (dataObj)
    {
      selObjects.push_back(dataObj);
    }
  }
  return selObjects;
}

QList<pqCMBLIDARPieceObject*> pqCMBLIDARPieceTable::getVisiblePieceObjects()
{
  QList<pqCMBLIDARPieceObject*> selObjects;
  for (int i = 0; i < this->TableWidget->rowCount(); i++)
  {
    if (this->TableWidget->item(i, VisibilityCol)->checkState() == Qt::Checked)
    {
      pqCMBLIDARPieceObject* dataObj = this->getRowObject(i);
      if (dataObj)
      {
        selObjects.push_back(dataObj);
      }
    }
  }
  return selObjects;
}

pqCMBLIDARPieceObject* pqCMBLIDARPieceTable::getCurrentObject(bool onlyIfSelectable /*=true*/)
{
  return this->getRowObject(this->getCurrentRow(onlyIfSelectable));
}

int pqCMBLIDARPieceTable::getCurrentRow(bool onlyIfSelectable /*=true*/)
{
  if (this->TableWidget->selectedItems().count() == 0)
  {
    return -1;
  }

  int selectedRow = this->TableWidget->selectedItems()[0]->row();
  if (!onlyIfSelectable || this->TableWidget->item(selectedRow, PieceIndexCol)->isSelected())
  {
    return selectedRow;
  }
  return -1;
}

pqCMBLIDARPieceObject* pqCMBLIDARPieceTable::getRowObject(int row)
{
  return (row >= 0 && row < this->TableWidget->rowCount())
    ? static_cast<pqCMBLIDARPieceObject*>(
        this->TableWidget->item(row, VisibilityCol)->data(Qt::UserRole).value<void*>())
    : NULL;
}

int pqCMBLIDARPieceTable::getObjectRow(pqCMBLIDARPieceObject* dataObj)
{
  for (int row = 0; row < this->TableWidget->rowCount(); row++)
  {
    if (dataObj == this->getRowObject(row))
    {
      return row;
    }
  }
  return -1;
}

void pqCMBLIDARPieceTable::setCurrentPiece(int pieceId)
{
  if (this->getCurrentObject() && this->getCurrentObject()->getPieceIndex() == pieceId)
  {
    return;
  }

  pqCMBLIDARPieceObject* dataObj = NULL;
  for (int row = 0; row < this->TableWidget->rowCount(); row++)
  {
    dataObj = this->getRowObject(row);
    if (dataObj && dataObj->getPieceIndex() == pieceId)
    {
      this->TableWidget->setCurrentItem(this->TableWidget->item(row, VisibilityCol));
      this->TableWidget->item(row, VisibilityCol)->setSelected(true);
      break;
    }
  }
}

void pqCMBLIDARPieceTable::setCurrentOnRatio(int onRatio)
{
  int currentRow = this->getCurrentRow();
  if (currentRow >= 0 && this->getRowObject(currentRow)->getDisplayOnRatio() != onRatio)
  {
    this->TableWidget->item(currentRow, DisplayOnRatioCol)->setText(QString::number(onRatio));
  }
}

void pqCMBLIDARPieceTable::checkAll()
{
  this->TableWidget->blockSignals(true);
  // if something is selected, we want it to stay selected;  but if nothing is
  // selected, don't want that to change (so we clearSelection() at end of
  // function)
  int currentRow = this->getCurrentRow();
  QList<int> newChecked;
  QList<int> newUnChecked;
  for (int row = 0; row < this->TableWidget->rowCount(); row++)
  {
    QTableWidgetItem* item = this->TableWidget->item(row, VisibilityCol);
    if (item->checkState() == Qt::Unchecked)
    {
      newChecked.append(item->row());
      item->setCheckState(Qt::Checked);
      this->setVisibility(row, 1);
    }
  }
  if (this->LinkedTable)
  {
    this->LinkedTable->updateCheckedItems(newChecked, newUnChecked);
  }

  if (currentRow < 0)
  {
    this->TableWidget->clearSelection();
    if (this->LinkedTable)
    {
      this->LinkedTable->getWidget()->clearSelection();
    }
  }
  else
  {
    this->TableWidget->selectRow(currentRow);
    if (this->LinkedTable)
    {
      this->LinkedTable->getWidget()->selectRow(currentRow);
    }
  }
  this->TableWidget->blockSignals(false);

  if (newChecked.size() > 0)
  {
    emit this->requestRender();
  }

  emit this->checkedObjectsChanged(newChecked, newUnChecked);
}

void pqCMBLIDARPieceTable::uncheckAll()
{
  this->TableWidget->blockSignals(true);
  QList<int> newChecked;
  QList<int> newUnChecked;
  for (int row = 0; row < this->TableWidget->rowCount(); row++)
  {
    QTableWidgetItem* item = this->TableWidget->item(row, VisibilityCol);
    if (item->checkState() == Qt::Checked)
    {
      newUnChecked.append(item->row());
      this->getRowObject(item->row())->setHighlight(0);
      this->setVisibility(row, 0);
      item->setCheckState(Qt::Unchecked);
    }
  }
  this->TableWidget->blockSignals(false);
  if (this->LinkedTable)
  {
    this->LinkedTable->updateCheckedItems(newChecked, newUnChecked);
  }
  if (newUnChecked.size() > 0)
  {
    emit this->requestRender();
  }
  emit this->checkedObjectsChanged(newChecked, newUnChecked);
}

void pqCMBLIDARPieceTable::initialize(bool advancedTable)
{
  // Set up the data table
  if (advancedTable)
  {
    this->TableWidget->setColumnCount(7);
    this->TableWidget->setHorizontalHeaderLabels(QStringList()
      << tr("Visible") << tr("File Name") << tr("Piece ID") << tr("Display\nRatio")
      << tr("Display\n# Points") << tr("Save\nRatio") << tr("Save\n# Points"));
  }
  else
  {
    this->TableWidget->setColumnCount(3);
    this->TableWidget->setHorizontalHeaderLabels(
      QStringList() << tr("Visible") << tr("File Name") << tr("Piece ID"));
  }

  this->TableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
  this->TableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
  this->TableWidget->verticalHeader()->hide();

  QObject::connect(this->TableWidget, SIGNAL(itemSelectionChanged()), this,
    SLOT(onCurrentObjectChanged()), Qt::QueuedConnection);
  QObject::connect(this->TableWidget, SIGNAL(itemChanged(QTableWidgetItem*)), this,
    SLOT(onItemChanged(QTableWidgetItem*)) /*, Qt::QueuedConnection*/);
}

void pqCMBLIDARPieceTable::clear()
{
  this->TableWidget->clearContents();
  this->TableWidget->setRowCount(0);
  this->LinkedTable = 0;
  this->ClipEnabled = false;
  this->configureTable(false);
}

void pqCMBLIDARPieceTable::onClearSelection()
{
  this->TableWidget->blockSignals(true);
  this->TableWidget->clearSelection();
  if (this->LinkedTable)
  {
    this->LinkedTable->getWidget()->clearSelection();
  }
  this->TableWidget->blockSignals(false);
}

void pqCMBLIDARPieceTable::configureTable(int advancedTable)
{
  if (advancedTable)
  {
    this->TableWidget->setColumnCount(7);
    this->TableWidget->setHorizontalHeaderLabels(QStringList()
      << tr("Visible") << tr("File Name") << tr("Piece ID") << tr("Display\nRatio")
      << tr("Display\n# Points") << tr("Save\nRatio") << tr("Save\n# Points"));

    for (int row = 0; row < this->TableWidget->rowCount(); row++)
    {
      this->addAdvancedColumns(row, this->getRowObject(row));
    }
    this->TableWidget->resizeColumnsToContents();
    int currentRow = this->getCurrentRow();
    if (currentRow >= 0)
    {
      this->TableWidget->selectRow(currentRow);
    }
  }
  else
  {
    this->TableWidget->removeColumn(NumSavePtsCol);
    this->TableWidget->removeColumn(SaveOnRatioCol);
    this->TableWidget->removeColumn(NumDisplayPtsCol);
    this->TableWidget->removeColumn(DisplayOnRatioCol);
  }
}

void pqCMBLIDARPieceTable::AddLIDARPiece(pqCMBLIDARPieceObject* dataObj, int visible)
{
  this->TableWidget->insertRow(this->TableWidget->rowCount());
  int row = this->TableWidget->rowCount() - 1;

  Qt::ItemFlags commFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

  // The new file name column should also have a checkbox
  // so that it will control all its pieces --- TODO
  std::string filename = vtksys::SystemTools::GetFilenameName(dataObj->getFileName()).c_str();
  QTableWidgetItem* fileItem = new QTableWidgetItem(filename.c_str());
  QVariant vfiledata;
  vfiledata.setValue(static_cast<void*>(const_cast<char*>(dataObj->getFileName().c_str())));
  fileItem->setData(Qt::UserRole, vfiledata);
  this->TableWidget->setItem(row, FileNameCol, fileItem);
  //fileItem->setFlags(commFlags | Qt::ItemIsUserCheckable);
  //fileItem->setCheckState(visible ? Qt::Checked : Qt::Unchecked);
  fileItem->setFlags(commFlags);

  QTableWidgetItem* objItem = new QTableWidgetItem();
  QVariant vdata;
  vdata.setValue(static_cast<void*>(dataObj));
  objItem->setData(Qt::UserRole, vdata);

  this->TableWidget->setItem(row, VisibilityCol, objItem);
  objItem->setFlags(commFlags | Qt::ItemIsUserCheckable);
  objItem->setCheckState(visible ? Qt::Checked : Qt::Unchecked);

  // everybody can use the same brush.  Future changes to the brush
  // (color only?) will be done using the existing brush as a baseline
  QColor objIsUpToDateColor(255, 255, 255);
  QBrush defaultItemBrush(objIsUpToDateColor);
  objItem->setBackground(defaultItemBrush);

  if (dataObj->getPieceName().size() > 0)
  {
    this->TableWidget->setItem(
      row, PieceIndexCol, new QTableWidgetItem(dataObj->getPieceName().c_str()));
  }
  else
  {
    this->TableWidget->setItem(
      row, PieceIndexCol, new QTableWidgetItem(QString::number(dataObj->getPieceIndex())));
  }
  this->TableWidget->item(row, PieceIndexCol)->setFlags(commFlags);
  this->TableWidget->item(row, PieceIndexCol)->setTextAlignment(Qt::AlignCenter);
  this->TableWidget->item(row, PieceIndexCol)->setBackground(defaultItemBrush);

  if (this->TableWidget->columnCount() > 3)
  {
    this->addAdvancedColumns(row, dataObj);
  }

  this->TableWidget->resizeColumnsToContents();
  this->setVisibility(row, visible);
}

void pqCMBLIDARPieceTable::addAdvancedColumns(int row, pqCMBLIDARPieceObject* dataObj)
{
  Qt::ItemFlags commFlags = this->TableWidget->item(row, 2)->flags();
  QBrush backgroundBrush = this->TableWidget->item(row, 2)->background();

  this->TableWidget->setItem(row, NumDisplayPtsCol,
    new QTableWidgetItem(QString::number(dataObj->getNumberOfDisplayPointsEstimate())));
  this->TableWidget->item(row, NumDisplayPtsCol)->setFlags(commFlags);
  this->TableWidget->item(row, NumDisplayPtsCol)->setTextAlignment(Qt::AlignCenter);
  this->TableWidget->item(row, NumDisplayPtsCol)->setBackground(backgroundBrush);

  this->TableWidget->setItem(
    row, DisplayOnRatioCol, new QTableWidgetItem(QString::number(dataObj->getDisplayOnRatio())));
  this->TableWidget->item(row, DisplayOnRatioCol)->setFlags(commFlags | Qt::ItemIsEditable);
  this->TableWidget->item(row, DisplayOnRatioCol)->setTextAlignment(Qt::AlignCenter);
  this->TableWidget->item(row, DisplayOnRatioCol)->setBackground(backgroundBrush);

  this->TableWidget->setItem(row, NumSavePtsCol,
    new QTableWidgetItem(QString::number(dataObj->getNumberOfSavePointsEstimate())));
  this->TableWidget->item(row, NumSavePtsCol)->setFlags(commFlags);
  this->TableWidget->item(row, NumSavePtsCol)->setTextAlignment(Qt::AlignCenter);
  this->TableWidget->item(row, NumSavePtsCol)->setBackground(backgroundBrush);

  this->TableWidget->setItem(
    row, SaveOnRatioCol, new QTableWidgetItem(QString::number(dataObj->getSaveOnRatio())));
  this->TableWidget->item(row, SaveOnRatioCol)->setFlags(commFlags | Qt::ItemIsEditable);
  this->TableWidget->item(row, SaveOnRatioCol)->setTextAlignment(Qt::AlignCenter);
  this->TableWidget->item(row, SaveOnRatioCol)->setBackground(backgroundBrush);
}

void pqCMBLIDARPieceTable::updateWithPieceInfo(int row)
{
  this->updateWithPieceInfo(this->getRowObject(row), row);
}

void pqCMBLIDARPieceTable::updateWithPieceInfo(pqCMBLIDARPieceObject* dataObj, int row /*=-1*/)
{
  if (dataObj)
  {
    if (row == -1)
    {
      row = this->getObjectRow(dataObj);
    }

    QColor objNeedsUpdatingColor(255, 221, 218); // light red / rose
    QColor objIsUpToDateColor(255, 255, 255);

    bool isUpToDate = dataObj->isObjectUpToDate(this->ClipEnabled, this->ClipBBox);
    // everybody can use the same brush
    QBrush brush = this->TableWidget->item(row, VisibilityCol)->background();
    brush.setColor(isUpToDate ? objIsUpToDateColor : objNeedsUpdatingColor);

    this->TableWidget->blockSignals(true);
    this->TableWidget->item(row, VisibilityCol)->setBackground(brush);
    this->TableWidget->item(row, FileNameCol)->setBackground(brush);
    this->TableWidget->item(row, PieceIndexCol)->setBackground(brush);

    if (this->TableWidget->columnCount() > 3)
    {
      this->TableWidget->item(row, NumDisplayPtsCol)->setBackground(brush);
      this->TableWidget->item(row, NumSavePtsCol)->setBackground(brush);
      this->TableWidget->item(row, DisplayOnRatioCol)->setBackground(brush);
      this->TableWidget->item(row, SaveOnRatioCol)->setBackground(brush);

      this->TableWidget->item(row, NumDisplayPtsCol)
        ->setText(QString::number(dataObj->getNumberOfDisplayPointsEstimate()));
      this->TableWidget->item(row, NumSavePtsCol)
        ->setText(QString::number(dataObj->getNumberOfSavePointsEstimate()));
      this->TableWidget->item(row, DisplayOnRatioCol)
        ->setText(QString::number(dataObj->getDisplayOnRatio()));
      this->TableWidget->item(row, SaveOnRatioCol)
        ->setText(QString::number(dataObj->getSaveOnRatio()));
    }
    this->TableWidget->blockSignals(false);
  }
}

int pqCMBLIDARPieceTable::computeDisplayNumberOfPointsEstimate(pqCMBLIDARPieceObject* dataObj)
{
  int numDisplayPoints;
  bool isUpToDate = dataObj->isObjectUpToDate(this->ClipEnabled, this->ClipBBox);
  bool clipUpToDate = true;
  if (isUpToDate)
  {
    numDisplayPoints = dataObj->getNumberOfReadPoints();
  }
  else
  {
    if (!this->ClipEnabled)
    {
      numDisplayPoints = dataObj->getNumberOfPoints() / dataObj->getDisplayOnRatio();
    }
    else
    {
      clipUpToDate = dataObj->isClipUpToDate(this->ClipEnabled, this->ClipBBox);

      // depends on clip state how to calculate...
      if (clipUpToDate)
      {
        numDisplayPoints = double(dataObj->getNumberOfReadPoints() * dataObj->getReadOnRatio()) /
          dataObj->getDisplayOnRatio();
      }
      else
      {
        numDisplayPoints = dataObj->getNumberOfPoints() / dataObj->getDisplayOnRatio();
      }
    }
  }

  dataObj->setNumberOfDisplayPointsEstimate(numDisplayPoints);
  return numDisplayPoints;
}

int pqCMBLIDARPieceTable::computeSaveNumberOfPointsEstimate(pqCMBLIDARPieceObject* dataObj)
{
  int numSavePoints;
  if (!this->ClipEnabled)
  {
    numSavePoints = dataObj->getNumberOfPoints() / dataObj->getSaveOnRatio();
  }
  else
  {
    numSavePoints = (dataObj->getNumberOfDisplayPointsEstimate() * dataObj->getDisplayOnRatio()) /
      dataObj->getSaveOnRatio();
    if (numSavePoints > dataObj->getNumberOfPoints())
    {
      numSavePoints = dataObj->getNumberOfPoints();
    }
  }
  dataObj->setNumberOfSavePointsEstimate(numSavePoints);
  return numSavePoints;
}

QMap<int, int> pqCMBLIDARPieceTable::getSelectedPieceOnRatios()
{
  QMap<int, int> selIndices;
  QTableWidgetItem* item;
  int pieceId;
  for (int i = 0; i < this->TableWidget->rowCount(); i++)
  {
    item = this->TableWidget->item(i, VisibilityCol);
    if (item->checkState() == Qt::Checked)
    {
      pieceId = this->TableWidget->item(i, PieceIndexCol)->text().toInt();
      selIndices[pieceId] = this->TableWidget->item(i, DisplayOnRatioCol)->text().toInt();
    }
  }

  return selIndices;
}

void pqCMBLIDARPieceTable::onCurrentObjectChanged()
{
  int currentRow = this->getCurrentRow(false);
  if (this->LinkedTable)
  {
    if (currentRow >= 0)
    {
      this->LinkedTable->setSelection(currentRow);
    }
    else
    {
      this->LinkedTable->getWidget()->clearSelection();
    }
  }
  emit this->currentObjectChanged(this->getCurrentObject());
}

void pqCMBLIDARPieceTable::onItemChanged(QTableWidgetItem* item)
{
  pqCMBLIDARPieceObject* dataObj = this->getRowObject(item->row());
  if (!dataObj)
  {
    return;
  }
  if (item->column() == VisibilityCol)
  {
    QList<int> newChecked;
    QList<int> newUnChecked;
    if (item->checkState() == Qt::Checked)
    {
      newChecked.append(item->row());
      this->setVisibility(item->row(), 1);
    }
    else
    {
      newUnChecked.append(item->row());
      this->setVisibility(item->row(), 0);
    }
    emit this->currentObjectChanged(this->getCurrentObject());
    if (this->LinkedTable)
    {
      this->LinkedTable->updateCheckedItems(newChecked, newUnChecked);
    }
    emit this->checkedObjectsChanged(newChecked, newUnChecked);
  }
  else if (item->column() == DisplayOnRatioCol)
  {
    int onRatio = item->text().toInt();
    if (onRatio != dataObj->getDisplayOnRatio())
    {
      dataObj->setDisplayOnRatio(onRatio);
      this->computeDisplayNumberOfPointsEstimate(dataObj);
      this->computeSaveNumberOfPointsEstimate(dataObj);
      emit this->objectOnRatioChanged(dataObj, onRatio);
    }
  }
  else if (item->column() == SaveOnRatioCol)
  {
    int onRatio = item->text().toInt();
    if (onRatio != dataObj->getSaveOnRatio())
    {
      dataObj->setSaveOnRatio(onRatio);
      this->computeSaveNumberOfPointsEstimate(dataObj);
      emit this->objectOnRatioChanged(dataObj, onRatio);
    }
  }
}

void pqCMBLIDARPieceTable::setVisibility(int row, int visibilityState)
{
  bool signalsAlreadyBlocked = this->TableWidget->signalsBlocked();
  if (!signalsAlreadyBlocked)
  {
    this->TableWidget->blockSignals(true);
  }
  pqCMBLIDARPieceObject* dataObj = this->getRowObject(row);
  if (dataObj)
  {
    dataObj->setVisibility(visibilityState);
    bool selectableFlag = this->TableWidget->item(row, 2)->flags() & Qt::ItemIsSelectable;
    if ((visibilityState && selectableFlag) || (!visibilityState && !selectableFlag))
    {
      if (!signalsAlreadyBlocked)
      {
        this->TableWidget->blockSignals(false);
      }
      return; // already in the desired selectable state... but how???
    }
    for (int column = 2; column < this->TableWidget->columnCount(); column++)
    {
      Qt::ItemFlags flags = this->TableWidget->item(row, column)->flags();
      flags ^= Qt::ItemIsSelectable;
      this->TableWidget->item(row, column)->setFlags(flags);
    }
    // may not have been selectable before turning on the visibility
    if (visibilityState && !this->TableWidget->item(row, 0)->isSelected())
    {
      this->TableWidget->selectRow(row);
    }
  }
  if (!signalsAlreadyBlocked)
  {
    this->TableWidget->blockSignals(false);
  }
}

void pqCMBLIDARPieceTable::updateCheckedItems(QList<int> newChecked, QList<int> newUnChecked)
{
  this->TableWidget->blockSignals(true);
  QList<int>::iterator rowIter;
  for (rowIter = newChecked.begin(); rowIter != newChecked.end(); rowIter++)
  {
    this->TableWidget->item(*rowIter, VisibilityCol)->setCheckState(Qt::Checked);
    this->setVisibility(*rowIter, 1);
  }
  for (rowIter = newUnChecked.begin(); rowIter != newUnChecked.end(); rowIter++)
  {
    this->TableWidget->item(*rowIter, VisibilityCol)->setCheckState(Qt::Unchecked);
    this->setVisibility(*rowIter, 0);
  }
  this->TableWidget->blockSignals(false);
}

void pqCMBLIDARPieceTable::setSelection(int selectionRow)
{
  this->TableWidget->blockSignals(true);
  this->TableWidget->selectRow(selectionRow);
  this->TableWidget->blockSignals(false);
}

void pqCMBLIDARPieceTable::setOnRatioOfSelectedPieces(int newOnRatio)
{
  for (int i = 0; i < this->TableWidget->rowCount(); i++)
  {
    if (this->TableWidget->item(i, VisibilityCol)->checkState() == Qt::Checked)
    {
      this->TableWidget->item(i, DisplayOnRatioCol)->setText(QString::number(newOnRatio));
    }
  }
}

void pqCMBLIDARPieceTable::selectObject(pqPipelineSource* selSource)
{
  for (int i = 0; i < this->TableWidget->rowCount(); i++)
  {
    pqCMBLIDARPieceObject* pieceObj = this->getRowObject(i);
    if (pieceObj &&
      (pieceObj->getElevationSource() == selSource || pieceObj->getSource() == selSource))
    {
      this->TableWidget->selectRow(i);
      break;
    }
  }
}

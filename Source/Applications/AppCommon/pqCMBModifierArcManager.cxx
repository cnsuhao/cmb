//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "pqCMBModifierArcManager.h"

#include "pqCMBLIDARPieceObject.h"

#include "pqApplicationCore.h"
#include "pqObjectBuilder.h"
#include "pqCMBArc.h"

#include <QTableWidget>
#include <QTableWidgetItem>
#include <QDebug>
#include <QDialog>
#include <QHeaderView>
#include <QGridLayout>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFileInfo>

#include <fstream>
#include <sstream>

#include "pqSMAdaptor.h"
#include "pqCMBModifierArc.h"
#include "pqDataRepresentation.h"
#include "pqPipelineSource.h"
#include "pqRenderView.h"
#include "vtkSMRepresentationProxy.h"
#include <vtksys/SystemTools.hxx>
#include "ui_qtArcFunctionControl.h"
#include "ui_qtModifierArcDialog.h"
#include "vtkPVArcInfo.h"

// enum for different column types
enum DataTableCol
{
  VisibilityCol=0,
  Id,
  Symmetric,
  Relative,
  MinDisp,
  MaxDisp,
  LeftDist,
  RightDist
};

//-----------------------------------------------------------------------------
pqCMBModifierArcManager::pqCMBModifierArcManager(QLayout *layout,
                                                 pqServer *server,
                                                 pqRenderView *renderer)
{
  this->CurrentModifierArc = NULL;
  this->UI = new Ui_qtArcFunctionControl();
  QWidget * w = new QWidget();
  this->UI->setupUi(w);
  if(layout != NULL)
  {
    layout->addWidget(w);
    this->Dialog = NULL;
    this->UI_Dialog = NULL;
  }
  else
  {
    this->Dialog = new QDialog();
    this->UI_Dialog = new Ui_qtModifierArcDialog();
    this->UI_Dialog->setupUi(this->Dialog);
    this->UI_Dialog->modifierLayout->addWidget(w);
    QPushButton* applyButton = this->UI_Dialog->buttonBox->button(QDialogButtonBox::Apply);
    connect(applyButton, SIGNAL(clicked()), this, SLOT(accepted()));
    //connect(this->Dialog, SIGNAL(rejected()), this, SLOT(clear()));
  }
  this->TableWidget = this->UI->ModifyArcTable;
  this->clear();
  this->initialize();
  QObject::connect(this->UI->addLineButton, SIGNAL(clicked()),
                   this, SLOT(addLine()));
  QObject::connect(this->UI->removeLineButton, SIGNAL(clicked()),
                   this, SLOT(removeArc()));
  QObject::connect(this->UI->buttonUpdateLine, SIGNAL(clicked()),
                   this, SLOT(update()));
  QObject::connect(this->UI->ApplyAgain, SIGNAL(clicked()),
                   this, SLOT(applyAgain()));
    {
    QGridLayout* gridlayout = new QGridLayout(this->UI->displacementChartFrame);
    gridlayout->setMargin(0);
    this->DisplacementProfile = new pqGeneralTransferFunctionWidget();
    gridlayout->addWidget(this->DisplacementProfile);
    this->DisplacementProfile->addFunction(NULL, false);
    connect( this, SIGNAL(changeDisplacementFunctionType(bool)),
             this->DisplacementProfile, SLOT(setFunctionType(bool)));
    connect(this->DisplacementProfile, SIGNAL(controlPointsModified()),
            this->DisplacementProfile, SLOT(render()));
    connect(this->UI->dispacementMinDepthValue, SIGNAL(valueChanged(double)),
            this->DisplacementProfile, SLOT(setMinY(double)));
    connect(this->UI->displacementMaxDepthValue, SIGNAL(valueChanged(double)),
            this->DisplacementProfile, SLOT(setMaxY(double)));
    connect(this->UI->leftValue, SIGNAL(valueChanged(double)),
            this->DisplacementProfile, SLOT(setMinX(double)));
    connect(this->UI->rightValue, SIGNAL(valueChanged(double)),
            this->DisplacementProfile, SLOT(setMaxX(double)));
    }

    {
    QGridLayout* gridlayout = new QGridLayout(this->UI->weightingChartFrame);
    gridlayout->setMargin(0);
    this->WeightingFunction = new pqGeneralTransferFunctionWidget();
    gridlayout->addWidget(this->WeightingFunction);
    this->WeightingFunction->addFunction(NULL, false);
    this->WeightingFunction->setMinY(0);
    this->WeightingFunction->setMaxY(1);
    connect( this, SIGNAL(changeWeightFunctionType(bool)),
             this->WeightingFunction, SLOT(setFunctionType(bool)));
    connect(this->WeightingFunction, SIGNAL(controlPointsModified()),
            this->WeightingFunction, SLOT(render()));
    connect(this->UI->leftValue, SIGNAL(valueChanged(double)),
            this->WeightingFunction, SLOT(setMinX(double)));
    connect(this->UI->rightValue, SIGNAL(valueChanged(double)),
            this->WeightingFunction, SLOT(setMaxX(double)));
    }

  QObject::connect(this->UI->Save, SIGNAL(clicked()), this, SLOT(onSaveProfile()));
  QObject::connect(this->UI->Load, SIGNAL(clicked()), this, SLOT(onLoadProfile()));

  QObject::connect(this->UI->SaveFunctions, SIGNAL(clicked()),
                   this, SLOT(onSaveArc()));
  QObject::connect(this->UI->LoadFunctions, SIGNAL(clicked()),
                   this, SLOT(onLoadArc()));

  this->ArcWidgetManager = new qtCMBArcWidgetManager(server, renderer);
  QObject::connect(this->ArcWidgetManager, SIGNAL(ArcSplit2(pqCMBArc*, QList<vtkIdType>)),
                   this, SLOT(doneModifyingArc()));
  QObject::connect(this->ArcWidgetManager, SIGNAL(ArcModified2(pqCMBArc*)),
                   this, SLOT(doneModifyingArc()));
  QObject::connect(this->ArcWidgetManager, SIGNAL(Finish()),
                   this, SLOT(doneModifyingArc()));
  this->UI->ArcControlTabs->setTabEnabled(1, false);
  this->UI->ArcControlTabs->setTabEnabled(2, false);
  this->UI->ArcControlTabs->setTabEnabled(3, false);
  QObject::connect(this, SIGNAL(selectionChanged(int)), this, SLOT(selectLine(int)));
  QObject::connect(this, SIGNAL(orderChanged()), this, SLOT(sendOrder()));
  this->check_save();
}

//-----------------------------------------------------------------------------
pqCMBModifierArcManager::~pqCMBModifierArcManager()
{
  this->UI->ArcControlArea->takeWidget();
  this->clear();
  delete this->Dialog;
  delete this->UI;
  delete this->ArcWidgetManager;
}

//-----------------------------------------------------------------------------
void pqCMBModifierArcManager::initialize()
{
  this->TableWidget->setColumnCount(8);
  this->TableWidget->setHorizontalHeaderLabels(
      QStringList() << tr("Apply") << tr("ID")
                    << tr("Symmetric") << tr("Relative\nDisplacement")
                    << tr("Minimum\nDisplacement\nDepth")
                    << tr("Maximum\nDisplacement\nDepth")
                    << tr("Left\nFunction\nDistance")
                    << tr("Right\nFunction\nDistance"));

  this->TableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
  this->TableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
  this->TableWidget->verticalHeader()->hide();

  QObject::connect(this->TableWidget, SIGNAL(itemSelectionChanged()),
                   this, SLOT(onCurrentObjectChanged()), Qt::QueuedConnection);
  QObject::connect(this->TableWidget, SIGNAL(itemChanged(QTableWidgetItem*)),
                   this, SLOT(onItemChanged(QTableWidgetItem*)));
  QObject::connect(this->UI->DatasetTable, SIGNAL(itemChanged(QTableWidgetItem*)),
                   this, SLOT(onDatasetChange(QTableWidgetItem*)));
  QObject::connect(this->TableWidget, SIGNAL(itemSelectionChanged()),
                   this, SLOT(onSelectionChange()));

  this->UI->points->setColumnCount(6);
  this->UI->points->setHorizontalHeaderLabels(
                QStringList() << tr("X") << tr("Y")
                              << tr("Minimum\nDisplacement\nDepth")
                              << tr("Maximum\nDisplacement\nDepth")
                              << tr("Left\nFunction\nDistance")
                              << tr("Right\nFunction\nDistance"));
  this->UI->points->setSelectionMode(QAbstractItemView::SingleSelection);
  this->UI->points->setSelectionBehavior(QAbstractItemView::SelectRows);
  this->UI->points->verticalHeader()->hide();

  this->UI->DisplacementSplineFrame->setVisible(false);
  QObject::connect(this->UI->DisplacementSplineCont, SIGNAL(toggled(bool)),
                   this, SLOT(dispSplineBox(bool)));

  this->UI->WeightingSplineFrame->setVisible(false);
  QObject::connect(this->UI->WeightingSplineControl, SIGNAL(toggled(bool)),
                   this, SLOT(weightSplineBox(bool)));

}

//-----------------------------------------------------------------------------
void pqCMBModifierArcManager::accepted()
{
  foreach(QString filename, ServerProxies.keys())
  {
    foreach(int pieceIdx, ServerProxies[filename].keys())
    {
      vtkSMSourceProxy* source = ServerProxies[filename][pieceIdx];
      for(unsigned int i = 0; i < this->ArcLines.size(); ++i)
      {
        if(this->ArcLines[i]!= NULL)
        {
          this->ArcLines[i]->updateArc(source);
        }
      }
    }
  }
  this->sendOrder();
  //this->clear();
  if(this->Dialog != NULL) this->Dialog->hide();
  emit(applyFunctions());
}

void pqCMBModifierArcManager::showDialog()
{
  if(this->Dialog != NULL)
  {
    this->Dialog->show();
  }
  foreach(QString filename, ServerProxies.keys())
  {
    foreach(int pieceIdx, ServerProxies[filename].keys())
    {
      vtkSMSourceProxy* source = ServerProxies[filename][pieceIdx];
      for(unsigned int i = 0; i < this->ArcLines.size(); ++i)
      {
        if(this->ArcLines[i]!= NULL)
        {
          QList< QVariant > v;
          v << this->ArcLines[i]->getId();
          pqSMAdaptor::setMultipleElementProperty(source->GetProperty("AddArc"), v);
          v.clear();
          v << this->ArcLines[i]->getId() << 1;
          pqSMAdaptor::setMultipleElementProperty(source->GetProperty("ArcEnable"), v);
          source->UpdateVTKObjects();
        }
      }
    }
  }
  addApplyControl();
}

//-----------------------------------------------------------------------------
void pqCMBModifierArcManager::clear()
{
  selectLine(-1);
  this->CurrentModifierArc = NULL;
  this->TableWidget->clearContents();
  this->TableWidget->setRowCount(0);
  this->UI->points->clearContents();
  this->UI->points->setRowCount(0);
  for(unsigned int i = 0; i < ArcLines.size(); ++i)
    {
    delete ArcLines[i];
    }

  ArcLines.clear();
  ServerProxies.clear();
  ServerRows.clear();
  ArcLinesApply.clear();
}

void pqCMBModifierArcManager::enableSelection()
{
  this->TableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
}

void pqCMBModifierArcManager::disableSelection()
{
  this->TableWidget->setSelectionMode(QAbstractItemView::NoSelection);
}

//-----------------------------------------------------------------------------
void pqCMBModifierArcManager::onClearSelection()
{
  this->TableWidget->blockSignals(true);
  this->TableWidget->clearSelection();
  //TODO clear the selected values form system
  this->TableWidget->blockSignals(false);
}

//-----------------------------------------------------------------------------
void pqCMBModifierArcManager::AddLinePiece(pqCMBModifierArc *dataObj, int visible)
{
  this->TableWidget->insertRow(this->TableWidget->rowCount());
  int row =  this->TableWidget->rowCount()-1;

  Qt::ItemFlags commFlags(
    Qt::ItemIsEnabled | Qt::ItemIsSelectable);

  setRow(row, dataObj);

  QTableWidgetItem* objItem = new QTableWidgetItem();
  QVariant vdata;
  vdata.setValue(static_cast<void*>(dataObj));
  objItem->setData(Qt::UserRole, vdata);

  this->TableWidget->setItem(row, VisibilityCol, objItem);
  objItem->setFlags(commFlags | Qt::ItemIsUserCheckable);
  objItem->setCheckState(visible ? Qt::Checked : Qt::Unchecked);

  this->TableWidget->resizeColumnsToContents();
  connect(dataObj, SIGNAL(functionChanged(int)),
          this, SLOT(onLineChange(int)));
  unselectAllRows();
  this->TableWidget->setRangeSelected(QTableWidgetSelectionRange(row,0,row,RightDist), true);
  if(visible)
    emit orderChanged();
}

void pqCMBModifierArcManager::unselectAllRows()
{
  this->TableWidget->setRangeSelected(QTableWidgetSelectionRange(0,0,this->TableWidget->rowCount()-1,RightDist), false);
}

void pqCMBModifierArcManager::setRow(int row, pqCMBModifierArc * dataObj)
{
  Qt::ItemFlags commFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

  QString s = QString::number(dataObj->getId());
  QTableWidgetItem* v = new QTableWidgetItem(s);
  this->TableWidget->setItem(row, Id, v);
  v->setFlags(commFlags);

  v = new QTableWidgetItem( dataObj->getSymmetry()?"Y":"N" );
  this->TableWidget->setItem(row, Symmetric, v);
  v->setFlags(commFlags);

  v = new QTableWidgetItem( dataObj->getRelative()?"Y":"N" );
  this->TableWidget->setItem(row, Relative, v);
  v->setFlags(commFlags);

  v = new QTableWidgetItem( QString::number(dataObj->getDisplacementDepth(pqCMBModifierArc::MIN)) );
  this->TableWidget->setItem(row, MinDisp, v);
  v->setFlags(commFlags);

  v = new QTableWidgetItem( QString::number(dataObj->getDisplacementDepth(pqCMBModifierArc::MAX)) );
  this->TableWidget->setItem(row, MaxDisp, v);
  v->setFlags(commFlags);

  v = new QTableWidgetItem( QString::number((dataObj->getSymmetry())?-dataObj->getDistanceRange(pqCMBModifierArc::MAX):dataObj->getDistanceRange(pqCMBModifierArc::MIN)) );
  this->TableWidget->setItem(row, LeftDist, v);
  v->setFlags(commFlags);

  v = new QTableWidgetItem( QString::number(dataObj->getDistanceRange(pqCMBModifierArc::MAX)) );
  this->TableWidget->setItem(row, RightDist, v);
  v->setFlags(commFlags);
}

//-----------------------------------------------------------------------------
void pqCMBModifierArcManager::onCurrentObjectChanged()
{
}

void pqCMBModifierArcManager::onLineChange(int id)
{
  pqCMBModifierArc * line = NULL;
  for(unsigned int i = 0; i < ArcLines.size(); ++i)
    {
    if(ArcLines[i] != NULL && ArcLines[i]->getId() == id)
      {
      line = ArcLines[i];
      }
    }
  if(line == NULL) return;
  for( int i = 0; i < this->TableWidget->rowCount(); ++i)
    {
    QTableWidgetItem * tmp = this->TableWidget->item( i, Id );
    if(tmp->text().toInt() == id)
      {
      this->setRow(i, line);
      }
    }
}

bool pqCMBModifierArcManager::remove(int id, bool all)
{
  this->TableWidget->blockSignals(true);

  QList<QTableWidgetSelectionRange>     selected = this->TableWidget->selectedRanges();
  int row = selected.front().topRow();
  int sid = this->TableWidget->item( row, Id )->text().toInt();

  if(id == -1)//Just remove the selected one
    {
    id = sid;
    }
  if(id == sid)
    {
    this->TableWidget->removeRow(row);
    }
  if(all)
    {
    int r = this->TableWidget->rowCount() - 1;
    for(; r >= 0; --r)
      {
      QTableWidgetItem *        tmp = this->TableWidget->item( r, Id );
      if(tmp->text().toInt() == id)
        {
        this->TableWidget->removeRow(r);
        }
      }
    }
  else
    {
    for(int i = 0; i < this->TableWidget->rowCount(); ++i)
      {
      QTableWidgetItem *        tmp = this->TableWidget->item( i, Id );
      if(id == tmp->text().toInt())
        {
        this->TableWidget->blockSignals(false);
        emit(orderChanged());
        onClearSelection();
        return true;
        }
      }
    }
  ArcLines[id] = NULL;
  this->TableWidget->blockSignals(false);
  emit(orderChanged());
  onClearSelection();
  this->check_save();
  return false;
}

//-----------------------------------------------------------------------------
void pqCMBModifierArcManager::onItemChanged(
  QTableWidgetItem* item)
{
  int id = this->TableWidget->item( item->row(), Id )->text().toInt();
  pqCMBModifierArc* dl = ArcLines[id];
  if(dl == NULL) return;
  if(item->column() == VisibilityCol)
    {
    emit(orderChanged());
    }
}

void pqCMBModifierArcManager::onDatasetChange(QTableWidgetItem* item)
{
  if(this->CurrentModifierArc == NULL) return;
  int id = this->CurrentModifierArc->getId();
  if(item->column() == 0)
    {
    QString fname = this->UI->DatasetTable->item( item->row(), 1 )->text();
    int pieceIdx = this->UI->DatasetTable->item( item->row(), 2 )->text().toInt();
    bool visible = item->checkState() == Qt::Checked;
    ArcLinesApply[id][fname][pieceIdx] = visible;
    emit orderChanged();
    }
}

void pqCMBModifierArcManager::applyAgain()
{
  Qt::ItemFlags commFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
  this->TableWidget->blockSignals(true);
  QList<QTableWidgetSelectionRange>     selected = this->TableWidget->selectedRanges();
  if (!selected.empty())
    {
    int row = selected.front().topRow();
    int id = this->TableWidget->item( row, Id )->text().toInt();
    bool visible = this->TableWidget->item( row, VisibilityCol )->checkState() == Qt::Checked;
    pqCMBModifierArc* dl = ArcLines[id];
    this->TableWidget->insertRow(this->TableWidget->rowCount());
    row =  this->TableWidget->rowCount()-1;

    setRow(row, dl);

    QTableWidgetItem* objItem = new QTableWidgetItem();
    QVariant vdata;
    vdata.setValue(static_cast<void*>(dl));
    objItem->setData(Qt::UserRole, vdata);

    this->TableWidget->setItem(row, VisibilityCol, objItem);
    objItem->setFlags(commFlags | Qt::ItemIsUserCheckable);
    objItem->setCheckState(visible ? Qt::Checked : Qt::Unchecked);

    this->TableWidget->resizeColumnsToContents();
    if(visible) emit orderChanged();
    }
  this->TableWidget->blockSignals(false);
}

void pqCMBModifierArcManager::onSelectionChange()
{
  this->TableWidget->blockSignals(true);
  QList<QTableWidgetSelectionRange>     selected = this->TableWidget->selectedRanges();
  if (!selected.empty())
    {
    int row = selected.front().topRow();
    int id = this->TableWidget->item( row, Id )->text().toInt();
    emit selectionChanged(id);
    }
  this->TableWidget->blockSignals(false);
}

std::vector<int> pqCMBModifierArcManager::getCurrentOrder(QString fname, int pindx)
{
  std::vector<int> result;
  for(int i = 0; i < this->TableWidget->rowCount(); ++i)
    {
    QTableWidgetItem * tmp = this->TableWidget->item( i, Id );
    int id = tmp->text().toInt();
    tmp = this->TableWidget->item( i, VisibilityCol );
    if(tmp->checkState() == Qt::Checked && ArcLinesApply[id][fname][pindx])
      {
      result.push_back(id);
      }
    }
  return result;
}

void pqCMBModifierArcManager::addLine()
{
  emit this->addingNewArc();
  selectLine(-2);
}

void pqCMBModifierArcManager::selectLine(int sid)
{
  if(this->CurrentModifierArc != NULL)
    {
    if(sid == this->CurrentModifierArc->getId()) return;
    this->UI->ArcControlArea->takeWidget();
    QObject::disconnect( this->UI->dispacementMinDepthValue, SIGNAL(valueChanged(double)),
                        this->CurrentModifierArc, SLOT(setMinDisplacementDepth(double)) );
    QObject::disconnect( this->UI->displacementMaxDepthValue, SIGNAL(valueChanged(double)),
                        this->CurrentModifierArc, SLOT(setMaxDisplacementDepth(double)) );
    QObject::disconnect( this->UI->leftValue, SIGNAL(valueChanged(double)),
                        this->CurrentModifierArc, SLOT(setLeftDistance(double)) );
    QObject::disconnect( this->UI->rightValue, SIGNAL(valueChanged(double)),
                        this->CurrentModifierArc, SLOT(setRightDistance(double)) );
    QObject::disconnect( this->UI->Symmetric, SIGNAL(clicked(bool)),
                        this->CurrentModifierArc, SLOT(setSymmetry(bool)));
    QObject::disconnect( this->UI->Relative, SIGNAL(clicked(bool)),
                        this->CurrentModifierArc, SLOT(setRelative(bool)) );
    this->CurrentModifierArc->switchToNotEditable();
    this->CurrentModifierArc = NULL;
    this->UI->removeLineButton->setEnabled(false);
    this->UI->buttonUpdateLine->setEnabled(false);
    this->UI->addLineButton->setEnabled(true);
    this->UI->ArcControlTabs->setTabEnabled(1, false);
    this->UI->ArcControlTabs->setTabEnabled(2, false);
    this->UI->ArcControlTabs->setTabEnabled(3, false);
    this->UI->ApplyAgain->setEnabled(false);
    this->UI->LoadFunctions->setEnabled(true);
    if(this->UI_Dialog != NULL)
      {
      QPushButton* applyButton = this->UI_Dialog->buttonBox->button(QDialogButtonBox::Apply);
      applyButton->setEnabled(true);
      }
    }
  if(sid == -1 || sid < -2)
    {
    return;
    }
  else if(sid == -2) //create new one
    {
    pqCMBModifierArc * dig = new pqCMBModifierArc();
    this->ArcWidgetManager->setActiveArc(dig->GetCmbArc());
    this->ArcWidgetManager->create();
    QWidget * arc =this->ArcWidgetManager->getActiveWidget();
    this->UI->ArcControlArea->setWidget(arc);
    //TODO disable selection and add button
    arc->setMinimumHeight(300);
    arc->setVisible(true);
    QObject::connect(arc,SIGNAL(contourDone()),
                     this,SLOT(doneModifyingArc()));
    QObject::connect( dig, SIGNAL(requestRender()),
                     this, SIGNAL(requestRender()) );

    addNewArc(dig);

    this->disableSelection();
    this->UI->addLineButton->setEnabled(false);
    this->UI->SaveFunctions->setEnabled(false);
    this->UI->LoadFunctions->setEnabled(false);
    if(this->UI_Dialog != NULL)
      {
      QPushButton* applyButton = this->UI_Dialog->buttonBox->button(QDialogButtonBox::Apply);
      applyButton->setEnabled(false);
      }
    }
  else
    {
    if(static_cast<size_t>(sid)<ArcLines.size() && ArcLines[sid] != NULL)
      {
      this->CurrentModifierArc = ArcLines[sid];
      this->CurrentModifierArc->switchToEditable();
      this->ArcWidgetManager->setActiveArc(this->CurrentModifierArc->GetCmbArc());
      this->ArcWidgetManager->edit();
      this->UI->ArcControlArea->setWidget(this->ArcWidgetManager->getActiveWidget());
      this->UI->removeLineButton->setEnabled(true);
      this->UI->buttonUpdateLine->setEnabled(true);
      this->UI->ApplyAgain->setEnabled(true);
      this->UI->LoadFunctions->setEnabled(true);
      }
    }
  if(this->CurrentModifierArc != NULL)
    {
    this->UI->Symmetric->setEnabled(true);
    this->updateLineFunctions();
    this->setDatasetTable(sid);
    this->setUpPointsTable();
    connect(this->UI->dispacementMinDepthValue, SIGNAL(valueChanged(double)),
            this->CurrentModifierArc, SLOT(setMinDisplacementDepth(double)));
    connect(this->UI->displacementMaxDepthValue, SIGNAL(valueChanged(double)),
            this->CurrentModifierArc, SLOT(setMaxDisplacementDepth(double)));
    connect(this->UI->leftValue, SIGNAL(valueChanged(double)),
            this->CurrentModifierArc, SLOT(setLeftDistance(double)));
    connect(this->UI->rightValue, SIGNAL(valueChanged(double)),
            this->CurrentModifierArc, SLOT(setRightDistance(double)));
    connect(this->UI->Symmetric, SIGNAL(clicked(bool)),
            this->CurrentModifierArc, SLOT(setSymmetry(bool)));
    connect(this->CurrentModifierArc, SIGNAL(functionChanged(int)),
            this, SLOT(updateLineFunctions()));
    connect(this->UI->Relative, SIGNAL(clicked(bool)),
            this->CurrentModifierArc, SLOT(setRelative(bool)));
    this->UI->ArcControlTabs->setTabEnabled(1, true);
    this->UI->ArcControlTabs->setTabEnabled(2, true);
    this->UI->ArcControlTabs->setTabEnabled(3, true);
    }
}

void pqCMBModifierArcManager::addNewArc(pqCMBModifierArc* dig)
{
  this->CurrentModifierArc = dig;
  int sid = ArcLines.size();
  ArcLines.push_back(dig);
  addApplyControl();
  //TODO THIS NEEDS TO BE BETTER
  dig->setId(sid);
  foreach(QString filename, ServerProxies.keys())
  {
    foreach(int pieceIdx, ServerProxies[filename].keys())
    {
      vtkSMSourceProxy* source = ServerProxies[filename][pieceIdx];
      QList< QVariant > v;
      v <<sid;
      pqSMAdaptor::setMultipleElementProperty(source->GetProperty("AddArc"), v);
      v.clear();
      v <<sid << 1;
      pqSMAdaptor::setMultipleElementProperty(source->GetProperty("ArcEnable"), v);
      source->UpdateVTKObjects();
    }
  }
  this->AddLinePiece(dig);
  this->check_save();
}

void pqCMBModifierArcManager::update()
{
  if(this->CurrentModifierArc == NULL)
    {
    return;
    }
  {//Update point functions
    pqCMBArc * arc = CurrentModifierArc->GetCmbArc();
    vtkPVArcInfo* ai = arc->getArcInfo();
    for(size_t i = 0; ai != NULL && i < static_cast<size_t>(ai->GetNumberOfPoints()); ++i)
    {
      double min,max;
      QTableWidgetItem * tmp = this->UI->points->item( i, 2);
      min = tmp->data(Qt::DisplayRole).toDouble();
      tmp = this->UI->points->item( i, 3);
      max = tmp->data(Qt::DisplayRole).toDouble();
      CurrentModifierArc->setDepthParams(i,min,max);
      tmp = this->UI->points->item( i, 4);
      min = tmp->data(Qt::DisplayRole).toDouble();
      tmp = this->UI->points->item( i, 5);
      max = tmp->data(Qt::DisplayRole).toDouble();
      CurrentModifierArc->setDisplacementParams(i,
                                                (CurrentModifierArc->getSymmetry())?0:min,
                                                max);
    }
  }
  foreach(QString filename, ServerProxies.keys())
    {
    foreach(int pieceIdx, ServerProxies[filename].keys())
      {
      vtkSMSourceProxy* source = ServerProxies[filename][pieceIdx];
      this->CurrentModifierArc->updateArc(source);
      }
    }
  setUpPointsTable();
  emit(functionsUpdated());
  emit(requestRender());
}

void pqCMBModifierArcManager::removeArc()
{
  if(this->CurrentModifierArc == NULL)
    {
    return;
    }
  int id = this->CurrentModifierArc->getId();
  this->enableSelection();
  selectLine(-1);

  pqCMBModifierArc * tmp = ArcLines[id];
  bool hasMore = this->remove(id, false);
  if(!hasMore)
    {
    QObject::disconnect( tmp, SIGNAL(requestRender()),
                         this, SIGNAL(requestRender()) );
    ArcLines[id] = NULL;
    foreach(QString filename, ServerProxies.keys())
      {
      foreach(int pieceIdx, ServerProxies[filename].keys())
        {
        vtkSMSourceProxy* source = ServerProxies[filename][pieceIdx];
        tmp->removeFromServer(source);
        }
      }

    delete tmp;
    }
  emit(requestRender());
}

void pqCMBModifierArcManager::addProxy(QString s, int pid, pqPipelineSource* ps)
{
  vtkSMSourceProxy* source = NULL;
  source = vtkSMSourceProxy::SafeDownCast(ps->getProxy() );
  ServerProxies[s].insert(pid, source);
  setUpTable();

  for(unsigned int i = 0; i < this->ArcLines.size(); ++i)
  {
    if(this->ArcLines[i]!= NULL)
    {
      QList< QVariant > v;
      v << this->ArcLines[i]->getId();
      pqSMAdaptor::setMultipleElementProperty(source->GetProperty("AddArc"), v);
      v.clear();
      v << this->ArcLines[i]->getId() << 1;
      pqSMAdaptor::setMultipleElementProperty(source->GetProperty("ArcEnable"), v);
      source->UpdateVTKObjects();
      this->ArcLines[i]->updateArc(source);
      ArcLinesApply[i][s][pid] = true;
    }
  }

  this->sendOrder();
}

void pqCMBModifierArcManager::clearProxies()
{
  ServerProxies.clear();
  ServerRows.clear();
  ArcLinesApply.clear();
}

void pqCMBModifierArcManager::updateLineFunctions()
{
  if(this->CurrentModifierArc == NULL) return;
  pqCMBModifierArc * dig = this->CurrentModifierArc;
  this->UI->dispacementMinDepthValue->setValue(dig->getDisplacementDepth(pqCMBModifierArc::MIN));
  this->UI->displacementMaxDepthValue->setValue(dig->getDisplacementDepth(pqCMBModifierArc::MAX));
  this->UI->leftValue->setValue(dig->getDistanceRange(pqCMBModifierArc::MIN));
  this->UI->rightValue->setValue(dig->getDistanceRange(pqCMBModifierArc::MAX));
  bool isSymmetric = dig->getSymmetry();
  if(isSymmetric)
    {
    this->UI->leftValue->setEnabled(false);
    this->UI->Symmetric->setChecked(true);
    }
  else
    {
    this->UI->leftValue->setEnabled(true);
    this->UI->Symmetric->setChecked(false);
    }
  this->UI->DisplacementSplineCont->setChecked(dig->getDisplacementFunctionUseSpline());
  this->UI->WeightingSplineControl->setChecked(dig->getWeightingFunctionUseSpline());

  this->UI->Relative->setChecked(dig->getRelative());
  this->DisplacementProfile->changeFunction(0, dig->getDisplacementProfile(), true);
  this->DisplacementProfile->setMinX(dig->getDistanceRange(pqCMBModifierArc::MIN));
  this->DisplacementProfile->setMaxX(dig->getDistanceRange(pqCMBModifierArc::MAX));
  this->DisplacementProfile->setMinY(dig->getDisplacementDepth(pqCMBModifierArc::MIN));
  this->DisplacementProfile->setMaxY(dig->getDisplacementDepth(pqCMBModifierArc::MAX));

  this->WeightingFunction->changeFunction(0, dig->getWeightingFunction(), true);
  this->WeightingFunction->setMinY(0);
  this->WeightingFunction->setMaxY(1);
  this->WeightingFunction->setMinX(dig->getDistanceRange(pqCMBModifierArc::MIN));
  this->WeightingFunction->setMaxX(dig->getDistanceRange(pqCMBModifierArc::MAX));

  this->DisplacementProfile->render();
  this->WeightingFunction->render();
}

void pqCMBModifierArcManager::doneModifyingArc()
{
  if(this->CurrentModifierArc != NULL)
    {
    foreach(QString filename, ServerProxies.keys())
      {
      foreach(int pieceIdx, ServerProxies[filename].keys())
        {
        vtkSMSourceProxy* source = ServerProxies[filename][pieceIdx];
        this->CurrentModifierArc->updateArc(source);
        }
      }
    }

  this->enableSelection();
  this->unselectAllRows();
  emit this->modifyingArcDone();
  selectLine(-1);
}

void pqCMBModifierArcManager::addApplyControl()
{
  QMap< QString, QMap<int, bool> > newVis;
  foreach(QString filename, ServerProxies.keys())
    {
    foreach(int pieceIdx, ServerProxies[filename].keys())
      {
      newVis[filename][pieceIdx] = true;
      }
    }
  ArcLinesApply.resize(this->ArcLines.size(), newVis);
}

void pqCMBModifierArcManager::sendOrder()
{
  foreach(QString filename, ServerProxies.keys())
    {
    foreach(int pieceIdx, ServerProxies[filename].keys())
      {
      vtkSMSourceProxy* source = ServerProxies[filename][pieceIdx];
      std::vector<int> order = this->getCurrentOrder(filename, pieceIdx);
      QList< QVariant > v;
      v << static_cast<int>(order.size());
      pqSMAdaptor::setMultipleElementProperty(source->GetProperty("ResizeOrder"), v);
      source->UpdateVTKObjects();
      v.clear();
      v << -1;
      pqSMAdaptor::setMultipleElementProperty(source->GetProperty("ResizeOrder"), v);
      source->UpdateVTKObjects();
      for (unsigned int i = 0; i < order.size(); ++i)
        {
        v.clear();
        v << static_cast<int>(i) << order[i];
        pqSMAdaptor::setMultipleElementProperty(source->GetProperty("SetOrderValue"), v);
        source->UpdateVTKObjects();
        }
      v.clear();
      v << -1 << -1;
      pqSMAdaptor::setMultipleElementProperty(source->GetProperty("SetOrderValue"), v);
      source->UpdateVTKObjects();
      source->UpdatePipeline();
      source->UpdatePropertyInformation();
      }
    }
  emit requestRender();
}

void pqCMBModifierArcManager::setUpTable()
{
  QTableWidget* tmp = this->UI->DatasetTable;
  tmp->blockSignals(true);
  tmp->clearContents();
  tmp->setRowCount(0);
  Qt::ItemFlags commFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
  tmp->setSelectionMode(QAbstractItemView::NoSelection);
  foreach(QString filename, ServerProxies.keys())
    {
    foreach(int pieceIdx, ServerProxies[filename].keys())
      {
      tmp->insertRow(tmp->rowCount());
      int row =  tmp->rowCount()-1;
      ServerRows[filename][pieceIdx] = row;
      QTableWidgetItem* objItem = new QTableWidgetItem();
      QVariant vdata;
      objItem->setData(Qt::UserRole, vdata);

      tmp->setItem(row, 0, objItem);
      objItem->setFlags(commFlags | Qt::ItemIsUserCheckable);
      objItem->setCheckState(Qt::Checked);

      objItem = new QTableWidgetItem(filename);
      tmp->setItem(row, 1, objItem);
      objItem->setFlags(commFlags);

      objItem = new QTableWidgetItem( QString::number(pieceIdx) );
      tmp->setItem(row, 2, objItem);
      objItem->setFlags(commFlags);
      }
    }
  tmp->resizeColumnsToContents();
  tmp->blockSignals(false);
}

void pqCMBModifierArcManager::setUpPointsTable()
{
  QTableWidget* tmp = this->UI->points;
  tmp->blockSignals(true);

  tmp->clearContents();
  tmp->setRowCount(0);
  if(CurrentModifierArc == NULL)
  {
    tmp->blockSignals(false);
    return;
  }
  Qt::ItemFlags commFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
  tmp->setSelectionMode(QAbstractItemView::NoSelection);
  pqCMBArc * arc = CurrentModifierArc->GetCmbArc();
  vtkPVArcInfo* ai = arc->getArcInfo();
  for(size_t i = 0; ai != NULL && i < static_cast<size_t>(ai->GetNumberOfPoints()); ++i)
  {
    double pt[3];
    ai->GetPointLocation(i, pt);
    int row = tmp->rowCount();
    tmp->insertRow(tmp->rowCount());
    QTableWidgetItem * qtwi = new QTableWidgetItem(QString::number(pt[0]));
    tmp->setItem(row, 0, qtwi);
    qtwi->setFlags(commFlags);
    qtwi = new QTableWidgetItem(QString::number(pt[1]));
    tmp->setItem(row, 1, qtwi);
    qtwi->setFlags(commFlags);
    double min,max;
    CurrentModifierArc->getDepthParams(i,min,max);
    qtwi = new QTableWidgetItem();
    qtwi->setData(Qt::DisplayRole, min);
    tmp->setItem(row, 2, qtwi);
    qtwi = new QTableWidgetItem();
    qtwi->setData(Qt::DisplayRole, max);
    tmp->setItem(row, 3, qtwi);
    CurrentModifierArc->getDisplacementParams(i, min, max);
    qtwi = new QTableWidgetItem();
    qtwi->setData(Qt::DisplayRole, (CurrentModifierArc->getSymmetry())?-max:min);
    tmp->setItem(row, 4, qtwi);
    qtwi = new QTableWidgetItem();
    qtwi->setData(Qt::DisplayRole, max);
    tmp->setItem(row, 5, qtwi);
  }
  tmp->resizeColumnsToContents();
  tmp->blockSignals(false);
}

void pqCMBModifierArcManager::setDatasetTable(int inId)
{
  if(inId < 0 || static_cast<size_t>(inId) >= ArcLines.size()) return;
  this->UI->DatasetTable->blockSignals(true);
  foreach(QString filename, ServerProxies.keys())
    {
    foreach(int pieceIdx, ServerProxies[filename].keys())
      {
      int row = ServerRows[filename][pieceIdx];
      bool visOnDs = ArcLinesApply[inId][filename][pieceIdx];
      this->UI->DatasetTable->item( row, 0 )->setCheckState( visOnDs ? Qt::Checked : Qt::Unchecked);
      }
    }
  this->UI->DatasetTable->blockSignals(false);
}

void pqCMBModifierArcManager::disableAbsolute()
{
  this->UI->Relative->setChecked( true );
  this->UI->Relative->setEnabled( false );
  for(int i = 0; i < this->TableWidget->rowCount(); ++i)
  {
    QTableWidgetItem * tmp = this->TableWidget->item( i, Id );
    int id = tmp->text().toInt();
    pqCMBModifierArc * ma = ArcLines[id];
    if(ma != NULL)
    {
      ma->setRelative(true);
      this->setRow(i,ma);
    }
  }
}

void pqCMBModifierArcManager::enableAbsolute()
{
  this->UI->Relative->setEnabled( true );
}

void
pqCMBModifierArcManager::dispSplineControlChanged()
{
  /*TODO: This is not implimented yet.  We are waiting to see if they require more control
  DispSplineTension;
  DispSplineCont
  DispSplineBias*/
}

void pqCMBModifierArcManager::weightingSplineControlChanged()
{
  /*TODO: This is not implimented yet.  We are waiting to see if they require more control
  WeightingSplineTension
  WeightingSplineContinuity
  WeightingSplineBias*/
}

void pqCMBModifierArcManager::dispSplineBox(bool v)
{
  /*Not showing the frame for now unless they want more control*/
  //this->UI->DisplacementSplineFrame->setVisible(v);
  if(this->CurrentModifierArc != NULL)
  {
    this->CurrentModifierArc->setDisplacementFunctionType(v);
  }
  emit changeDisplacementFunctionType(v);
}

void  pqCMBModifierArcManager::weightSplineBox(bool v)
{
  /*Not showing the frame for now unless they want more control*/
  //this->UI->WeightingSplineFrame->setVisible(v);
  if(this->CurrentModifierArc != NULL)
  {
    this->CurrentModifierArc->setWeightingFunctionType(v);
  }
  emit changeWeightFunctionType(v);
}

void pqCMBModifierArcManager::onSaveProfile()
{
  if(this->CurrentModifierArc != NULL)
  {
    QString fileName = QFileDialog::getSaveFileName(NULL, tr("Save File"),
                                                    "",
                                                    tr("Function Profile (*.fpr)"));
    if(fileName.isEmpty())
    {
      return;
    }
    std::ofstream out(fileName.toStdString().c_str());
    this->CurrentModifierArc->writeFunction(out);
  }
}

void pqCMBModifierArcManager::onLoadProfile()
{
  if(this->CurrentModifierArc != NULL)
  {
    QStringList fileNames =
      QFileDialog::getOpenFileNames(NULL,
                                    "Open File...",
                                    "",
                                    "Function Profile (*.fpr)");
    if(fileNames.count()==0)
    {
      return;
    }
    std::string fname = fileNames[0].toStdString();
    std::ifstream in(fname.c_str());
    this->CurrentModifierArc->readFunction(in);
    this->updateLineFunctions();
  }
}

void pqCMBModifierArcManager::onSaveArc()
{
  QString fileName = QFileDialog::getSaveFileName(NULL, tr("Save File"),
                                                  "",
                                                  tr("Function Profile (*.mar)"));
  if(fileName.isEmpty())
  {
    return;
  }

  QList<pqOutputPort*> ContourInputs;
  std::ofstream out(fileName.toStdString().c_str());
  out << 1 << "\n";
  out << ArcLines.size() << "\n";

  for(unsigned int i = 0; i < ArcLines.size(); ++i)
  {
    ArcLines[i]->write(out);
    pqOutputPort *port = ArcLines[i]->GetCmbArc()->getSource()->getOutputPort(0);
    ContourInputs.push_back( port );
  }
  {
    QFileInfo fi(fileName);
    std::string fname = QFileInfo(fi.dir(), fi.baseName()+".vtp").absoluteFilePath().toStdString();
    QMap<QString, QList<pqOutputPort*> > namedInputs;
    namedInputs["Input"] = ContourInputs;

    //now that we have collected all the contour info, write out the file
    pqApplicationCore* core = pqApplicationCore::instance();
    pqObjectBuilder* builder = core->getObjectBuilder();
    pqPipelineSource* writer = builder->createFilter("writers",
                                                     "SceneGenV2ContourWriter",
                                                     namedInputs, core->getActiveServer());

    vtkSMSourceProxy *proxy = vtkSMSourceProxy::SafeDownCast(writer->getProxy());

    pqSMAdaptor::setElementProperty( proxy->GetProperty("FileName"),
                                     fname.c_str() );
    proxy->UpdateProperty("FileName");
    proxy->UpdatePipeline();

    //now that the file has been written, delete the writer
    builder->destroy(writer);
  }
}

void pqCMBModifierArcManager::onLoadArc()
{
  QStringList fileNames =
  QFileDialog::getOpenFileNames(NULL,
                                "Open File...",
                                "",
                                "Function Profile (*.mar)");
  if(fileNames.count()==0)
  {
    return;
  }

  std::string fname = fileNames[0].toStdString();
  unsigned int start = ArcLines.size();
  int rc = this->TableWidget->rowCount();
  {
    QFileInfo fi(fileNames[0]);
    pqPipelineSource *reader = NULL;
    QStringList files;
    files << QFileInfo(fi.dir(), fi.baseName()+".vtp").absoluteFilePath();

    pqApplicationCore* core = pqApplicationCore::instance();
    pqObjectBuilder* builder = core->getObjectBuilder();
    builder->blockSignals(true);

    reader = builder->createReader("sources", "XMLPolyDataReader", files,
                                   core->getActiveServer());
    builder->blockSignals(false);

    if (reader)
    {
      pqPipelineSource* extractContour = builder->createFilter("filters",
                                                               "CmbExtractContours", reader);
      vtkSMSourceProxy *proxy = vtkSMSourceProxy::SafeDownCast( extractContour->getProxy() );
      proxy->UpdatePipeline();
      proxy->UpdatePropertyInformation();
      int max =pqSMAdaptor::getElementProperty(proxy->GetProperty("NumberOfContoursInfo")).toInt();
      pqCMBModifierArc * dig = new pqCMBModifierArc(proxy);
      addNewArc(dig);
      for ( int i = 1; i < max; ++i)
      {
        pqSMAdaptor::setElementProperty(proxy->GetProperty("ContourIndex"),i);
        proxy->UpdateProperty("ContourIndex");
        proxy->UpdatePipeline();
        pqCMBModifierArc * dig = new pqCMBModifierArc(proxy);
        QObject::connect( dig, SIGNAL(requestRender()),
                          this, SIGNAL(requestRender()) );
        addNewArc(dig);
      }
    }
  }
  std::ifstream in(fname.c_str());
  int version;
  in >> version;
  unsigned int num;
  in >> num;
  for(unsigned int i = 0; i < num; ++i)
  {
    unsigned int at = start + i;
    ArcLines[at]->read(in);
    this->setRow( rc+i, ArcLines[at] );
    ArcLines[at]->switchToNotEditable();
  }
  accepted();
  onClearSelection();
  this->CurrentModifierArc = NULL;
  this->check_save();
}

void pqCMBModifierArcManager::check_save()
{
  for(unsigned int i = 0; i < ArcLines.size(); ++i)
  {
    if(ArcLines[i] != NULL)
    {
      this->UI->SaveFunctions->setEnabled(true);
      return;
    }
  }
  this->UI->SaveFunctions->setEnabled(false);
}

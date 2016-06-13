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

#include "ui_qtArcEditWidget.h"

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
#include <QPointer>

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
#include "qtCMBArcWidget.h"
#include "pq3DWidgetFactory.h"
#include "vtkSMRenderViewProxy.h"

#include "vtkSMPropertyHelper.h"
#include "vtkSMProxyProperty.h"
#include "vtkSMSourceProxy.h"
#include "vtkSMVectorProperty.h"
#include "vtkSMRepresentationProxy.h"
#include "vtkNew.h"

#include "qtCMBManualFunctionWidget.h"
#include "qtCMBProfileWedgeFunctionWidget.h"
#include "cmbManualProfileFunction.h"
#include "cmbProfileFunction.h"
#include "cmbProfileWedgeFunction.h"
#include "pqDataRepresentation.h"
#include "pqPipelineSource.h"
#include "vtkSMNewWidgetRepresentationProxy.h"

#include "vtkCMBArcEditClientOperator.h"

#include "pqRepresentationHelperFunctions.h"

#include <vtkPoints.h>
#include <vtkActor.h> 
#include <vtkPolyDataMapper.h>
#include <vtkGlyph3D.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRendererCollection.h>
#include <QVTKWidget.h>

#include "qtCMBArcWidget.h"

#include "vtkSMCMBGlyphPointSourceProxy.h"

namespace
{

// enum for different column types
enum DataTableCol
{
  VisibilityCol=0,
  Id,
  Mode,
  Relative
};

enum EditMode
{
  NoEditMode = 0,
  EditFunction,
  EditArc
};

QStringList pointModesStrings = QStringList() << "Continuous" << "End Points" << "Advance";

}

class pqCMBModifierArcManagerInternal
{
public:
  QPointer<pqPipelineSource> ArcPointSelectSource;
  vtkSMNewWidgetRepresentationProxy * editableWidget;
  //qtCMBArcWidget* PointSelectionWidget;
  QPointer<pqPipelineSource> SphereSource;
  QPointer<pqDataRepresentation> Representation;
  QPointer<pqPipelineSource> LineGlyphFilter;
  Ui_qtArcFunctionControl * UI;
  Ui_qtModifierArcDialog * UI_Dialog;
  Ui_ArcEditWidget * arcEditWidget;
  EditMode mode;
  qtCMBArcWidget * CurrentArcWidget;
  int selectedRow;
  vtkBoundingBox boundingBox;
};

//-----------------------------------------------------------------------------
pqCMBModifierArcManager::pqCMBModifierArcManager(QLayout *layout,
                                                 pqServer *server,
                                                 pqRenderView *renderer)
{
  useNormal = false;
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();
  this->view = renderer;
  this->server = server;
  this->Internal = new pqCMBModifierArcManagerInternal;
  //this->Internal->PointSelectionWidget = NULL;
  this->Internal->editableWidget = NULL;

  this->Internal->SphereSource = builder->createSource("sources", "SphereSource", server);
  this->Internal->LineGlyphFilter = builder->createSource("filters",
                                                          "ArcPointGlyphingFilter", server);
  this->Internal->Representation =
      builder->createDataRepresentation( this->Internal->LineGlyphFilter->getOutputPort(0),
                                         this->view,
                                         "GeometryRepresentation");

  //vtkSMSourceProxy* source = this->Internal->SphereSource->getProxy();
  //pqSMAdaptor::setElementProperty(this->Internal->SphereSource->getProxy()->GetProperty("SetRadius"),1000);

  this->Internal->SphereSource->getProxy()->MarkModified(this->Internal->SphereSource->getProxy());

  vtkSMProxy *repProxy = this->Internal->Representation->getProxy();
  RepresentationHelperFunctions::CMB_COLOR_REP_BY_ARRAY( repProxy, "Color", vtkDataObject::POINT);

  pqSMAdaptor::setEnumerationProperty(repProxy->GetProperty("Representation"), "3D Glyphs");
  //vtkSMPropertyHelper(repProxy, "ImmediateModeRendering").Set(0);
  vtkSMPropertyHelper(repProxy, "GlyphType").Set(this->Internal->SphereSource->getProxy());
  vtkSMPropertyHelper(repProxy, "Scaling").Set(true);
  vtkSMPropertyHelper(repProxy, "ScaleMode").Set(2);
  vtkSMPropertyHelper(repProxy, "Orient").Set(true);
  vtkSMPropertyHelper(repProxy, "OrientationMode").Set(1);
  vtkSMPropertyHelper(repProxy, "MapScalars").Set(0);
  vtkSMPropertyHelper(repProxy, "SelectScaleArray").Set("Scaling");
  vtkSMPropertyHelper(repProxy, "SelectOrientationVectors").Set("Orientation");

  // This is a work arround for a bug in ParaView
  vtkSMPropertyHelper(repProxy, "ScaleFactor").Set(1.0);

  repProxy->UpdateVTKObjects();

  this->CurrentModifierArc = NULL;
  this->ManualFunctionWidget = NULL;
  this->WedgeFunctionWidget = NULL;
  this->selectedFunction = NULL;
  this->Internal->UI = new Ui_qtArcFunctionControl();
  QWidget * w = new QWidget();
  this->Internal->UI->setupUi(w);
  if(layout != NULL)
  {
    layout->addWidget(w);
    this->Dialog = NULL;
    this->Internal->UI_Dialog = NULL;
  }
  else
  {
    this->Dialog = new QDialog();
    this->Internal->UI_Dialog = new Ui_qtModifierArcDialog();
    this->Internal->UI_Dialog->setupUi(this->Dialog);
    this->Internal->UI_Dialog->modifierLayout->addWidget(w);
    QPushButton* applyButton = this->Internal->UI_Dialog->buttonBox->button(QDialogButtonBox::Apply);
    connect(applyButton, SIGNAL(clicked()), this, SLOT(accepted()));
    //connect(this->Dialog, SIGNAL(rejected()), this, SLOT(clear()));
  }
  functionLayout = new QGridLayout(this->Internal->UI->functionControlArea);
  functionLayout->setMargin(0);

  this->TableWidget = this->Internal->UI->ModifyArcTable;
  this->clear();
  this->initialize();
  QObject::connect(this->Internal->UI->addLineButton, SIGNAL(clicked()),
                   this, SLOT(addLine()), Qt::UniqueConnection);
  QObject::connect(this->Internal->UI->removeLineButton, SIGNAL(clicked()),
                   this, SLOT(removeArc()), Qt::UniqueConnection);
  QObject::connect(this->Internal->UI->buttonUpdateLine, SIGNAL(clicked()),
                   this, SLOT(update()), Qt::UniqueConnection);

  QObject::connect(this->Internal->UI->EditArc, SIGNAL(clicked()),
                   this, SLOT(editArc()), Qt::UniqueConnection);

  QObject::connect(this->Internal->UI->DeleteFunction, SIGNAL(clicked()),
                   this, SLOT(deleteFunction()), Qt::UniqueConnection);
  QObject::connect(this->Internal->UI->CloneFunction, SIGNAL(clicked()),
                   this, SLOT(cloneFunction()), Qt::UniqueConnection);

  QObject::connect(this->Internal->UI->FunctionName, SIGNAL(currentIndexChanged(int)),
                   this, SLOT(onFunctionSelectionChange()), Qt::UniqueConnection);
  QObject::connect(this->Internal->UI->FunctionType, SIGNAL(currentIndexChanged(int)),
                   this,  SLOT(functionTypeChanged(int)), Qt::UniqueConnection);

  QObject::connect(this->Internal->UI->Save, SIGNAL(clicked()),
                   this, SLOT(onSaveArc()), Qt::UniqueConnection);
  QObject::connect(this->Internal->UI->Load, SIGNAL(clicked()),
                   this, SLOT(onLoadArc()), Qt::UniqueConnection);
  QObject::connect(this->Internal->UI->importFunctions, SIGNAL(clicked()),
                   this, SLOT(importFunction()), Qt::UniqueConnection);

  QObject::connect(this->Internal->UI->FunctionName, SIGNAL(editTextChanged(QString const&)),
                   this, SLOT(nameChanged(QString)), Qt::UniqueConnection);

  this->ArcWidgetManager = new qtCMBArcWidgetManager(server, renderer);
  QObject::connect(this->ArcWidgetManager, SIGNAL(ArcSplit2(pqCMBArc*, QList<vtkIdType>)),
                   this, SLOT(doneModifyingArc()), Qt::UniqueConnection);
  QObject::connect(this->ArcWidgetManager, SIGNAL(ArcModified2(pqCMBArc*)),
                   this, SLOT(doneModifyingArc()), Qt::UniqueConnection);
  QObject::connect(this->ArcWidgetManager, SIGNAL(Finish()),
                   this, SLOT(doneModifyingArc()), Qt::UniqueConnection);
  QObject::connect( this->ArcWidgetManager, SIGNAL(selectedId(vtkIdType)),
                    this, SLOT(addPoint(vtkIdType)), Qt::UniqueConnection);
  QObject::connect(this, SIGNAL(selectionChanged(int)), this, SLOT(selectLine(int)),
                   Qt::UniqueConnection);
  QObject::connect(this, SIGNAL(orderChanged()), this, SLOT(sendOrder()), Qt::UniqueConnection);

  QObject::connect(this->Internal->UI->ComputeChange, SIGNAL(clicked()),
                   this, SLOT(computeChange()), Qt::UniqueConnection);
  this->check_save();
  this->addPointMode = false;

  this->Internal->arcEditWidget =  new Ui_ArcEditWidget();
  this->Internal->arcEditWidget->setupUi(this->Internal->UI->EditControl);
  this->Internal->mode = NoEditMode;
  this->updateUiControls();
  this->Internal->CurrentArcWidget = NULL;
  this->Internal->selectedRow = -1;

  this->Internal->UI->amountAdded->setText("NA");
  this->Internal->UI->amountRemoved->setText("NA");
  this->Internal->UI->PointBathChangeControl->hide();
}

//-----------------------------------------------------------------------------
pqCMBModifierArcManager::~pqCMBModifierArcManager()
{
  if(this->Internal->ArcPointSelectSource)
  {
    pqApplicationCore::instance()->getObjectBuilder()->destroy(this->Internal->ArcPointSelectSource);
    this->Internal->ArcPointSelectSource = NULL;
  }
  delete this->Internal->arcEditWidget;
  delete this->Internal->UI;
  delete this->Internal;
  this->clear();
  delete this->Dialog;
  delete this->ArcWidgetManager;
  //delete this->Internal->PointSelectionWidget;
  //TODO clean up editableWidget
}

//-----------------------------------------------------------------------------
void pqCMBModifierArcManager::initialize()
{
  this->TableWidget->setColumnCount(4);
  this->TableWidget->setHorizontalHeaderLabels(QStringList() << tr("Apply")
                                                             << tr("ID")
                                                             << tr("       Mode       ")
                                                             << tr("Relative"));

  this->TableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
  this->TableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
  this->TableWidget->verticalHeader()->hide();

  QObject::connect(this->TableWidget, SIGNAL(itemSelectionChanged()),
                   this, SLOT(onCurrentObjectChanged()), Qt::QueuedConnection);
  QObject::connect(this->TableWidget, SIGNAL(itemChanged(QTableWidgetItem*)),
                   this, SLOT(onItemChanged(QTableWidgetItem*)), Qt::UniqueConnection);
  QObject::connect(this->Internal->UI->DatasetControl, SIGNAL(clicked(bool)),
                   this->Internal->UI->DatasetTable, SLOT(setVisible(bool)), Qt::UniqueConnection);
  QObject::connect(this->Internal->UI->DatasetTable, SIGNAL(itemChanged(QTableWidgetItem*)),
                   this, SLOT(onDatasetChange(QTableWidgetItem*)), Qt::UniqueConnection);
  QObject::connect(this->TableWidget, SIGNAL(itemSelectionChanged()),
                   this, SLOT(onSelectionChange()), Qt::UniqueConnection);

  this->Internal->UI->DatasetControl->setChecked(false);
  this->Internal->UI->DatasetTable->hide();

  this->Internal->UI->points->setColumnCount(2);
  this->Internal->UI->points->setHorizontalHeaderLabels( QStringList() << tr("Point")
                                                                       << tr("Function"));
  this->Internal->UI->points->setSelectionMode(QAbstractItemView::SingleSelection);
  this->Internal->UI->points->setSelectionBehavior(QAbstractItemView::SelectRows);
  this->Internal->UI->points->verticalHeader()->hide();

  QObject::connect(this->Internal->UI->points, SIGNAL(itemSelectionChanged()),
                   this, SLOT(onPointsSelectionChange()), Qt::UniqueConnection);
  QObject::connect(this->Internal->UI->AddPoint, SIGNAL(clicked()),
                   this, SLOT(addPoint()), Qt::UniqueConnection);
  QObject::connect(this->Internal->UI->RemovePoint, SIGNAL(clicked()),
                   this, SLOT(deletePoint()), Qt::UniqueConnection);

}

//-----------------------------------------------------------------------------
void pqCMBModifierArcManager::accepted()
{
  foreach(QString filename, ServerProxies.keys())
  {
    foreach(int pieceIdx, ServerProxies[filename].keys())
    {
      vtkSMSourceProxy* source = ServerProxies[filename][pieceIdx].source;
      for(unsigned int i = 0; i < this->ArcLines.size(); ++i)
      {
        if(this->ArcLines[i]!= NULL)
        {
          this->ArcLines[i]->updateArc(source, this->Internal->boundingBox);
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
      vtkSMSourceProxy* source = ServerProxies[filename][pieceIdx].source;
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
  this->Internal->mode = NoEditMode;
  this->selectFunction(NULL);
  this->selectedFunction = NULL;
  this->TableWidget->clearContents();
  this->TableWidget->setRowCount(0);
  this->Internal->UI->points->clearContents();
  this->Internal->UI->points->setRowCount(0);
  for(unsigned int i = 0; i < ArcLines.size(); ++i)
    {
    delete ArcLines[i];
    }

  ArcLines.clear();
  ServerProxies.clear();
  ServerRows.clear();
  ArcLinesApply.clear();
  this->Internal->boundingBox.Reset();
  this->updateUiControls();
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
  this->onSelectionChange();
  this->updateUiControls();
  //TODO clear the selected values form system
  this->TableWidget->blockSignals(false);
}

//-----------------------------------------------------------------------------
void pqCMBModifierArcManager::AddLinePiece(pqCMBModifierArc *dataObj, int visible)
{
  this->TableWidget->insertRow(this->TableWidget->rowCount());
  int row =  this->TableWidget->rowCount()-1;

  Qt::ItemFlags commFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

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
          this, SLOT(onLineChange(int)), Qt::UniqueConnection);
  //unselectAllRows();
  this->TableWidget->selectRow(row);
  if(visible)
    emit orderChanged();
}

void pqCMBModifierArcManager::AddFunction(cmbProfileFunction * fun)
{
  QVariant vdata;
  vdata.setValue(static_cast<void*>(fun));
  this->Internal->UI->FunctionName->addItem(fun->getName().c_str(), vdata);
  this->Internal->UI->DeleteFunction->setEnabled(this->Internal->UI->FunctionName->count()>1);
}

void pqCMBModifierArcManager::unselectAllRows()
{
  this->TableWidget->setRangeSelected(
                      QTableWidgetSelectionRange(0,0,
                                                 this->TableWidget->rowCount()-1,
                                                 Id),
                      false);
}

void pqCMBModifierArcManager::setRow(int row, pqCMBModifierArc * dataObj)
{
  Qt::ItemFlags commFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

  QVariant vdata;
  vdata.setValue(static_cast<void*>(dataObj));

  QString s = QString::number(dataObj->getId());
  QTableWidgetItem* v = new QTableWidgetItem(s);
  this->TableWidget->setItem(row, Id, v);
  v->setFlags(commFlags);

  QTableWidgetItem* objItem = new QTableWidgetItem();
  objItem->setFlags(commFlags | ((useNormal)?Qt::NoItemFlags:Qt::ItemIsUserCheckable));
  objItem->setCheckState(dataObj->isRelative() ? Qt::Checked : Qt::Unchecked);
  this->TableWidget->setItem(row, Relative, objItem);
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
void pqCMBModifierArcManager::onItemChanged( QTableWidgetItem* item)
{
  int id = this->TableWidget->item( item->row(), Id )->text().toInt();
  pqCMBModifierArc* dl = ArcLines[id];
  if(dl == NULL) return;
  if(item->column() == VisibilityCol)
    {
    emit(orderChanged());
    }
  if(item->column() == Relative)
  {
    bool relative = item->checkState() == Qt::Checked;
    dl->setRelative(relative);
    if(WedgeFunctionWidget)
    {
      WedgeFunctionWidget->setRelative(relative);
    }
  }
}

void pqCMBModifierArcManager::onDatasetChange(QTableWidgetItem* item)
{
  if(this->CurrentModifierArc == NULL) return;
  int id = this->CurrentModifierArc->getId();
  if(item->column() == 0)
    {
    QString fname = this->Internal->UI->DatasetTable->item( item->row(), 1 )->text();
    int pieceIdx = this->Internal->UI->DatasetTable->item( item->row(), 2 )->text().toInt();
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
  if(this->Internal->selectedRow != -1)
  {
    QTableWidgetItem * tmp = this->TableWidget->item( this->Internal->selectedRow, Id );
    if(tmp != NULL)
    {
      int id = tmp->text().toInt();
      pqCMBModifierArc * ma = ArcLines[id];
      tmp = this->TableWidget->item( this->Internal->selectedRow, Relative);
      tmp->setFlags((tmp->flags() | Qt::ItemIsUserCheckable) ^ Qt::ItemIsUserCheckable);
    
      QTableWidgetItem * qtwi = new QTableWidgetItem(pointModesStrings[ma->getFunctionMode()]);
      Qt::ItemFlags commFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
      this->TableWidget->setCellWidget(this->Internal->selectedRow, Mode, NULL);
      this->TableWidget->setItem(this->Internal->selectedRow, Mode, qtwi);
      qtwi->setFlags(commFlags);
    }
    this->Internal->selectedRow = -1;
  }
  QList<QTableWidgetSelectionRange>     selected = this->TableWidget->selectedRanges();
  if (!selected.empty())
  {
    int row = selected.front().topRow();
    this->Internal->selectedRow = row;
    int id = this->TableWidget->item( row, Id )->text().toInt();
    pqCMBModifierArc * ma = ArcLines[id];

    QTableWidgetItem * tmp = this->TableWidget->item( this->Internal->selectedRow, Relative);
    tmp->setFlags(tmp->flags() | ((useNormal)?Qt::NoItemFlags:Qt::ItemIsUserCheckable));

    switch(this->Internal->mode)
    {
      case EditArc:
      {
        QTableWidgetItem * qtwi = new QTableWidgetItem(pointModesStrings[ma->getFunctionMode()]);
        Qt::ItemFlags commFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        this->TableWidget->setCellWidget(this->Internal->selectedRow, Mode, NULL);
        this->TableWidget->setItem(this->Internal->selectedRow, Mode, qtwi);
        tmp->setFlags((tmp->flags() | Qt::ItemIsUserCheckable) ^ Qt::ItemIsUserCheckable);
        qtwi->setFlags(commFlags);
        break;
      }
      case NoEditMode:
        this->Internal->mode = EditFunction;
      case EditFunction:
      {
        QComboBox* combo = new QComboBox();
        combo->addItems(pointModesStrings);
        combo->setCurrentIndex(ma->getFunctionMode());
        this->TableWidget->setItem(this->Internal->selectedRow, Mode, NULL);
        this->TableWidget->setCellWidget(this->Internal->selectedRow, Mode, combo);
        QObject::connect( combo, SIGNAL(currentIndexChanged(int)),
                          this,  SLOT(functionModeChanged(int)), Qt::UniqueConnection );
        break;
      }
    }
    emit selectionChanged(id);
  }
  else
  {
    this->Internal->mode = NoEditMode;
  }
  this->TableWidget->blockSignals(false);
}

void pqCMBModifierArcManager::onFunctionSelectionChange()
{
  cmbProfileFunction* fun = NULL;
  int	index = this->Internal->UI->FunctionName->currentIndex();
  if(index != -1)
  {
    QVariant d = this->Internal->UI->FunctionName->itemData(index);
    void * dv = d.value<void *>();
    fun = static_cast<cmbProfileFunction*>(dv);
  }
  selectFunction(fun);
}

void pqCMBModifierArcManager::onPointsSelectionChange()
{
  QList<QTableWidgetItem *>selected =	this->Internal->UI->points->selectedItems();
  if( !selected.empty() )
  {
    vtkSMSourceProxy * pointDisplaySource =
                        vtkSMSourceProxy::SafeDownCast(this->Internal->LineGlyphFilter->getProxy());
    QVariant qv = selected[0]->data(Qt::UserRole);
    pqCMBModifierArc::pointFunctionWrapper * wrapper =
                          static_cast<pqCMBModifierArc::pointFunctionWrapper*>(qv.value<void *>());
    this->Internal->UI->FunctionName->setCurrentIndex(this->Internal->UI->FunctionName->findText(wrapper->getName().c_str()));
    selectFunction(const_cast<cmbProfileFunction*>(wrapper->getFunction()));
    {
      double pt[3];
      vtkPVArcInfo* ai =  CurrentModifierArc->GetCmbArc()->getArcInfo();
      pointDisplaySource->InvokeCommand("clearVisible");
      double ml = this->Internal->boundingBox.GetMaxLength() * 0.015625;
      if(this->CurrentModifierArc->getFunctionMode() != pqCMBModifierArc::Single)
      {
        pqSMAdaptor::setElementProperty(pointDisplaySource->GetProperty("setVisible"),
                                        wrapper->getPointIndex());
        pqSMAdaptor::setElementProperty(pointDisplaySource->GetProperty("setScale"), ml);
        pointDisplaySource->UpdateVTKObjects();
      }
      pqSMAdaptor::setElementProperty(pointDisplaySource->GetProperty("setVisible"), -1);
      pointDisplaySource->UpdatePipeline();
      this->Internal->Representation->getProxy()->UpdateVTKObjects();
      this->view->forceRender();
      emit requestRender();
    }
  }
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
  vtkSMSourceProxy * pointDisplaySource =
                        vtkSMSourceProxy::SafeDownCast(this->Internal->LineGlyphFilter->getProxy());
  if(this->CurrentModifierArc != NULL)
    {
    if(sid == this->CurrentModifierArc->getId()) return;
    pqSMAdaptor::setInputProperty(pointDisplaySource->GetProperty("Input"), NULL, 0);
    pointDisplaySource->MarkModified(pointDisplaySource);
    pointDisplaySource->UpdateVTKObjects();
    this->CurrentModifierArc->switchToNotEditable();
    this->CurrentModifierArc = NULL;
    this->Internal->UI->removeLineButton->setEnabled(false);
    this->Internal->UI->buttonUpdateLine->setEnabled(false);
    this->Internal->UI->addLineButton->setEnabled(true);
    if(this->Internal->UI_Dialog != NULL)
      {
      QPushButton* applyButton = this->Internal->UI_Dialog->buttonBox->button(QDialogButtonBox::Apply);
      applyButton->setEnabled(true);
      }
    }
  if(sid == -1 || sid < -2)
  {
    this->Internal->UI->points->setRowCount(0);
    this->Internal->mode = NoEditMode;
    return;
  }
  else if(sid == -2) //create new one
  {
    this->Internal->mode = EditArc;
    pqCMBModifierArc * dig = new pqCMBModifierArc();
    this->ArcWidgetManager->setActiveArc(dig->GetCmbArc());
    this->ArcWidgetManager->create();
    QWidget * tmpWidget =this->ArcWidgetManager->getActiveWidget();
    tmpWidget->hide();
    this->Internal->CurrentArcWidget = dynamic_cast<qtCMBArcWidget*>(tmpWidget);

    QObject::connect(this->Internal->arcEditWidget->Close, SIGNAL(toggled(bool)),
                     this->Internal->CurrentArcWidget, SLOT(closeLoop(bool)), Qt::UniqueConnection);
    QObject::connect(this->Internal->arcEditWidget->EditMode, SIGNAL(toggled(bool)),
                     this->Internal->CurrentArcWidget, SLOT(EditMode()), Qt::UniqueConnection);
    QObject::connect(this->Internal->arcEditWidget->ModifyMode, SIGNAL(toggled(bool)),
                     this->Internal->CurrentArcWidget, SLOT(ModifyMode()), Qt::UniqueConnection);
    this->Internal->arcEditWidget->EditMode->setChecked(true);

    addNewArc(dig);

    this->disableSelection();
    this->Internal->UI->addLineButton->setEnabled(false);
    if(this->Internal->UI_Dialog != NULL)
    {
      QPushButton* applyButton = this->Internal->UI_Dialog->buttonBox->button(QDialogButtonBox::Apply);
      applyButton->setEnabled(false);
    }
  }
  else
    {
    if(static_cast<size_t>(sid)<ArcLines.size() && ArcLines[sid] != NULL)
      {
      this->CurrentModifierArc = ArcLines[sid];
      this->CurrentModifierArc->switchToEditable();
      this->Internal->mode = EditFunction;
      this->ArcWidgetManager->setActiveArc(this->CurrentModifierArc->GetCmbArc());
      this->Internal->UI->removeLineButton->setEnabled(true);
      this->Internal->UI->buttonUpdateLine->setEnabled(true);
      }
    }
  if(this->CurrentModifierArc != NULL)
  {
    if(this->CurrentModifierArc->GetCmbArc()->getSource() != NULL)
    {
      pqSMAdaptor::setInputProperty(pointDisplaySource->GetProperty("Input"),
                                    this->CurrentModifierArc->GetCmbArc()->getSource()->getProxy(),
                                    0);
      pointDisplaySource->MarkModified(pointDisplaySource);
      pointDisplaySource->UpdateVTKObjects();
    }
    this->updateLineFunctions();
    this->setDatasetTable(sid);
    this->setUpPointsTable();
    this->Internal->UI->points->selectRow(0);

  }
  this->updateUiControls();
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
      vtkSMSourceProxy* source = ServerProxies[filename][pieceIdx].source;
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
  switch(this->Internal->mode)
  {
    case EditArc:
      this->Internal->mode = EditFunction;
      this->Internal->CurrentArcWidget->finishContour();
      if(this->Internal->UI_Dialog != NULL)
      {
        QPushButton* applyButton =
                            this->Internal->UI_Dialog->buttonBox->button(QDialogButtonBox::Apply);
        applyButton->setEnabled(true);
      }
      break;
    case NoEditMode:
    case EditFunction:
      break;
  }

  if(this->CurrentModifierArc == NULL)
    {
    return;
    }
  {//Update point functions
    pqCMBArc * arc = CurrentModifierArc->GetCmbArc();
    vtkPVArcInfo* ai = arc->getArcInfo();
  }
  foreach(QString filename, ServerProxies.keys())
    {
    foreach(int pieceIdx, ServerProxies[filename].keys())
      {
      vtkSMSourceProxy* source = ServerProxies[filename][pieceIdx].source;
      if(source == NULL) continue;
      this->CurrentModifierArc->updateArc(source, this->Internal->boundingBox);
      }
    }
  setUpPointsTable();
  updateUiControls();
  emit(functionsUpdated());
  emit(requestRender());
  double removed = 0;
  double added = 0;

  bool isN1 = false;
  bool allNull = true;
  foreach(QString filename, ServerProxies.keys())
  {
    foreach(int pieceIdx, ServerProxies[filename].keys())
    {
      vtkSMSourceProxy* source = ServerProxies[filename][pieceIdx].source;
      if(source == NULL) continue;
      allNull = false;
      source->UpdatePropertyInformation();
      double r = pqSMAdaptor::getElementProperty(source->GetProperty("AmountRemoved")).toDouble();
      double a = pqSMAdaptor::getElementProperty(source->GetProperty("AmountAdded")).toDouble();
      if(r == -1 || a == -1)
      {
        isN1 = true;
        continue;
      }
      removed += r;
      added += a;
    }
  }
  if(isN1)
  {
    this->Internal->UI->amountAdded->setText("NA");
    this->Internal->UI->amountRemoved->setText("NA");
    if(!allNull) this->Internal->UI->PointBathChangeControl->show();
  }
  else
  {
    this->Internal->UI->amountAdded->setText(QString::number(added));
    this->Internal->UI->amountRemoved->setText(QString::number(removed));
  }
  if(allNull) this->Internal->UI->PointBathChangeControl->hide();
}

void pqCMBModifierArcManager::computeChange()
{
  double spacing = this->Internal->UI->SamplingSize->value();
  double radius = this->Internal->UI->SearchRadius->value();
  double removed = 0;
  double added = 0;

  QList< QVariant > v;
  v << spacing << radius;

  foreach(QString filename, ServerProxies.keys())
  {
    foreach(int pieceIdx, ServerProxies[filename].keys())
    {
      vtkSMSourceProxy* source = ServerProxies[filename][pieceIdx].source;
      if(source == NULL) continue;
      pqSMAdaptor::setMultipleElementProperty(source->GetProperty("computeDisplacementChange"), v);
      source->UpdateVTKObjects();
      source->UpdatePropertyInformation();
      double r = pqSMAdaptor::getElementProperty(source->GetProperty("AmountRemoved")).toDouble();
      double a = pqSMAdaptor::getElementProperty(source->GetProperty("AmountAdded")).toDouble();
      if(r == -1 || a == -1)
      {
        continue;
      }
      removed += r;
      added += a;
    }
  }
  this->Internal->UI->amountAdded->setText(QString::number(added));
  this->Internal->UI->amountRemoved->setText(QString::number(removed));
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
        vtkSMSourceProxy* source = ServerProxies[filename][pieceIdx].source;
        tmp->removeFromServer(source);
        }
      }

    delete tmp;
    }
  emit(requestRender());
}

void pqCMBModifierArcManager::addProxy(QString s, int pid, vtkBoundingBox box, pqPipelineSource* ps)
{
  assert(ps != NULL);
  assert(!s.isNull());
  vtkSMSourceProxy* source = NULL;
  source = vtkSMSourceProxy::SafeDownCast(ps->getProxy() );
  if(source == NULL)
  {
    return;
  }
  ServerProxies[s].insert(pid, DataSource(box, source));
  setUpTable();

  this->Internal->boundingBox.AddBox(box);

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
      this->ArcLines[i]->updateArc(source, this->Internal->boundingBox);
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
  if(!this->ArcLines.empty())
  {
    QMap< QString, QMap<int, bool> > newVis;
    ArcLinesApply.resize(this->ArcLines.size(), newVis);
  }
  this->Internal->boundingBox.Reset();
}

void pqCMBModifierArcManager::updateLineFunctions()
{
  if(this->CurrentModifierArc == NULL) return;
  pqCMBModifierArc * dig = this->CurrentModifierArc;
  std::vector<cmbProfileFunction *> funs;
  dig->getFunctions(funs);
  this->Internal->UI->FunctionName->blockSignals(true);
  this->Internal->UI->FunctionName->clear();
  for(unsigned int i = 0; i < funs.size(); ++i)
  {
    this->AddFunction(funs[i]);
  }
  this->functionModeChanged(CurrentModifierArc->getFunctionMode());
  QString name;
  QList<QTableWidgetItem *>selected =	this->Internal->UI->points->selectedItems();
  if( !selected.empty() )
  {
    QVariant qv = selected[0]->data(Qt::UserRole);
    pqCMBModifierArc::pointFunctionWrapper * wrapper =
                          static_cast<pqCMBModifierArc::pointFunctionWrapper*>(qv.value<void *>());
    name = wrapper->getName().c_str();
  }

  this->Internal->UI->FunctionName->setCurrentIndex(this->Internal->UI->FunctionName->findText(name));
  this->Internal->UI->FunctionName->blockSignals(false);
  onFunctionSelectionChange();
}

void pqCMBModifierArcManager::selectFunction(cmbProfileFunction* fun)
{
  delete ManualFunctionWidget;
  delete WedgeFunctionWidget;
  ManualFunctionWidget = NULL;
  WedgeFunctionWidget = NULL;
  selectedFunction = NULL;
  if(fun == NULL)
  {
    this->Internal->UI->CloneFunction->setEnabled(false);
    this->Internal->UI->DeleteFunction->setEnabled(false);
    this->Internal->UI->Save->setEnabled(false);
  }
  else
  {
    this->Internal->UI->CloneFunction->setEnabled(true);
    this->Internal->UI->DeleteFunction->setEnabled(true);
    this->Internal->UI->Save->setEnabled(true);
    this->Internal->UI->FunctionType->setCurrentIndex(static_cast<int>(fun->getType()));
    selectedFunction = fun;
    this->nameChanged(fun->getName().c_str());
    QList<QTableWidgetItem *>selected =	this->Internal->UI->points->selectedItems();
    if( !selected.empty() )
    {
      QVariant qv = selected[0]->data(Qt::UserRole);
      pqCMBModifierArc::pointFunctionWrapper * wrapper =
                          static_cast<pqCMBModifierArc::pointFunctionWrapper*>(qv.value<void *>());
      this->Internal->UI->points->item(selected[0]->row(), 1)->setText(selectedFunction->getName().c_str());
      this->Internal->UI->points->resizeColumnsToContents();
      switch (this->CurrentModifierArc->getFunctionMode())
      {
        case pqCMBModifierArc::EndPoints:
          if(wrapper->getPointIndex()!=0)
          {
            CurrentModifierArc->setEndFun(selectedFunction->getName());
            break;
          }
        case pqCMBModifierArc::Single:
          CurrentModifierArc->setStartFun(selectedFunction->getName());
          break;
        case pqCMBModifierArc::PointAssignment:
          CurrentModifierArc->addFunctionAtPoint(wrapper->getPointId(),selectedFunction);
      }
    }

    switch(fun->getType())
    {
      case cmbProfileFunction::MANUAL:
        ManualFunctionWidget =
            new qtCMBManualFunctionWidget(dynamic_cast<cmbManualProfileFunction*>(fun), NULL);
        functionLayout->addWidget(this->ManualFunctionWidget);
        break;
      case cmbProfileFunction::WEDGE:
        WedgeFunctionWidget =
          new qtCMBProfileWedgeFunctionWidget(NULL, dynamic_cast<cmbProfileWedgeFunction*>(fun));
        functionLayout->addWidget(this->WedgeFunctionWidget);
        break;
    }
  }
}

void pqCMBModifierArcManager::functionTypeChanged(int type)
{
  if(selectedFunction == NULL) return;
  if(type == selectedFunction->getType()) return;
  std::string name = selectedFunction->getName();
  cmbProfileFunction* fun =
        this->CurrentModifierArc->setFunction(name,
                                              static_cast<cmbProfileFunction::FunctionType>(type));
  int	index = this->Internal->UI->FunctionName->currentIndex();
  if(index != -1)
  {
    QVariant vdata;
    vdata.setValue(static_cast<void*>(fun));
    this->Internal->UI->FunctionName->setItemData(index, vdata); //todo
  }

  selectFunction(fun);
}

void pqCMBModifierArcManager::doneModifyingArc()
{
  if(this->CurrentModifierArc != NULL)
    {
    foreach(QString filename, ServerProxies.keys())
      {
      foreach(int pieceIdx, ServerProxies[filename].keys())
        {
        vtkSMSourceProxy* source = ServerProxies[filename][pieceIdx].source;
        this->CurrentModifierArc->updateArc(source, this->Internal->boundingBox);
        }
      }
    }

  this->enableSelection();
  this->CurrentModifierArc = NULL;
  //this->unselectAllRows();
  emit this->modifyingArcDone();
  {
    QList<QTableWidgetSelectionRange>     selected = this->TableWidget->selectedRanges();
    if (!selected.empty())
    {
      int row = selected.front().topRow();
      int id = this->TableWidget->item( row, Id )->text().toInt();
      selectLine(id);
    }
  }
  this->onSelectionChange();
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
      vtkSMSourceProxy* source = ServerProxies[filename][pieceIdx].source;
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
  QTableWidget* tmp = this->Internal->UI->DatasetTable;
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
  pqCMBModifierArc::pointFunctionWrapper * wrapper = NULL;
  QList<QTableWidgetItem *> selected =	this->Internal->UI->points->selectedItems();
  if( !selected.empty() )
  {
    QVariant qv = selected[0]->data(Qt::UserRole);
    wrapper = static_cast<pqCMBModifierArc::pointFunctionWrapper*>(qv.value<void *>());
  }
  QTableWidget* tmp = this->Internal->UI->points;
  tmp->setSelectionMode(QAbstractItemView::SingleSelection);
  tmp->setSelectionBehavior(QAbstractItemView::SelectRows);
  tmp->blockSignals(true);

  tmp->clearContents();
  tmp->setRowCount(0);
  if(CurrentModifierArc == NULL)
  {
    tmp->blockSignals(false);
    return;
  }
  tmp->setSelectionMode(QAbstractItemView::SingleSelection);

  pqCMBArc * arc = CurrentModifierArc->GetCmbArc();
  vtkPVArcInfo* ai = arc->getArcInfo();
  std::vector<pqCMBModifierArc::pointFunctionWrapper const*> points;
  CurrentModifierArc->getPointFunctions(points);
  for(size_t i = 0; i < points.size(); ++i)
  {
    this->addItemToTable(points[i], wrapper == points[i] || (wrapper == NULL && i == 0 ));
  }
  tmp->resizeColumnsToContents();
  tmp->blockSignals(false);
  onPointsSelectionChange();
}

void pqCMBModifierArcManager::setDatasetTable(int inId)
{
  if(inId < 0 || static_cast<size_t>(inId) >= ArcLines.size()) return;
  this->Internal->UI->DatasetTable->blockSignals(true);
  foreach(QString filename, ServerProxies.keys())
    {
    foreach(int pieceIdx, ServerProxies[filename].keys())
      {
      int row = ServerRows[filename][pieceIdx];
      bool visOnDs = ArcLinesApply[inId][filename][pieceIdx];
      this->Internal->UI->DatasetTable->item( row, 0 )->setCheckState( visOnDs ? Qt::Checked : Qt::Unchecked);
      }
    }
  this->Internal->UI->DatasetTable->blockSignals(false);
}

void pqCMBModifierArcManager::disableAbsolute()
{
  useNormal = true;
  this->TableWidget->blockSignals(true);
  for(unsigned int i = 0; i < ArcLines.size(); ++i)
  {
    QTableWidgetItem * tmp = this->TableWidget->item( i, Relative);
    tmp->setFlags((tmp->flags() | Qt::ItemIsUserCheckable) ^ Qt::ItemIsUserCheckable);
    tmp->setCheckState( Qt::Checked );

    ArcLines[i]->setRelative(true);
  }
  this->TableWidget->blockSignals(false);
}

void pqCMBModifierArcManager::enableAbsolute()
{
  useNormal = false;
  for(unsigned int i = 0; i < ArcLines.size(); ++i)
  {
    QTableWidgetItem * tmp = this->TableWidget->item( i, Relative);
    tmp->setFlags(tmp->flags() | Qt::ItemIsUserCheckable);
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
  out << 2 << "\n";

  CurrentModifierArc->write(out);
  pqOutputPort *port = CurrentModifierArc->GetCmbArc()->getSource()->getOutputPort(0);
  ContourInputs.push_back( port );

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
  QFileDialog::getOpenFileNames(NULL, "Open File...", "", "Function Profile (*.mar)");
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

  unsigned int at = start + 0;
  ArcLines[at]->read(in);
  this->setRow( rc+0, ArcLines[at] );
  ArcLines[at]->switchToNotEditable();

  accepted();
  onClearSelection();
  this->CurrentModifierArc = NULL;
  this->check_save();
}

void pqCMBModifierArcManager::importFunction()
{
  QStringList fileNames =
  QFileDialog::getOpenFileNames(NULL, "Open File...", "", "Function Profile (*.mar)");
  if(fileNames.count()==0)
  {
    return;
  }

  std::string fname = fileNames[0].toStdString();
  std::ifstream in(fname.c_str());
  int version;
  in >> version;
  this->CurrentModifierArc->read(in, true);
  this->updateLineFunctions();
}

void pqCMBModifierArcManager::check_save()
{
  for(unsigned int i = 0; i < ArcLines.size(); ++i)
  {
    if(ArcLines[i] != NULL)
    {
      this->Internal->UI->Save->setEnabled(true);
      return;
    }
  }
  this->Internal->UI->Save->setEnabled(false);
}


void pqCMBModifierArcManager::nameChanged(QString n)
{
  if(selectedFunction == NULL || CurrentModifierArc == NULL) return;
  if(n.isEmpty()) return;

  if(CurrentModifierArc->updateLabel(n.toStdString(), selectedFunction))
  {
    int	index = this->Internal->UI->FunctionName->currentIndex();
    if(index != -1)
    {
      this->Internal->UI->FunctionName->setItemText(index, n);
    }
    QPalette palette;
    palette.setColor(QPalette::Base,Qt::white);
    palette.setColor(QPalette::Text,Qt::black);
    this->Internal->UI->FunctionName->setPalette(palette);
    for(int row = 0; row < this->Internal->UI->points->rowCount(); ++row )
    {
      QTableWidgetItem * item = this->Internal->UI->points->item(row, 1);
      pqCMBModifierArc::pointFunctionWrapper * other =
        static_cast<pqCMBModifierArc::pointFunctionWrapper*>(item->data(Qt::UserRole).value<void *>());
      item->setText(other->getName().c_str());
    }
    this->Internal->UI->points->resizeColumnsToContents();
  }
  else
  {
    QPalette palette;
    palette.setColor(QPalette::Base,Qt::red);
    palette.setColor(QPalette::Text,Qt::black);
    this->Internal->UI->FunctionName->setPalette(palette);
  }
}

void pqCMBModifierArcManager::setToDefault()
{
  //TODO redo this
  //if(selectedFunction && CurrentModifierArc)
  //{
  //  CurrentModifierArc->setStartFun(selectedFunction->getName());
  //}
}

void pqCMBModifierArcManager::deleteFunction()
{
  if(selectedFunction && CurrentModifierArc)
  {
    this->Internal->UI->FunctionName->blockSignals(true);
    if(CurrentModifierArc->deleteFunction(selectedFunction->getName()))
    {
      int	index = this->Internal->UI->FunctionName->currentIndex();
      if(index != -1)
      {
        this->Internal->UI->FunctionName->removeItem(index);
        if(index != 0)
        {
          index--;
        }
        this->Internal->UI->FunctionName->setCurrentIndex(index);
      }
      onFunctionSelectionChange();
    }
    this->Internal->UI->FunctionName->blockSignals(false);
  }
  this->Internal->UI->DeleteFunction->setEnabled(this->Internal->UI->FunctionName->count() > 1);
}

void pqCMBModifierArcManager::cloneFunction()
{
  if(selectedFunction && CurrentModifierArc)
  {
    int	count = this->Internal->UI->FunctionName->count();
    this->AddFunction(CurrentModifierArc->cloneFunction(selectedFunction->getName()));
    this->Internal->UI->FunctionName->setCurrentIndex(count);
  }
}

void pqCMBModifierArcManager::functionModeChanged(int m)
{
  if(CurrentModifierArc == NULL) return;
  pqCMBModifierArc::FunctionMode mode = static_cast<pqCMBModifierArc::FunctionMode>(m);
  if(mode == CurrentModifierArc->getFunctionMode()) return;
  CurrentModifierArc->setFunctionMode(mode);
  this->Internal->UI->pointsFrame->show();
  switch(CurrentModifierArc->getFunctionMode())
  {
    case pqCMBModifierArc::EndPoints:
      this->Internal->UI->AddPoint->hide();
      this->Internal->UI->RemovePoint->hide();
      break;
    case pqCMBModifierArc::PointAssignment:
      this->Internal->UI->AddPoint->show();
      this->Internal->UI->RemovePoint->show();
      break;
    case pqCMBModifierArc::Single:
      this->Internal->UI->pointsFrame->hide();
      this->pointDisplayed(0);
      break;
  }
  this->setUpPointsTable();
}

void pqCMBModifierArcManager::pointDisplayed(int index)
{
  QString name;
  if(CurrentModifierArc == NULL) return;
  switch(CurrentModifierArc->getFunctionMode())
  {
    case pqCMBModifierArc::EndPoints:
      if(index == 1)
      {
        name = CurrentModifierArc->getEndFun()->getName().c_str();
        break;
      }
    case pqCMBModifierArc::Single:
      //this->ArcWidgetManager->highlightPoint(0);
      name = CurrentModifierArc->getStartFun()->getName().c_str();
      break;
    case pqCMBModifierArc::PointAssignment:
      break;
  }
  this->Internal->UI->FunctionName->setCurrentIndex(this->Internal->UI->FunctionName->findText(name));
}

int pqCMBModifierArcManager::addPoint(vtkIdType id)
{
  this->addPointMode = false;
  std::vector<cmbProfileFunction*> funs;
  CurrentModifierArc->getFunctions(funs);
  int result = -1;
  if(!CurrentModifierArc->pointHasFunction(id))
  {
    result = this->addItemToTable(CurrentModifierArc->addFunctionAtPoint(id, funs[0]), true);
  }
  this->updateUiControls();
  onSelectionChange();
  return result;
}

class vtkPointSelectedCallback : public vtkCommand
{
  friend class pqCMBModifierArcManager;
public:
  static vtkPointSelectedCallback *New()
  {
    vtkPointSelectedCallback *cb = new vtkPointSelectedCallback;

    return cb;
  }

  virtual void Execute(vtkObject *vtkNotUsed(caller),
                       unsigned long pointID,
                       void *vtkNotUsed(callData))
  {
    int index = static_cast<int>(pointID);
    vtkSMNewWidgetRepresentationProxy * widgetProxy = this->arcWidget->getWidgetProxy();
    vtkContourWidget *widget = vtkContourWidget::SafeDownCast(widgetProxy->GetWidget());
    vtkCMBArcWidgetRepresentation *widgetRep =
    vtkCMBArcWidgetRepresentation::SafeDownCast(widget->GetRepresentation());
    int row = -1;
    if(index >= 0 && index < this->arcInfo->GetNumberOfPoints())
    {
      vtkIdType id;
      this->arcInfo->GetPointID(index, id);
      row = manager->addPoint(id);
    }

    widgetRep->SetPointSelectMode(0);
    this->arcWidget->finishContour();
    this->arcWidget->setWidgetVisible(false);
    this->arcWidget->setVisible(false);
    this->arcWidget->deselect();
    this->arcWidget->hideWidget();
    this->arcWidget->getWidgetProxy()->UpdatePropertyInformation();
    this->arcWidget->setView(NULL);
    this->arcWidget->hide();
    if(row >= 0) manager->Internal->UI->points->selectRow(row);
  }

private:
  qtCMBArcWidget * arcWidget;
  vtkPVArcInfo* arcInfo;
  pqCMBModifierArcManager * manager;
};

void pqCMBModifierArcManager::addPoint()
{
  this->editArc();
  this->Internal->mode = EditFunction;

  vtkSMNewWidgetRepresentationProxy * widgetProxy =
                                                this->Internal->CurrentArcWidget->getWidgetProxy();
  vtkContourWidget *widget = vtkContourWidget::SafeDownCast(widgetProxy->GetWidget());
  vtkCMBArcWidgetRepresentation *widgetRep =
                          vtkCMBArcWidgetRepresentation::SafeDownCast(widget->GetRepresentation());
  widgetRep->PickableOn();
  widgetRep->SetPointSelectMode(1);
  vtkPointSelectedCallback * psc = vtkPointSelectedCallback::New();
  psc->arcWidget = this->Internal->CurrentArcWidget;
  psc->manager = this;
  psc->arcInfo = this->CurrentModifierArc->GetCmbArc()->getArcInfo();

  widgetRep->SetPointSelectCallBack(psc);

  this->addPointMode = true;
  this->updateUiControls();
}

void pqCMBModifierArcManager::deletePoint()
{
  if(this->addPointMode)
  {
    ArcWidgetManager->cancelSelectPoint();
    this->addPointMode = false;
    return;
  }

  QList<QTableWidgetItem *>selected =	this->Internal->UI->points->selectedItems();
  if( !selected.empty() )
  {
    QVariant qv = selected[0]->data(Qt::UserRole);
    pqCMBModifierArc::pointFunctionWrapper * wrapper =
                          static_cast<pqCMBModifierArc::pointFunctionWrapper*>(qv.value<void *>());
    CurrentModifierArc->removeFunctionAtPoint(wrapper->getPointId());
    this->Internal->UI->points->removeRow(selected[0]->row());

  }
}

int pqCMBModifierArcManager::addItemToTable(pqCMBModifierArc::pointFunctionWrapper const* mp,
                                             bool select)
{
  if(mp == NULL) return -1;
  QTableWidget* tmp = this->Internal->UI->points;

  Qt::ItemFlags commFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

  double pt[3];
  vtkPVArcInfo* ai =  CurrentModifierArc->GetCmbArc()->getArcInfo();
  ai->GetPointLocation(mp->getPointIndex(), pt);

  int row = tmp->rowCount();
  if(tmp->rowCount() != 0)
  {
    //If there are enough points, binary would be significantly faster
    for(row = 0; row < tmp->rowCount(); ++row)
    {
      QTableWidgetItem * item = tmp->item(row, 0);
      QVariant qv = item->data(Qt::UserRole);
      pqCMBModifierArc::pointFunctionWrapper * wrapper =
                      static_cast<pqCMBModifierArc::pointFunctionWrapper*>(qv.value<void *>());
      if(mp->getPointIndex() < wrapper->getPointIndex()) break;
    }
  }

  tmp->insertRow(row);

  QTableWidgetItem * qtwi = new QTableWidgetItem(QString::number(mp->getPointIndex()));
  QVariant vdata;
  vdata.setValue(static_cast<void*>(const_cast<pqCMBModifierArc::pointFunctionWrapper *>(mp)));
  qtwi->setData(Qt::UserRole, vdata);
  qtwi->setFlags(commFlags);
  tmp->setItem(row, 0, qtwi);
  qtwi = new QTableWidgetItem(QString(mp->getName().c_str()));
  qtwi->setData(Qt::UserRole, vdata);
  qtwi->setFlags(commFlags);
  tmp->setItem(row, 1, qtwi);
  if(select)
  {
    tmp->selectRow(row);
  }
  if (tmp->rowCount() == 1)
  {
    tmp->resizeColumnsToContents();
  }
  return row;
}

void pqCMBModifierArcManager::updateUiControls()
{
  switch(this->Internal->mode)
  {
    case NoEditMode:
      this->Internal->UI->EditControl->hide();
      this->Internal->UI->FunctionWidget->hide();
      this->Internal->UI->buttonUpdateLine->setEnabled(false);
      this->Internal->UI->addLineButton->setEnabled(true);
      this->Internal->UI->EditArc->setEnabled(false);
      this->Internal->UI->pointsFrame->hide();
      this->Internal->UI->DatasetControl->hide();
      this->Internal->UI->Load->setEnabled(true);
      this->Internal->UI->importFunctions->setEnabled(false);
      break;
    case EditArc:
      this->Internal->UI->EditControl->show();
      this->Internal->UI->FunctionWidget->hide();
      this->Internal->UI->buttonUpdateLine->setEnabled(true);
      this->Internal->UI->addLineButton->setEnabled(false);
      this->Internal->UI->EditArc->setEnabled(false);
      this->Internal->UI->pointsFrame->hide();
      this->Internal->UI->DatasetControl->hide();
      this->Internal->UI->Load->setEnabled(false);
      this->Internal->UI->importFunctions->setEnabled(false);
      break;
    case EditFunction:
      this->Internal->UI->EditControl->hide();
      this->Internal->UI->FunctionWidget->show();
      this->Internal->UI->buttonUpdateLine->setEnabled(true);
      this->Internal->UI->addLineButton->setEnabled(!this->addPointMode);
      this->Internal->UI->EditArc->setEnabled(!this->addPointMode);
      this->Internal->UI->DatasetControl->show();
      this->Internal->UI->Load->setEnabled(true);
      this->Internal->UI->importFunctions->setEnabled(true);
      if(this->CurrentModifierArc->getFunctionMode() == pqCMBModifierArc::Single)
      {
        this->Internal->UI->pointsFrame->hide();
      }
      else
      {
        this->Internal->UI->pointsFrame->show();
      }
      break;
  }
}

void pqCMBModifierArcManager::editArc()
{
  this->Internal->mode = EditArc;
  this->updateUiControls();
  this->onSelectionChange();

  this->ArcWidgetManager->setActiveArc(this->CurrentModifierArc->GetCmbArc());

  this->Internal->CurrentArcWidget->setView(this->view);

  vtkSMPropertyHelper(this->Internal->CurrentArcWidget->getWidgetProxy(), "Enabled").Set(1);
  this->Internal->CurrentArcWidget->getWidgetProxy()->UpdateVTKObjects();

  this->Internal->CurrentArcWidget->setWidgetVisible(true);
  this->Internal->CurrentArcWidget->setEnabled(true);

  vtkNew<vtkCMBArcEditClientOperator> editOp;
  editOp->SetArcIsClosed(CurrentModifierArc->GetCmbArc()->isClosedLoop());
  editOp->Operate(CurrentModifierArc->GetCmbArc()->getSource()->getProxy(),
                  this->Internal->CurrentArcWidget->getWidgetProxy());
  this->Internal->arcEditWidget->ModifyMode->setChecked(true);
  this->Internal->CurrentArcWidget->setModified();
  this->Internal->CurrentArcWidget->select();
  this->view->forceRender();
}

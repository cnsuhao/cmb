//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// this include
#include "pqSMTKInfoPanel.h"
#include "ui_pqSMTKInfoPanel.h"

// Qt includes
#include <QHeaderView>
#include <QLineEdit>
#include <QStringList>

// VTK includes
#include "vtkCommand.h"
#include "vtkEventQtSlotConnect.h"
#include <vtksys/SystemTools.hxx>

// ParaView Server Manager includes
#include "vtkPVArrayInformation.h"
#include "vtkPVCompositeDataInformation.h"
#include "vtkPVDataInformation.h"
#include "vtkPVDataSetAttributesInformation.h"
#include "vtkSMDomain.h"
#include "vtkSMDomainIterator.h"
#include "vtkSMDoubleVectorProperty.h"
#include "vtkSMOutputPort.h"
#include "vtkSMPropertyIterator.h"
#include "vtkSmartPointer.h"

// ParaView widget includes

// ParaView core includes
#include "pqActiveObjects.h"
#include "pqNonEditableStyledItemDelegate.h"
#include "pqObjectBuilder.h"
#include "pqOutputPort.h"
#include "pqPipelineSource.h"
#include "pqSMAdaptor.h"
#include "pqServer.h"
#include "pqTimeKeeper.h"

// ParaView components includes

// ModelBuilder includes
#include "pqCMBModelManager.h"

// SMTK includes
#include "smtk/extension/qt/qtActiveObjects.h"
#include "smtk/extension/qt/qtSelectionManager.h"
#include "smtk/mesh/core/Collection.h"
#include "smtk/mesh/core/DimensionTypes.h"
#include "smtk/mesh/core/Manager.h"
#include "smtk/model/CellEntity.h"

#include <array>

class pqSMTKInfoPanel::pqUi : public QObject, public Ui::pqSMTKInfoPanel
{
public:
  pqUi(QObject* p)
    : QObject(p)
  {
  }
};

pqSMTKInfoPanel::pqSMTKInfoPanel(QPointer<pqCMBModelManager> modelManager, QWidget* p)
  : QWidget(p)
  , ModelManager(modelManager)
  , OutputPort(NULL)
{
  this->VTKConnect = vtkEventQtSlotConnect::New();
  this->Ui = new pqUi(this);
  this->Ui->setupUi(this);
  QObject::connect(this->Ui->compositeTree,
    SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), this,
    SLOT(onCurrentItemChanged(QTreeWidgetItem*)), Qt::QueuedConnection);
  this->updateInformation(); // initialize state.

  this->connect(&pqActiveObjects::instance(), SIGNAL(portChanged(pqOutputPort*)), this,
    SLOT(setOutputPort(pqOutputPort*)));

  QObject::connect(qtActiveObjects::instance().smtkSelectionManager().get(),
    SIGNAL(broadcastToReceivers(const smtk::model::EntityRefs&, const smtk::mesh::MeshSets&,
      const smtk::model::DescriptivePhrases&, const std::string&)),
    this, SLOT(onSelectionChangedUpdateInfoPanel(const smtk::model::EntityRefs&,
            const smtk::mesh::MeshSets&, const smtk::model::DescriptivePhrases&)));
}

pqSMTKInfoPanel::~pqSMTKInfoPanel()
{
  this->VTKConnect->Disconnect();
  this->VTKConnect->Delete();
}

void pqSMTKInfoPanel::setOutputPort(pqOutputPort* source)
{
  if (this->OutputPort == source)
  {
    return;
  }

  this->VTKConnect->Disconnect();
  if (this->OutputPort)
  {
    QObject::disconnect(this->OutputPort->getSource(), SIGNAL(dataUpdated(pqPipelineSource*)), this,
      SLOT(updateInformation()));
  }

  this->OutputPort = source;

  if (this->OutputPort)
  {
    QObject::connect(this->OutputPort->getSource(), SIGNAL(dataUpdated(pqPipelineSource*)), this,
      SLOT(updateInformation()));
  }

  this->updateInformation();
}

/// get the proxy for which properties are displayed
pqOutputPort* pqSMTKInfoPanel::getOutputPort()
{
  return this->OutputPort;
}

void pqSMTKInfoPanel::updateInformation()
{
  this->Ui->compositeTree->clear();
  this->Ui->compositeTree->setVisible(false);
  this->Ui->filename->setText(tr("NA"));
  this->Ui->filename->setToolTip(tr("NA"));
  this->Ui->filename->setStatusTip(tr("NA"));
  this->Ui->path->setText(tr("NA"));
  this->Ui->path->setToolTip(tr("NA"));
  this->Ui->path->setStatusTip(tr("NA"));

  vtkPVDataInformation* dataInformation = NULL;
  pqPipelineSource* source = NULL;
  if (this->OutputPort)
  {
    source = this->OutputPort->getSource();
    if (this->OutputPort->getOutputPortProxy())
    {
      dataInformation = this->OutputPort->getDataInformation();
    }
  }

  if (!source || !dataInformation)
  {
    this->fillDataInformation(0);
    return;
  }

  vtkPVCompositeDataInformation* compositeInformation =
    dataInformation->GetCompositeDataInformation();

  if (compositeInformation->GetDataIsComposite())
  {
    QTreeWidgetItem* root = this->fillCompositeInformation(dataInformation);
    this->Ui->compositeTree->setVisible(true);
    root->setExpanded(true);
    root->setSelected(true);
  }

  this->fillDataInformation(dataInformation);

  // Find the first property that has a vtkSMFileListDomain. Assume that
  // it is the property used to set the filename.
  vtkSmartPointer<vtkSMPropertyIterator> piter;
  piter.TakeReference(source->getProxy()->NewPropertyIterator());
  for (piter->Begin(); !piter->IsAtEnd(); piter->Next())
  {
    vtkSMProperty* prop = piter->GetProperty();
    if (prop->IsA("vtkSMStringVectorProperty"))
    {
      vtkSmartPointer<vtkSMDomainIterator> diter;
      diter.TakeReference(prop->NewDomainIterator());
      for (diter->Begin(); !diter->IsAtEnd(); diter->Next())
      {
        if (diter->GetDomain()->IsA("vtkSMFileListDomain"))
        {
          vtkSMProperty* smprop = piter->GetProperty();
          if (smprop->GetInformationProperty())
          {
            smprop = smprop->GetInformationProperty();
            source->getProxy()->UpdatePropertyInformation(smprop);
          }
          QString filename = pqSMAdaptor::getElementProperty(smprop).toString();
          QString path = vtksys::SystemTools::GetFilenamePath(filename.toUtf8().data()).c_str();

          this->Ui->properties->show();
          this->Ui->filename->setText(
            vtksys::SystemTools::GetFilenameName(filename.toUtf8().data()).c_str());
          this->Ui->filename->setToolTip(filename);
          this->Ui->filename->setStatusTip(filename);
          this->Ui->path->setText(path);
          this->Ui->path->setToolTip(path);
          this->Ui->path->setStatusTip(path);
          break;
        }
      }
      if (!diter->IsAtEnd())
      {
        break;
      }
    }
  }

  // Check if there are timestep values. If yes, display them.
  vtkSMDoubleVectorProperty* tsv =
    vtkSMDoubleVectorProperty::SafeDownCast(source->getProxy()->GetProperty("TimestepValues"));
  this->Ui->timeValues->clear();
  this->Ui->timeValues->setItemDelegate(new pqNonEditableStyledItemDelegate(this));
  //
  QAbstractItemModel* pModel = this->Ui->timeValues->model();
  pModel->blockSignals(true);
  this->Ui->timeValues->blockSignals(true);
  //
  if (tsv)
  {
    unsigned int numElems = tsv->GetNumberOfElements();
    for (unsigned int i = 0; i < numElems; i++)
    {
      QTreeWidgetItem* item = new QTreeWidgetItem(this->Ui->timeValues);
      item->setData(0, Qt::DisplayRole, i);
      item->setData(1, Qt::DisplayRole, QString::number(tsv->GetElement(i), 'g', 17));
      item->setData(1, Qt::ToolTipRole, QString::number(tsv->GetElement(i), 'g', 17));
      item->setFlags(item->flags() | Qt::ItemIsEditable);
    }
  }
  this->Ui->timeValues->blockSignals(false);
  pModel->blockSignals(false);
}

void pqSMTKInfoPanel::fillDataInformation(vtkPVDataInformation* dataInformation)
{
  this->Ui->properties->setVisible(false);
  // out with the old
  this->Ui->type->setText(tr("NA"));
  this->Ui->numberOfCells->setText(tr("NA"));
  this->Ui->numberOfPoints->setText(tr("NA"));
  this->Ui->numberOfRows->setText(tr("NA"));
  this->Ui->numberOfColumns->setText(tr("NA"));
  this->Ui->memory->setText(tr("NA"));

  this->Ui->dataArrays->clear();

  this->Ui->xRange->setText(tr("NA"));
  this->Ui->yRange->setText(tr("NA"));
  this->Ui->zRange->setText(tr("NA"));

  this->Ui->groupExtent->setVisible(false);
  this->Ui->dataTimeLabel->setVisible(false);

  // if dataInformation->GetNumberOfDataSets() == 0, that means that the data
  // information does not have any valid values.
  if (!dataInformation || dataInformation->GetNumberOfDataSets() == 0)
  {
    return;
  }

  this->Ui->type->setText(tr(dataInformation->GetPrettyDataTypeString()));

  QString numCells = QString("%1").arg(dataInformation->GetNumberOfCells());
  this->Ui->numberOfCells->setText(numCells);

  QString numPoints = QString("%1").arg(dataInformation->GetNumberOfPoints());
  this->Ui->numberOfPoints->setText(numPoints);

  QString numRows = QString("%1").arg(dataInformation->GetNumberOfRows());
  this->Ui->numberOfRows->setText(numRows);

  QString numColumns =
    QString("%1").arg(dataInformation->GetRowDataInformation()->GetNumberOfArrays());
  this->Ui->numberOfColumns->setText(numColumns);

  if (dataInformation->GetDataSetType() == VTK_TABLE)
  {
    this->Ui->dataTypeProperties->setCurrentWidget(this->Ui->Table);
  }
  else
  {
    this->Ui->dataTypeProperties->setCurrentWidget(this->Ui->DataSet);
  }

  QString memory = QString("%1 MB").arg(dataInformation->GetMemorySize() / 1000.0, 0, 'g', 2);
  this->Ui->memory->setText(memory);

  if (dataInformation->GetHasTime())
  {
    this->Ui->dataTimeLabel->setVisible(true);
    const char* timeLabel = dataInformation->GetTimeLabel();
    this->Ui->dataTimeLabel->setText(QString("Current data %2: %1")
                                       .arg(dataInformation->GetTime())
                                       .arg(timeLabel ? timeLabel : "time"));
    this->Ui->groupDataTime->setTitle(timeLabel);
  }
  else
  {
    this->Ui->groupDataTime->setTitle(tr("Time"));
  }

  vtkPVDataSetAttributesInformation* info[6];
  info[0] = dataInformation->GetPointDataInformation();
  info[1] = dataInformation->GetCellDataInformation();
  info[2] = dataInformation->GetVertexDataInformation();
  info[3] = dataInformation->GetEdgeDataInformation();
  info[4] = dataInformation->GetRowDataInformation();
  info[5] = dataInformation->GetFieldDataInformation();

  QPixmap pixmaps[6] = { QPixmap(":/pqWidgets/Icons/pqPointData16.png"),
    QPixmap(":/pqWidgets/Icons/pqCellData16.png"), QPixmap(":/pqWidgets/Icons/pqPointData16.png"),
    QPixmap(":/pqWidgets/Icons/pqCellData16.png"), QPixmap(":/pqWidgets/Icons/pqSpreadsheet16.png"),
    QPixmap(":/pqWidgets/Icons/pqGlobalData16.png") };

  if (dataInformation->IsDataStructured())
  {
    this->Ui->groupExtent->setVisible(true);
    int ext[6];
    dataInformation->GetExtent(ext);
    if (ext[1] >= ext[0] && ext[3] >= ext[2] && ext[5] >= ext[4])
    {
      int dims[3];
      dims[0] = ext[1] - ext[0] + 1;
      dims[1] = ext[3] - ext[2] + 1;
      dims[2] = ext[5] - ext[4] + 1;

      this->Ui->xExtent->setText(
        QString("%1 to %2 (dimension: %3)").arg(ext[0]).arg(ext[1]).arg(dims[0]));
      this->Ui->yExtent->setText(
        QString("%1 to %2 (dimension: %3)").arg(ext[2]).arg(ext[3]).arg(dims[1]));
      this->Ui->zExtent->setText(
        QString("%1 to %2 (dimension: %3)").arg(ext[4]).arg(ext[5]).arg(dims[2]));
    }
    else
    {
      this->Ui->xExtent->setText(tr("NA"));
      this->Ui->yExtent->setText(tr("NA"));
      this->Ui->zExtent->setText(tr("NA"));
    }
  }

  for (int k = 0; k < 6; k++)
  {
    if (info[k])
    {
      int numArrays = info[k]->GetNumberOfArrays();
      for (int i = 0; i < numArrays; i++)
      {
        vtkPVArrayInformation* arrayInfo;
        arrayInfo = info[k]->GetArrayInformation(i);
        // name, type, data type, data range
        QTreeWidgetItem* item = new QTreeWidgetItem(this->Ui->dataArrays);
        item->setData(0, Qt::DisplayRole, arrayInfo->GetName());
        item->setData(0, Qt::DecorationRole, pixmaps[k]);
        QString dataType = vtkImageScalarTypeNameMacro(arrayInfo->GetDataType());
        item->setData(1, Qt::DisplayRole, dataType);
        int numComponents = arrayInfo->GetNumberOfComponents();
        QString dataRange;
        double range[2];
        for (int j = 0; j < numComponents; j++)
        {
          if (j != 0)
          {
            dataRange.append(", ");
          }
          arrayInfo->GetComponentRange(j, range);
          QString componentRange = QString("[%1, %2]").arg(range[0]).arg(range[1]);
          dataRange.append(componentRange);
        }
        item->setData(2, Qt::DisplayRole, dataType == "string" ? tr("NA") : dataRange);
        item->setData(2, Qt::ToolTipRole, dataRange);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        if (arrayInfo->GetIsPartial())
        {
          item->setForeground(0, QBrush(QColor("darkBlue")));
          item->setData(0, Qt::DisplayRole, QString("%1 (partial)").arg(arrayInfo->GetName()));
        }
        else
        {
          item->setForeground(0, QBrush(QColor("darkGreen")));
        }
      }
    }
  }
  this->Ui->dataArrays->header()->resizeSections(QHeaderView::ResizeToContents);
  this->Ui->dataArrays->setItemDelegate(new pqNonEditableStyledItemDelegate(this));

  double bounds[6];
  dataInformation->GetBounds(bounds);
  QString xrange;
  if (bounds[0] == VTK_DOUBLE_MAX && bounds[1] == -VTK_DOUBLE_MAX)
  {
    xrange = tr("Not available");
  }
  else
  {
    xrange = QString("%1 to %2 (delta: %3)");
    xrange = xrange.arg(bounds[0], -1, 'g', 3);
    xrange = xrange.arg(bounds[1], -1, 'g', 3);
    xrange = xrange.arg(bounds[1] - bounds[0], -1, 'g', 3);
  }
  this->Ui->xRange->setText(xrange);

  QString yrange;
  if (bounds[2] == VTK_DOUBLE_MAX && bounds[3] == -VTK_DOUBLE_MAX)
  {
    yrange = tr("Not available");
  }
  else
  {
    yrange = QString("%1 to %2 (delta: %3)");
    yrange = yrange.arg(bounds[2], -1, 'g', 3);
    yrange = yrange.arg(bounds[3], -1, 'g', 3);
    yrange = yrange.arg(bounds[3] - bounds[2], -1, 'g', 3);
  }
  this->Ui->yRange->setText(yrange);

  QString zrange;
  if (bounds[4] == VTK_DOUBLE_MAX && bounds[5] == -VTK_DOUBLE_MAX)
  {
    zrange = tr("Not available");
  }
  else
  {
    zrange = QString("%1 to %2 (delta: %3)");
    zrange = zrange.arg(bounds[4], -1, 'g', 3);
    zrange = zrange.arg(bounds[5], -1, 'g', 3);
    zrange = zrange.arg(bounds[5] - bounds[4], -1, 'g', 3);
  }
  this->Ui->zRange->setText(zrange);
}

QTreeWidgetItem* pqSMTKInfoPanel::fillCompositeInformation(
  vtkPVDataInformation* info, QTreeWidgetItem* parentItem /*=0*/)
{
  QTreeWidgetItem* node = 0;

  QString label = info ? info->GetPrettyDataTypeString() : tr("NA");
  if (parentItem)
  {
    node = new QTreeWidgetItem(parentItem, QStringList(label));
  }
  else
  {
    node = new QTreeWidgetItem(this->Ui->compositeTree, QStringList(label));
    this->Ui->compositeTree->setItemDelegate(new pqNonEditableStyledItemDelegate(this));
  }
  if (!info)
  {
    return node;
  }

  // we save a ptr to the data information to easily locate the data
  // information.
  node->setData(0, Qt::UserRole, QVariant::fromValue((void*)info));
  node->setFlags(node->flags() | Qt::ItemIsEditable);

  vtkPVCompositeDataInformation* compositeInformation = info->GetCompositeDataInformation();

  if (!compositeInformation->GetDataIsComposite() || compositeInformation->GetDataIsMultiPiece())
  {
    return node;
  }

  bool isNonOverlappingAMR =
    (strcmp(info->GetCompositeDataClassName(), "vtkNonOverlappingAMR") == 0);
  bool isOverlappingAMR = (strcmp(info->GetCompositeDataClassName(), "vtkOverlappingAMR") == 0);
  bool isHB = (strcmp(info->GetCompositeDataClassName(), "vtkHierarchicalBoxDataSet") == 0);

  bool isAMR = isHB || isOverlappingAMR || isNonOverlappingAMR;

  unsigned int numChildren = compositeInformation->GetNumberOfChildren();
  for (unsigned int cc = 0; cc < numChildren; cc++)
  {
    vtkPVDataInformation* childInfo = compositeInformation->GetDataInformation(cc);
    QTreeWidgetItem* childItem = this->fillCompositeInformation(childInfo, node);
    childItem->setFlags(childItem->flags() | Qt::ItemIsEditable);
    const char* name = compositeInformation->GetName(cc);
    if (name && name[0])
    {
      // use name given to the block.
      childItem->setText(0, name);
    }
    else if (isAMR)
    {
      childItem->setText(0, QString("Level %1").arg(cc));
    }
    else if (childInfo && childInfo->GetCompositeDataClassName())
    {
      childItem->setText(0, QString("Block %1").arg(cc));
    }
    else
    {
      // Use the data classname in the name for the leaf node.
      childItem->setText(0, QString("%1: %2").arg(cc).arg(childItem->text(0)));
    }
  }

  return node;
}

void pqSMTKInfoPanel::onCurrentItemChanged(QTreeWidgetItem* item)
{
  if (item)
  {
    vtkPVDataInformation* info =
      reinterpret_cast<vtkPVDataInformation*>(item->data(0, Qt::UserRole).value<void*>());
    this->fillDataInformation(info);
  }
}

//-----------------------------------------------------------------------------
void pqSMTKInfoPanel::onSelectionChangedUpdateInfoPanel(const smtk::model::EntityRefs& erefs,
  const smtk::mesh::MeshSets& meshes, const smtk::model::DescriptivePhrases& desPhrases)
{
  (void)erefs;
  (void)desPhrases;

  this->Ui->entityNumberOf3DCells->setText(tr("NA"));
  this->Ui->entityNumberOf2DCells->setText(tr("NA"));
  this->Ui->entityNumberOf1DCells->setText(tr("NA"));
  this->Ui->entityNumberOf0DCells->setText(tr("NA"));
  this->Ui->entityNumberOfPoints->setText(tr("NA"));

  if (!meshes.empty())
  {
    std::array<smtk::mesh::HandleRange, 5> ranges;
    for (auto& mesh : meshes)
    {
      ranges[0].merge(mesh.cells(smtk::mesh::Dims0).range());
      ranges[1].merge(mesh.cells(smtk::mesh::Dims1).range());
      ranges[2].merge(mesh.cells(smtk::mesh::Dims2).range());
      ranges[3].merge(mesh.cells(smtk::mesh::Dims3).range());
      ranges[4].merge(mesh.points().range());
    }

    QString num3DCells = QString("%1").arg(ranges[3].size());
    this->Ui->entityNumberOf3DCells->setText(num3DCells);

    QString num2DCells = QString("%1").arg(ranges[2].size());
    this->Ui->entityNumberOf2DCells->setText(num2DCells);

    QString num1DCells = QString("%1").arg(ranges[1].size());
    this->Ui->entityNumberOf1DCells->setText(num1DCells);

    QString num0DCells = QString("%1").arg(ranges[0].size());
    this->Ui->entityNumberOf0DCells->setText(num0DCells);

    QString numPoints = QString("%1").arg(ranges[4].size());
    this->Ui->entityNumberOfPoints->setText(numPoints);
  }
}

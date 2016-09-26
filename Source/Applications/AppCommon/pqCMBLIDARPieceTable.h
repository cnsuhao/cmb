//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqCMBLIDARPieceTable - .
// .SECTION Description
// .SECTION Caveats

#ifndef __pqCMBLIDARPieceTable_h
#define __pqCMBLIDARPieceTable_h

#include "cmbAppCommonExport.h"
#include <QObject>
#include <QMap>
#include <QList>
#include "vtkBoundingBox.h"
#include "cmbSystemConfig.h"

class QTableWidget;
class pqCMBLIDARPieceObject;
class pqDataRepresentation;
class pqPipelineSource;
class QTableWidgetItem;

class CMBAPPCOMMON_EXPORT pqCMBLIDARPieceTable : public QObject
{
  Q_OBJECT

public:
  pqCMBLIDARPieceTable(QTableWidget *widget, bool advancedTable = false);
  ~pqCMBLIDARPieceTable() override;

  QTableWidget *getWidget() const { return this->TableWidget;}

  void AddLIDARPiece(pqCMBLIDARPieceObject *dataObj, int visible=1);
  QList<pqCMBLIDARPieceObject *> getVisiblePieceObjects();
  QList<pqCMBLIDARPieceObject *> getAllPieceObjects();

  pqCMBLIDARPieceObject* getCurrentObject(bool onlyIfSelectable = true);
  void updateWithPieceInfo(int pieceIndex);
  void updateWithPieceInfo(pqCMBLIDARPieceObject *dataObj, int row = -1);

  void setOnRatioOfSelectedPieces( int newOnRatio);

  void setLinkedTable(pqCMBLIDARPieceTable* linkedTable)
    { this->LinkedTable = linkedTable; }

  void setClipEnabled(bool state)
    { this->ClipEnabled = state; }
  bool getClipEnabled()
    { return this->ClipEnabled; }

  void setClipBBox(vtkBoundingBox &clipBBox)
    { this->ClipBBox = clipBBox; }
  vtkBoundingBox &getClipBBox()
    { return this->ClipBBox; }

  int getCurrentRow(bool onlyIfSelectable = true);
  int computeSaveNumberOfPointsEstimate(pqCMBLIDARPieceObject *dataObj);
  int computeDisplayNumberOfPointsEstimate(pqCMBLIDARPieceObject *dataObj);
  void selectObject(pqPipelineSource* selSource);

public slots:
  void setCurrentPiece(int pieceId);
  void setCurrentOnRatio(int onRatio);
  void clear();
  void onClearSelection();
  void checkAll();
  void uncheckAll();
  void onCurrentObjectChanged();
  void onItemChanged(QTableWidgetItem*);
  void configureTable(int advancedTable);

signals:
  void objectOnRatioChanged(pqCMBLIDARPieceObject*, int);
  void currentObjectChanged(pqCMBLIDARPieceObject*);
  void checkedObjectsChanged(QList<int> newChecked, QList<int> newUnChecked);
  void requestRender();

protected:
  QTableWidget* TableWidget;
  pqCMBLIDARPieceTable *LinkedTable;

  void initialize(bool advancedTable);
  pqCMBLIDARPieceObject* getRowObject(int row);
  int getObjectRow(pqCMBLIDARPieceObject *dataObj);
  QMap<int, int> getSelectedPieceOnRatios();
  void updateCheckedItems(QList<int> newChecked, QList<int> newUnChecked);
  void setSelection(int selectionRow);
  void setVisibility(int row, int visibilityState);
  void addAdvancedColumns(int row, pqCMBLIDARPieceObject *dataObj);
  bool ClipEnabled;
  vtkBoundingBox ClipBBox;
};

#endif /* __pqCMBLIDARPieceTable_h */

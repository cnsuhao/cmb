/*=========================================================================

  Program:   CMB
  Module:    pqCMBLIDARPieceTable.h

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
  virtual ~pqCMBLIDARPieceTable();

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

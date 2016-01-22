//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqCMBModifierArcManager - .
// .SECTION Description
// .SECTION Caveats

#ifndef __pqCMBModifierArcManager_h
#define __pqCMBModifierArcManager_h

#include "cmbAppCommonExport.h"
#include <QObject>
#include <QMap>
#include <QList>
#include <QPointer>
#include <QGridLayout>
#include <QDialog>
#include <vector>
#include <fstream>
#include "qtCMBArcWidgetManager.h"
#include "cmbSystemConfig.h"
#include "pqGeneralTransferFunctionWidget.h"
#include "pqCMBSceneObjectBase.h"

class QTableWidget;
class pqCMBModifierArc;
class pqDataRepresentation;
class pqPipelineSource;
class QTableWidgetItem;
class vtkSMSourceProxy;
class Ui_qtArcFunctionControl;
class Ui_qtModifierArcDialog;
class qtCMBArcWidgetManager;
class pqServer;
class pqRenderView;
class pqPipelineSource;

class CMBAPPCOMMON_EXPORT pqCMBModifierArcManager : public QObject
{
  Q_OBJECT

public:
  pqCMBModifierArcManager(QLayout *layout,
                     pqServer *server,
                     pqRenderView *renderer);
  virtual ~pqCMBModifierArcManager();

  QTableWidget *getWidget() const { return this->TableWidget;}
  qtCMBArcWidgetManager* arcWidgetManager() const {return this->ArcWidgetManager;}

  void AddLinePiece(pqCMBModifierArc *dataObj, int visible=1);

  std::vector<int> getCurrentOrder(QString, int pindx);

  bool remove(int i, bool all = true);

  void setSourceType(pqCMBSceneObjectBase::enumObjectType st)
  { this->SourceType = st; }

  pqCMBSceneObjectBase::enumObjectType getSourceType() const
  { return this->SourceType; }

public slots:
  void showDialog();
  void clear();
  void onClearSelection();
  void onItemChanged(QTableWidgetItem*);
  void onDatasetChange(QTableWidgetItem*);
  void onCurrentObjectChanged();
  void unselectAllRows();
  void onSelectionChange();
  void enableSelection();
  void disableSelection();
  void applyAgain();
  void addProxy(QString, int pid, pqPipelineSource*);
  void clearProxies();
  void addLine();
  void selectLine(int lid);
  void update();
  void removeArc();
  void doneModifyingArc();
  void sendOrder();
  void disableAbsolute();
  void enableAbsolute();
  void onSaveProfile();
  void onLoadProfile();
  void onSaveArc();
  void onLoadArc();

signals:
  void currentObjectChanged(pqCMBModifierArc*);
  void selectionChanged(int);
  void requestRender();
  void removed(int i);
  void orderChanged();
  void functionsUpdated();
  void applyFunctions();
  void updateDisplacementSlineControl(double, double, double);
  void updateWeightSlineControl(double, double, double);
  void changeDisplacementFunctionType(bool);
  void changeWeightFunctionType(bool);

  void addingNewArc();
  void modifyingArcDone();

protected:
  QTableWidget * TableWidget;
  std::vector<pqCMBModifierArc*> ArcLines;
  pqCMBModifierArc * CurrentModifierArc;
  QMap< QString, QMap<int, vtkSMSourceProxy*> > ServerProxies;
  QMap< QString, QMap<int, int> > ServerRows;
  std::vector< QMap< QString, QMap<int, bool> > > ArcLinesApply;
  qtCMBArcWidgetManager* ArcWidgetManager;
  pqCMBSceneObjectBase::enumObjectType SourceType;

  void initialize();
  void setRow(int row, pqCMBModifierArc * line);

  Ui_qtArcFunctionControl * UI;
  Ui_qtModifierArcDialog * UI_Dialog;
  QDialog * Dialog;

  QPointer<pqGeneralTransferFunctionWidget> DisplacementProfile;
  QPointer<pqGeneralTransferFunctionWidget> WeightingFunction;

  void addApplyControl();

  void setUpTable();

  void setUpPointsTable();

  void setDatasetTable(int inId);

  void addNewArc(pqCMBModifierArc* arc);
  void check_save();

protected slots:
  void updateLineFunctions();
  void accepted();
  void dispSplineControlChanged();
  void weightingSplineControlChanged();
  void dispSplineBox(bool);
  void weightSplineBox(bool);

protected slots:
  void onLineChange(int Id);
};

#endif /* __pqCMBModifierArcManager_h */

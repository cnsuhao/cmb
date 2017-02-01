//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqCMBMeshViewerMainWindow
// .SECTION Description
// The main window for the application.

#ifndef __pqCMBMeshViewerMainWindow_h
#define __pqCMBMeshViewerMainWindow_h

#include "pqCMBCommonMainWindow.h"
#include <QVariant>
#include "cmbSystemConfig.h"

class pqOutputPort;
class vtkDataSet;
class vtkIntArray;
class pqPipelineSource;
class pqDataRepresentation;
class pqCMBMeshViewerMainWindowCore;
class QTreeWidget;
class QTreeWidgetItem;
class vtkSMProxy;

class pqCMBMeshViewerMainWindow : public pqCMBCommonMainWindow
{
  typedef pqCMBCommonMainWindow Superclass;
  Q_OBJECT

public:
  pqCMBMeshViewerMainWindow();
  ~pqCMBMeshViewerMainWindow() override;

  enum enumInputTreeColumn
    {
    TREE_INPUT_COL,
    TREE_ACTIVE_COL,
    TREE_VISIBLE_COL
    };

public slots:

  void onMeshLoaded();
  void onMeshModified();
  void onFilterPropertiesModified(bool);
  void updateChangeMaterialButton();
  void setSelectionMode();
  void applyFilters();
  void onApplySmoothing();
  void onInputChanged();
  void onExportSubset();
  void extractSelection();
  void invertSelection();
  void onRemoveInputSource();
  void onInputSelectionChanged();
  void onInputClicked(QTreeWidgetItem*, int);
  void onInputNameChanged(QTreeWidgetItem*, int){};
  void showPointLabel(bool);
  void showCellLabel(bool);
  void onDefineContourWidget();
  void onInvokeBoxWidget();
  void onInvokePlaneWidget();
  void onContourFinished();
  void onContourChanged();
  void onUpdateBoxInteraction();
  void onEndPlaneInteraction();
  void onUpdatePlaneInteraction();
  void updateBoxWidget();
  void updatePlaneWidget();
  void onClearSelection();
  void onSelectAll();

protected slots:
  // Description:
  // Updates the enable state of various menus.
  void updateEnableState();

  // Description:
  // open About dialog
  void onHelpAbout() override;
  void onHelpHelp() override;

  // Description:
  // Called when starting and external process (to disable starting another)
  // and when completing an external process (to reenable)
  void onEnableExternalProcesses(bool state) override;

  // Description:
  // Called when "Find Data" action is invoked.
  void showQueryDialog();

  // Description:
  // Invoked when selection is changed.
  void updateSelection() override;

  // Description:
  // Invoked when "Set Selection Material ID to" button is clicked, which
  // will then call the mainwindowcore to change the material ids for selected cells
  // with the id selected in the material id dropdown list.
  virtual void onChangeSelectionMaterialId();

  // Description:
  // slots when shortcuts are activated
  void onExtractShortcutActivated();
  void onSetActiveShortcutActivated();

  // Description:
  // slots when shortcuts are activated
  void onShapeSelectionOptionClicked();

  // Description:
  // slots to switch HistogramView and SpreadsheetView
  void onSwitchHistogramAndSpreadsheet(bool showHistogram);

  // Description:
  // slots to start cone selection
  void onEditConeSelection()
    { this->onStartConeSelection(true); }
  void onStartConeSelection(bool showDialog=false);

protected:
  using pqCMBCommonMainWindow::updateEnableState;

  // Description
  void setupMenuActions();
  pqCMBMeshViewerMainWindowCore* getThisCore();

  // Initializes the application.
  virtual void initializeApplication();

  void addInputItem(QTreeWidgetItem* parent,
    pqDataRepresentation* extractRep, bool select=false);
  pqPipelineSource* getInputSourceFromItem(
    QTreeWidgetItem * item);
  pqDataRepresentation* getInputRepresentationFromItem(
    QTreeWidgetItem * item);
  void removeSubsets(QTreeWidgetItem* parent);
  void clearAllInputsList();
  QTreeWidgetItem* findItemFromSource(
    QTreeWidgetItem* parentItem, pqPipelineSource* source);
  QTreeWidget* inputTreeWidget();
  void removeInputItem(
    QTreeWidgetItem* selItem, bool checkRemovable=true);
  bool isItemInActiveChain(QTreeWidgetItem* item);
  bool hasChildItem(
    QTreeWidgetItem* parent, QTreeWidgetItem* child);

  void hideBoxWidget();
  void hidePlaneWidget();
  int getShapeSelectionOption();

private:
  pqCMBMeshViewerMainWindow(const pqCMBMeshViewerMainWindow&); // Not implemented.
  void operator=(const pqCMBMeshViewerMainWindow&); // Not implemented.

  class vtkInternal;
  vtkInternal* Internal;
};

#endif




//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqCMBModelBuilderMainWindow
// .SECTION Description
// The main window for the CMB application.

#ifndef __pqCMBModelBuilderMainWindow_h
#define __pqCMBModelBuilderMainWindow_h

#include "pqCMBCommonMainWindow.h"
#include "qtCMBPanelsManager.h" // for qtCMBPanelsManager::PanelType
#include "smtk/extension/qt/qtMeshSelectionItem.h" // for qtMeshSelectionItem::MeshListUpdateType
#include "smtk/PublicPointerDefs.h" // for smtk item pointers
#include <QVariant>
#include <vtkType.h>
#include "cmbSystemConfig.h"

class pqPipelineSource;
class pqCMBModelBuilderMainWindowCore;
class pqCMBSceneTree;
class pqCMBProxyInformationWidget;
class QDockWidget;
class pqDataRepresentation;
class pqSearchBox;

class pqCMBModelBuilderMainWindow : public pqCMBCommonMainWindow
{
  typedef pqCMBCommonMainWindow Superclass;
  Q_OBJECT

public:
  pqCMBModelBuilderMainWindow();
  ~pqCMBModelBuilderMainWindow() override;

  // TODO: When CMB have multiple views, call pqView->widget()->
  // installEventViewer to each of them to override PV mouse bindings
  bool eventFilter(QObject *watched, QEvent *event) override;

public slots:
  // Description:
  // CMB model related slots.
  void onCMBModelCleared();
  void onCMBModelModified();
  void onNewModelCreated();
  void onModelRepresentationAdded(pqDataRepresentation*);
  void onNewMeshCreated();
  bool onCloseSession();
  bool autoCloseSession(const smtk::model::SessionRef&);

  // Description:
  // Overwrite super class to set immediate mode rendering to true,
  // because the new model mapper is controlling display list separately.
  void onViewChanged() override;

  // Description:
  // Texture related slots
  void addTextureFileName(const char *filename);
  const QStringList &getTextureFileNames();

  void toggleSessionCentricMenus(bool sessionCentric);

protected slots:

  virtual void onAskedToExit();

  void onShowCenterAxisChanged(bool enabled);
  void onActiveRepresentationChanged(pqDataRepresentation*);
  void filterDisplayPanel();

  // Description:
  // Slots for smtk related signals.
  void addNewSessions(const QStringList&);
  void addNewSession(const QString&);
  void onCreateNewSession();
  void onCreateNewModel(const QString& sessionType);
  void onRequestMeshSelection();
  void onRequestMeshCellSelection(
    const smtk::attribute::MeshSelectionItemPtr& meshSelectItem);
  void onRequestMeshEdgePointSelection(
    const smtk::attribute::MeshSelectionItemPtr& meshSelectItem);
  void onMeshSelectionItemCreated(
  smtk::extension::qtMeshSelectionItem* meshItem,
    const std::string& opName, const smtk::common::UUID& uuid);

  // Description:
  // Updates the enable state of various menus.
  void updateEnableState();

  // Description:
  // open About and Help dialog
  void onHelpAbout() override;
  void onHelpHelp() override;

  // Description:
  // Tree widgets interaction slots
  void onSelectionFinished();

  // Description:
  // Slots for Tree widgets related buttons
  void updateSelectionUI(bool disable);

 // Description:
  // Signals when selection actions are invoked.
  void onSurfaceRubberBandSelect(bool);

  void onLoadScene();
  void onUnloadScene();
  void onSceneFileLoaded();

  // Description
  // Sim Builder
  void initSimBuilder();
  void loadSimulation();
  void loadSimulationTemplate();
  void loadSimulationScenario();
  void saveSimulationScenario();
  void onSimFileLoaded(const char* filename);

protected:
  using pqCMBCommonMainWindow::updateEnableState;
  // Description
  // Initializes the application.
  void initializeApplication();
  void initializePlugins();
  void setupMenuActions();
  void setupToolbars();
  void updateSMTKSelection();

  // Description:
  // Called whenever a selection is made. converts the selection to a value
  // based selection so that we select a face rather than a cell.
  void updateCMBSelection();

  // Description
  // returns the instance of pqCMBSceneTree.
  pqCMBSceneTree* getpqCMBSceneTree();

  // Description:
  // Called whenever a cell grow is made. converts the selection to a value
  // based selection so that we can control selections of individual cells.
  int getNumberOfSelectedCells(pqOutputPort* selPort);
  bool multipleCellsSelected();
  void meshSelectionFinished();
  void clearSelectedPorts();

  // Description
  // Some convenient methods
  void clearGUI() override;
  void UpdateInfoTable();
  void updateDataInfo();
  void setToolbarEnableState(QToolBar* toolbar, bool enabled);

  // clear existing UI panels and reset the types to default
  void resetUIPanels();
  QDockWidget* initUIPanel(qtCMBPanelsManager::PanelType enType,
    bool recreate=false);
  pqCMBProxyInformationWidget* getInfoWidget();

  // override base class, because ModelBuilder is providing its own color editor
  QDockWidget* initPVColorEditorDock() override {return NULL;}
  // ModelBuilder is not using the InspectorPanel provided from base class
  void initInspectorDock() override;
  pqSearchBox* createSearchBox();

private:
  pqCMBModelBuilderMainWindow(const pqCMBModelBuilderMainWindow&); // Not implemented.
  void operator=(const pqCMBModelBuilderMainWindow&); // Not implemented.

  pqCMBModelBuilderMainWindowCore* getThisCore();

  class vtkInternal;
  vtkInternal* Internal;

};

#endif

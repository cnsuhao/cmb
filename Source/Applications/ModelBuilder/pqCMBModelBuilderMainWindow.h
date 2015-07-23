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
class pqProxyInformationWidget;
class QDockWidget;
class pqDataRepresentation;

class pqCMBModelBuilderMainWindow : public pqCMBCommonMainWindow
{
  typedef pqCMBCommonMainWindow Superclass;
  Q_OBJECT

public:
  pqCMBModelBuilderMainWindow();
  virtual ~pqCMBModelBuilderMainWindow();

public slots:
  // Description:
  // CMB model related slots.
  void onCMBModelCleared();
  void onCMBModelModified();
  void onNewModelCreated();

  // Description:
  // Overwrite super class to set immediate mode rendering to true,
  // because the new model mapper is controlling display list separately.
  virtual void onViewChanged();

  // Description:
  // Texture related slots
  void setTextureMap(const QString& filename,
    int numberOfRegistrationPoints, double *points);
  void addTextureFileName(const char *filename);
  const QStringList &getTextureFileNames();

protected slots:

  void onShowCenterAxisChanged(bool enabled);
  void onActiveRepresentationChanged(pqDataRepresentation*);

  // Description:
  // Slots for smtk related signals.  
  void addNewSessions(const QStringList&);
  void addNewSession(const QString&);
  void onCreateNewSession();
  void onRequestMeshSelection();
  void onRequestMeshCellSelection(
    const smtk::attribute::MeshSelectionItemPtr& meshSelectItem);
  void onRequestMeshEdgePointSelection(
    const smtk::attribute::MeshSelectionItemPtr& meshSelectItem);
  void onMeshSelectionItemCreated(
  smtk::attribute::qtMeshSelectionItem* meshItem,
    const std::string& opName, const smtk::common::UUID& uuid);

  // Description:
  // Updates the enable state of various menus.
  void updateEnableState();

  // Description:
  // open About and Help dialog
  void onHelpAbout();
  void onHelpHelp();

  // Description:
  // Tree widgets interaction slots
  void onSelectionFinished();

  // Description:
  // Slots for Tree widgets related buttons
  void updateSelectionUI(bool disable);

 // Description:
  // Signals when selection actions are invoked.
  void onSurfaceRubberBandSelect(bool);

  void onZoomModeChanged(int mode);
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
  // Description
  // Initializes the application.
  void initializeApplication();
  void setupMenuActions();
  void setupToolbars();
  void updateSMTKSelection();

  // Overwrite base class to use CMB's selection helper
  virtual void setupZoomToBox();

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
  void clearGUI();
  void UpdateInfoTable();
  void updateDataInfo();
  void setToolbarEnableState(QToolBar* toolbar, bool enabled);

  // clear existing UI panels and reset the types to default
  void resetUIPanels();
  QDockWidget* initUIPanel(qtCMBPanelsManager::PanelType enType,
    bool recreate=false);
  pqProxyInformationWidget* getInfoWidget();

private:
  pqCMBModelBuilderMainWindow(const pqCMBModelBuilderMainWindow&); // Not implemented.
  void operator=(const pqCMBModelBuilderMainWindow&); // Not implemented.

  pqCMBModelBuilderMainWindowCore* getThisCore();

  class vtkInternal;
  vtkInternal* Internal;

};

#endif

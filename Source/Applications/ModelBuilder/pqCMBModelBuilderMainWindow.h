/*=========================================================================

  Program:   CMB
  Module:    pqCMBModelBuilderMainWindow.h

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
// .NAME pqCMBModelBuilderMainWindow
// .SECTION Description
// The main window for the CMB application.

#ifndef __pqCMBModelBuilderMainWindow_h
#define __pqCMBModelBuilderMainWindow_h

#include "pqCMBCommonMainWindow.h"
#include <QCheckBox>
#include <QModelIndex>
#include <QTreeWidgetItem>
#include <QVariant>
#include <vtkType.h>
#include "qtCMBPanelsManager.h"
#include "cmbSystemConfig.h"

class vtkDataSet;
class vtkIntArray;
class pqPipelineSource;
class qtCMBTree;
class pqCMBModelBuilderMainWindowCore;
class pqCMBSceneTree;
class pqCMBModel;
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
  void onCreateSimpleModel();

  // Description:
  // Overwrite super class to set immediate mode rendering to true,
  // because the new model mapper is controlling display list separately.
  virtual void onViewChanged();

  // Description:
  // Texture related slots
  void editTexture();
  void setTextureMap(const QString& filename,
    int numberOfRegistrationPoints, double *points);
  void unsetTextureMap();
  void addTextureFileName(const char *filename);
  const QStringList &getTextureFileNames();

protected slots:

  void onShowCenterAxisChanged(bool enabled);
  void onActiveRepresentationChanged(pqDataRepresentation*);
  void addNewBridges(const QStringList&);
  void addNewBridge(const QString&);
  void onCreateNewBridge();

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
  //QTreeWidgetItem* onAddingMaterial();
  //void onRemovingMaterial();
  //QTreeWidgetItem* onAddingBC();
  //void onRemovingBC();
  void onClearSelection();
  void updateSelectionUI(bool disable);

  // Description:
  // Called when the Accept/Reset buttons are clicked
  void onSaveState();
  void onResetState();

  // Description:
  // Called when starting and external process (to disable starting another)
  // and when completing an external process (to re-enable)
  void onEnableExternalProcesses(bool state);

  // Description:
  // Set the color to the selections in the tree
  void setSolidColorOnSelections(const QColor&);

 // Description:
  // Signals when selection actions are invoked.
  void onSurfaceRubberBandSelect(bool);
  void onGrowFromCell(bool);
  void onGrowAndMerge(bool);
  void onGrowAndRemove(bool);

  // Description:
  // Signal when convert lat-long atcion is invoked.
  void onConvertLatLong(bool);

  // Description:
  // Signals when convert arc-node action is invoked for 2D model
  void onConvertArcNodes(bool);

  // Description:
  // Grow related slots
  void onClearGrowResult();
  void onAcceptGrowFacets();

  // Description:
  // Option to show/hide the shared model entities
  void onHideSharedEntities(bool);
  void onHideModelFaces(bool);
  void onHideOuterModelBoundary(bool);

  void onZoomModeChanged(int mode);
  void onLoadScene();
  void onUnloadScene();
  void onSceneFileLoaded();
  void onDisplayBathymetryDialog();

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
  void updateUIByDimension();
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
  void updateCellGrowSelection();
  void mergeCellGrowSelection();
  void removeCellGrowSelection();
  int getNumberOfSelectedCells(pqOutputPort* selPort);
  bool multipleCellsSelected();
  void growFinished();
  void clearSelectedPorts();

  // Description
  // Some convenient methods
  void clearGUI();
  void updateGrowGUI(bool);
  void setGrowButtonsState(bool);
  void UpdateInfoTable();
  void updateDataInfo();
  void UpdateModelState(int accepted);
  void updateSourceMaterials(
    QTreeWidgetItem* matItem, QList<QTreeWidgetItem*> shellItems);
  void selectModelEntitiesByMode(QList<vtkIdType>& faces, int clearSelFirst);
  void removeFacesFromBCGroups(QList<vtkIdType>&);
  void updateCurrentModelFaces(int origFaceId, int numFaces,
    vtkDataSet* faceData, QList<QVariant> origCellIds);
  void setToolbarEnableState(QToolBar* toolbar, bool enabled);

  // init UI panels as Dock widgets
  void initUIPanels();
  QDockWidget* initUIPanel(qtCMBPanelsManager::PanelType enType,
    bool recreate=false);
  pqProxyInformationWidget* getInfoWidget();

  void SetCheckBoxStateQuiet(QCheckBox* box, bool state);
private:
  pqCMBModelBuilderMainWindow(const pqCMBModelBuilderMainWindow&); // Not implemented.
  void operator=(const pqCMBModelBuilderMainWindow&); // Not implemented.

  pqCMBModelBuilderMainWindowCore* getThisCore();

  class vtkInternal;
  vtkInternal* Internal;

};

#endif

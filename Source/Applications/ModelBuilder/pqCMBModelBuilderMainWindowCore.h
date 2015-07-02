//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqCMBModelBuilderMainWindowCore
// .SECTION Description
// The core application for the CMB main window

#ifndef _pqCMBModelBuilderMainWindowCore_h
#define _pqCMBModelBuilderMainWindowCore_h

#include "pqCMBCommonMainWindowCore.h"
#include "cmbSystemConfig.h"
#include "smtk/PublicPointerDefs.h"

class SimBuilderCore;
class pqOutputPort;
class pqCMBRubberBandHelper;
class pqScalarBarWidget;
class pqCMBSceneTree;
class pqCMBModelManager;
class pqSMTKModelPanel;
class pqSMTKMeshPanel;

class pqCMBModelBuilderMainWindowCore :  public pqCMBCommonMainWindowCore
{
  typedef pqCMBCommonMainWindowCore Superclass;
  Q_OBJECT

public:
  pqCMBModelBuilderMainWindowCore(QWidget* parent);
  virtual ~pqCMBModelBuilderMainWindowCore();

  // Description
  // Setup a representation-selection toolbar
  void setupSelectionRepresentationToolbar(QToolBar* parent);

  // Description
  // Setup a representation-selection toolbar
  void setupColorByAttributeToolbar(QToolBar* parent);

  // Description:
  // Get flag to determine how selection is performed in display
  // 0 = Model Faces, 1 = Shells, 2 = Materials
  int getRubberSelectionMode();

  /// Returns the selection and pick helper used for 3D views.
  pqCMBRubberBandHelper* cmbRenderViewSelectionHelper() const;

  // Description
  // returns the active SimBuilderCore.
  SimBuilderCore* getSimBuilder();

  // Description
  // set pqCMBSceneTree for the core.
  void setpqCMBSceneTree(pqCMBSceneTree* );

  // Description
  // Check whether pqCMBSceneTree is empty.
  bool isSceneEmpty();

  // Description:
  // Get the internal smtk dockable model panel;
  pqSMTKModelPanel* modelPanel();

  // Description:
  // Get the internal smtk dockable mesh panel;
  pqSMTKMeshPanel* meshPanel();

  // Descirption:
  // override the base class to give an editor by default
  virtual pqCMBDisplayProxyEditor *getAppearanceEditor();

  // Description:
  // The application's model manager
  pqCMBModelManager* modelManager();

signals:

  void currentDataSourceChanged();
  void cleanupOutputFiles();
  void rubberSelectionModeChanged();
  void newSceneLoaded();
  void newModelCreated();

public slots:
  // Description
  /// Called when a new server is connected.
  virtual void onRemovingServer(pqServer*);
  virtual void onServerCreationFinished(pqServer *server);
  virtual void onColorByModeChanged(const QString &);
  virtual void onColorByAttribute();
  virtual void colorByAttributeFieldSelected(const QString& attdeftype,
        const QString& itemname);

  virtual void onFileOpen(const QStringList& Files);
  void onCloseData(bool modelOnly=false);
  void clearSimBuilder();
  int onLoadSimulation(bool templateonly = false, bool isScenario = false);
  int onLoadScenario();
  void onSaveScenario();

  void onSaveSimulation();
  void onExportSimFile();
  int onLoadSimulationTemplate()
    {return this->onLoadSimulation(true);}
  void clearpqCMBSceneTree();
  void onLoadScene();

  // Resets the center of rotation to the center of the active
  // source in the active view.
  virtual void resetCenterOfRotationToCenterOfCurrentData();

  // Description:
  // Called when a CMB model is loaded/cleared
  void onModelLoaded();
  void onCMBModelCleared();

  // Description:
  // Called when a new VTK connection is setup for the rep.
  void onVTKConnectionChanged(pqDataRepresentation*);

  // Description:
  // Selection related slots
  void onRubberBandSelect(bool);
  void onRubberBandSelectPoints(bool);
  void onRubberBandSelectCells(bool);
  void zoomOnSelection();
  void updateSMTKSelection();

  // Description:
  // Determines how selection is performed in display
  // 0 = Model Faces, 1 = Shells, 2 = Materials
  void setRubberSelectionMode(int mode);

  // Description:
  // Called when a new reader is created by the GUI.
  // We add the reader to the recent files menu.
  void onReaderCreated(pqPipelineSource* reader, const QString& filename);

  void updateScalarBarWidget(
    pqScalarBarWidget* scalarBar,
    const QString& strDefType, bool show);

  int loadModelFile(const QString& filename);

  void processSceneInfo(const QString& filename,
    pqPipelineSource* source);
  void processMapInfo(const QString& filename,
    pqPipelineSource* source);

  void loadJSONFile(const QString& filename);
  bool processModelInfo(const smtk::model::OperatorResult& result,
                        const smtk::model::SessionRef& sref,
                        bool hasNewModels, bool bGeometryChanged);
  void selectRepresentationBlock( pqDataRepresentation*, unsigned int );

  // Called to show the settings dialog.
  // Subclass should override this method to add app-specific options.
  // This superclass method will add app-common options to the dialog
  // See also, applyAppSettings
  virtual void onEditSettings();

  // Description:
  // Subclass should override this method to load app-specific
  // settings, and this Superclass method will load common settings.
  // See Also, onEditSettings()
  virtual void applyAppSettings();

protected:
  virtual void buildRenderWindowContextMenuBehavior(QObject* parent_widget);
  virtual void setSimBuilderModelManager();

private:
  // Description:
  // On successful completion of volume mesher, preview the mesh
  int previewWindow(QString path);

  FileBasedMeshingParameters generateLegacyVolumeMesherInput();

  class vtkInternal;
  vtkInternal* const Internal;
};

#endif // !_pqCMBModelBuilderMainWindowCore_h

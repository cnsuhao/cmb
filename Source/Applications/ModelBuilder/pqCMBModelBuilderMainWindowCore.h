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
class pqSMTKInfoPanel;
class pqSMTKModelPanel;
class pqSMTKMeshPanel;
class vtkSMModelManagerProxy;

class pqCMBModelBuilderMainWindowCore :  public pqCMBCommonMainWindowCore
{
  typedef pqCMBCommonMainWindowCore Superclass;
  Q_OBJECT

public:
  pqCMBModelBuilderMainWindowCore(QWidget* parent);
  ~pqCMBModelBuilderMainWindowCore() override;

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

  // Description:
  // Get the internal smtk dockable info panel;
  pqSMTKInfoPanel* infoPanel();

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
  void newMeshCreated();

  void sessionCentricModelingPreferenceChanged(bool);

public slots:
  // Description
  /// Called when a new server is connected.
  void onRemovingServer(pqServer*) override;
  void onServerCreationFinished(pqServer *server) override;
  virtual void onColorByModeChanged(const QString &);
  virtual void onColorByAttribute();
  virtual void colorByAttributeFieldSelected(const QString& attdeftype,
        const QString& itemname);

  void onFileOpen(const QStringList& Files) override;
  void onCloseData() override
    { this->onCloseData(false); }
  void onCloseData(bool modelOnly);
  bool onCloseSession(const smtk::model::SessionRef&);
  void clearSimBuilder();
  void resetSimulationModel();
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
  void resetCenterOfRotationToCenterOfCurrentData() override;

  // Description:
  // Called when a CMB model is loaded/cleared
  void onModelLoaded();
  void onCMBModelCleared();

  // Description:
  // Called when a new VTK connection is setup for the rep.
  void onVTKConnectionChanged(pqDataRepresentation*);

  // Description:
  // Called when the pqCMBModelManager initializes/re-initializes with a new model manager.
  void modelManagerChanged();

  // Description:
  // Selection related slots
  void onRubberBandSelect(bool);
  void onRubberBandSelectPoints(bool);
  void onRubberBandSelectCells(bool);
  void zoomOnSelection() override;
  void updateSMTKSelection();

  // Description:
  // Determines how selection is performed in display
  // 0 = Model Faces, 1 = Shells, 2 = Materials
  void setRubberSelectionMode(int mode);

  // Description:
  // Called when a new reader is created by the GUI.
  // We add the reader to the recent files menu.
  void onReaderCreated(pqPipelineSource* reader, const QString& filename) override;

  void updateScalarBarWidget(
    pqScalarBarWidget* scalarBar,
    const QString& strDefType, bool show);

  int loadModelFile(const QString& filename);

  void processSceneInfo(const QString& filename,
    pqPipelineSource* source);
  void processMapInfo(const QString& filename,
    pqPipelineSource* source);

  void loadJSONFile(const QString& filename);
  bool processOperatorResult(const smtk::model::OperatorResult& result,
                        const smtk::model::SessionRef& sref,
                        bool hasNewModels, bool bModelGeometryChanged,
                        bool hasNewMeshes);
  void selectRepresentationBlock( pqDataRepresentation*, unsigned int, bool ctrlKey );

  // Called to show the settings dialog.
  // Subclass should override this method to add app-specific options.
  // This superclass method will add app-common options to the dialog
  // See also, applyAppSettings
  void onEditSettings() override;

  // Description:
  // Subclass should override this method to load app-specific
  // settings, and this Superclass method will load common settings.
  // See Also, onEditSettings()
  void applyAppSettings() override;

  // Description:
  // Call pqCMBModelManager::startNewSession with the app setting's entry
  // whether to create a default model.
  bool startNewSession(const std::string& sessionName);
  bool startNewSession(
    const std::string& sessionName,
    bool createDefaultModel,
    bool useExistingSession);

  // Description:
  // Override the zoomToSelection function to do real task
  bool  zoomToSelection() override;

  /// Call setCameraManipulationMode() to match \a dimension, but only if the user preference is enabled.
  void autoSwitchCameraManipulationMode(int dimension);

protected:
  void buildRenderWindowContextMenuBehavior(QObject* parent_widget) override;
  virtual void setSimBuilderModelManager();
  virtual void processModifiedMeshes(
    const smtk::attribute::MeshItemPtr& modifiedMeshes);
  virtual void processModifiedEntities(
  const smtk::attribute::ModelEntityItemPtr& resultEntities);

private:
  // Description:
  // On successful completion of volume mesher, preview the mesh
  int previewWindow(QString path);

  class vtkInternal;
  vtkInternal* const Internal;
};

#endif // !_pqCMBModelBuilderMainWindowCore_h

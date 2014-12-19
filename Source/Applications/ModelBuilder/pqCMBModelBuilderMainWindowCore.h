/*=========================================================================

  Program:   CMB
  Module:    pqCMBModelBuilderMainWindowCore.h

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
// .NAME pqCMBModelBuilderMainWindowCore
// .SECTION Description
// The core application for the CMB main window

#ifndef _pqCMBModelBuilderMainWindowCore_h
#define _pqCMBModelBuilderMainWindowCore_h

#include "pqCMBCommonMainWindowCore.h"
#include "cmbSystemConfig.h"

class pqOutputPort;
class pqCMBRubberBandHelper;
class pqServerResource;

class vtkObject;
class vtkPVModelFaceObjectInformation;
class vtkPVMultiBlockRootObjectInfo;
class vtkSMProxyLink;

class qtCMBCreateSimpleGeometry;
class SimBuilderCore;
class pqCMBSceneTree;
class vtkCollection;
class cmbScalarBarWidget;
class ModelManager;
class qtSMTKModelPanel;
#include "smtk/PublicPointerDefs.h"

class pqCMBModelBuilderMainWindowCore :  public pqCMBCommonMainWindowCore
{
  typedef pqCMBCommonMainWindowCore Superclass;
  Q_OBJECT

public:
  pqCMBModelBuilderMainWindowCore(QWidget* parent);
  virtual ~pqCMBModelBuilderMainWindowCore();

  // Description
  // Setup a variable-selection toolbar
  void setupVariableToolbar(QToolBar* parent);
  void updateVariableToolbar(QToolBar* parent);

  // Description
  // Setup a representation-selection toolbar
  void setupSelectionRepresentationToolbar(QToolBar* parent);

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

  // Description
  // Check whether an analysis mesh was created.
  bool checkAnalysisMesh();

  // Description:
  // Create simple models from a specified geometric shape and
  // resolution.
  void createRectangleModel(double* boundingBox, int baseResolution,
                            int heightResolution);
  void createEllipseModel(double* values, int resolution);

  // Description:
  // Get the internal smtk dockable model panel;
  qtSMTKModelPanel* modelPanel();

  // Descirption:
  // override the base class to give an editor by default
  virtual pqCMBDisplayProxyEditor *getAppearanceEditor();

  // Description:
  // The application's model manager
  ModelManager* modelManager();

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

  virtual void onFileOpen(const QStringList& Files);
  void onCloseData(bool modelOnly=false);
  void clearSimBuilder();
  void onSaveBCSs();
  int onLoadSimulation(bool templateonly = false, bool isScenario = false);
  int onLoadScenario();
  void onSaveScenario();
  void onSaveMeshToModelInfo();
  void onLoadMeshToModelInfo();

  void onSaveSimulation();
  void onExportSimFile();
  int onLoadSimulationTemplate()
    {return this->onLoadSimulation(true);}
  void onSaveData();
  void onSaveAsData();
  void clearpqCMBSceneTree();
  void onLoadScene();
  void setFaceMeshRepresentationType(const char* strType,
    vtkCollection* selFaces);
  void setMeshRepresentationColor(const QColor&,
    vtkCollection* selFaces, vtkCollection* selEdges);

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
  // Called when user click "Generate Omicron Input"
  void onGenerateOmicronInput();

  // Description:
  // Called when user click "Spawn Surface Mesher"
  void onSpawnSurfaceMesher();

  // Description:
  // Called when user click "Spawn Volume Mesher"
  void onSpawnVolumeMesher();

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
  // Determines how the edge/face is colored in display
  //enum EntityColorByMode
  //{
  //  ColorByNone      = 0,
  //  ColorByEntity    = 1,
  //  ColorByDomain    = 2,
  //  ColorByMaterial  = 3,
  //  ColorByBCS       = 4
  //};
  void setColorByFaceMode(int mode);
  void setColorByEdgeMode(int mode);

  // Description:
  // Determines how 2d model face is colored in display
  //  = Domain Set, 1 = Model Face
  void setColorByEdgeDomainMode(int mode);

  // Description:
  // Called when a new reader is created by the GUI.
  // We add the reader to the recent files menu.
  void onReaderCreated(pqPipelineSource* reader, const QString& filename);

  // Description:
  // On successful completion of a remus job, load the results
  int loadRemusOutput(remus::proto::JobResult result);

  // Description:
  // On successful completion of surface mesher, inquire about loading output
  void loadMesherOutput(remus::proto::JobResult);

  // Description:
  // Accept mesh result (if accep preview)
  void acceptMeshResult();
  virtual void onPreviewAccepted() {this->acceptMeshResult();}

  // Description:
  // Cleanup files created by the volume mesher (if reject preview)
  void cleanupVolumeMeshFiles();
  virtual void onPreviewRejected() {this->cleanupVolumeMeshFiles();}

  // Description:
  // Cleanup files created by the surface mesher (if reject preview)
  void cleanupSurfaceMeshFiles();

  // Description:
  // Color the model based on assigned attributes.
  void onColorByAttribute();
  void onColorEdgeByAttribute();
  void onEdgeAttCategoryChanged();
  void onColorFaceByAttribute();
  void onFaceAttCategoryChanged();

  void onNumOfAttriubtesChanged();
  void toggleAttFaceColorLegend(bool);
  void toggleAttEdgeColorLegend(bool);
  // color face(3d) or edge (2d) by attributes
  void updateColorByAttributeMode(int mode, bool isEdge=false);
  // color face(2d) by domain attributes;
  void updateColorDomainByAttributeMode(int mode);
  void onEdgeDomainColorByAttribute();
  void onEdgeDomainAttCategoryChanged();
  bool canColorByAttribute();
  void updateScalarBarWidget(
    cmbScalarBarWidget* scalarBar,
    const QString& strDefType, bool show);

  void hideDisplayPanelPartialComponents();

  //Input is the mesh file path, and the output is the bcs file name
  //e.g "foo_mesh.bcs"
  QString saveBCSFileForOmicronInput(QString meshInputFilePath);
  bool loadOmicronModelInputData();
  int loadModelFile(const QString& filename);
  int loadBCFile(const QString& filename);

  void processSceneInfo(const QString& filename,
    pqPipelineSource* source);
  void processMapInfo(const QString& filename,
    pqPipelineSource* source);
  void clearCurrentEntityWidgets();

  void loadJSONFile(const QString& filename);
  bool processModelInfo(const smtk::model::OperatorResult& result,
                             bool hasNewModels);
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
  void applyColorSettings();

protected:
  virtual void buildRenderWindowContextMenuBehavior(QObject* parent_widget);

private:
  // Description:
  // On successful completion of volume mesher, preview the mesh
  int previewWindow(QString path);

  FileBasedMeshingParameters generateLegacyVolumeMesherInput();

  class vtkInternal;
  vtkInternal* const Internal;
};

#endif // !_pqCMBModelBuilderMainWindowCore_h

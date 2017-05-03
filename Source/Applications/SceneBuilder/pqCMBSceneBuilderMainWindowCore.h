//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef _pqCMBSceneBuilderMainWindowCore_h
#define _pqCMBSceneBuilderMainWindowCore_h

#include "cmbSystemConfig.h"
#include "pqCMBCommonMainWindowCore.h"
#include "pqCMBSceneObjectBase.h"

class vtkDiscreteModelWrapper;
class vtkPolyData;
class pqCMBSceneTree;
class pqCMBSceneNode;
class pqCMBSceneObjectBase;
class vtkTransform;
class vtkBoundingBox;
class QWidget;
class pqCMBGlyphObject;
class vtkHydroModelPolySource;

class pqCMBSceneBuilderMainWindowCore : public pqCMBCommonMainWindowCore
{
  typedef pqCMBCommonMainWindowCore Superclass;
  Q_OBJECT

public:
  pqCMBSceneBuilderMainWindowCore(QWidget* parent);
  ~pqCMBSceneBuilderMainWindowCore() override;

  void setpqCMBSceneTree(pqCMBSceneTree* tree);

  int getNumberOfSelectedLeafNode();

  QString getOutputFileName() const;

signals:
  void newSceneLoaded();
  void sceneSaved();

public slots:
  /// Called when a new server is connected.
  void onServerCreationFinished(pqServer* server) override;

  /// Called when a new reader is created by the GUI.
  /// We add the reader to the recent files menu.
  void onReaderCreated(pqPipelineSource* reader, const QString& filename) override;

  // Resets the center of rotation to the center of the active
  // source in the active view.
  void resetCenterOfRotationToCenterOfCurrentData() override;

  // Description:
  // Closes the currently opened solid.
  void onCloseData() override;
  void closeData() override;
  void onPackageScene();

  // Description:
  // Get filename for and create omicron input (file)
  void onCreateOmicronInput();
  void onSpawnModelMesher();

  void onSpawnSurfaceMesher();

  void modifierInputSelectionType();
  void onSpawnArcModifier();

  void onExport2DBCSFile();
  void exportSelectedSolids();
  void exportSelectedPolygons();
  void exportCMBFile();
  void rejectMesh();
  void addOmicronModelFaceData(vtkPolyData* outputPD);
  bool clientTransformSource(
    pqPipelineSource* serverSource, vtkHydroModelPolySource* clientSource, vtkTransform* transform);

  // Description:
  // Saves the data (geometry + region and material IDs) in a file.
  void onSaveData() override;
  void onSaveAsData() override;

  void onRubberBandSelect(bool);
  void onPreviewAccepted() override;
  void onPreviewRejected() override;
  void selectGlyph(QList<pqCMBGlyphObject*>& glyphList);

  // Description:
  // Update the selected nodes.
  void updateSelection(
    const QList<pqCMBSceneNode*>* unselected, const QList<pqCMBSceneNode*>* newlySelected);
  void selectNode(pqCMBSceneNode* node, bool updateBox = true);
  void unselectNode(pqCMBSceneNode* node, bool updateBox = true);

  // Description:
  // Determines how selection is visualized
  // 0 = outline, 1 = points, 2 = wireframe, 3 = surface
  void setSelectionMode(int mode);

  void updateBoxWidget();
  void updateBoxWidgetForGlyph();
  void clearSelectedGlyphList();

  // Deal with the color being changed by ParaView Dialog
  void updateNodeColor();

  // Zoom onto the selected object
  void zoomOnSelection() override;

  // stitch selected TINs
  void stitchTINs();

  void onSelectGlyph(bool checked);

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

protected:
  void buildRenderWindowContextMenuBehavior(QObject* parent_widget) override;

private slots:

  // Description:
  // Load a solid file.
  void processSceneInfo(const QString& filename, pqPipelineSource* source);
  void processOSDLInfo(const QString& filename, pqPipelineSource* source);
  void processMapInfo(const QString& filename, pqPipelineSource* source);
  void processScene(const QString& filename, pqPipelineSource* source);
  //void closeData();
  void newScene();

  void createSurfaceMesh();
  int setupSurfaceMesherDialog();

  void arcDeformData(pqCMBSceneObjectBase::enumObjectType dt);
  void applyArcToSurface();
  int setupModifierDialog(pqCMBSceneObjectBase::enumObjectType dt);

  void createOmicronInput(QString* filename, double volumeConstraint);
  bool exportSolidsCMBModel(QList<pqOutputPort*>& inputs, const QString& cmbFileName, bool is2D);

  // Description:
  // Save the model.
  void saveData(const QString& filename) override;

  void boxWidgetInteraction();

  void checkForChooseMesher(int mesherIndex);
  int selectSurfaceMesher();
  int setupModelMesherDialog(); // returns 0 if not setup
  void previewMesherOutput();

  // Preps a selected pqCMBSceneObjectBase
  void prepSelectedObject(pqCMBSceneObjectBase* obj);

  void RemoveBoxWidgetPropertyLinks();

  void updateBoxInteraction();
  void updateBoxInteractionForGlyph(double sdelta[3], double odelta[3], double tdelta[3]);

private:
  void AddLineSegmentObjectsToModel(vtkDiscreteModelWrapper* wrapper, bool voiPresent);
  bool AddContourObjectsToModel(vtkDiscreteModelWrapper* wrapper);
  void updateWidgetPanel(pqCMBSceneObjectBase*);
  void clearColorNode();
  double computeVolumeContraint(double volumeFactor);
  double getMinVOIBounds();
  void setupBoxWidget();
  void CreatTransPipelineMesh(const QStringList& selectedNames, vtkBoundingBox& bbox,
    QList<pqPipelineSource*>& transformFilters, pqCMBSceneObjectBase::enumObjectType dt);

  class vtkInternal;
  vtkInternal* const Internal;
  int SelectionMode;
  pqCMBSceneTree* Tree;
  vtkTransform* SelectedTransform;
  // The Leaf nodes that are currently selected
  QList<pqCMBSceneNode*> SelectedLeaves;
  // Node used for color information
  pqCMBSceneNode* ColorNode;
  double BoxTranslation[3], BoxScale[3], BoxOrientation[3];
  bool PreviewMeshOutput;
  QPointer<pqPipelineSource> TINStitcher;
};

#endif // !_pqCMBSceneBuilderMainWindowCore_h

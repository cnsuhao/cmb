//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef _pqCMBGeologyBuilderMainWindowCore_h
#define _pqCMBGeologyBuilderMainWindowCore_h

#include "pqCMBCommonMainWindowCore.h"
#include "pqCMBSceneObjectBase.h"
#include "cmbSystemConfig.h"

class vtkDiscreteModelWrapper;
class vtkPolyData;
class pqCMBSceneTree;
class pqCMBSceneNode;
class vtkTransform;
class QWidget;
class pqCMBGlyphObject;
class vtkHydroModelPolySource;

class pqCMBGeologyBuilderMainWindowCore :  public pqCMBCommonMainWindowCore
{
  typedef pqCMBCommonMainWindowCore Superclass;
  Q_OBJECT

public:
  pqCMBGeologyBuilderMainWindowCore(QWidget* parent);
  virtual ~pqCMBGeologyBuilderMainWindowCore();

  void setpqCMBSceneTree(pqCMBSceneTree * tree);

  int getNumberOfSelectedLeafNode();

  QString getOutputFileName() const;
  void setupGeoToolbar(QToolBar* geoToolbar);

signals:
  void newSceneLoaded();
  void sceneSaved();

public slots:
  /// Called when a new server is connected.
  virtual void onServerCreationFinished(pqServer *server);

  /// Called when a new reader is created by the GUI.
  /// We add the reader to the recent files menu.
  void onReaderCreated(pqPipelineSource* reader, const QString& filename);

  // Resets the center of rotation to the center of the active
  // source in the active view.
  void resetCenterOfRotationToCenterOfCurrentData();

  // Description:
  // Closes the currently opened solid.
  void onCloseData();
  void closeData();
  void onPackageScene();

  // Description:
  // Get filename for and create omicron input (file)
  void onCreateOmicronInput();
  void onSpawnModelMesher();

  void onSpawnSurfaceMesher();

  void onExport2DBCSFile();
  void exportSelectedSolids();
  void exportSelectedPolygons();
  void exportCMBFile();
  void rejectMesh();
  void addOmicronModelFaceData( vtkPolyData *outputPD );
  bool clientTransformSource(pqPipelineSource* serverSource,
    vtkHydroModelPolySource* clientSource, vtkTransform* transform);

  // Description:
  // Saves the data (geometry + region and material IDs) in a file.
  void onSaveData();
  void onSaveAsData();

  void onRubberBandSelect(bool);
  virtual void onPreviewAccepted();
  virtual void onPreviewRejected();
  void selectGlyph(QList<pqCMBGlyphObject*> &glyphList);

  // Description:
  // Update the selected nodes.
  void updateSelection(const QList<pqCMBSceneNode*> *unselected,
                      const QList<pqCMBSceneNode*> *newlySelected);
  void selectNode(pqCMBSceneNode *node, bool updateBox = true);
  void unselectNode(pqCMBSceneNode *node, bool updateBox = true);

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
  void zoomOnSelection();

  // stitch selected TINs
  void stitchTINs();

  void onSelectGlyph(bool checked);

  // Geology special slots:
  // invoked when the colorby dropdown is changed
  void onBoreholeRadiusChanged(double radius);
  void onBHEditColorMap();
  void onCSEditColorMap();

protected:
  virtual void buildRenderWindowContextMenuBehavior(QObject* parent_widget);

private slots:

  // Description:
  // Load a solid file.
  void processSceneInfo(const QString& filename,
                       pqPipelineSource* source);
  void processOSDLInfo(const QString& filename,
                       pqPipelineSource* source);
  void processMapInfo(const QString& filename,
                       pqPipelineSource* source);
  void processScene(const QString& filename,
                    pqPipelineSource* source);
  void processBoreholeInfo(const QString& filename,
    pqPipelineSource* source);

  //void closeData();
  void newScene();

  void createSurfaceMesh( );
  int setupSurfaceMesherDialog();

  void createOmicronInput(QString *filename, double volumeConstraint);
  bool exportSolidsCMBModel(QList<pqOutputPort*>& inputs,
    const QString& cmbFileName, bool is2D);

  // Description:
  // Save the model.
  void saveData(const QString& filename);

  void boxWidgetInteraction();

  void checkForChooseMesher(int mesherIndex);
  int selectSurfaceMesher();
  int setupModelMesherDialog();  // returns 0 if not setup
  void previewMesherOutput();

  // Preps a selected pqCMBSceneObjectBase
  void prepSelectedObject(pqCMBSceneObjectBase *obj);

  void RemoveBoxWidgetPropertyLinks();

  void updateBoxInteraction();
  void updateBoxInteractionForGlyph(
    double sdelta[3], double odelta[3], double tdelta[3]);
  void colorBoreHoles();
  void colorCrossSections();

private:
  void AddLineSegmentObjectsToModel(vtkDiscreteModelWrapper* wrapper,
    bool voiPresent);
  bool AddContourObjectsToModel(vtkDiscreteModelWrapper* wrapper);
  void updateWidgetPanel(pqCMBSceneObjectBase*);
  void clearColorNode();
  double computeVolumeContraint(double volumeFactor);
  double getMinVOIBounds();
  void setupBoxWidget();
  void CreateProperLiDARForSurfaceMesh( const QStringList &selectedNames,
   QList<pqPipelineSource*> &transformFilters);
  void colorObjects(
    int association, const char* arrayName,
    pqCMBSceneObjectBase::enumObjectType enObjType);

  class vtkInternal;
  vtkInternal* const Internal;
  int SelectionMode;
  pqCMBSceneTree *Tree;
  vtkTransform *SelectedTransform;
  // The Leaf nodes that are currently selected
  QList <pqCMBSceneNode *> SelectedLeaves;
  // Node used for color information
  pqCMBSceneNode *ColorNode;
  double BoxTranslation[3], BoxScale[3], BoxOrientation[3];
  bool PreviewMeshOutput;
  QPointer<pqPipelineSource> TINStitcher;

};


#endif // !_pqCMBGeologyBuilderMainWindowCore_h

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef _pqCMBMeshViewerMainWindowCore_h
#define _pqCMBMeshViewerMainWindowCore_h

#include "pqCMBCommonMainWindowCore.h"
#include "cmbSystemConfig.h"

class vtkDiscreteModelWrapper;
class vtkPolyData;
class vtkTransform;
class QWidget;
class pqObjectBuilder;
class pqProxyWidget;
class QScrollArea;
class pqXYBarChartView;
class pqServerManagerModelItem;
class vtkSMProxy;
class pqSpreadSheetViewModel;
class pqDataRepresentation;
class vtkSMViewProxy;
class pqServer;
class vtkPVContourRepresentationInfo;
class vtkSMNewWidgetRepresentationProxy;
class vtkTransform;
class vtkSMPropertyLink;

class pqCMBMeshViewerMainWindowCore :  public pqCMBCommonMainWindowCore
{
  typedef pqCMBCommonMainWindowCore Superclass;
  Q_OBJECT

public:
  pqCMBMeshViewerMainWindowCore(QWidget* parent);
  ~pqCMBMeshViewerMainWindowCore() override;

  void destroyInputRepresentations();

  QString getOutputFileName() const;
  bool isMeshLoaded();
  bool is2DMesh();
  void exportMesh(pqPipelineSource* meshSource);

  void setupInspectorPanel (QWidget *parent);
  void setupSelectionPanel(QWidget* parent);

  pqPipelineSource* meshSource();
  pqDataRepresentation* activeRepresentation();
  pqDataRepresentation* fullMeshRepresentation();
  pqPipelineSource* currentInputSource();
  pqPipelineSource* activeSource();
  void changeSelectionMaterialId(int newId);
  void changeMeshMaterialId(int newId);
  pqDataRepresentation* extractSelectionAsInput();
  vtkSMProxy* getActiveSelection(int selFieldType=0);
  void setActiveSelection(vtkSMSourceProxy* selSource);
  bool invertCurrentSelection();
  void setFiltersSource(pqPipelineSource* selSource);
  pqDataRepresentation* getRepresentationFromSource(pqPipelineSource*);
  pqSpreadSheetViewModel* spreadSheetViewModel();
  vtkSMViewProxy* spreadSheetView();
  vtkSMProxy* spreadSheetRepresentation();
  vtkSMNewWidgetRepresentationProxy* createBoxWidget();
  vtkSMNewWidgetRepresentationProxy* createPlaneWidget();
  void linkContourPlaneWidget(
    vtkSMNewWidgetRepresentationProxy* planeWidget);
  void linkContourBoxWidget(
    vtkSMNewWidgetRepresentationProxy* boxWidget, bool enable=true);
  void linkBoxWidget(vtkSMPropertyLink* positionLink,
    vtkSMPropertyLink* rotationLink, vtkSMPropertyLink* scaleLink,
    vtkSMNewWidgetRepresentationProxy* boxWidget,
    pqDataRepresentation* dataRep, bool enable);
  void updatePlaneInteraction(
    vtkSMNewWidgetRepresentationProxy* planeWidget);
  pqPipelineSource* meshSculptingSource();
  pqDataRepresentation* meshSculptingRepresentation();
  void createSelectedNodesRepresentation();

  void setSmoothMeshPanelParent(QWidget* parent);
  void applySmoothing();
  bool startConeSelection(bool showDialog);
  void stopConeSelection();
  pqDataRepresentation* coneRepresentation();

  // Destroy source
  void destroySource(
    pqObjectBuilder* builder, pqPipelineSource* source);
  void destroyRepresentation(pqDataRepresentation* selRep);

  // Description:
  // Select surface points with contour
  void contourSelectSurface(pqContourWidget* contourWidget,
    bool isCell, int selectContourType);
  // Select through cells with contour
  void contourSelectThrough(pqContourWidget* contourWidget,
    int selectContourType);
  void setShapeSelectionOption(pqContourWidget* contourWidget,
    int selectCellThrough, int selectShapeType);
  bool hasContourSelection();
  bool hasConeSelection();
  int shapeSelectionOption();
  QString getCurrentMeshFile() const;

signals:
  void newMeshLoaded();
  void meshModified();
  void filterPropertiesChanged(bool acceptable);

public slots:
  /// Called when a new server is connected.
  void onServerCreationFinished(pqServer *server) override;

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

  // Description:
  // Saves the data (geometry + region/material IDs) in a file.
  void onSaveData() override;
  void onSaveAsData() override;

  void onFrustumSelect(bool);
  void onRubberBandSelectCell(bool checked);
  void onRubberBandSelectPoints(bool checked);
  void updateSelection(bool isContourSel = false);
  void clearSelection();
  void selectAll();

  // Description:
  // Determines how selection is visualized
  // 0 = outline, 1 = points, 2 = wireframe, 3 = surface
  void setSelectionMode(int mode);

  // Description:
  // Set the display mode of histogram
  // 0 = by quality, 1 = by region
  void setHistogramMode(int mode);

  // Description:
  // Zoom onto the selected object
  void zoomOnSelection() override;

  // Description:
  // Accept/Reset changes on the filters, and update the representation
  void acceptFilterPanels(bool ignoreList=false);
  void resetFilterPanels();

  // Description:
  // Create a contour widget, and put render view in define contour stage
  pqContourWidget* defineContourWidget();
  void clearContourSelection(pqContourWidget*);

  // Description:
  // Move mesh points according to current contour selection and box widget
  // return true on success; false on failure
  bool moveMeshScultpingPoints();
  bool moveMeshPoints(pqPipelineSource* source,
    vtkSMProxy* transformProxy=NULL);

protected:

  void destroySources();
  void removeFiltertPanel(pqProxyWidget* panel);

  void createFilters(pqPipelineSource* source);
  pqPipelineSource* createFilter(const char* filterxmlname,
    pqPipelineSource* inputsrc);
  void createFilterPanels();
  void setInputArray(
    pqPipelineSource* filter,
    const char* propName, const char* arrayname);
  void initScrollArea(QScrollArea* area);
  QScrollArea* createScrollArea(QWidget* parent);

  void createHistogramViews();
  pqXYBarChartView* createHistogramView(pqPipelineSource* source);
  void removeHistogramView();
  void updateMeshHistogram();
  void resetFilterInputArrays();
  bool getMeshSaveAsFileName(QString& filename);
  void updateMeshContourSelection(
    pqContourWidget* contourWidget, int selectCellThrough,
    vtkSMProxy* selectionSource, int selectContourType,
    int fieldType, int GenerateSelectedOutput=0);
  void createSmoothMeshPanel();

private slots:

  // Description:
  // Load a solid file.
  void processMesh(const QString& filename,
                    pqPipelineSource* source);
  void updateApplyState(bool changesAvailable);
  void updateMeshThreshold();
  void updateMeshQuality();
  void updateQualityThreshold();
  void filterModified();

  // Description:
  // Save the mesh.
  void saveMesh(const QString& filename,
    pqPipelineSource* meshSource = NULL);
  void updateFilters();
  void updateSource(pqPipelineSource* source);

  void onUpdateConeInteraction();

private:

  class vtkInternal;
  vtkInternal* const Internal;

  // The Leaf nodes that are currently selected
  bool PreviewMeshOutput;
  void getContourDisplayBounds(
    vtkPVContourRepresentationInfo* contourInfo, double bounds[6]);

};


#endif // !_pqCMBMeshViewerMainWindowCore_h

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef _pqCMBPointsBuilderMainWindowCore_h
#define _pqCMBPointsBuilderMainWindowCore_h

#include "pqCMBCommonMainWindowCore.h"
#include "pqVariableType.h"
#include "cmbSystemConfig.h"

#include <vtkIOStream.h>

#include <QObject>
#include <QWidget>
#include <QPointer>
#include <QList>
#include <string>
#include <list>

#include "pqCMBModifierArc.h"

class QWidget;

class pqDataRepresentation;
class pqPipelineSource;
class pqDataRepresentation;
class pqRenderView;
class pqServer;
class pqCMBEnumPropertyWidget;
class vtkSMSourceProxy;
class qtCMBProgressWidget;
class pqCMBLIDARPieceTable;
class pqCMBLIDARPieceObject;
class pqCMBDisplayProxyEditor;
class pqOutputPort;
class pqContourWidget;
class vtkSMProxy;
class pqCMBContourTreeItem;
class QTreeWidgetItem;
class QTableWidgetItem;
class pqCMBPointsBuilderMainWindowCore;
class pqCMBLIDARReaderManager;
class pqCMBLIDARTerrainExtractionManager;
class qtCMBArcWidget;
class pqCMBLoadDataReaction;

class pqCMBPointsBuilderMainWindowCore :  public pqCMBCommonMainWindowCore
{
  typedef pqCMBCommonMainWindowCore Superclass;
  Q_OBJECT

  friend class pqCMBLIDARTerrainExtractionManager;
  friend class pqCMBLIDARReaderManager;

public:
  pqCMBPointsBuilderMainWindowCore(QWidget* parent);
  virtual ~pqCMBPointsBuilderMainWindowCore();

  bool IsDataLoaded();
  void setupControlPanel(QWidget* parent);
  QWidget* getControlPanel();
  void updateSelection(pqOutputPort* selPort);
  void setupProgressBar(QStatusBar*);
  pqCMBLIDARTerrainExtractionManager *getTerrainExtractionManager();
  pqCMBLIDARReaderManager *getReaderManager()
    { return this->ReaderManager; }

  void startMultiFileRead();
  void endMultiFileRead();

signals:
  void newDataLoaded();
  void requestingRender();
  void renderRequested();
  void openMoreFiles();

public slots:
  /// Called when a new server is connected.
  virtual void onServerCreationFinished(pqServer *server);

  virtual void onOpenMoreFiles(pqCMBLoadDataReaction*);

  QString getLIDARFileTitle() const;

  /// Called when a new reader is created by the GUI.
  /// We add the reader to the recent files menu.
  void onReaderCreated(pqPipelineSource*, const QStringList&);

  // Resets the center of rotation to the center of the active
  // source in the active view.
  void resetCenterOfRotationToCenterOfCurrentData();

  // Description:
  // Closes the currently opened solid.
  void onCloseData();
  void closeData();

  // Description:
  // Saves the data (geometry + region and material IDs) in a file.
  void onSaveData();
  void onSaveAsData();
  void onAcceptToLoad();

  // Description:
  // Opens dialog to filter data
  void showFilterDialog();

  void onRubberBandSelect(bool);
  void onVTKConnectionChanged(pqDataRepresentation* connRep);

  // Zoom onto the selected object
  void onPiecesSelectionChanged(pqCMBLIDARPieceObject*);

  void onConvertFromLatLong(bool state);
  void onSaveContour();
  void onLoadContour();

private slots:

  void onUpdateSelectedPieces();
  void OnPreviewSelected();
  void onEnableClip();
  void onClippingBoxChanged();
  void applyTargetNumberOfPoints();
  void onFilterChanged();
  void onUseFiltersToggled(bool);
  void onAddThreshold();
  void onRemoveFilter();
  void onActiveFilterChanged();
  pqCMBContourTreeItem* onAddContourGroup();
  void onAddContourWidget();
  void onRemoveContour();
  void onContourFinished();
  void onContourChanged();

  void zoomSelection();
  // the updateFocusFlag is set to false when calling from updateFocus so
  // that we don't create a cycle
  void clearSelection(bool updateFocusFlag = true);

  void onAdvancedCheckBox(int);
  void onObjectsCheckStateChanged(QList<int>, QList<int>);
  void onObjectOnRatioChanged(pqCMBLIDARPieceObject*, int);
  void onCurrentObjectChanged(pqCMBLIDARPieceObject*);
  void onTabChanged(int tabIndex);

  void selectAll();
  void unselectAll();
  void abort();
  void updateProgress(const QString& text, int progress);

  // Description:
  // Save the model.
  void onRequestRender();
  void onRenderRequested();
  void onThresholdSelectionChanged();
  void onThresholdItemChanged(QTableWidgetItem*);
  void onPolygonItemChanged(QList<QTreeWidgetItem*>, int , int);
  void onPolygonTableSelectionChanged(QTreeWidgetItem*);
  void onPolygonTreeItemsDropped(QTreeWidgetItem* toGroup,
    int fromGroup, QList<QTreeWidgetItem*> );
  void onPolygonItemRemoved(QList<pqContourWidget*>);

  void onUpdateContours();
  void onElevationFilter(bool useFilter);
  void onUpdateElevationFilter();
  void onElevationMinChanged(double minZ);
  void onElevationMaxChanged(double maxZ);
  void onModifierArcWidgetStart();
  void onModifierArcWidgetFinish();
  //slot for dealing with changes to preferences
  void onDefaultMaxNumberOfTargetPointsChanged();
private:

  void saveContour(const char* filename);
  void loadContour(const char* filename);
  void clearCurrentLIDARData();
  int ImportLIDARFile(const char* filename);
  void ImportLIDARFiles(const QStringList& files, pqPipelineSource* reader = NULL);

  void initControlPanel();
  void setupReadClipping();
  void setupElevationFilter(double minZ, double maxZ);
  void updateElevationFilterExtent();

  void enableAbort(bool enabled);
  bool isUpdateNeeded();
  bool isObjectUpToDate(pqCMBLIDARPieceObject* dataObj);

  int calculateMainOnRatio(int totalNumberOfPoints);
  int calculateOnRatioForPiece(int onRatio, int numberOfPointsInPiece);
  int getMinimumNumberOfPointsPerPiece();

  int onSavePieces(int onRatio, bool askMultiOutput=true);

  void unselectCheckBoxes();

  bool generateAndValidateOutFileNames(
    QList<pqCMBLIDARPieceObject*> pieces,
    const QString& filename, QList<QString>& outFiles);
  bool ExportDem(QList<pqPipelineSource*> pieces,
                 const QString& writerName, const QString& fileName);
  bool WritePiece(pqPipelineSource* source, const QString& writerName,
    const QString& fileName);
  bool WritePieces(QList<pqPipelineSource*> pieces,
    const QString& writerName, const QString& fileName, bool writeAsSinglePiece);
  bool WriteFile(const QString& fileName);

  void hideDisplayPanelPartialComponents();
  void updatePieceRepresentations(QList<pqCMBLIDARPieceObject*> pieces,
    bool forceRead = false);
  void enableButtons(bool enabled);
  void updateLoadAndUpdateButtons(bool shouldUpdateFocus = true,
    bool focusOnTableIfRowIsSelected = false);
  void updateFocus(bool focusOnTableIfRowIsSelected = false);
  void updateZoomAndClearState();
  void updatePointTotals();

  pqCMBLIDARPieceTable *getLIDARPieceTable();
  bool savePieces(
    QList<pqCMBLIDARPieceObject*> pieces, const QString& filename,
    bool saveAsSinglePiece, bool loadAsDisplayed,
    bool multiOutput = false);

  void updateContourSource(vtkSMSourceProxy* contourSource,
    vtkSMSourceProxy* currentContours, bool forceUpdate);
  void updateThresholdSource(vtkSMSourceProxy* thresholdSource,
    vtkSMSourceProxy* currentThresholds, bool forceUpdate);
  bool updateThresholdTransform(pqCMBLIDARPieceObject *dataObj);

  void initThresholdTable();
  void initPolygonsTree();
  void addContourFilter(pqContourWidget*);
  void updateTransformPanel(bool enable);
  void resetAllPiecesWithNoFilters();
  void clearThresholdFilters();
  void clearContourFilters();
  pqCMBContourTreeItem* getContourGroupNodeWithId(int id);
  int getContourGroupIdWithNode(QTreeWidgetItem* node);
  int getContourGroupIdWithContour(
    pqContourWidget* contourWidget);
  QTreeWidgetItem* addContourNode(pqContourWidget* contourW,
    int orthoPlane);
  class vtkInternal;
  vtkInternal* const Internal;
  pqCMBLIDARReaderManager *ReaderManager;
  pqCMBLIDARTerrainExtractionManager *TerrainExtractionManager;

  std::vector<pqCMBModifierArc *> pqCMBModifierArcs;
  qtCMBArcWidget* createArcWidget( int normal, double position );
};


#endif // !_pqCMBPointsBuilderMainWindowCore_h

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqCMBLIDARDataImporter - imports a LIDAR file into pqPipelineSource.
// .SECTION Description
// .SECTION Caveats


#ifndef __pqCMBLIDARDataImporter_h
#define __pqCMBLIDARDataImporter_h

#include "cmbAppCommonExport.h"
#include <QObject>
#include <QPointer>
#include <QMap>
#include <QDialog>
#include "vtkBoundingBox.h"
#include "cmbSystemConfig.h"

namespace Ui { class qtLIDARDataImportDialog; }

class pqDataRepresentation;
class pqPipelineSource;
class pqDataRepresentation;
class pqRenderView;
class pqServer;
class pqCMBEnumPropertyWidget;
class vtkSMPropertyLink;
class qtCMBProgressWidget;
class pqCMBLIDARPieceTable;
class pqCMBLIDARPieceObject;
class pqCMBDisplayProxyEditor;

class CMBAPPCOMMON_EXPORT pqCMBLIDARDataImporter : public QObject
{
  Q_OBJECT

public:
  pqCMBLIDARDataImporter();
  virtual ~pqCMBLIDARDataImporter();

  const char *importLIDARFile(const char* filename);

  void setCurrentServer(pqServer* server)
  { this->CurrentServer = server;}
  pqServer* getCurrentServer()
  { return this->CurrentServer;}
  pqCMBLIDARPieceTable* getPieceMainTable()
  { return this->PieceMainTable; }

signals:
  void requestRender();
  void renderRequested();

public slots:
  void onVTKConnectionChanged(pqDataRepresentation* connRep);
  void onPiecesSelectionChanged(pqCMBLIDARPieceObject*);

protected slots:
  //void onSavePiecesAsDisplayed();
  //void onAcceptToLoadAsDisplayed();
  //void onSavePiecesAsOriginal();
  //void onAcceptToLoadAsOriginal();
  void onAcceptToLoad();

  void onUpdateSelectedPieces();
  void OnPreviewSelected();
  void onSetSliderPosition(int sliderPos);
  void onCurrentPieceRatioChanged();
  void onEnableClip();
  void onClippingBoxChanged();
  void applyTargetNumberOfPoints();

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
  //void loadSelectdPieces();
  void onRequestRender();
  void onRenderRequested();


  //BTX

private:
  void setupAppearanceEditor (QWidget *parent);
  void hideDisplayPanelPartialComponents();
  void setupProgressBar();
  void enableAbort(bool enabled);
  int onSavePieces(int onRatio, bool askMultiOutput=true);
  bool isNewFileNeeded();

  bool isUpdateNeeded();
  bool isObjectUpToDate(pqCMBLIDARPieceObject* dataObj);

  bool isValidFile(const char* filename);
  void clearCurrentLIDARData();
  int ImportLIDARData(const char* filename);
  //void updateRepresentationLink();
  int getPieceInfo(QList<int> &pieceInfo);
  int calculateMainOnRatio(int totalNumberOfPoints);
  int calculateOnRatioForPiece(int onRatio, int numberOfPointsInPiece);
  void initialClippingSetup();

  pqPipelineSource* loadPieces(QList<pqCMBLIDARPieceObject*> pieces, int onRatio=0);
  pqPipelineSource* appendPieces(QList<pqCMBLIDARPieceObject*> pieces);

  bool savePieces(
    QList<pqCMBLIDARPieceObject*> pieces, const QString& filename,
    bool multiOutput = false);

  bool generateAndValidateOutFileNames(
    QList<pqCMBLIDARPieceObject*> pieces,
    const QString& filename, QList<QString>& outFiles);
  bool WritePiece(pqPipelineSource* source, const QString& writerName,
    const QString& fileName);
  bool WritePieces(QList<pqPipelineSource*> pieces,
    const QString& writerName, const QString& fileName, bool writeAsSinglePiece);
  bool WriteFile(const QString& fileName);

  void updatePieceRepresentations(QList<pqCMBLIDARPieceObject*> pieces);
  void setObjectsVisibility(QList<pqCMBLIDARPieceObject*> pieces, int visible);

  pqPipelineSource* readPieces(pqPipelineSource* reader,
    QList<QVariant> pieces);
  void readData(pqPipelineSource* reader, QList<QVariant> pieces);

  void updateRepresentationWidget(pqDataRepresentation* dataRep);

  void setRenderView(pqRenderView* renderView);
  //void InsertLIDARDataRow(
  //  int pieceId, int numPoints, int onRatio, int selected=1);
  //void updateNumPointsOfPiece(int pieceId, int numPts);
  void addProgressWidget(QWidget*);

  //QMap<int, int> getSelectedPieces();
  void enableButtons(bool enabled);
  void setupSliderBar();
  void updateSelectionButtons(int hasSelection);
  void updateLoadAndUpdateButtons(bool shouldUpdateFocus = true,
    bool focusOnTableIfRowIsSelected = false);
  void updateFocus(bool focusOnTableIfRowIsSelected = false);
  void updateZoomAndClearState();
  void updatePointTotals();

  QMap<int, pqCMBLIDARPieceObject*> PieceIdObjectMap;

  pqCMBEnumPropertyWidget* RepresentationWidget;
  Ui::qtLIDARDataImportDialog* ImportDialog;
  QDialog *MainDialog;
  pqCMBDisplayProxyEditor* AppearanceEditor;
  QWidget* AppearanceEditorContainer;

  std::string FileName;
  pqServer* CurrentServer;
  QPointer<qtCMBProgressWidget> ProgressBar;
  pqRenderView* CurrentRenderView;
  vtkSMPropertyLink* RepPropLink;
  QPointer<pqPipelineSource> ReaderSource;
  QPointer<pqPipelineSource> CurrentWriter;
  QPointer<pqPipelineSource> OutlineSource;
  QPointer<pqDataRepresentation> OutlineRepresentation;
  double DataBounds[6];
  bool RenderNeeded;
  vtkBoundingBox ClipBBox;
  int MinimumNumberOfPointsPerPiece;
  std::string OutputFileName;

  pqCMBLIDARPieceObject* LastSelectedObject;
  pqCMBLIDARPieceTable* PieceMainTable;
  //ETX
};

#endif /* __pqCMBLIDARDataImporter_h */

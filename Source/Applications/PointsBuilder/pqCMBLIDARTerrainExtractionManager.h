//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef _pqCMBLIDARTerrainExtractionManager_h
#define _pqCMBLIDARTerrainExtractionManager_h

#include <QObject>
#include <vector>
#include "cmbSystemConfig.h"

class pqCMBPointsBuilderMainWindowCore;
class qtCMBLIDARPanelWidget;
class pqContourWidget;
class pqOutputPort;
class pqDataRepresentation;
class pqPipelineSource;
class QIcon;
class QTreeWidgetItem;
class QFileInfo;

class pqCMBLIDARTerrainExtractionManager :  public QObject
{
  Q_OBJECT

public:
  pqCMBLIDARTerrainExtractionManager(pqCMBPointsBuilderMainWindowCore *core,
    qtCMBLIDARPanelWidget *panel);
  virtual ~pqCMBLIDARTerrainExtractionManager();

  pqPipelineSource *getTerrainFilter()
    { return this->TerrainExtractFilter; }
  pqPipelineSource *getFullTerrainFilter()
    { return this->FullProcessTerrainExtractFilter; }

  void onExitExtraction(bool changeTabs = true);

public slots:
  void onShow();

signals:
  void enableMenuItems(bool state);

protected slots:
  void onProcesssFullData();

  //tree controls
  void onItemClicked(QTreeWidgetItem* item, int col);
  void onItemSelectionChanged();

  //auto save slots
  bool onAutoSaveExtractFileName(); //returns true if the user selected a file

  //cache directory slots
  bool onSelectCacheDirectory(); //returns true if the user selected a directory

  //contour controls
  void onDefineContourWidget();
  void onContourFinished();
  void onContourChanged();
  void onRemoveContourWidget();
  void onElevationFilter(bool useElevation);

  //resolution controls
  void onResolutionScaleChange( QString scaleString );
  void ComputeDetailedResolution( );

  //mask size control
  void onMaskSizeTextChanged(QString);

  //if we are saving the refine results
  void onSaveRefineResultsChange( bool change );

protected:

  //resolution controls
  double ComputeResolution( pqPipelineSource *extractionFilter, bool computeDetailedScale );

  //methods called from onShow()
  void GuessCacheDirectory( );
  void ComputeBasicResolution(  );

  pqDataRepresentation *createPreviewRepresentation(QString &filename);
  pqPipelineSource *setupFullProcessTerrainFilter();

  void setupExtractionPanel();

  pqPipelineSource *PrepDataForTerrainExtraction();

  void addExtractionOutputToTree(int minLevel, int maxLevel,
    double initialScale, QFileInfo &autoSaveInfo);
  void getNumPointsCounts(QTreeWidgetItem* item,
    qulonglong &loadedNumPoints, qulonglong &actualNumPoints);
  bool setSubTreeVisibility(QTreeWidgetItem* item, bool visible, QIcon *icon);
  void makeAllInvisible();
  void clearTree();
  void destroyTreeRepresentations(QTreeWidgetItem *treeNode);
  void updateRepresentationsElevationFilter(QTreeWidgetItem *treeNode,
    bool useElevation);

  // Description:
  // Some internal ivars.
  bool CacheRefineDataForFullProcess;
  bool SaveRefineResults;

  double DetailedScale;
  double InputDims[2];

  qtCMBLIDARPanelWidget *LIDARPanel;
  pqCMBPointsBuilderMainWindowCore *LIDARCore;
  pqContourWidget* Contour;

  pqPipelineSource *TerrainExtractFilter;
  pqPipelineSource *FullProcessTerrainExtractFilter;

  QList<QVariant> DataTransform;

  std::vector< pqPipelineSource* > PDSources;

  QIcon* IconVisible;
  QIcon* IconInvisible;

};

#endif /* __pqCMBLIDARTerrainExtractionManager_h */

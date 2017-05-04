//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtCMBArcWidgetManager
// .SECTION Description
//  Create and controls the arc editing singelton widget
// .SECTION Caveats

#ifndef __qtCMBArcWidgetManager_h
#define __qtCMBArcWidgetManager_h

#include "cmbAppCommonExport.h"
#include "cmbSystemConfig.h"
#include "vtkType.h"
#include <QList>
#include <QObject>

class pqCMBArc;
class pqCMBSceneNode;
class qtArcWidget;
class qtCMBArcEditWidget;
class pqRenderView;
class pqServer;
class vtkDoubleArray;
class vtkIdTypeArray;

class CMBAPPCOMMON_EXPORT qtCMBArcWidgetManager : public QObject
{
  Q_OBJECT

public:
  qtCMBArcWidgetManager(pqServer* server, pqRenderView* view);
  ~qtCMBArcWidgetManager() override;

  pqCMBArc* createpqCMBArc();

  int create();
  int edit();

  bool hasActiveNode();
  bool hasActiveArc();
  bool isActive();
  void setActiveNode(pqCMBSceneNode* node);
  void setActiveArc(pqCMBArc*);
  pqCMBSceneNode* getActiveNode();
  pqCMBArc* getActiveArc();
  qtArcWidget* createDefaultContourWidget(int& normal, double& pos);

  QWidget* getActiveWidget() { return ActiveWidget; }

  //this converts the old V1 reader format of information
  //to the new cmbScenePolyline object. Nobody else should use this method.
  pqCMBArc* createLegacyV1Contour(const int& normal, const double& position, const int& closedLoop,
    vtkDoubleArray* nodePositions, vtkIdTypeArray* SelIndices);

signals:
  void Busy();
  void Ready();
  void Finish();

  //This signal is emitted when an arc is split into multiple new arcs
  void ArcSplit(pqCMBSceneNode*, QList<vtkIdType>);
  void ArcSplit2(pqCMBArc*, QList<vtkIdType>);

  //this signal is emitted whenever an arc is modified, including when
  //split
  void ArcModified(pqCMBSceneNode*);
  void ArcModified2(pqCMBArc*);

  void editingStarted();

  void selectedId(vtkIdType);

public slots:
  // slot for operations on an arc or sub-arc
  void straightenArc(vtkIdType startIdx, vtkIdType endIdx);
  void collapseSubArc(vtkIdType startIdx, vtkIdType endIdx);
  void makeArc(vtkIdType startIdx, vtkIdType endIdx);
  void startSelectPoint();
  void cancelSelectPoint();

protected slots:
  // called when a whole arc is done creating or modifying.
  void updateArcNode();
  // called when a sub arc modification is done
  void updateModifiedArc(qtArcWidget* subArcWidget, vtkIdType startPID, vtkIdType endPID);
  // called when the edit widget is closed
  void editingFinished();

  void updateActiveView(pqRenderView* view) { View = view; }
  void updateActiveServer(pqServer* server) { Server = server; }

protected:
  void getDefaultArcPlane(int& normal, double& pos);
  void resetArcPlane(int normal, double pos);
  qtArcWidget* createContourWidget(int normal, double position);
  void modifyArc(vtkIdType startIdx, vtkIdType endIdx, int opType);

  qtArcWidget* Widget;
  qtCMBArcEditWidget* EditWidget;

  pqCMBSceneNode* Node;
  pqCMBArc* Arc;

  pqRenderView* View;
  pqServer* Server;
  QWidget* ActiveWidget;
};

#endif /* __qtCMBArcWidgetManager_h */

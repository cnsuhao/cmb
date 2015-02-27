/*=========================================================================

  Program:   CMB
  Module:    qtCMBArcWidgetManager.h

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
// .NAME qtCMBArcWidgetManager
// .SECTION Description
//  Create and controls the arc editing singelton widget
// .SECTION Caveats


#ifndef __qtCMBArcWidgetManager_h
#define __qtCMBArcWidgetManager_h

#include "cmbAppCommonExport.h"
#include <QList>
#include <QObject>
#include "vtkType.h"
#include "cmbSystemConfig.h"

class pqCMBArc;
class pqCMBSceneNode;
class qtCMBArcWidget;
class qtCMBArcEditWidget;
class pqRenderView;
class pqServer;
class vtkDoubleArray;
class vtkIdTypeArray;

class CMBAPPCOMMON_EXPORT qtCMBArcWidgetManager : public QObject
{
  Q_OBJECT

public:
  qtCMBArcWidgetManager(pqServer *server, pqRenderView *view);
  virtual ~qtCMBArcWidgetManager();

  pqCMBArc* createpqCMBArc();

  int create();
  int edit();

  bool hasActiveNode( );
  bool hasActiveArc();
  bool isActive();
  void setActiveNode(pqCMBSceneNode *node);
  void setActiveArc(pqCMBArc*);
  pqCMBSceneNode* getActiveNode( );
  pqCMBArc* getActiveArc();
  qtCMBArcWidget* createDefaultContourWidget(int& normal, double& pos);

  QWidget* getActiveWidget() { return ActiveWidget; }

  //this converts the old V1 reader format of information
  //to the new cmbScenePolyline object. Nobody else should use this method.
  pqCMBArc* createLegacyV1Contour(const int &normal,
    const double &position, const int &closedLoop,
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

public slots:
  // slot for operations on an arc or sub-arc
  void straightenArc(vtkIdType startIdx, vtkIdType endIdx);
  void collapseSubArc(vtkIdType startIdx, vtkIdType endIdx);
  void makeArc(vtkIdType startIdx, vtkIdType endIdx);

protected slots:
  // called when a whole arc is done creating or modifying.
  void updateArcNode();
  // called when a sub arc modification is done
  void updateModifiedArc(
    qtCMBArcWidget* subArcWidget, vtkIdType startPID, vtkIdType endPID);
  // called when the edit widget is closed
  void editingFinished();

  void updateActiveView( pqRenderView *view ){ View=view;}
  void updateActiveServer( pqServer *server ){ Server=server;}

protected:
  void getDefaultArcPlane(int& normal, double& pos);
  void resetArcPlane(int normal, double pos);
  qtCMBArcWidget* createContourWidget( int normal, double position );
  void modifyArc(vtkIdType startIdx, vtkIdType endIdx, int opType);

  qtCMBArcWidget* Widget;
  qtCMBArcEditWidget* EditWidget;

  pqCMBSceneNode *Node;
  pqCMBArc  *Arc;

  pqRenderView *View;
  pqServer *Server;
  QWidget* ActiveWidget;
};

#endif /* __qtCMBArcWidgetManager_h */

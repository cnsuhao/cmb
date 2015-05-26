//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqCMBModelEdge - a client side CMB model edge object.
// .SECTION Description
// .SECTION Caveats

#ifndef __pqCMBModelEdge_h
#define __pqCMBModelEdge_h

#include "pqCMBModelEntity.h"
#include "cmbSystemConfig.h"

class vtkDiscreteModelEdge;
class pqPipelineSource;
class pqDataRepresentation;
class pqRenderView;
class pqServer;
class vtkSMProxy;
class vtkSMSourceProxy;
class vtkDiscreteModel;
class vtkDiscreteLookupTable;
class vtkModelEntity;
class pqCMBModel;

class  pqCMBModelEdge : public pqCMBModelEntity
{
  typedef pqCMBModelEntity Superclass;
  Q_OBJECT

public:
  pqCMBModelEdge();
  virtual ~pqCMBModelEdge();

  // Description:
  // static methods to create "this" client side object
  static pqCMBModelEdge *createObject(
    vtkSMProxy* modelWrapper, vtkDiscreteModelEdge* cmbModelEdge,
    pqServer *server, pqRenderView *view, bool updateRep = true);
  static pqCMBModelEdge *createObject(pqPipelineSource *source,
    pqServer *server, pqRenderView *view,
    vtkDiscreteModelEdge* cmbModelEdge, vtkSMProxy* modelWrapper,bool updateRep = true);

  // Description:
  // Get adjacent faces of this edge. return the number of adjacent faces
  // NOTE: the QList is not cleared. The adjacent faces are appended if
  // they are not in the list already.
  int GetAdjacentModelFaces(QList<vtkModelEntity*>& faces);

  vtkDiscreteModelEdge* getModelEdgeEntity() const;
  bool splitSelectedNodes(const QList<vtkIdType>& ptIds,
    pqCMBModel* CMBModel, vtkSMProxy* ModelWrapper,
    QList<vtkIdType>& newEdges, QList<vtkIdType>& newVTXs);
  void colorByColorMode(vtkDiscreteLookupTable*, int colorMode);
  void setEdgePointsVisibility(int visible);

protected:

  // Description:
  // initialize method
  virtual void init();

private:

  int ColorMode;
};

#endif /* __pqCMBModelEdge_h */

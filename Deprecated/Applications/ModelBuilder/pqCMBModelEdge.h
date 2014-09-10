/*=========================================================================

  Program:   CMB
  Module:    pqCMBModelEdge.h

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

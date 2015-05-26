//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqCMBModelVertex - a client side CMB model edge object.
// .SECTION Description
// .SECTION Caveats

#ifndef __pqCMBModelVertex_h
#define __pqCMBModelVertex_h

#include "pqCMBModelEntity.h"
#include "cmbSystemConfig.h"

class vtkDiscreteModelVertex;
class pqPipelineSource;
class pqDataRepresentation;
class pqRenderView;
class pqServer;
class vtkSMProxy;
class vtkSMSourceProxy;

class  pqCMBModelVertex : public pqCMBModelEntity
{
  typedef pqCMBModelEntity Superclass;
  Q_OBJECT

public:
  pqCMBModelVertex();
  virtual ~pqCMBModelVertex();

  // Description:
  // static methods to create "this" client side object
  static pqCMBModelVertex *createObject(
    vtkSMProxy* modelWrapper, vtkDiscreteModelVertex* CMBModelVertex,
    pqServer *server, pqRenderView *view, bool updateRep = true);
  static pqCMBModelVertex *createObject(pqPipelineSource *source,
    /* pqServer *server,*/ pqRenderView *view,
    vtkDiscreteModelVertex* CMBModelVertex, vtkSMProxy* modelWrapper,
    bool updateRep = true);

  vtkDiscreteModelVertex* getModelVertexEntity() const;
  vtkIdType getPointId();

protected:

 // Description:
 // initialize method
 virtual void init();

};

#endif /* __pqCMBModelVertex_h */

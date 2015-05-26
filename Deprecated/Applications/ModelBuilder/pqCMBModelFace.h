//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqCMBModelFace - a client side CMB model face object.
// .SECTION Description
// .SECTION Caveats

#ifndef __pqCMBModelFace_h
#define __pqCMBModelFace_h

#include "pqCMBModelEntity.h"
#include "cmbSystemConfig.h"

class vtkDiscreteModelFace;
class pqPipelineSource;
class pqDataRepresentation;
class pqRenderView;
class pqServer;
class vtkSMProxy;
class vtkSMSourceProxy;
class vtkDiscreteLookupTable;

class QTreeWidgetItem;

class  pqCMBModelFace : public pqCMBModelEntity
{
  typedef pqCMBModelEntity Superclass;
  Q_OBJECT

public:
  pqCMBModelFace();
  virtual ~pqCMBModelFace();

enum PolygonFaceColorByMode
{
  PolygonFaceColorByNone      = 0,
  PolygonFaceColorByModelFace = 1,
  PolygonFaceColorByMaterial  = 2,
  PolygonFaceColorByAttribute = 3
};

  // Description:
  // static methods to create "this" client side object
  static pqCMBModelFace *createObject(
    vtkSMProxy* modelWrapper, vtkDiscreteModelFace* cmbModelFace,
    pqServer *server, pqRenderView *view, bool updateRep = true);
  static pqCMBModelFace *createObject(pqPipelineSource *source,
    pqServer *server, pqRenderView *view,
    vtkDiscreteModelFace* cmbModelFace, vtkSMProxy* modelWrapper,
    bool updateRep = true);

  void colorByColorMode(vtkDiscreteLookupTable*, int colorMode);
  //Description:
  // For 2D model, the face can be colored by "Model Face"=1 or "Domain set"=2
  void colorByEdgeDomainMode(vtkDiscreteLookupTable*, int colorMode);
  vtkDiscreteModelFace* getModelFaceEntity() const;

protected:

 // Description:
 // initialize method
 virtual void init();

};

#endif /* __pqCMBModelFace_h */

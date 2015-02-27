/*=========================================================================

  Program:   CMB
  Module:    pqCMBModelFace.h

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

/*=========================================================================

  Program:   CMB
  Module:    pqCMBFloatingEdge.h

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
// .NAME pqCMBFloatingEdge - a client side CMB floating edge object.
// .SECTION Description
// .SECTION Caveats

#ifndef __pqCMBFloatingEdge_h
#define __pqCMBFloatingEdge_h

#include "pqCMBModelEntity.h"
#include "cmbSystemConfig.h"

class vtkDiscreteModel;
class vtkDiscreteModelEdge;
class pqPipelineSource;
class pqDataRepresentation;
class pqRenderView;
class pqServer;
class vtkSMProxy;
class vtkSMSourceProxy;

class QTreeWidgetItem;

class  pqCMBFloatingEdge : public pqCMBModelEntity
{
  typedef pqCMBModelEntity Superclass;
  Q_OBJECT

public:

  pqCMBFloatingEdge();
  virtual ~pqCMBFloatingEdge();


  // Description:
  // static methods to create "this" client side object
  static pqCMBFloatingEdge *createObject(
    vtkSMProxy* modelWrapper, vtkDiscreteModelEdge* cmbModelEdge,
    pqServer *server, pqRenderView *view, bool updateRep = true);

  static pqCMBFloatingEdge *createObject(
    pqPipelineSource *linesource,
    pqServer *server, pqRenderView *view,
    vtkDiscreteModelEdge* cmbModelEdge, vtkSMProxy* modelWrapper,
    bool updateRep = true);





  // Description:
  // Get/Set line resolution
  void setLineResolution(int res,
                         vtkDiscreteModel* Model, vtkSMProxy* Wrapper);
  int getLineResolution();

  bool isSelected() {return this->selected;}
  vtkDiscreteModelEdge* getModelEdgeEntity();
  static int updateLineSource(
    vtkSMProxy* modelProxy, vtkDiscreteModelEdge* cmbModelEdge,

    pqPipelineSource* polySource);

  void select();
  void deselect();


protected:

 // Description:
 // initialize method
 virtual void init();


 // Description:
 // Some ivars
 double HighlightColor[3];
 double OriginalColor[3];
 std::string OrigColorArrayName;


 bool selected;
};

#endif /* __pqCMBFloatingEdge_h */

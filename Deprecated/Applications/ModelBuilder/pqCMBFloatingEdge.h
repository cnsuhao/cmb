//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
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

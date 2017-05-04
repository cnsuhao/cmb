//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqCMBLine - represents a Line segment object.
// .SECTION Description
// .SECTION Caveats

#ifndef __pqCMBLine_h
#define __pqCMBLine_h

#include "cmbAppCommonExport.h"
#include "cmbSystemConfig.h"
#include "pqCMBSceneObjectBase.h"

class pqRenderView;
class pqServer;
class qtLineWidget;
class vtkSMSourceProxy;

class CMBAPPCOMMON_EXPORT pqCMBLine : public pqCMBSceneObjectBase
{
public:
  pqCMBLine();
  ~pqCMBLine() override;

  pqCMBLine(
    pqCMBSceneObjectBase* refObj, pqServer* server, pqRenderView* view, bool updateRep = true);

  pqCMBLine(double point1[3], double point2[3], pqServer* server, pqRenderView* view,
    bool updateRep = true);

  static void getDefaultBounds(pqRenderView* theView, double bounds[6]);

  pqCMBSceneObjectBase* duplicate(
    pqServer* server, pqRenderView* view, bool updateRep = true) override;

  void setSelectionInput(vtkSMSourceProxy* selectionInput) override;
  virtual void select();
  virtual void deselect();
  vtkSMSourceProxy* getSelectionInput() const override { return NULL; }

  void getColor(double color[4]) const override;
  void setColor(double color[4], bool updateRep = true) override;
  void getBounds(double bounds[6]) const override;
  void getDataBounds(double bounds[6]) const override;

  //return 1 on success; 0 on failure.
  virtual int getPointPosition(int pointIdx, double& x, double& y, double& z);
  virtual int getPoint1Position(double& x, double& y, double& z)
  {
    return getPointPosition(1, x, y, z);
  }
  virtual int getPoint2Position(double& x, double& y, double& z)
  {
    return getPointPosition(2, x, y, z);
  }

  void setVisibility(bool mode) override;

  qtLineWidget* getLineWidget() { return this->LineWidget; }

  pqCMBSceneObjectBase::enumObjectType getType() const override;
  bool isDefaultConstrained() const override { return true; }

protected:
  void initialize(
    double point1[3], double point2[3], pqServer* server, pqRenderView* view, bool updateRep);

  qtLineWidget* LineWidget;
};

#endif /* __pqCMBLine_h */

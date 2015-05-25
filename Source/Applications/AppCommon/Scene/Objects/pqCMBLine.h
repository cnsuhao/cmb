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
#include "pqCMBSceneObjectBase.h"
#include "cmbSystemConfig.h"

class pqRenderView;
class pqServer;
class pqCMBLineWidget;
class vtkSMSourceProxy;

class CMBAPPCOMMON_EXPORT pqCMBLine : public pqCMBSceneObjectBase
{
public:
  pqCMBLine();
  virtual ~pqCMBLine();

  pqCMBLine(pqCMBSceneObjectBase* refObj,
               pqServer *server,
               pqRenderView *view,
               bool updateRep = true);

  pqCMBLine(double point1[3],
               double point2[3],
               pqServer *server,
               pqRenderView *view,
               bool updateRep = true);

  static void getDefaultBounds(pqRenderView* theView, double bounds[6]);

  virtual pqCMBSceneObjectBase *duplicate(pqServer *server, pqRenderView *view,
                                bool updateRep = true);

 virtual void setSelectionInput(vtkSMSourceProxy *selectionInput);
 virtual void select();
 virtual void deselect();
 virtual vtkSMSourceProxy *getSelectionInput() const
  {return NULL;}

 virtual void getColor(double color[4]) const;
 virtual void setColor(double color[4], bool updateRep = true);
 virtual void getBounds(double bounds[6]) const;
 virtual void getDataBounds(double bounds[6]) const;

 //return 1 on success; 0 on failure.
 virtual int getPointPosition( int pointIdx, double &x, double &y, double &z);
 virtual int getPoint1Position(double &x, double &y, double &z)
   {return getPointPosition( 1,x, y, z);}
 virtual int getPoint2Position(double &x, double &y, double &z)
    {return getPointPosition( 2, x, y, z);}

 virtual void setVisibility(bool mode);

 virtual void updateRepresentation();

 pqCMBLineWidget* getLineWidget()
  { return this->LineWidget;}

 virtual pqCMBSceneObjectBase::enumObjectType getType() const;
 virtual bool isDefaultConstrained() const{return true;}

protected:
 void initialize(double point1[3],
                 double point2[3],
                 pqServer *server,
                 pqRenderView *view,
                 bool updateRep);

  pqCMBLineWidget* LineWidget;
};

#endif /* __pqCMBLine_h */

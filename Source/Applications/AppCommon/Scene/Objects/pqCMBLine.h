/*=========================================================================

  Program:   CMB
  Module:    pqCMBLine.h

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

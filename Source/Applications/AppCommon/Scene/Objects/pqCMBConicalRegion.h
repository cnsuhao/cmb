/*=========================================================================

  Program:   CMB
  Module:    pqCMBConicalRegion.h

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
// .NAME pqCMBConicalRegion - represents a Conical Region.
// .SECTION Description
// .SECTION Caveats


#ifndef __pqCMBConicalRegion_h
#define __pqCMBConicalRegion_h

#include "pqCMBSceneObjectBase.h"
#include "cmbSystemConfig.h"


class  CMBAPPCOMMON_EXPORT pqCMBConicalRegion : public pqCMBSceneObjectBase
{
public:
  pqCMBConicalRegion();
  pqCMBConicalRegion(double baseCenter[3], double baseRadius,
                        double height, double topRadius, double direction[3],
                        int resolution,
                        pqServer *server, pqRenderView *view,
                        bool updateRep=true);
  pqCMBConicalRegion(pqPipelineSource*source,
              pqRenderView *view, pqServer *server);

  virtual ~pqCMBConicalRegion();
  virtual pqCMBSceneObjectBase *duplicate(pqServer *server, pqRenderView *view,
                                bool updateRep = true);
  virtual pqCMBSceneObjectBase::enumObjectType getType() const;
  void setBaseCenter(double pnt[3]);
  void getBaseCenter(double pnt[3]) const;
  void setDirection(double pnt[3]);
  void getDirection(double pnt[3]) const;
  void setHeight(double height);
  double getHeight() const;
  void setBaseRadius(double radius);
  double getBaseRadius() const;
  void setTopRadius(double radius);
  double getTopRadius() const;
  void setResolution(int res);
  int getResolution() const;
protected:

};

#endif /* __pqCMBConicalRegion_h */

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
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

  ~pqCMBConicalRegion() override;
  pqCMBSceneObjectBase *duplicate(pqServer *server, pqRenderView *view,
                                bool updateRep = true) override;
  pqCMBSceneObjectBase::enumObjectType getType() const override;
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

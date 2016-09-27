//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqCMBVOI - represents a Volume of Interest  object.
// .SECTION Description
// .SECTION Caveats


#ifndef __pqCMBVOI_h
#define __pqCMBVOI_h

#include "pqCMBSceneObjectBase.h"
#include "cmbSystemConfig.h"


class  CMBAPPCOMMON_EXPORT pqCMBVOI : public pqCMBSceneObjectBase
{
public:
  pqCMBVOI();
  pqCMBVOI(double origin[3], double bounds[6],
              pqServer *server, pqRenderView *view,
              bool updateRep=true);
  pqCMBVOI(pqPipelineSource*source,
              pqRenderView *view, pqServer *server);

  ~pqCMBVOI() override;
  pqCMBSceneObjectBase *duplicate(pqServer *server, pqRenderView *view,
                                bool updateRep = true) override;
  pqCMBSceneObjectBase::enumObjectType getType() const override;
  void setVOI(double minPnt[3], double maxPnt[3]);
  int getVOI(double minPnt[3], double maxPnt[3]) const;

  bool contains(pqCMBSceneObjectBase *object) const;

protected:

};

#endif /* __pqCMBVOI_h */

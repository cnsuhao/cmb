//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqCMBPlane - represents a Plane (for example the ground plane.
// .SECTION Description
// .SECTION Caveats


#ifndef __pqCMBPlane_h
#define __pqCMBPlane_h

#include "pqCMBTexturedObject.h"
#include "cmbSystemConfig.h"


class  CMBAPPCOMMON_EXPORT pqCMBPlane : public pqCMBTexturedObject
{
public:
  pqCMBPlane();
  pqCMBPlane(double point1[3], double point2[3],
              pqServer *server, pqRenderView *view,
              bool updateRep=true);
  pqCMBPlane(pqPipelineSource*source,
              pqRenderView *view, pqServer *server);

  virtual ~pqCMBPlane();
  virtual pqCMBSceneObjectBase *duplicate(pqServer *server, pqRenderView *view,
                                bool updateRep = true);
  virtual pqCMBSceneObjectBase::enumObjectType getType() const;
  int getPlaneInfo(double p1[3], double p2[3]) const;
  void setPlaneInfo(double p1[3], double p2[3]);

protected:

};

#endif /* __pqCMBPlane_h */

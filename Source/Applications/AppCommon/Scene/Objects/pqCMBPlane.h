/*=========================================================================

  Program:   CMB
  Module:    pqCMBPlane.h

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

/*=========================================================================

  Program:   CMB
  Module:    pqCMBVOI.h

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

  virtual ~pqCMBVOI();
  virtual pqCMBSceneObjectBase *duplicate(pqServer *server, pqRenderView *view,
                                bool updateRep = true);
  virtual pqCMBSceneObjectBase::enumObjectType getType() const;
  void setVOI(double minPnt[3], double maxPnt[3]);
  int getVOI(double minPnt[3], double maxPnt[3]) const;

  bool contains(pqCMBSceneObjectBase *object) const;

protected:

};

#endif /* __pqCMBVOI_h */

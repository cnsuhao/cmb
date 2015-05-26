//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqCMBCrossSection - represents a Geology cross section.
// .SECTION Description
// .SECTION Caveats


#ifndef __pqCMBCrossSection_h
#define __pqCMBCrossSection_h

#include "pqCMBSceneObjectBase.h"
#include "cmbSystemConfig.h"


class  CMBAPPCOMMON_EXPORT pqCMBCrossSection : public pqCMBSceneObjectBase
{
public:
  pqCMBCrossSection();
  pqCMBCrossSection(pqPipelineSource*source,
              pqRenderView *view, pqServer *server);

  virtual ~pqCMBCrossSection();
  virtual pqCMBSceneObjectBase *duplicate(pqServer *server, pqRenderView *view,
                                bool updateRep = true);
  virtual pqCMBSceneObjectBase::enumObjectType getType() const;
};

#endif /* __pqCMBCrossSection_h */

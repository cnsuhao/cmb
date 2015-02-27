/*=========================================================================

  Program:   CMB
  Module:    pqCMBBoreHole.h

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
// .NAME pqCMBBoreHole - represents a geology Borehole object.
// .SECTION Description
// .SECTION Caveats


#ifndef __pqCMBBoreHole_h
#define __pqCMBBoreHole_h

#include "pqCMBSceneObjectBase.h"
#include "cmbSystemConfig.h"
#include <QPointer>

class  CMBAPPCOMMON_EXPORT pqCMBBoreHole : public pqCMBSceneObjectBase
{
public:
  pqCMBBoreHole();
  pqCMBBoreHole(pqPipelineSource*source,
              pqRenderView *view, pqServer *server);

  virtual ~pqCMBBoreHole();
  virtual pqCMBSceneObjectBase *duplicate(pqServer *server, pqRenderView *view,
                                bool updateRep = true);
  virtual pqCMBSceneObjectBase::enumObjectType getType() const;
  virtual void setTubeRadius(double);
  /// Returns the Bounds of the data. - Returns the output of the TubeFilter
  virtual void getDataBounds(double bounds[6]) const;
  virtual pqPipelineSource * getSelectionSource() const;
  virtual void setSelectionInput(vtkSMSourceProxy *selectionInput);
  virtual vtkSMSourceProxy *getSelectionInput() const;

protected:
  QPointer<pqPipelineSource> TubeFilter;

};

#endif /* __pqCMBBoreHole_h */

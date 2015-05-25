//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
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

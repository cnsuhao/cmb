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

#include "cmbSystemConfig.h"
#include "pqCMBSceneObjectBase.h"
#include <QPointer>

class CMBAPPCOMMON_EXPORT pqCMBBoreHole : public pqCMBSceneObjectBase
{
public:
  pqCMBBoreHole();
  pqCMBBoreHole(pqPipelineSource* source, pqRenderView* view, pqServer* server);

  ~pqCMBBoreHole() override;
  pqCMBSceneObjectBase* duplicate(
    pqServer* server, pqRenderView* view, bool updateRep = true) override;
  pqCMBSceneObjectBase::enumObjectType getType() const override;
  virtual void setTubeRadius(double);
  /// Returns the Bounds of the data. - Returns the output of the TubeFilter
  void getDataBounds(double bounds[6]) const override;
  pqPipelineSource* getSelectionSource() const override;
  void setSelectionInput(vtkSMSourceProxy* selectionInput) override;
  vtkSMSourceProxy* getSelectionInput() const override;

protected:
  QPointer<pqPipelineSource> TubeFilter;
};

#endif /* __pqCMBBoreHole_h */

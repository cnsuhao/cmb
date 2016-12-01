//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef _pqBandedPolyDataContourPanel_h
#define _pqBandedPolyDataContourPanel_h

#include "pqPropertyWidget.h"
#include "pqVariableType.h"
#include "cmbSystemConfig.h"

/// Custom panel for the Contour filter
class pqBandedPolyDataContourPanel : public pqPropertyWidget
{
  typedef pqPropertyWidget base;

  Q_OBJECT

public:
  pqBandedPolyDataContourPanel(vtkSMProxy* pxy, vtkSMPropertyGroup*, QWidget* p=0);
  ~pqBandedPolyDataContourPanel() override;

  void apply() override;
  void reset() override;

private slots:
  /// Called to update the enable state for certain widgets.
  void updateEnableState();

private:
  class pqImplementation;
  pqImplementation* const Implementation;
};

#endif

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

#include "pqObjectPanel.h"
#include "pqVariableType.h"
#include "cmbSystemConfig.h"

/// Custom panel for the Contour filter
class pqBandedPolyDataContourPanel : public pqObjectPanel
{
  typedef pqObjectPanel base;

  Q_OBJECT

public:
  pqBandedPolyDataContourPanel(pqProxy* proxy, QWidget* p);
  ~pqBandedPolyDataContourPanel() override;

private slots:
  /// Called if the user accepts pending modifications
  void onAccepted();
  /// Called if the user rejects pending modifications
  void onRejected();

  /// Called to update the enable state for certain widgets.
  void updateEnableState();

private:
  class pqImplementation;
  pqImplementation* const Implementation;
};

#endif

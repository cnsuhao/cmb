//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef _qtSceneBuilderOptions_h
#define _qtSceneBuilderOptions_h

#include "qtCMBOptionsContainer.h"
#include <QColor>
#include "cmbSystemConfig.h"

/// options container for pages of scene builder options
class qtSceneBuilderOptions : public qtCMBOptionsContainer
{
  Q_OBJECT

public:
  // Get the global instace for the qtSceneBuilderOptions.
  static qtSceneBuilderOptions* instance();

  qtSceneBuilderOptions(QWidget *parent=0);
  ~qtSceneBuilderOptions() override;

  // set the current page
  void setPage(const QString &page) override;
  // return a list of strings for pages we have
  QStringList getPageList() override;

  // apply the changes
  void applyChanges() override;
  // reset the changes
  void resetChanges() override;

  // tell qtCMBOptionsDialog that we want an apply button
  bool isApplyUsed() const override { return true; }

  /// Get the options
  QColor        initialNewObjectColor();

private:
  class pqInternal;
  pqInternal* Internal;
  static qtSceneBuilderOptions* Instance;
};

#endif

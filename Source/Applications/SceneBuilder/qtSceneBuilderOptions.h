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
  virtual ~qtSceneBuilderOptions();

  // set the current page
  virtual void setPage(const QString &page);
  // return a list of strings for pages we have
  virtual QStringList getPageList();

  // apply the changes
  virtual void applyChanges();
  // reset the changes
  virtual void resetChanges();

  // tell qtCMBOptionsDialog that we want an apply button
  virtual bool isApplyUsed() const { return true; }

  /// Get the options
  QColor        initialNewObjectColor();

private:
  class pqInternal;
  pqInternal* Internal;
  static qtSceneBuilderOptions* Instance;
};

#endif

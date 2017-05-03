//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef _qtCMBApplicationOptions_h
#define _qtCMBApplicationOptions_h

#include "cmbAppCommonExport.h"
#include "cmbSystemConfig.h"
#include "qtCMBOptionsContainer.h"

class pqSettings;

/// options container for pages of cmb suite apps' common options
class CMBAPPCOMMON_EXPORT qtCMBApplicationOptions : public qtCMBOptionsContainer
{
  Q_OBJECT

public:
  // Get the global instance for the qtCMBApplicationOptions.
  static qtCMBApplicationOptions* instance();

  qtCMBApplicationOptions(QWidget* parent = 0);
  ~qtCMBApplicationOptions() override;

  // set the current page
  void setPage(const QString& page) override;
  // return a list of strings for pages we have
  QStringList getPageList() override;

  // apply the changes
  void applyChanges() override;
  // reset the changes
  void resetChanges() override;
  // restore the defaults
  void restoreDefaults() override;

  // tell qtCMBOptionsDialog that we want an apply button
  bool isApplyUsed() const override { return true; }

  /// Get the common cmb application settings.
  pqSettings* cmbAppSettings();
  int maxNumberOfCloudPoints();
  std::string defaultMeshStorageDirectory();
  std::string defaultTempScratchDirectory();
  std::string defaultRepresentationType() { return "Surface"; }
  void loadGlobalPropertiesFromSettings();
  void loadBuiltinColorPresets();

signals:
  void defaultMaxNumberOfPointsChanged();

protected slots:
  void chooseMeshStorageDir();
  void chooseTempScratchDir();
  // save the options into settings
  virtual void saveOptions();

private:
  class pqInternal;
  pqInternal* Internal;
  static qtCMBApplicationOptions* Instance;
};

#endif

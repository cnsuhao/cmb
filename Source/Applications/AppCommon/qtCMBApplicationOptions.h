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
#include "qtCMBOptionsContainer.h"
#include "cmbSystemConfig.h"

class pqSettings;

/// options container for pages of cmb suite apps' common options
class CMBAPPCOMMON_EXPORT qtCMBApplicationOptions : public qtCMBOptionsContainer
{
  Q_OBJECT

public:
  // Get the global instance for the qtCMBApplicationOptions.
  static qtCMBApplicationOptions* instance();

  qtCMBApplicationOptions(QWidget *parent=0);
  virtual ~qtCMBApplicationOptions();

  // set the current page
  virtual void setPage(const QString &page);
  // return a list of strings for pages we have
  virtual QStringList getPageList();

  // apply the changes
  virtual void applyChanges();
  // reset the changes
  virtual void resetChanges();
  // restore the defaults
  virtual void restoreDefaults();

  // tell qtCMBOptionsDialog that we want an apply button
  virtual bool isApplyUsed() const { return true; }

  /// Get the common cmb application settings.
  pqSettings* cmbAppSettings();
  int maxNumberOfCloudPoints();
  std::string defaultMeshStorageDirectory();
  std::string defaultTempScratchDirectory();
  std::string defaultRepresentationType(){ return "Surface"; }
  void loadGlobalPropertiesFromSettings();

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

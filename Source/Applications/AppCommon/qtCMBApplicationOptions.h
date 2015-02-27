/*=========================================================================

Program:   CMB
Module:    qtCMBApplicationOptions.h

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

  // tell qtCMBOptionsDialog that we want an apply button
  virtual bool isApplyUsed() const { return true; }

  /// Get the common cmb application settings.
  pqSettings* cmbAppSettings();
  int maxNumberOfCloudPoints();
  std::string defaultMeshStorageDirectory();
  std::string defaultTempScratchDirectory();
  std::string defaultRepresentationType();
  void loadBuiltinColorPresets();
  void loadGlobalPropertiesFromSettings();

protected slots:
  void resetColorsToDefault();
  void lodThresholdSliderChanged(int);
  void outlineThresholdSliderChanged(int);
  void chooseMeshStorageDir();
  void chooseTempScratchDir();
  void choosePresetColorTable();

private:
  class pqInternal;
  pqInternal* Internal;
  static qtCMBApplicationOptions* Instance;

  void savePresetColorTableSettings();
  void restorePresetColorTableSettings();
};

#endif

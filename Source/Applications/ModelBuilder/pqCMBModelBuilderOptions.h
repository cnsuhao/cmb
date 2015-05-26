//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef _pqCMBModelBuilderOptions_h
#define _pqCMBModelBuilderOptions_h

#include "qtCMBOptionsContainer.h"
#include "cmbSystemConfig.h"

/// options container for pages of model builder and sim builder options
class pqCMBModelBuilderOptions : public qtCMBOptionsContainer
{
  Q_OBJECT

public:
  // Get the global instace for the pqCMBModelBuilderOptions.
  static pqCMBModelBuilderOptions* instance();

  pqCMBModelBuilderOptions(QWidget *parent=0);
  virtual ~pqCMBModelBuilderOptions();

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

  // Get the options
  std::string defaultSimBuilderTemplateDirectory();
  std::string default3DModelFaceColorMode();
  std::string default2DModelFaceColorMode();
  std::string default2DModelEdgeColorMode();
  QColor defaultEdgeColor();
  QColor defaultPolygonColor();

protected slots:
  void chooseSimBuilderTemplateDirectory();

private:
  class pqInternal;
  pqInternal* Internal;
  static pqCMBModelBuilderOptions* Instance;
};

#endif

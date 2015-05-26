//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef _qtCMBOptionsContainer_h
#define _qtCMBOptionsContainer_h

#include "cmbAppCommonExport.h"
#include "cmbSystemConfig.h"
#include "qtCMBOptionsPage.h"


/// \class qtCMBOptionsContainer
/// \brief
///   The qtCMBOptionsContainer class is used to add multiple pages of
///   options to the qtCMBOptionsDialog.
///
/// Grouping the options pages into container objects can make is
/// easier to maintain a set of options. The container makes it
/// possible to reuse a UI form. If several objects have the same
/// properties, the same page can be used for each of the objects.
class CMBAPPCOMMON_EXPORT qtCMBOptionsContainer : public qtCMBOptionsPage
{
  Q_OBJECT

public:
  /// \brief
  ///   Constructs an options container.
  /// \param parent The parent widget.
  qtCMBOptionsContainer(QWidget *parent=0);
  virtual ~qtCMBOptionsContainer();

  /// \brief
  ///   Gets the page path prefix.
  /// \return
  ///   The page path prefix.
  const QString &getPagePrefix() const;

  /// \brief
  ///   Sets the page path prefix.
  /// \param prefix The new page path prefix.
  void setPagePrefix(const QString &prefix);

  /// \brief
  ///   Sets the currently displayed page.
  /// \param page The page hierarchy name.
  virtual void setPage(const QString &page) = 0;

  /// \brief
  ///   Gets the list of available pages in the container.
  /// \param pages Used to return the list of available pages.
  virtual QStringList getPageList() = 0;

private:
  QString *Prefix; ///< Stores the page prefix.
};

#endif

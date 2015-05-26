//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME smtkModelEntitySection - the Attribute Section
// .SECTION Description
// .SECTION See Also
// qtSection

#ifndef __smtkModelEntitySection_h
#define __smtkModelEntitySection_h

#include "smtk/extension/qt/qtModelEntitySection.h"
#include "cmbSystemConfig.h"

class smtkModelEntitySectionInternals;
class QListWidgetItem;

class smtkModelEntitySection : public smtk::attribute::qtModelEntitySection
{
  Q_OBJECT

public:
  smtkModelEntitySection(smtk::SectionPtr, QWidget* p);
  virtual ~smtkModelEntitySection();

public slots:

protected:
  virtual void createWidget( );

private:

  smtkModelEntitySectionInternals *Internals;

}; // class
#endif

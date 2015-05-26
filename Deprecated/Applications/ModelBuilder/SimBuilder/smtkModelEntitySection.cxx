//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "smtkModelEntitySection.h"

#include "smtk/extension/qt/qtUIManager.h"
#include "smtk/extension/qt/qtTableWidget.h"
#include "smtk/extension/qt/qtAttribute.h"
#include "smtk/extension/qt/qtItem.h"

#include "smtk/attribute/ModelEntitySection.h"
#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/Definition.h"
#include "smtk/attribute/ItemDefinition.h"
#include "smtk/attribute/Manager.h"
#include "smtk/attribute/ValueItem.h"
#include "smtk/attribute/ValueItemDefinition.h"

#include <QGridLayout>
#include <QComboBox>
#include <QTableWidgetItem>
#include <QVariant>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QKeyEvent>
#include <QModelIndex>
#include <QModelIndexList>
#include <QMessageBox>
#include <QSplitter>

//----------------------------------------------------------------------------
class smtkModelEntitySectionInternals
{
public:

};

//----------------------------------------------------------------------------
smtkModelEntitySection::smtkModelEntitySection(
  smtk::SectionPtr dataObj, QWidget* p) : qtModelEntitySection(dataObj, p)
{
  this->Internals = new smtkModelEntitySectionInternals;
}

//----------------------------------------------------------------------------
smtkModelEntitySection::~smtkModelEntitySection()
{
  delete this->Internals;
}
//----------------------------------------------------------------------------
void smtkModelEntitySection::createWidget( )
{
}

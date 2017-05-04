//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "pqCMBDisplayProxyEditor.h"

#include "pqDataRepresentation.h"

/// constructor
pqCMBDisplayProxyEditor::pqCMBDisplayProxyEditor(pqDataRepresentation* repr, QWidget* p)
  : pqProxyWidget(repr ? repr->getProxy() : NULL, p)
  , m_rep(repr)
{
}

/// destructor
pqCMBDisplayProxyEditor::~pqCMBDisplayProxyEditor()
{
}

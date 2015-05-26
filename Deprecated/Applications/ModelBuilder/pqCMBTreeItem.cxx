//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "pqCMBTreeItem.h"

#include "vtkModel.h"
//-----------------------------------------------------------------------------
pqCMBTreeItem::pqCMBTreeItem(QTreeWidgetItem* pNode, int nodeType)
: QTreeWidgetItem(pNode, nodeType)
{
  this->init();
}
//-----------------------------------------------------------------------------
pqCMBTreeItem::~pqCMBTreeItem()
{
// the model face object is kept in the model,
// and the model will take care of them.
  if(this->ModelEntity && this->ModelEntity->getSource() == NULL)
    {
    delete this->ModelEntity;
    this->ModelEntity = NULL;
    }
}

//-----------------------------------------------------------------------------
void pqCMBTreeItem::init()
{
  this->ModelEntity = NULL;
}

//-----------------------------------------------------------------------------
void pqCMBTreeItem::setModelObject(
  pqCMBModelEntity* entity, bool setWidget)
{
  this->ModelEntity = entity;
  if(this->ModelEntity && setWidget)
    {
    this->ModelEntity->addModelWidget(this);
    }

}

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqCMBTreeItem - a CMB tree widget item object.
// .SECTION Description
// .SECTION Caveats

#ifndef __pqCMBTreeItem_h
#define __pqCMBTreeItem_h

#include <QTreeWidgetItem>
#include <QPointer>
#include "pqCMBModelEntity.h"
#include "cmbSystemConfig.h"

class  pqCMBTreeItem : public QTreeWidgetItem
{
public:
  pqCMBTreeItem(QTreeWidgetItem* pNode, int nodeType=0);
  virtual ~pqCMBTreeItem();

  // Description:
  // Get/Set the model entity object
  virtual pqCMBModelEntity* getModelObject()
    {return this->ModelEntity;}
  virtual void setModelObject(pqCMBModelEntity* entity, bool setWidget=true);

protected:

  virtual void init();

  QPointer<pqCMBModelEntity> ModelEntity;
};

#endif /* __pqCMBTreeItem_h */

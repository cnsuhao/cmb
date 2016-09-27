//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqCMBContourTreeItem - a LIDAR tree widget item object.
// .SECTION Description
// .SECTION Caveats

#ifndef __pqCMBContourTreeItem_h
#define __pqCMBContourTreeItem_h

#include <QTreeWidgetItem>
#include "cmbSystemConfig.h"
class pqContourWidget;

class  pqCMBContourTreeItem : public QTreeWidgetItem
{
public:
  pqCMBContourTreeItem(QTreeWidgetItem* pNode, int itemId, int nodeType=0);
  ~pqCMBContourTreeItem() override;

  // Description:
  // Get/Set the model entity object
  virtual pqContourWidget* getContourObject()
    {return this->ContourObject;}
  virtual void setContourObject(pqContourWidget* entity);
  bool isGroupType()
    {return (this->type() !=0 );}
  int itemId()
    {return this->ItemId;}
  void setItemId(int id)
    { this->ItemId = id; }

protected:

  virtual void init();
  int ItemId;
  pqContourWidget* ContourObject;
};

#endif /* __pqCMBContourTreeItem_h */

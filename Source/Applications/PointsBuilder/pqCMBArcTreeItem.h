//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqCMBArcTreeItem - a LIDAR tree widget item object.
// .SECTION Description
// .SECTION Caveats

#ifndef __pqCMBArcTreeItem_h
#define __pqCMBArcTreeItem_h

#include <QTreeWidgetItem>
#include "cmbSystemConfig.h"
class qtArcWidget;

class  pqCMBArcTreeItem : public QTreeWidgetItem
{
public:
  pqCMBArcTreeItem(QTreeWidgetItem* pNode, int itemId, int nodeType=0);
  ~pqCMBArcTreeItem() override;

  // Description:
  // Get/Set the model entity object
  virtual qtArcWidget* getArcObject()
    {return this->ArcObject;}
  virtual void setArcObject(qtArcWidget* entity);
  bool isGroupType()
    {return (this->type() !=0 );}
  int itemId()
    {return this->ItemId;}
  void setItemId(int id)
    { this->ItemId = id; }

protected:

  virtual void init();
  int ItemId;
  qtArcWidget* ArcObject;
};

#endif /* __pqCMBArcTreeItem_h */

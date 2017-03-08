//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "pqCMBArcTreeItem.h"

//-----------------------------------------------------------------------------
pqCMBArcTreeItem::pqCMBArcTreeItem(QTreeWidgetItem* pNode,
  int itemId, int nodeType)
: QTreeWidgetItem(pNode, nodeType)
{
  this->ItemId = itemId;
  this->init();
}
//-----------------------------------------------------------------------------
pqCMBArcTreeItem::~pqCMBArcTreeItem()
{
// the model face object is kept in the model,
// and the model will take care of them.

  if(this->ArcObject &&
    static_cast<qtArcWidget*>(this->ArcObject)==NULL)
    {
    this->ArcObject = NULL;
    }
}

//-----------------------------------------------------------------------------
void pqCMBArcTreeItem::init()
{
  this->ArcObject = NULL;
}

//-----------------------------------------------------------------------------
void pqCMBArcTreeItem::setArcObject(qtArcWidget* entity)
{
  this->ArcObject = entity;
}
/*
  int visible;
  vtkIdType bFaceId, bBCId;
  for(int r=0; r<rootNode->childCount(); r++)
    {
    QTreeWidgetItem* item = rootNode->child(r);
    if(item->type() != UndefinedNodesGroup)
      {
      bBCId = this->getItemObject(item)->GetUniquePersistentId();
      if(bBCId == BCId)
        {
        continue;
        }

      visible = item->data(TREE_VISIBLE_COL, Qt::UserRole).toInt();
      if(visible && item->childCount()>0)
        {
        for(int i=0; i<item->childCount();i++)
          {
          //bFaceId = item->child(i)->text(BCTree_FACE_COL).toInt();
          bFaceId = this->getItemObject(item->child(i))->GetUniquePersistentId();
          if(bFaceId == faceId)
            {
            return true;
            }
          }
        }
      }
    }

  return false;
  */

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "pqCMBContourTreeItem.h"

//-----------------------------------------------------------------------------
pqCMBContourTreeItem::pqCMBContourTreeItem(QTreeWidgetItem* pNode,
  int itemId, int nodeType)
: QTreeWidgetItem(pNode, nodeType)
{
  this->ItemId = itemId;
  this->init();
}
//-----------------------------------------------------------------------------
pqCMBContourTreeItem::~pqCMBContourTreeItem()
{
// the model face object is kept in the model,
// and the model will take care of them.

  if(this->ContourObject &&
    static_cast<pqContourWidget*>(this->ContourObject)==NULL)
    {
    this->ContourObject = NULL;
    }
}

//-----------------------------------------------------------------------------
void pqCMBContourTreeItem::init()
{
  this->ContourObject = NULL;
}

//-----------------------------------------------------------------------------
void pqCMBContourTreeItem::setContourObject(pqContourWidget* entity)
{
  this->ContourObject = entity;
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

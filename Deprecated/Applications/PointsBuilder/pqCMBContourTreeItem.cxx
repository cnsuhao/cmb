/*=========================================================================

  Program:   CMB
  Module:    pqCMBContourTreeItem.cxx

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/
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

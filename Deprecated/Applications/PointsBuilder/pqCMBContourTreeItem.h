/*=========================================================================

  Program:   CMB
  Module:    pqCMBContourTreeItem.h

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
  virtual ~pqCMBContourTreeItem();

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

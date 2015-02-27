/*=========================================================================

  Program:   CMB
  Module:    pqCMBTreeItem.h

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

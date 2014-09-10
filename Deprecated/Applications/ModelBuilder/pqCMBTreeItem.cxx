/*=========================================================================

  Program:   CMB
  Module:    pqCMBTreeItem.cxx

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

/*=========================================================================

  Program:   CMB
  Module:    $RCSfile: cmbSceneNodeReplaceEvent.h,v $

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
// .NAME cmbSceneNodeReplaceEvent - Class that represents an event that
// replaces one set of nodes with another
// .SECTION Description
// .SECTION Caveats
#ifndef _cmbSceneNodeReplaceEvent_h
#define _cmbSceneNodeReplaceEvent_h

#include "cmbEvent.h"
#include <vector>
#include "cmbSystemConfig.h"

class pqCMBSceneNode;

class CMBAPPCOMMON_EXPORT cmbSceneNodeReplaceEvent : public cmbEvent
{

public:
  cmbSceneNodeReplaceEvent(std::size_t creationSize, std::size_t deletionSize);
    ~cmbSceneNodeReplaceEvent();

    virtual void undo();
    virtual void redo();

    void addCreatedNode(pqCMBSceneNode *node)
    {
        this->CreatedNodes.push_back(node);
    }

    void addDeletedNode(pqCMBSceneNode *node)
    {
        this->DeletedNodes.push_back(node);
    }

protected:
  std::vector<pqCMBSceneNode*> CreatedNodes;
  std::vector<pqCMBSceneNode*> DeletedNodes;

};

#endif // _cmbSceneNodeReplaceEvent_h

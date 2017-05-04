//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME cmbSceneNodeReplaceEvent - Class that represents an event that
// replaces one set of nodes with another
// .SECTION Description
// .SECTION Caveats
#ifndef _cmbSceneNodeReplaceEvent_h
#define _cmbSceneNodeReplaceEvent_h

#include "cmbEvent.h"
#include "cmbSystemConfig.h"
#include <vector>

class pqCMBSceneNode;

class CMBAPPCOMMON_EXPORT cmbSceneNodeReplaceEvent : public cmbEvent
{

public:
  cmbSceneNodeReplaceEvent(std::size_t creationSize, std::size_t deletionSize);
  ~cmbSceneNodeReplaceEvent() override;

  void undo() override;
  void redo() override;

  void addCreatedNode(pqCMBSceneNode* node) { this->CreatedNodes.push_back(node); }

  void addDeletedNode(pqCMBSceneNode* node) { this->DeletedNodes.push_back(node); }

protected:
  std::vector<pqCMBSceneNode*> CreatedNodes;
  std::vector<pqCMBSceneNode*> DeletedNodes;
};

#endif // _cmbSceneNodeReplaceEvent_h

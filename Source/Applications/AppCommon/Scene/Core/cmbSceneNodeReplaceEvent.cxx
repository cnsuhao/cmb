/*=========================================================================

  Program:   CMB
  Module:    $RCSfile: cmbSceneNodeReplaceEvent.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME cmbSceneNodeReplaceEvent - represents a nodal replacement event.
// .SECTION Description
// .SECTION Caveats

#include "cmbSceneNodeReplaceEvent.h"
#include "pqCMBSceneTree.h"
#include "pqCMBSceneNode.h"

//-----------------------------------------------------------------------------
cmbSceneNodeReplaceEvent::cmbSceneNodeReplaceEvent(std::size_t creationSize,
                                                   std::size_t deletionSize)
{
    this->CreatedNodes.reserve(creationSize);
    this->DeletedNodes.reserve(deletionSize);
}

//-----------------------------------------------------------------------------
cmbSceneNodeReplaceEvent::~cmbSceneNodeReplaceEvent()
{
  // If we are not to apply the changes then delete the nodes created
  // Else delete the nodes that are marked for deletion
  if (!this->isApplied())
    {
    std::vector<pqCMBSceneNode*>::iterator it;
    for(it = this->CreatedNodes.begin(); it != this->CreatedNodes.end(); ++it)
      {
      delete (*it);
      }
    }
  else
    {
    std::vector<pqCMBSceneNode*>::iterator it;
    for(it = this->DeletedNodes.begin(); it != this->DeletedNodes.end(); ++it)
      {
      delete (*it);
      }
    }
}

//-----------------------------------------------------------------------------
void cmbSceneNodeReplaceEvent::undo()
{
  // Make sure the event has not been applied
  if ((!this->isApplied()) ||
      (this->CreatedNodes.empty() && this->DeletedNodes.empty()))
    {
    return;
    }
  std::vector<pqCMBSceneNode*>::iterator it;
  pqCMBSceneTree *tree;
  if (this->CreatedNodes.empty())
    {
    tree = this->DeletedNodes[0]->getTree();
    }
  else
    {
    tree = this->CreatedNodes[0]->getTree();
    }

  for(it = this->CreatedNodes.begin(); it != this->CreatedNodes.end(); ++it)
    {
    tree->detachNode(*it);
    }

  for(it = this->DeletedNodes.begin(); it != this->DeletedNodes.end(); ++it)
    {
    tree->attachNode(*it);
    }
  cmbEvent::undo();
}

//-----------------------------------------------------------------------------
void cmbSceneNodeReplaceEvent::redo()
{
  // Make sure the event has  been applied
  if (this->isApplied() ||
      (this->CreatedNodes.empty() && this->DeletedNodes.empty()))
    {
    return;
    }
  std::vector<pqCMBSceneNode*>::iterator it;
  pqCMBSceneTree *tree;
  if (this->CreatedNodes.empty())
    {
    tree = this->DeletedNodes[0]->getTree();
    }
  else
    {
    tree = this->CreatedNodes[0]->getTree();
    }

  for(it = this->CreatedNodes.begin(); it != this->CreatedNodes.end(); ++it)
    {
    tree->attachNode(*it);
    }

  for(it = this->DeletedNodes.begin(); it != this->DeletedNodes.end(); ++it)
    {
    tree->detachNode(*it);
    }
  cmbEvent::redo();
}
//-----------------------------------------------------------------------------

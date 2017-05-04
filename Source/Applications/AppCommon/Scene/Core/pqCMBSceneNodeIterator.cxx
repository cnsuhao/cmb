//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqCMBSceneNodeIterator - iterates over a Scene Sub-Tree.
// .SECTION Description
// .SECTION Caveats

#include "pqCMBSceneNodeIterator.h"
#include "pqCMBFacetedObject.h"
#include "pqCMBGlyphObject.h"
#include "pqCMBSceneNode.h"

pqCMBSceneNodeIterator::pqCMBSceneNodeIterator(pqCMBSceneNode* root)
{
  this->Root = root;
  this->reset();
}

pqCMBSceneNodeIterator::~pqCMBSceneNodeIterator()
{
}

void pqCMBSceneNodeIterator::reset()
{
  while (!this->Stack.empty())
  {
    this->Stack.pop();
  }
  this->Stack.push(Root);
}

void pqCMBSceneNodeIterator::addChildren(pqCMBSceneNode* node)
{
  const std::vector<pqCMBSceneNode*>& children = node->getChildren();
  for (std::vector<pqCMBSceneNode*>::const_reverse_iterator i = children.rbegin();
       i != children.rend(); ++i)
  {
    pqCMBSceneNode* child = *i;
    if (!child->isMarkedForDeletion())
    {
      this->Stack.push(child);
    }
  }
}

pqCMBSceneNode* pqCMBSceneNodeIterator::next()
{
  if (this->Stack.empty())
  {
    return NULL;
  }

  pqCMBSceneNode* node = this->Stack.top();
  this->Stack.pop();
  this->addChildren(node);
  return node;
}

SceneObjectNodeIterator::SceneObjectNodeIterator(pqCMBSceneNode* root)
  : pqCMBSceneNodeIterator(root)
  , NoTypeSet(true)
  , NoSurfaceTypeSet(true)
  , FilterSurfaceType(pqCMBSceneObjectBase::Other)
{
  this->FilterTypes.insert(pqCMBSceneObjectBase::Unknown);
}

SceneObjectNodeIterator::~SceneObjectNodeIterator()
{
}

pqCMBSceneNode* SceneObjectNodeIterator::next()
{
  pqCMBSceneNode* node;
  while ((node = pqCMBSceneNodeIterator::next()))
  {
    if (node->isMarkedForDeletion())
    {
      continue;
    }
    if (node->getDataObject())
    {
      if (this->NoTypeSet)
      {
        return node;
      }
      if (this->FilterTypes.count(node->getDataObject()->getType()))
      {
        if (this->NoSurfaceTypeSet)
        {
          return node;
        }
        if (node->getDataObject()->getType() == pqCMBSceneObjectBase::Faceted)
        {
          if (dynamic_cast<pqCMBFacetedObject*>(node->getDataObject())->getSurfaceType() ==
            this->FilterSurfaceType)
          {
            return node;
          }
        }
        else if (node->getDataObject()->getType() == pqCMBSceneObjectBase::Glyph)
        {
          if (dynamic_cast<pqCMBGlyphObject*>(node->getDataObject())->getSurfaceType() ==
            this->FilterSurfaceType)
          {
            return node;
          }
        }
      }
    }
  }
  return NULL;
}

void SceneObjectNodeIterator::setTypeFilter(pqCMBSceneObjectBase::enumObjectType objectType)
{
  this->NoTypeSet = false;
  this->NoSurfaceTypeSet = true;
  this->FilterTypes.clear();
  this->FilterTypes.insert(objectType);
}

void SceneObjectNodeIterator::addObjectTypeFilter(pqCMBSceneObjectBase::enumObjectType objectType)
{
  this->NoTypeSet = false;
  this->NoSurfaceTypeSet = true;
  this->FilterTypes.insert(objectType);
}

void SceneObjectNodeIterator::setTypeFilter(pqCMBSceneObjectBase::enumObjectType objectType,
  pqCMBSceneObjectBase::enumSurfaceType surfaceType)
{
  this->NoTypeSet = false;
  this->NoSurfaceTypeSet = false;
  this->FilterTypes.clear();
  this->FilterTypes.insert(objectType);
  this->FilterSurfaceType = surfaceType;
}

void SceneObjectNodeIterator::clearTypeFilter()
{
  this->NoTypeSet = true;
  this->NoSurfaceTypeSet = true;
}

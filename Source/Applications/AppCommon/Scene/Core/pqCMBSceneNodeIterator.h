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


#ifndef __pqCMBSceneNodeIterator_h
#define __pqCMBSceneNodeIterator_h

#include "cmbAppCommonExport.h"
#include <stack>
#include <set>
#include "pqCMBSceneObjectBase.h"
#include "cmbSystemConfig.h"

class pqCMBSceneNode;

class CMBAPPCOMMON_EXPORT pqCMBSceneNodeIterator
{
public:
  pqCMBSceneNodeIterator(pqCMBSceneNode *root);
  virtual ~pqCMBSceneNodeIterator();
  virtual pqCMBSceneNode *next();
  void reset();
protected:
  void addChildren(pqCMBSceneNode *node);
  std::stack<pqCMBSceneNode *> Stack;
  pqCMBSceneNode *Root;
};

class CMBAPPCOMMON_EXPORT SceneObjectNodeIterator : public pqCMBSceneNodeIterator
{
public:
  SceneObjectNodeIterator(pqCMBSceneNode *root);
  void setTypeFilter(pqCMBSceneObjectBase::enumObjectType objectType);
  void setTypeFilter(pqCMBSceneObjectBase::enumObjectType objectType,
                     pqCMBSceneObjectBase::enumSurfaceType surfaceType);
  void addObjectTypeFilter(pqCMBSceneObjectBase::enumObjectType objectType);
  void clearTypeFilter();
  virtual ~SceneObjectNodeIterator();
  virtual pqCMBSceneNode *next();
protected:
  bool NoTypeSet;
  bool NoSurfaceTypeSet;
  std::set<pqCMBSceneObjectBase::enumObjectType> FilterTypes;
  pqCMBSceneObjectBase::enumSurfaceType FilterSurfaceType;
};



#endif /* __pqCMBSceneNodeIterator_h */

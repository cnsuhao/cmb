//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME cmbEvent - The base class that represents an event that can undone and replayed.
// .SECTION Description
//  All derived classes need to redefine the undo and redo methods and then
// call the superclass's undo redo methods respecively
// .SECTION Caveats
#ifndef _cmbEvent_h
#define _cmbEvent_h

#include "cmbAppCommonExport.h"
#include "cmbSystemConfig.h"

class CMBAPPCOMMON_EXPORT cmbEvent
{

public:
  cmbEvent()
    : Applied(true)
  {
  }
  virtual ~cmbEvent() {}
  // This method is called to undo the event
  virtual void undo() { this->Applied = false; }
  // This method is called to redo (or replay) the event
  virtual void redo() { this->Applied = true; }
  // Returns true if the event is currently applied
  // i.e. it has not be undone
  bool isApplied() const { return this->Applied; }
protected:
  bool Applied;
};

#endif // !_cmbEvent_h

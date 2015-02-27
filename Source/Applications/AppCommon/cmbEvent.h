/*=========================================================================

  Program:   CMB
  Module:    cmbEvent.h

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
  cmbEvent(): Applied(true)
    {}
  virtual ~cmbEvent()
  {}
  // This method is called to undo the event
  virtual void undo()
  {
    this->Applied = false;
  }
  // This method is called to redo (or replay) the event
  virtual void redo()
  {
    this->Applied = true;
  }
  // Returns true if the event is currently applied
  // i.e. it has not be undone
  bool isApplied() const
  {
    return this->Applied;
  }
protected:
  bool Applied;
};

#endif // !_cmbEvent_h


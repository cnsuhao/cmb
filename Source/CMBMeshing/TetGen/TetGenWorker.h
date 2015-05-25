//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef TetGenWorker_h
#define TetGenWorker_h

#include <set>

#include <remus/worker/Job.h>
#include <remus/worker/Worker.h>
#include <remus/worker/ServerConnection.h>

// for TetGen itself
#include "tetgen.h"

class TetGenWorker : public remus::worker::Worker
{
public:
  TetGenWorker(remus::common::FileHandle const& fhandle,
               remus::worker::ServerConnection const& connection);

  //will get a tetgen job from the remus server
  //and call tetgen
  void meshJob();
};
#endif

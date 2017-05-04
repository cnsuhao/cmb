//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "FindFile.h"
#include "TetGenWorker.h"

#include <remus/proto/JobRequirements.h>

int main(int argc, char* argv[])
{
  remus::worker::ServerConnection connection;
  if (argc >= 2)
  {
    //let the server connection handle parsing the command arguments
    connection = remus::worker::make_ServerConnection(std::string(argv[1]));
  }

  //this is a file based mesher, we need to determine the location of the
  //file containing the xml smtk attributes. We know it should be in the exact
  //same folder as the executable, and we know its name, that should be enough.
  remus::common::FileHandle rfile(resources::FindFile("CMBMeshTetGenWorkerInput", "xml"));

  TetGenWorker worker(rfile, connection);
  worker.meshJob();
  return 1;
}

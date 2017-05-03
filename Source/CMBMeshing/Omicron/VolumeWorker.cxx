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
#include "OmicronWorker.h"

#include <remus/proto/JobRequirements.h>

int main(int argc, char* argv[])
{
  remus::worker::ServerConnection connection;
  if (argc >= 2)
  {
    //let the server connection handle parsing the command arguments
    connection = remus::worker::make_ServerConnection(std::string(argv[1]));
  }

  remus::common::MeshIOType supportedType =
    remus::common::make_MeshIOType(remus::meshtypes::SceneFile(), remus::meshtypes::Mesh3D());

  remus::proto::JobRequirements reqs =
    remus::proto::make_JobRequirements(supportedType, "CMBMeshOmicronModelWorker", "");

  OmicronWorker worker(reqs, connection);
  worker.meshJob();
  return 1;
}

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "OmicronWorker.h"
#include <remus/common/MeshIOType.h>

int main(int argc, char* argv[])
{
  remus::worker::ServerConnection connection;
  if (argc >= 2)
  {
    //let the server connection handle parsing the command arguments
    connection = remus::worker::make_ServerConnection(std::string(argv[1]));
  }

  remus::common::MeshIOType supportedType = remus::common::make_MeshIOType(
    remus::meshtypes::SceneFile(), remus::meshtypes::Mesh3DSurface());

  //memory based version for surface meshing.
  remus::proto::JobRequirements reqs =
    remus::proto::make_JobRequirements(supportedType, "CMBMeshOmicronSurfaceWorker", "");
  OmicronWorker worker(reqs, connection);
  worker.meshJob();
  return 1;
}

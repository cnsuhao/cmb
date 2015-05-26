//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef cmbmesh_omicron_worker_h
#define cmbmesh_omicron_worker_h

#include <set>

#include <remus/worker/Worker.h>
#include <remus/worker/Job.h>

#include <vector>
#include "cmbSystemConfig.h"

namespace remus{ namespace common { class ExecuteProcess; } }

//simple struct that holds all the arguments to the omicron process
//that we are going to launch. This is filled by parsing the job object given
//to us by the server.
struct omicronSettings
{
  omicronSettings(remus::worker::Job& details);
  std::string executionDirectory;
  std::string executablePath;
  std::vector<std::string> args;
  bool valid;
};

//Construct
class OmicronWorker : public remus::worker::Worker
{
public:
  //construct a worker that can mesh a single type of mesh.
  //the connection object informs the worker where the server that holds the
  //jobs is currently running.
  //By default the Omicron Worker doesn't have a executable name set
  OmicronWorker(remus::proto::JobRequirements const & reqs,
                remus::worker::ServerConnection const& conn);

  //will wait for the omicron process to close
  //before destroying self
  ~OmicronWorker();

  //will launch an omicron process, if a process is currently
  //active it will block for the previous job to finish before starting
  void meshJob();

protected:
  omicronSettings parseJobDetails();

  void launchOmicron( omicronSettings& settings );

  //waits for omicron to exit and then cleans up the OmicronProcess
  //Omicron process ptr will be NULL when this is finished
  void cleanlyExitOmicron();

  bool pollOmicronStatus( remus::worker::Job& job );

  //will halt a mesh job if one is in progress, by forcibly
  //terminating the omicron process
  bool terminateMeshJob( remus::worker::Job& job );

  remus::common::ExecuteProcess* OmicronProcess;
};

#endif

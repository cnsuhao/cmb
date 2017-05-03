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

#include <remus/common/ExecuteProcess.h>
#include <remus/common/MeshIOType.h>

#define BOOST_FILESYSTEM_VERSION 3
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include <iostream>
#include <sstream>

namespace
{

void trim(std::string& s)
{
  boost::algorithm::trim_if(s, boost::algorithm::is_cntrl());
}

bool get_value(
  const remus::proto::JobSubmission& data, const std::string& key, remus::proto::JobContent& value)
{
  typedef remus::proto::JobSubmission::const_iterator IteratorType;
  IteratorType attIt = data.find(key);
  if (attIt == data.end())
  {
    return false;
  }
  value = attIt->second;
  return true;
}

remus::proto::JobStatus make_jobFailedStatus(const remus::worker::Job& j, const std::string& reason)
{
  remus::proto::JobProgress failure_message(reason);
  remus::proto::JobStatus status(j.id(), failure_message);
  //create a status with a message marks us as IN_PROGRESS, so we need to move
  //to a FAILED state.
  status.markAsFailed();
  return status;
}
}

omicronSettings::omicronSettings(remus::worker::Job& job)
  : executionDirectory()
  , executablePath()
  , args()
  , valid(true)
{
  //job details are the executable & arguments for the omicron instance we have.
  remus::proto::JobContent content;
  const bool key_read = get_value(job.submission(), "data", content);
  this->valid = key_read;

  if (key_read)
  {
    std::stringstream buffer(std::string(content.data(), content.dataSize()));

    //parse out the execution directory
    //this is the directory we need to move to before we launch the omicron
    //process
    getline(buffer, executionDirectory, ';');
    trim(executionDirectory);

    //parse out the executable path
    getline(buffer, executablePath, ';');
    trim(executablePath);

    //get all the arguments from the string which are split by the new line char
    std::string arg;
    while (getline(buffer, arg, ';'))
    {
      trim(arg);
      args.push_back(arg);
    }
  }
}

OmicronWorker::OmicronWorker(
  remus::proto::JobRequirements const& reqs, remus::worker::ServerConnection const& conn)
  : remus::worker::Worker(reqs, conn)
  , OmicronProcess(NULL)
{
}

OmicronWorker::~OmicronWorker()
{
  this->cleanlyExitOmicron();
}

void OmicronWorker::meshJob()
{
  remus::worker::Job job = this->getJob();

  //todo verify that job is valid
  //todo verify that job isn't saying we should terminate

  omicronSettings settings(job);

  if (!settings.valid)
  {
    this->updateStatus(make_jobFailedStatus(job, "Unable to read data send from client"));
    return;
  }

  this->launchOmicron(settings);

  //poll on omicron now
  bool valid = this->pollOmicronStatus(job);
  if (valid)
  {
    //the omicron worker doesn't have any real results to send back
    remus::proto::JobResult results = remus::proto::make_JobResult(job.id(), "FAKE RESULTS");
    this->returnResult(results);
  }

  this->cleanlyExitOmicron();
}

bool OmicronWorker::terminateMeshJob(remus::worker::Job& job)
{
  if (this->OmicronProcess)
  {
    //update the server with the fact that will had to kill the job
    remus::proto::JobStatus status(job.id(), remus::FAILED);
    this->updateStatus(status);

    this->OmicronProcess->kill();
    return true;
  }
  return false;
}

void OmicronWorker::launchOmicron(omicronSettings& settings)
{
  //wait for any current process to finish before starting new one
  this->cleanlyExitOmicron();

  //save the current path
  boost::filesystem::path cwd = boost::filesystem::current_path();

  //the first thing we need to do is move to the given execution directory
  //which contains all the input files for omicron
  boost::filesystem::current_path(settings.executionDirectory);

  //make a cleaned up path with no relative
  boost::filesystem::path executePath = boost::filesystem::absolute(settings.executablePath);
  this->OmicronProcess = new remus::common::ExecuteProcess(executePath.string(), settings.args);

  //actually launch the new process
  this->OmicronProcess->execute();

  //move back to the proper directory
  boost::filesystem::current_path(cwd);
}

void OmicronWorker::cleanlyExitOmicron()
{
  //waits for omicron to exit and then cleans up the OmicronProcess
  //Omicron process ptr will be NULL when this is finished
  if (this->OmicronProcess)
  {
    delete this->OmicronProcess;
    this->OmicronProcess = NULL;
  }
}

bool OmicronWorker::pollOmicronStatus(remus::worker::Job& job)
{
  //loop on polling of the omicron process
  typedef remus::common::ProcessPipe ProcessPipe;

  //poll on STDOUT and STDERRR only
  bool validExection = true;
  remus::proto::JobProgress prog(remus::IN_PROGRESS);

  while (this->OmicronProcess->isAlive() && validExection)
  {
    //todo we need to reverify with the worker interface if the job
    //was terminated.

    //poll till we have a data, waiting for-ever!
    ProcessPipe data = this->OmicronProcess->poll(-1);
    if (data.type == ProcessPipe::STDOUT)
    {
      //we have something on the output pipe
      prog.setMessage(data.text);
      std::cout << data.text << std::endl;
      this->updateStatus(remus::proto::JobStatus(job.id(), prog));
    }
  }

  //verify we exited normally, not segfault or numeric exception
  validExection &= this->OmicronProcess->exitedNormally();

  if (!validExection)
  { //we call terminate to make sure we send the message to the server
    //that we have failed to mesh the input correctly
    this->terminateMeshJob(job);
    return false;
  }
  return true;
}

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "qtCMBMeshingMonitor.h"
#include <QtConcurrentRun>

#include "pqSMAdaptor.h"
#include "vtkProcessModule.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxy.h"
#include "vtkSMProxyManager.h"

#include <remus/client/Client.h>
#include <remus/client/ServerConnection.h>

#include <algorithm>

namespace
{

bool createRemusClient(
  remus::client::Client** client, const remus::client::ServerConnection& connection)
{
  try
  {
    *client = new remus::client::Client(connection);
    return true;
  }
  catch (...)
  {
    return false;
  }
}

template <typename T>
bool sorted_insert(std::vector<T>& v, const T& elem)
{
  typedef typename std::vector<T>::iterator iter;
  iter item = std::lower_bound(v.begin(), v.end(), elem);
  if (item == v.end() || *item != elem)
  { //insert the element if it doesn't already exist
    v.insert(item, elem);
  }
  return true;
}

template <typename T>
typename std::vector<T>::iterator erase_remove(std::vector<T>& v, const T& elem)
{
  return v.erase(std::remove(v.begin(), v.end(), elem), v.end());
}
}

qtCMBMeshingMonitor::qtCMBMeshingMonitor(const remus::client::ServerConnection& conn)
  : RemusClient(NULL)
  , LastestStatusMessages()
  , LocalServerProxy(NULL)
  , MeshingServerProxy(NULL)
  , Timer()
{

  QFuture<bool> connected = QtConcurrent::run(createRemusClient, &this->RemusClient, conn);
  if (!connected.result())
  {
    throw std::exception();
  }

  QObject::connect(&this->Timer, SIGNAL(timeout()), this, SLOT(updateJobStates()));
  this->Timer.start(250);
}

qtCMBMeshingMonitor::qtCMBMeshingMonitor(const LocalMeshServer& localProcessHandle)
  : RemusClient(NULL)
  , LastestStatusMessages()
  , LocalServerProxy(NULL)
  , MeshingServerProxy(NULL)
  , Timer()
{
  QFuture<bool> connected =
    QtConcurrent::run(createRemusClient, &this->RemusClient, localProcessHandle.Connection);
  if (!connected.result())
  {
    throw std::exception();
  }

  this->LocalServerProxy = localProcessHandle.LocalServerProxy;

  QObject::connect(&this->Timer, SIGNAL(timeout()), this, SLOT(updateJobStates()));
  this->Timer.start(250);
}

qtCMBMeshingMonitor::~qtCMBMeshingMonitor()
{
  if (this->RemusClient)
  {
    delete RemusClient;
  }

  if (this->MeshingServerProxy)
  {
    this->MeshingServerProxy->Delete();
  }

  //always delete the local server after the clients
  if (this->LocalServerProxy)
  {
    this->LocalServerProxy->Delete();
  }
}

bool qtCMBMeshingMonitor::isConnected() const
{
  return (this->RemusClient);
}

bool qtCMBMeshingMonitor::monitorJob(const remus::proto::Job& job)
{
  //fetch status for the job, so we have the initial status
  MeshingJobState state(job, this->RemusClient->jobStatus(job));
  return sorted_insert(this->LastestStatusMessages, state);
}

bool qtCMBMeshingMonitor::terminateJob(const remus::proto::Job& job)
{
  remus::proto::JobStatus recvStatus = this->RemusClient->jobStatus(job);
  if (recvStatus.failed())
  {
    MeshingJobState state(job, recvStatus);
    erase_remove(this->LastestStatusMessages, state);

    emit this->jobStatus(job, recvStatus);
    emit this->jobFailed(job, recvStatus);
    return true;
  }
  //we couldn't terminate the job, could be because
  //1. the job doesn't exist
  //2. the job has been given to a worker, so it has to be finished
  //3. the job has already been completed
  return false;
}

const remus::client::ServerConnection& qtCMBMeshingMonitor::connection() const
{
  return this->RemusClient->connection();
}

void qtCMBMeshingMonitor::updateJobStates()
{
  std::vector<MeshingJobState> jobsToRemove;

  //if the last status was finished or failed we don't need to do anything else
  typedef std::vector<MeshingJobState>::iterator iter;
  for (iter i = this->LastestStatusMessages.begin(); i != this->LastestStatusMessages.end(); ++i)
  {
    remus::proto::JobStatus recvStatus = this->RemusClient->jobStatus(i->Job);

    const bool jobFinished = recvStatus.finished();
    const bool jobFailed = recvStatus.failed();
    const bool jobStatusChanged =
      recvStatus.status() != i->Status.status() || recvStatus.progress() != i->Status.progress();

    //emit a signal if the job status is different than the previous one
    if (jobStatusChanged)
    {
      emit this->jobStatus(i->Job, recvStatus);
      i->Status = recvStatus;
    }

    //handle the use cases of finished or failed jobs, we need to mark
    //this ids as invalid, and do a second loop to remove them from
    //the map
    if (jobFinished)
    {
      jobsToRemove.push_back(*i);
      emit this->jobFinished(i->Job, recvStatus);
    }
    else if (jobFailed)
    {
      jobsToRemove.push_back(*i);
      emit this->jobFailed(i->Job, recvStatus);
    }
  }

  typedef std::vector<MeshingJobState>::const_iterator vit;
  for (vit i = jobsToRemove.begin(); i != jobsToRemove.end(); ++i)
  {
    erase_remove(this->LastestStatusMessages, *i);
  }
}

qtCMBMeshingMonitor::LocalMeshServer qtCMBMeshingMonitor::launchLocalMeshServer()
{

  //create the project manager on the data server
  qtCMBMeshingMonitor::LocalMeshServer handle;
  vtkSMProxyManager* pxm = vtkSMProxyManager::GetProxyManager();
  handle.LocalServerProxy = pxm->NewProxy("utilities", "MeshServerLauncher");
  handle.LocalServerProxy->UpdateVTKObjects();

  //update all information properties
  handle.LocalServerProxy->UpdatePropertyInformation();

  handle.Created =
    pqSMAdaptor::getElementProperty(handle.LocalServerProxy->GetProperty("IsAlive")).toBool();
  if (handle.Created)
  {
    std::string hostname =
      pqSMAdaptor::getElementProperty(handle.LocalServerProxy->GetProperty("HostName"))
        .toString()
        .toStdString();
    int portNum =
      pqSMAdaptor::getElementProperty(handle.LocalServerProxy->GetProperty("PortNumber")).toInt();

    handle.Connection = remus::client::ServerConnection(hostname, portNum);
  }

  return handle;
}

/*=========================================================================

  Program:   Visualization Toolkit
  Module:    qtCMBMeshingClient.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "qtCMBMeshingClient.h"
#include <QtGui>
#include <QtConcurrentRun>


#include "pqSMAdaptor.h"
#include "vtkProcessModule.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxy.h"
#include "vtkSMProxyManager.h"

#include <remus/client/Client.h>
#include <remus/client/ServerConnection.h>

namespace{


remus::proto::Job make_invalidJob()
{
  remus::meshtypes::MeshTypeBase baseType;
  return remus::proto::Job(boost::uuids::uuid(),
                           remus::common::MeshIOType(baseType,baseType));
}

bool createRemusClient(remus::client::Client** client,
                       const remus::client::ServerConnection& connection)
{
  try
    {
    *client = new remus::client::Client(connection);
    return true;
    }
  catch(...)
    {
    return false;
    }
}

remus::common::MeshIOType make_invalid_mesh_type()
{
  return remus::common::make_MeshIOType( remus::meshtypes::MeshTypeBase(),
                                         remus::meshtypes::MeshTypeBase() );
}

}

//-----------------------------------------------------------------------------
qtCMBMeshingClient::qtCMBMeshingClient( const remus::client::ServerConnection& conn ):
  CurrentJob( boost::uuids::uuid(), make_invalid_mesh_type() ),
  LastStatusMessage(boost::uuids::uuid(), remus::INVALID_STATUS)
{

  QFuture<bool> connected = QtConcurrent::run(createRemusClient,
                                              &this->RemusClient,
                                              conn);
  if(!connected.result())
    {
    throw std::exception();
    }

  this->LocalServerProxy = NULL;
  this->MeshingServerProxy = NULL;
}

//-----------------------------------------------------------------------------
qtCMBMeshingClient::qtCMBMeshingClient(const LocalMeshServer& localProcessHandle):
  CurrentJob( boost::uuids::uuid(), make_invalid_mesh_type() ),
  LastStatusMessage( boost::uuids::uuid(), remus::INVALID_STATUS)
{
  QFuture<bool> connected = QtConcurrent::run(createRemusClient,
                                              &this->RemusClient,
                                              localProcessHandle.Connection);
  if(!connected.result())
    {
    throw std::exception();
    }

  this->LocalServerProxy = localProcessHandle.LocalServerProxy;
  this->MeshingServerProxy = NULL;
}


//-----------------------------------------------------------------------------
qtCMBMeshingClient::~qtCMBMeshingClient()
{
  if(this->RemusClient)
    {
    delete RemusClient;
    }

  if(this->MeshingServerProxy)
    {
    this->MeshingServerProxy->Delete();
    }

  //always delete the local server after the clients
  if(this->LocalServerProxy)
    {
    this->LocalServerProxy->Delete();
    }
}


//-----------------------------------------------------------------------------
bool qtCMBMeshingClient::isConnected() const
{

  return (this->RemusClient);
}

//-----------------------------------------------------------------------------
remus::proto::Job qtCMBMeshingClient::submitJob(const std::string &jobCommand,
                                                remus::common::MeshIOType mType)
{

  QFuture<remus::proto::JobRequirementsSet> futureCanMesh =
      QtConcurrent::run(this->RemusClient,
            &remus::client::Client::retrieveRequirements, mType);

  remus::proto::Job submittedJob = make_invalidJob();

  if(futureCanMesh.result().size() > 0)
    {
    //just take the first, I only expect one worker to match currently
    remus::proto::JobSubmission sub((*futureCanMesh.result().begin()));

    sub["data"]=remus::proto::make_JobContent( jobCommand );

    submittedJob = this->RemusClient->submitJob(sub);
    }
  return submittedJob;
}

//-----------------------------------------------------------------------------
bool qtCMBMeshingClient::monitorJob(const remus::proto::Job& job)
{
  this->CurrentJob = job;
  //fetch status for the job, so we have the initial status
  this->LastStatusMessage = this->RemusClient->jobStatus(this->CurrentJob);
  return true;
}

//-----------------------------------------------------------------------------
remus::proto::JobStatus qtCMBMeshingClient::jobProgress(bool& newStatus)
{
  //if the last status was finished or failed we don't need to do anything else
  const bool jobFinished = this->LastStatusMessage.finished();
  const bool jobFailed = this->LastStatusMessage.failed();
  if(jobFinished || jobFailed)
    {
    return this->LastStatusMessage;
    }

  //otherwise fetch the status
  remus::proto::JobStatus recvStatus =
                                this->RemusClient->jobStatus(this->CurrentJob);

  if(recvStatus.id() != this->LastStatusMessage.id() ||
     recvStatus.status() != this->LastStatusMessage.status() ||
     recvStatus.progress() != this->LastStatusMessage.progress())
    {
    newStatus = true;
    this->LastStatusMessage = recvStatus;
    }

  return this->LastStatusMessage;
}

//-----------------------------------------------------------------------------
remus::proto::JobResult qtCMBMeshingClient::jobResults()
{
  remus::proto::JobResult recvResults =
                          this->RemusClient->retrieveResults(this->CurrentJob);
  return recvResults;
}

//-----------------------------------------------------------------------------
bool qtCMBMeshingClient::terminateJob()
{
  remus::proto::JobStatus recvStatus =
                                this->RemusClient->jobStatus(this->CurrentJob);
  this->LastStatusMessage = recvStatus;
  return recvStatus.failed();
}

//-----------------------------------------------------------------------------
const std::string& qtCMBMeshingClient::endpoint() const
{
  return this->RemusClient->connection().endpoint();
}

//-----------------------------------------------------------------------------
qtCMBMeshingClient::LocalMeshServer qtCMBMeshingClient::launchLocalMeshServer()
{

  //create the project manager on the data server
  qtCMBMeshingClient::LocalMeshServer handle;
  vtkSMProxyManager* pxm = vtkSMProxyManager::GetProxyManager();
  handle.LocalServerProxy = pxm->NewProxy("utilities", "MeshServerLauncher");
  handle.LocalServerProxy->UpdateVTKObjects();

  //update all information properties
  handle.LocalServerProxy->UpdatePropertyInformation();


  handle.Created = pqSMAdaptor::getElementProperty(
                      handle.LocalServerProxy->GetProperty("IsAlive")).toBool();
  if(handle.Created)
    {
    std::string hostname =
        pqSMAdaptor::getElementProperty(handle.LocalServerProxy->GetProperty(
                                          "HostName")).toString().toStdString();
    int portNum =
        pqSMAdaptor::getElementProperty(handle.LocalServerProxy->GetProperty(
                                          "PortNumber")).toInt();

    handle.Connection = remus::client::ServerConnection(hostname,portNum);
    }

  return handle;
}

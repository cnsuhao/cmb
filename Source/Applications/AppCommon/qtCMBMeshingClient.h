/*=========================================================================

  Program:   Visualization Toolkit
  Module:    qtCMBMeshingClient.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __qtCMBMeshingClient_h
#define __qtCMBMeshingClient_h

#include "cmbAppCommonExport.h"
#include <QObject>
#include <QString>
#include "cmbSystemConfig.h"

#include <remus/client/ServerConnection.h>
#include <remus/proto/Job.h>
#include <remus/proto/JobStatus.h>
#include <remus/proto/JobResult.h>

class vtkSMProxy;
namespace remus{namespace client{class Client;}}

//The meshing client is the client side interface
//to the meshing process that is happening on the local or remote
//machine. The client connects to the remus server while
//at the same time also making sure the server is connected to the remus
//server.
//
//This is done so that we can get faster response times for queries on
//progress, while allowing the server to push the heavy job data to the
//remus server

class CMBAPPCOMMON_EXPORT qtCMBMeshingClient : public QObject
{
  Q_OBJECT
public:

  //we need this object to be kept around, as
  //once a server is launched this is the only reference to
  //the server, so if we want to properly kill the server this is needs
  //to be deleted properly
  struct LocalMeshServer
    {
    LocalMeshServer():
      Created(false),
      Connection(),
      LocalServerProxy(NULL)
    {}

    bool Created;
    remus::client::ServerConnection Connection;
    vtkSMProxy* LocalServerProxy;
    };

  //Connect to the given remus server, while also
  //creating a server side class that does the same
  qtCMBMeshingClient(const remus::client::ServerConnection& conn );
  qtCMBMeshingClient(const LocalMeshServer& localProcessHandle);
  ~qtCMBMeshingClient();

  //will create a local mesh server and will return a struct containing
  //if server was constructed and the needed info to connect to it.
  static LocalMeshServer launchLocalMeshServer();

  //returns if the client is connected to a server
  bool isConnected() const;

  //submit job, returns the remus job object that represents that job.
  //if you want the meshing client to monitor said job, send that job
  //to monitorJob
  remus::proto::Job submitJob(const std::string& jobCommand,
                              remus::common::MeshIOType type);

  //instead of submitting a job, just monitor a job
  bool monitorJob(const remus::proto::Job& job);

  //progress of current running job
  remus::proto::JobStatus jobProgress(bool &newStatus);

  //retrieve the results of a job
  //Calling this method, means all future calls to this method are invalid
  //until you call monitorJob with a new remus job
  remus::proto::JobResult jobResults();

  //kill the currently running job
  //returns true if the job returns a failed status.
  bool terminateJob();

  //return the remus servers endpoint information as string
  //the string form will be tcp://ip.address:port
  //Since we are passing this as a string instead of the actual type
  //we lose out on the ability to use inproc connection type
  const std::string& endpoint() const;

private:
  remus::client::Client* RemusClient; //remus connection object to the server

  remus::proto::Job CurrentJob; //we only store a single active job at a time

  //store the last message we had from the server
  //update this each time a new status message comes in.
  remus::proto::JobStatus LastStatusMessage;

  //if the remus server is a local connection, this hold proxy on the server
  //that called execute process to make the server. Deleting this kills the
  //remus server
  vtkSMProxy* LocalServerProxy;

  //proxy to the servers connetion to the remus server, use this to send
  //heavy server side data to the remus server
  vtkSMProxy *MeshingServerProxy;

  Q_DISABLE_COPY(qtCMBMeshingClient)
};
#endif

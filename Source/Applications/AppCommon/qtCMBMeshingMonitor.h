/*=========================================================================

  Program:   Visualization Toolkit
  Module:    qtCMBMeshingMonitor.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __qtCMBMeshingMonitor_h
#define __qtCMBMeshingMonitor_h

#include "cmbAppCommonExport.h"
#include <QObject>
#include <QString>
#include <QTimer>
#include "cmbSystemConfig.h"

//Don't let QMOC see remus headers that include boost headers
//or bad things happen
#ifndef Q_MOC_RUN
  #include <remus/client/ServerConnection.h>
  #include <remus/proto/Job.h>
  #include <remus/proto/JobStatus.h>
  #include <remus/proto/JobResult.h>
#endif

#include <vector>

class vtkSMProxy;
namespace remus{namespace client{class Client;}}

//The meshing monitor is the client side interface
//to the meshing process that is happening on the local or remote
//machine. The client connects to the remus server while
//at the same time also making sure the server is connected to the remus
//server.
//
//This is done so that we can get faster response times for queries on
//progress, while allowing the server to push the heavy job data to the
//remus server

class CMBAPPCOMMON_EXPORT qtCMBMeshingMonitor : public QObject
{
  class MeshingJobState
  {
  public:
    MeshingJobState( const remus::proto::Job &j,
                     const remus::proto::JobStatus& jstatus ):
      Job( j ),
      Status( jstatus )
      { }

    bool operator<( const MeshingJobState& other ) const
      { return this->Job.id() < other.Job.id(); }

    bool operator ==(const MeshingJobState& other) const
      { return this->Job.id() == other.Job.id(); }

    bool operator !=(const MeshingJobState& other) const
      { return !(this->operator ==(other)); }

    remus::proto::Job Job;
    remus::proto::JobStatus Status;
  };

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
  qtCMBMeshingMonitor(const remus::client::ServerConnection& conn );
  qtCMBMeshingMonitor(const LocalMeshServer& localProcessHandle);
  ~qtCMBMeshingMonitor();

  //will create a local mesh server and will return a struct containing
  //if server was constructed and the needed info to connect to it.
  static LocalMeshServer launchLocalMeshServer();

  //returns if the client is connected to a server
  bool isConnected() const;

  //state that we should monitor a job
  bool monitorJob(const remus::proto::Job& job);

  //kill a running job
  //returns true if the job returns a failed status.
  bool terminateJob(const remus::proto::Job& job);

  //return the remus servers endpoint information as string
  //the string form will be tcp://ip.address:port
  //Since we are passing this as a string instead of the actual type
  //we lose out on the ability to use inproc connection type
  const std::string& endpoint() const;

signals:
  //emitted every time a job status changes. This includes
  //failures and finished events.
  void jobStatus(remus::proto::Job, remus::proto::JobStatus);

  //emitted every time a job status is changed to failed
  //once emitted we will stop tracking the job
  void jobFailed(remus::proto::Job, remus::proto::JobStatus);

  //emitted every time a job status is changed to finished
  //once emitted we will stop tracking the job
  void jobFinished(remus::proto::Job, remus::proto::JobStatus);

private slots:
  void updateJobStates();

private:
  remus::client::Client* RemusClient; //remus connection object to the server

  //store the last message we had from the server
  //update this each time a new status message comes in.
  std::vector<MeshingJobState> LastestStatusMessages;

  //if the remus server is a local connection, this hold proxy on the server
  //that called execute process to make the server. Deleting this kills the
  //remus server
  vtkSMProxy* LocalServerProxy;

  //proxy to the servers connetion to the remus server, use this to send
  //heavy server side data to the remus server
  vtkSMProxy *MeshingServerProxy;

  //Simple timer used to determine how often we poll the remus server
  //for the status of all the active jobs
  QTimer Timer;

  Q_DISABLE_COPY(qtCMBMeshingMonitor)
};
#endif

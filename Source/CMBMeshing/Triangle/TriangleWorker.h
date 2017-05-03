//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME TriangleWorker
// .SECTION Description
// Remus worker that uses triangle for meshing

#ifndef cmbmesh_triangle_worker_h
#define cmbmesh_triangle_worker_h

#include <set>

#include "cmbSystemConfig.h"
#include <remus/proto/JobResult.h>
#include <remus/worker/Job.h>
#include <remus/worker/ServerConnection.h>
#include <remus/worker/Worker.h>

// for Triangle
#ifndef ANSI_DECLARATORS
#define ANSI_DECLARATORS
#define VOID void
#endif

#ifndef REAL
#ifdef SINGLE
#define REAL float
#else /* not SINGLE */
#define REAL double
#endif /* not SINGLE */
#endif
extern "C" {
#include "triangle.h"
}
//undef triangle defines
#undef ANSI_DECLARATORS
#undef VOID
#undef TRIANGLE_REAL
// END for Triangle

//simple struct that holds all the arguments to the triangle process
//in the future this needs to be standarized as the json structure
//of the mesh job type
struct triangleParameters
{
  //this order can't change it is the order we serialize in
  bool MinAngleOn;
  bool MaxAreaOn;
  bool PreserveBoundaries;
  bool PreserveEdgesAndNodes;
  int NumberOfPoints;
  int NumberOfSegments;
  int NumberOfHoles;
  int NumberOfRegions;
  int NumberOfNodes;
  double MaxArea;
  double MinAngle;

  //holds the raw traingle data structures needed for meshing
  struct triangulateio in;
  struct triangulateio out;

  //convert the job details into the paramters needed for triangle meshing
  triangleParameters(remus::worker::Job& job);
  ~triangleParameters();

  bool valid() const { return this->NumberOfPoints >= 3 && this->NumberOfSegments >= 3; }

  //pass in the results by reference to avoid a copy when sending to
  //the server
  remus::proto::JobResult results(const remus::worker::Job& job);
};

class TriangleWorker : public remus::worker::Worker
{
public:
  TriangleWorker(remus::worker::ServerConnection const& connection);
  ~TriangleWorker();

  //will get a triangle job from the remus server
  //and will call triangle inside a its own thread to mesh the job
  void meshJob();

protected:
  bool buildTriangleArguments(const triangleParameters& params, std::string& options) const;

  void jobFailed(const remus::worker::Job& job);
};
#endif

/*=========================================================================

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/
// .NAME TriangleWorker
// .SECTION Description
// Remus worker that uses triangle for meshing

#ifndef cmbmesh_triangle_worker_h
#define cmbmesh_triangle_worker_h

#include <set>

#include <remus/worker/Job.h>
#include <remus/worker/ServerConnection.h>
#include <remus/worker/Worker.h>
#include <remus/proto/JobResult.h>
#include "cmbSystemConfig.h"

// for Triangle
#ifndef ANSI_DECLARATORS
#define ANSI_DECLARATORS
#define VOID void
#endif

#ifndef REAL
#ifdef SINGLE
#define REAL float
#else                           /* not SINGLE */
#define REAL double
#endif                          /* not SINGLE */
#endif
extern "C"
{
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

  bool valid() const
    {
    return this->NumberOfPoints >= 3 && this->NumberOfSegments >= 3;
    }


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

  bool buildTriangleArguments(const triangleParameters& params,
                              std::string& options) const;

  void jobFailed(const remus::worker::Job& job);
};
#endif

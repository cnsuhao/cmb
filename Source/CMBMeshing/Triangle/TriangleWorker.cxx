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

#include "TriangleWorker.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>

namespace
{
//----------------------------------------------------------------------------
template <typename T>
bool AllocFromStream(std::stringstream& buffer, T*& dest, int numElements)
{
  if (numElements <= 0)
  {
    return true;
  }

  //first we alloc dest;
  dest = static_cast<T*>(std::malloc(sizeof(T) * numElements));
  //now we fill it from the buffer
  char* cdest = reinterpret_cast<char*>(dest);
  const std::streamsize size = sizeof(T) * numElements;

  //strip away the new line character at the start
  if (buffer.peek() == '\n')
  {
    buffer.get();
  }

  const std::streamsize readLen = buffer.rdbuf()->sgetn(cdest, size);
  bool valid = readLen == size;
  return valid;
}

//----------------------------------------------------------------------------
template <typename T>
bool WriteToStream(std::stringstream& buffer, T* src, int numElements)
{
  if (numElements <= 0 || src == NULL)
  {
    return true;
  }
  //now we fill it from the buffer
  char* csrc = reinterpret_cast<char*>(src);
  const std::streamsize size = sizeof(T) * numElements;
  buffer.write(csrc, size);
  buffer << std::endl;
  return !buffer.bad();
}

void release_triangle_data(triangulateio* data)
{
  std::free(data->pointlist);
  std::free(data->pointattributelist);
  std::free(data->pointmarkerlist);
  std::free(data->trianglelist);
  std::free(data->triangleattributelist);
  std::free(data->trianglearealist);
  std::free(data->neighborlist);
  std::free(data->segmentlist);
  std::free(data->segmentmarkerlist);
  std::free(data->holelist);
  std::free(data->regionlist);
  std::free(data->edgelist);
  std::free(data->edgemarkerlist);
  std::free(data->normlist);

  return;
}
}

//----------------------------------------------------------------------------
triangleParameters::triangleParameters(remus::worker::Job& job)
{
  const remus::proto::JobContent& content = job.submission().find("data")->second;
  std::stringstream buffer(std::string(content.data(), content.dataSize()));

  buffer >> MinAngleOn;
  buffer >> MaxAreaOn;
  buffer >> PreserveBoundaries;
  buffer >> PreserveEdgesAndNodes;
  buffer >> NumberOfPoints;
  buffer >> NumberOfSegments;
  buffer >> NumberOfHoles;
  buffer >> NumberOfRegions;
  buffer >> NumberOfNodes;
  buffer >> MaxArea;
  buffer >> MinAngle;

  //first we init the input and output data structures to be empty
  std::memset(&this->in, 0, sizeof(triangulateio));
  std::memset(&this->out, 0, sizeof(triangulateio));

  //allocate the point list
  //copy from the string into the pointlist
  AllocFromStream(buffer, this->in.pointlist, this->NumberOfPoints * 2);

  AllocFromStream(buffer, this->in.segmentlist, this->NumberOfSegments * 2);

  if (this->NumberOfHoles > 0)
  {
    AllocFromStream(buffer, this->in.holelist, this->NumberOfHoles * 2);
  }

  if (this->NumberOfRegions > 0)
  {
    AllocFromStream(buffer, this->in.regionlist, this->NumberOfRegions * 4);
  }

  if (this->PreserveEdgesAndNodes)
  {
    this->in.numberofpointattributes = 1;
    AllocFromStream(buffer, this->in.segmentmarkerlist, this->NumberOfSegments);

    AllocFromStream(
      buffer, this->in.pointattributelist, this->NumberOfPoints * this->in.numberofpointattributes);
  }

  //make sure the input variable has all the right number of elements
  this->in.numberofsegments = this->NumberOfSegments;
  this->in.numberofpoints = this->NumberOfPoints;
  this->in.numberofholes = this->NumberOfHoles;
  this->in.numberofregions = this->NumberOfRegions;
  this->in.numberoftriangleattributes = this->NumberOfRegions > 0;
}

//----------------------------------------------------------------------------
triangleParameters::~triangleParameters()
{
  //not only do we share information between the meshing_data and the
  //input triangle data, triangle itself will share certain information between
  //the in and out structs.

  //so to get this all to work properly we need to set certain
  //points in the in and out triangle data structures to NULL
  const bool pointListShared = (this->in.pointlist == this->out.pointlist);
  const bool segmentListShared = (this->in.segmentlist == this->out.segmentlist);
  const bool segmentMarkerListShared = (this->in.segmentmarkerlist == this->out.segmentmarkerlist);
  const bool holeListShared = (this->in.holelist == this->out.holelist);
  const bool regionListShared = (this->in.regionlist == this->out.regionlist);
  const bool pointAttributeListShared =
    (this->in.pointattributelist == this->out.pointattributelist);

  //The free on this->in will release all the shared memory
  if (pointListShared)
  {
    this->out.pointlist = NULL;
  }
  if (segmentListShared)
  {
    this->out.segmentlist = NULL;
  }
  if (segmentMarkerListShared)
  {
    this->out.segmentmarkerlist = NULL;
  }
  if (holeListShared)
  {
    this->out.holelist = NULL;
  }
  if (regionListShared)
  {
    this->out.regionlist = NULL;
  }
  if (pointAttributeListShared)
  {
    this->out.pointattributelist = NULL;
  }

  release_triangle_data(&this->in);
  release_triangle_data(&this->out);
}

//----------------------------------------------------------------------------
remus::proto::JobResult triangleParameters::results(const remus::worker::Job& job)
{
  std::stringstream buffer;
  buffer << this->out.numberofpoints << std::endl;
  buffer << this->out.numberofsegments << std::endl;
  buffer << this->out.numberoftriangles << std::endl;

  WriteToStream(buffer, this->out.pointlist, this->out.numberofpoints * 2);
  WriteToStream(buffer, this->out.segmentlist, this->out.numberofsegments * 2);
  WriteToStream(buffer, this->out.trianglelist, this->out.numberoftriangles * 3);

  if (this->PreserveEdgesAndNodes)
  {
    WriteToStream(buffer, this->out.pointattributelist, this->out.numberofpoints);
    WriteToStream(buffer, this->out.segmentmarkerlist, this->out.numberofsegments);
  }

  if (this->NumberOfRegions > 0)
  {
    WriteToStream(buffer, this->out.triangleattributelist, this->out.numberoftriangles);
  }
  buffer << std::endl;
  return remus::proto::make_JobResult(job.id(), buffer.str());
}

//----------------------------------------------------------------------------
TriangleWorker::TriangleWorker(remus::worker::ServerConnection const& connection)
  : remus::worker::Worker(
      remus::proto::make_JobRequirements(
        remus::common::make_MeshIOType(remus::meshtypes::Edges(), remus::meshtypes::Mesh2D()),
        "CMBMeshTriangleWorker", ""),
      connection)
{
}
//----------------------------------------------------------------------------
TriangleWorker::~TriangleWorker()
{
}

//----------------------------------------------------------------------------
bool TriangleWorker::buildTriangleArguments(
  const triangleParameters& params, std::string& options) const
{
  bool valid = true;
  double value = 0;

  std::stringstream buffer;
  buffer << "p"; //generate a planar straight line graph
  buffer << "z"; //use 0 based indexing
  buffer << "V"; //enable quiet mode
  if (params.MaxAreaOn && params.NumberOfRegions == 0)
  {
    value = params.MaxArea;
    if (value < 0.0)
    {
      //invalid area constraint
      return false;
    }
    buffer << "a" << std::fixed << value;
  }
  else if (params.NumberOfRegions > 0)
  {
    //To get triangle region attributes use the "A" flag which does not play nice with the "a" flag
    buffer << "A";
  }
  if (params.MinAngleOn)
  {
    value = params.MinAngle;
    if (value < 0.0 || value > 33.)
    {
      //invalid area constraint
      return false;
    }
    buffer << "q" << std::fixed << value;
  }
  if (params.PreserveBoundaries)
  {
    buffer << "Y"; //preserve boundaries
  }

  //assign
  options = buffer.str();
  return valid;
}

//----------------------------------------------------------------------------
void TriangleWorker::meshJob()
{
  remus::worker::Job job = this->getJob();

  //extract the parameters of the job to launch, including the raw edges
  triangleParameters parms(job);

  bool canLaunchTriangle = false;

  std::string options;
  canLaunchTriangle = parms.valid();
  canLaunchTriangle = canLaunchTriangle && this->buildTriangleArguments(parms, options);
  if (!canLaunchTriangle)
  {
    this->jobFailed(job);
    return;
  }

  //the default triangle operation really can't fail. If it hits
  //a point where it can't mesh or fails it will just call exit which will
  //kill this worker, which will cause the remus server to mark the job
  //as failed.
  triangulate(const_cast<char*>(options.c_str()), &parms.in, &parms.out,
    static_cast<struct triangulateio*>(NULL));

  //send the data back to the server
  remus::proto::JobResult results = parms.results(job);
  this->returnResult(results);

  return;
}

//----------------------------------------------------------------------------
void TriangleWorker::jobFailed(const remus::worker::Job& job)
{
  remus::proto::JobStatus status(job.id(), remus::FAILED);
  this->updateStatus(status);
  return;
}

//undef triangle defines
#undef ANSI_DECLARATORS
#undef VOID
#undef TRIANGLE_REAL

//undef debug defines
#ifdef DUMP_DEBUG_DATA
#undef DUMP_DEBUG_DATA
#endif

int main(int argc, char* argv[])
{
  remus::worker::ServerConnection connection;
  if (argc >= 2)
  {
    //let the server connection handle parsing the command arguments
    connection = remus::worker::make_ServerConnection(std::string(argv[1]));
  }
  //the triangle worker is hardcoded to only hand 2D meshes output, and
  //input of RAW_EDGES
  TriangleWorker worker(connection);
  worker.meshJob();
  return 1;
}

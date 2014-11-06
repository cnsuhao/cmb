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

#include "TriangleWorker.h"
#include <sstream>
#include <iostream>
#include <fstream>

//#define DUMP_DEBUG_DATA

#ifdef DUMP_DEBUG_DATA
#include <time.h>
namespace
{
std::string make_timestamped_name(std::string name)
  {
  time_t theTime = time(NULL);
  struct tm *aTime = localtime(&theTime);


  const int day = aTime->tm_mday;
  const int hour = aTime->tm_hour;
  const int min = aTime->tm_min;
  const int sec = aTime->tm_sec;
  std::stringstream buffer;
  buffer << name << "-" << day << "-" << hour<< "-" << min << "-"<< sec;
  return buffer.str();
  }
}
#endif

namespace
{
//----------------------------------------------------------------------------
template<typename T>
bool AllocFromStream(std::stringstream& buffer, T* &dest, int numElements)
  {
  if(numElements <= 0)
    {return true;}

  //first we alloc dest;
  dest = static_cast<T*>( tl_alloc(sizeof(T),numElements,0) );
  //now we fill it from the buffer
  char* cdest = reinterpret_cast<char*>(dest);
  const std::streamsize size = sizeof(T)*numElements;

  //strip away the new line character at the start
  if(buffer.peek()=='\n')
    {buffer.get();}

  const std::streamsize readLen = buffer.rdbuf()->sgetn(cdest,size);
  bool valid = readLen == size;
  return valid;
  }

//----------------------------------------------------------------------------
template<typename T>
bool WriteToStream(std::stringstream& buffer, T* src, int numElements)
  {
  if(numElements <= 0 || src == NULL)
    {return true;}
  //now we fill it from the buffer
  char* csrc = reinterpret_cast<char*>(src);
  const std::streamsize size = sizeof(T)*numElements;
  buffer.write(csrc,size);
  buffer << std::endl;
  return !buffer.bad();
  }
}


//----------------------------------------------------------------------------
triangleParameters::triangleParameters(remus::worker::Job& job)
{
  const remus::proto::JobContent& content = job.submission().find("data")->second;
  std::stringstream buffer( std::string(content.data(),content.dataSize()));

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

  //for proper linking we have to call debug_initialize()
  //so that the global variables allocated_memory and max_allocated_memory
  //are properly defined
  debug_initialize();

  //first we allocate the input and output structures
  Init_triangluateio(&this->in);
  Init_triangluateio(&this->out);


  //allocate the point list
  //copy from the string into the pointlist
  AllocFromStream(buffer,this->in.pointlist,this->NumberOfPoints * 2);


  AllocFromStream(buffer,this->in.segmentlist,this->NumberOfSegments * 2);

  if (this->NumberOfHoles > 0)
    {
    AllocFromStream(buffer,this->in.holelist,this->NumberOfHoles * 2);
    }

  if (this->NumberOfRegions > 0)
    {
    AllocFromStream(buffer,this->in.regionlist,this->NumberOfRegions * 4);
    }

  if (this->PreserveEdgesAndNodes)
    {
    this->in.numberofpointattributes = 1;
    AllocFromStream(buffer,this->in.segmentmarkerlist,this->NumberOfSegments);

    AllocFromStream(buffer,this->in.pointattributelist,
                    this->NumberOfPoints * this->in.numberofpointattributes);
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
  //there is a bug in triangle that the hole list is shared between
  //the in and out structs. So we have to set the hole list to NULL before
  //freeing the out, or the program will crash
  bool pointListShared = (this->in.pointlist == this->out.pointlist);
  bool segmentListShared = (this->in.segmentlist == this->out.segmentlist);
  bool segmentMarkerListShared = (this->in.segmentmarkerlist == this->out.segmentmarkerlist);
  bool holeListShared = (this->in.holelist == this->out.holelist);
  bool regionListShared = (this->in.regionlist == this->out.regionlist);
  bool pointAttributeListShared = (this->in.pointattributelist == this->out.pointattributelist);

  Free_triangluateio(&this->in);
  if (pointListShared)
    {
    //The free on TIO->in released the memory
    this->out.pointlist=NULL;
    }
  if (segmentListShared)
    {
    //The free on TIO->in released the memory
    this->out.segmentlist=NULL;
    }
  if (segmentMarkerListShared)
    {
    //The free on TIO->in released the memory
    this->out.segmentmarkerlist=NULL;
    }
  if (holeListShared)
    {
    //The free on TIO->in released the memory
    this->out.holelist=NULL;
    }
  if (regionListShared)
    {
    this->out.regionlist=NULL;
    }
  if (pointAttributeListShared)
    {
    this->out.pointattributelist=NULL;
    }
  Free_triangluateio(&this->out);
}


//----------------------------------------------------------------------------
remus::proto::JobResult triangleParameters::results(
                                                const remus::worker::Job& job)
{
  std::stringstream buffer;
  buffer << this->out.numberofpoints << std::endl;
  buffer << this->out.numberofsegments << std::endl;
  buffer << this->out.numberoftriangles << std::endl;

  WriteToStream(buffer,this->out.pointlist, this->out.numberofpoints*2);
  WriteToStream(buffer,this->out.segmentlist, this->out.numberofsegments*2);
  WriteToStream(buffer,this->out.trianglelist, this->out.numberoftriangles*3);

  if (this->PreserveEdgesAndNodes)
    {
    WriteToStream(buffer,this->out.pointattributelist, this->out.numberofpoints);
    WriteToStream(buffer,this->out.segmentmarkerlist, this->out.numberofsegments);
    }

  if(this->NumberOfRegions > 0)
    {
    WriteToStream(buffer,this->out.triangleattributelist, this->out.numberoftriangles);
    }
  buffer << std::endl;
  return remus::proto::make_JobResult(job.id(), buffer.str());
}

//----------------------------------------------------------------------------
TriangleWorker::TriangleWorker(remus::worker::ServerConnection const& connection):
  remus::worker::Worker(remus::proto::make_JobRequirements(
      remus::common::make_MeshIOType(remus::meshtypes::Edges(),
                                     remus::meshtypes::Mesh2D()),
      "CMBMeshTriangleWorker",
      ""),
      connection)
{
}
//----------------------------------------------------------------------------
TriangleWorker::~TriangleWorker()
{
}

//----------------------------------------------------------------------------
bool TriangleWorker::buildTriangleArguments(const triangleParameters &params,
                                            std::string &options) const
{
  bool valid = true;
  double value = 0;

  std::stringstream buffer;
  buffer << "p";//generate a planar straight line graph
  buffer << "z";//use 0 based indexing
  buffer << "V";//enable quiet mode
  if(params.MaxAreaOn && params.NumberOfRegions == 0)
    {
    value = params.MaxArea;
    if (value < 0.0)
      {
      //invalid area constraint
      return false;
      }
    buffer << "a" << std::fixed << value;
    }
  else if(params.NumberOfRegions > 0)
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
    buffer << "Y";//preserve boundaries
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
  canLaunchTriangle = canLaunchTriangle && this->buildTriangleArguments(parms,
                                                                        options);
  if (!canLaunchTriangle)
    {
    this->jobFailed(job);
    return;
    }

#ifdef DUMP_DEBUG_DATA
  {
  std::string fname = make_timestamped_name("in_dump");
  char* raw = const_cast<char*>(fname.c_str());
  triangle_report_vtk(raw,&parms.in);
  }
#endif

  int ret = triangulate(const_cast<char*>(options.c_str()),
                        &parms.in,&parms.out,static_cast<struct triangulateio*>(NULL));

  if ( ret != 0 )
    {
    //triangle failed to mesh properly
    //C functions return not zero on failure
    this->jobFailed(job);
    return;
    }

#ifdef DUMP_DEBUG_DATA
  {
  std::string fname = make_timestamped_name("out_dump");
  char* raw = const_cast<char*>(fname.c_str());
  triangle_report_vtk(raw,&parms.out);
  }
#endif

  //send the data back to the server
  remus::proto::JobResult results = parms.results(job);
  this->returnResult(results);

  return;
}

//----------------------------------------------------------------------------
void TriangleWorker::jobFailed(const remus::worker::Job& job)
{
  remus::proto::JobStatus status(job.id(),remus::FAILED);
  this->updateStatus(status);
  return;
}

//undef triangle defines
#undef ANSI_DECLARATORS
#undef VOID
#undef TRIANGLE_REAL

//undef debug defines
#ifdef DUMP_DEBUG_DATA
# undef DUMP_DEBUG_DATA
#endif

int main (int argc, char* argv[])
{
  remus::worker::ServerConnection connection;
  if(argc>=2)
    {
    //let the server connection handle parsing the command arguments
    connection = remus::worker::make_ServerConnection(std::string(argv[1]));
    }
  //the triangle worker is hardcoded to only hand 2D meshes output, and
  //input of RAW_EDGES
  TriangleWorker worker(connection );
  worker.meshJob();
  return 1;
}

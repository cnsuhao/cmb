
//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================
#include "TetGenWorker.h"

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#include <smtk/attribute/Attribute.h>
#include <smtk/attribute/Definition.h>
#include <smtk/attribute/DoubleItem.h>
#include <smtk/attribute/IntItem.h>
#include <smtk/attribute/System.h>
#include <smtk/io/AttributeReader.h>
#include <smtk/io/AttributeWriter.h>
#include <smtk/io/Logger.h>

#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include "KnownFaces.h"
#include "VTKConverters.h"

namespace detail
{

//----------------------------------------------------------------------------
struct tetgen_wrapper
{
  tetgenbehavior behavior;
  tetgenio input;
  tetgenio output;

  std::vector<REAL> points;
  std::vector<int> cellClassification;

  //Stores the triangles in the input mesh, so that we can find the
  //sides of the tetgen that are part of the input mesh once we are done.
  //this is only needed to write out the bc file
  detail::KnownFaces knownFaces;

  void setPoints(const std::vector<REAL>& p)
  {
    points = p; //copy p to points so we can reference it
    input.numberofpoints = (p.size() / 3); //stored as flat double
    input.pointlist = &(points[0]); //this doesn't work since points isn't kept

  }

  void setClassification(const std::vector<int>& c)
  {

  cellClassification = c; //copy c so we can reference it
  input.numberoffacets = c.size();
  input.facetmarkerlist = &(cellClassification[0]);
  }

  void parse_behavior(const std::string& commandline_flags)
  {
    //parse_commandline requires a c str, not a const c str
    //plus parse_commandline could modify the str so don't pass it
    //a const_casted c_str()
    char* args = new char[commandline_flags.size()+1];
    std::copy(commandline_flags.begin(), commandline_flags.end(), args);
    args[commandline_flags.size()]='\0';
    this->behavior.parse_commandline(args);
    delete[] args;
  }

  tetgen_wrapper() {}
  ~tetgen_wrapper()
  {
    //set certain parts of input to NULL since we delete them
    this->input.pointlist = NULL;
    this->input.facetmarkerlist = NULL;
  }

  tetgen_wrapper(const tetgen_wrapper& other):
    behavior(other.behavior),
    input(other.input),
    output(other.output),
    points(other.points),
    cellClassification(other.cellClassification)
  {
  }

  tetgen_wrapper & operator= (tetgen_wrapper other)
  {
    std::swap(behavior, other.behavior);
    std::swap(input, other.input);
    std::swap(output, other.output);
    std::swap(points, other.points);
    std::swap(cellClassification, other.cellClassification);
    return *this;
  }

  bool tetrahedralize()
  {
    bool result = true;
    try
      {
      ::tetrahedralize(&this->behavior,
                       &this->input,
                       &this->output);
      }
    catch(int error) {  result = false; }
    return true;
  }

};

//----------------------------------------------------------------------------
std::string make_tetgenFlags( const remus::proto::JobContent& rawInstance )
{
  //p mesh
  //z zero based
  //M merge coplanar
  //A have region attribute
  //YY Preserve Surface Mesh
  const std::string default_flags = "pzMAYY";

  smtk::attribute::System manager;
  smtk::io::AttributeReader reader;
  smtk::io::Logger inputLogger;

  bool err = false;
  err = reader.readContents(manager,
                            rawInstance.data(),
                            rawInstance.dataSize(),
                            inputLogger);

  if(err)
    {
    return default_flags;
    }

  std::vector<smtk::attribute::AttributePtr> tetGenAttributes;
  manager.findAttributes("TetGen",tetGenAttributes);

  if(tetGenAttributes.size() != 1)
    {
    return default_flags;
    }

  const smtk::attribute::AttributePtr& attribute = tetGenAttributes[0];
  smtk::attribute::ConstDoubleItemPtr volumeCon =
                                          attribute->findDouble("VolumeConstraint");
  const double volumeConValue = volumeCon->value();

  smtk::attribute::ConstItemPtr mergeCoplanar = attribute->find("MergeCoplanarFacets");
  const bool mergeCoplanarIsEnabled = mergeCoplanar->isEnabled();

  smtk::attribute::ConstIntItemPtr preserveMode = attribute->findInt("PreservationMode");
  const int preserveModeIndex = preserveMode->discreteIndex();
  const std::string preserveModeAsString = preserveMode->valueAsString();

  smtk::attribute::ConstItemPtr verbosity = attribute->find("VerboseMode");
  const bool verboseIsEnabled = verbosity->isEnabled();

  std::stringstream buffer;
  buffer << "pzA"; //we always have a region attribute
  //append the volume constraint
  buffer << "a" << volumeConValue;

  if(mergeCoplanarIsEnabled)
    {
    buffer << "M";
    }

  //preserveModeIndex === 0 means do nothing so not part of the if statement
  if(preserveModeIndex == 1)
    {
    //preserve the Surface mesh
    buffer << "YY";
    }
  else if(preserveModeIndex == 2)
    {
    //preserve the exterior Surface mesh
    buffer << "Y";
    }

  if(verboseIsEnabled)
    {
    buffer << "V";
    }
  return buffer.str();
}

//----------------------------------------------------------------------------
detail::tetgen_wrapper make_tetgenInput( const remus::proto::JobContent& discreteMesh )
{
  std::istringstream serializedDiscreteMesh;
  { //copy discreteMesh into the stringstream
  const std::string discreteMeshStr(discreteMesh.data(),
                                    discreteMesh.dataSize());
  serializedDiscreteMesh.str(discreteMeshStr);
  } //release discreteMeshStr

  //construct the tetgen wrapper now input
  tetgen_wrapper wrapper;
  tetgenio& input = wrapper.input; //reference our wrappers input data-structure

  //read in a vtkDataArray and convert to a REAL vector
  wrapper.setPoints( readAndConvert_vtkDataArray<REAL>(serializedDiscreteMesh) );

  //read in a vtkDataArray and convert to an int vector
  std::vector<int> cellsConnections = readAndConvert_vtkDataArray<int>(serializedDiscreteMesh);

  //read in a vtkDataArray and convert to an int vector
  wrapper.setClassification( readAndConvert_vtkDataArray<int>(serializedDiscreteMesh) );

  //read in a vtkDataArray and convert to an REAL vector
  std::vector<REAL> regionInfo = readAndConvert_vtkDataArray<REAL>(serializedDiscreteMesh);

  const std::size_t numCells = input.numberoffacets;
  input.facetlist = new tetgenio::facet[input.numberoffacets];

  //cellsConnections is a pure vtk cell array of triangles, so it is:
  // 3, x, y, z, 3, x2, y2, z2, 3, x3, y3, z3, etc
  //which means each cell has length 4 ( x,y,z plus cell type id)
  for (std::size_t i = 0, cellIndex=0; i < numCells; i++, cellIndex +=4)
    {
    tetgenio::facet& f = input.facetlist[i];
    f.numberofpolygons = 1;
    f.polygonlist = new tetgenio::polygon[f.numberofpolygons];
    f.numberofholes = 0;
    f.holelist = NULL;
    tetgenio::polygon& p = f.polygonlist[0];
    p.numberofvertices = 3;
    p.vertexlist = new int[p.numberofvertices];

    //construct a face to be stored in the known face list
    detail::Face face(cellsConnections[cellIndex+1],
                      cellsConnections[cellIndex+2],
                      cellsConnections[cellIndex+3],
                      wrapper.cellClassification[i],
                      i);
    wrapper.knownFaces.add(face);

    //add the points of the face to the polygon
    p.vertexlist[0] = face.p1;
    p.vertexlist[1] = face.p2;
    p.vertexlist[2] = face.p3;
    }


  //now convert the regionInfo over to a region list
  //the region info is of 4 components ( region id, x,y,z of point),
  //while regionlist is of size 5
  input.numberofregions = regionInfo.size() / 4;
  input.regionlist = new REAL[input.numberofregions * 5];

  for(int i=0; i < input.numberofregions; ++i)
  {
    input.regionlist[(i * 5) + 0] = regionInfo[(i*4)+0];
    input.regionlist[(i * 5) + 1] = regionInfo[(i*4)+1];
    input.regionlist[(i * 5) + 2] = regionInfo[(i*4)+2];
    input.regionlist[(i * 5) + 3] = regionInfo[(i*4)+3];
    input.regionlist[(i * 5) + 4] = 0.0;
  }

  input.numberofedges = 0;

  return wrapper;

}


//3dm is a 1 based file format, so we need to add 1 to every
//index we write out
bool write_3dm_file(const std::string path, const tetgenio& output)
{
  std::ofstream outputFile(path.c_str(), std::ios::out);


  bool fileWritten = false; //holds if we wrote the file properly
  const bool can_write = outputFile.is_open(); //verify file is open

  if(can_write)
  {
  outputFile << "MESH3D" << std::endl;
  for (int i=0; i < output.numberoftetrahedra; ++i)
    {
    //important to note that coming out of tetgen we need to convert the
    //tetra attribute to an integer value, and we want to find the 'floor'
    //secondly we need to make sure that material_id is written out in
    //its natural form, and we don't force it into 1 base form
    const int material_id(static_cast<int>(output.tetrahedronattributelist[i]));
    outputFile << "E4T \t " << setw(8) << 1 + i << " "
                            << setw(8) << 1 + output.tetrahedronlist[4 * i] << " "
                            << setw(8) << 1 + output.tetrahedronlist[(4 * i)+1] << " "
                            << setw(8) << 1 + output.tetrahedronlist[(4 * i)+2] << " "
                            << setw(8) << 1 + output.tetrahedronlist[(4 * i)+3] << " "
                            << setw(8) << material_id << std::endl;
    }

  for(int i=0; i < output.numberofpoints; ++i)
    {
    outputFile << "ND \t "
               << setw(8) << 1 + i << " " << std::fixed
               << setw(12) <<  output.pointlist[3 * i] << " "
               << setw(12) <<  output.pointlist[(3 * i)+1] << " "
               << setw(12) <<  output.pointlist[(3 * i)+2] << std::endl;
    }

  outputFile << "END" << std::endl;

  //verify that the writes were good
  fileWritten = outputFile.good();
  outputFile.close();
  }

  return (can_write && fileWritten);
}

//----------------------------------------------------------------------------
bool write_bc_file(const std::string path,
                   const detail::KnownFaces& knownFaces,
                   const tetgenio& output)
{
  std::ofstream outputFile(path.c_str(), std::ios::out);


  bool fileWritten = false; //holds if we wrote the file properly
  const bool can_write = outputFile.is_open(); //verify file is open

  const int lookupTable[4][3] = { {0,1,2},
                                  {1,3,2},
                                  {0,3,2},
                                  {0,1,3},
                                };

  const int tetFaceSideTable[4] = {4, 1, 2, 3};

  if(can_write)
  {
  for (int i=0; i < output.numberoftetrahedra; ++i)
    {
    const int offset = 4 * i;
    for(int lt_index=0; lt_index < 4; ++lt_index)
      {
      const int offset_index_1 = offset + lookupTable[lt_index][0];
      const int offset_index_2 = offset + lookupTable[lt_index][1];
      const int offset_index_3 = offset + lookupTable[lt_index][2];

      const detail::Face faceToFind(output.tetrahedronlist[offset_index_1],
                                    output.tetrahedronlist[offset_index_2],
                                    output.tetrahedronlist[offset_index_3]);

      //query to see if this face is valid
      const detail::Face faceWithInfo = knownFaces.get(faceToFind);

      //now that we have a valid face, write it out
      if(faceWithInfo.valid())
        {
        //the face is sorted, so we need to print the original order that
        //is contained in the tetrahedronlist so that ModelBuilder can
        //map this file back to the shell it has stored in memory
        outputFile  << "FCS " << 1 + i << " "
                  << tetFaceSideTable[lt_index] << " "
                  << faceWithInfo.faceId << " "
                  << 1 + output.tetrahedronlist[offset_index_1] << " "
                  << 1 + output.tetrahedronlist[offset_index_2] << " "
                  << 1 + output.tetrahedronlist[offset_index_3] << " "
                  << 1 + faceWithInfo.surfaceId << std::endl;
        }
      }
    }
  fileWritten = outputFile.good();
  outputFile.close();
  }
  return (can_write && fileWritten);
}

//----------------------------------------------------------------------------
bool get_value(const remus::proto::JobSubmission& data, const std::string& key,
               remus::proto::JobContent& value)
{
  typedef remus::proto::JobSubmission::const_iterator IteratorType;
  IteratorType attIt = data.find(key);
  if(attIt == data.end())
    {
    return false;
    }
  value = attIt->second;
  return true;
}

//----------------------------------------------------------------------------
std::string make_outputFile(const std::string& input, const std::string newExt)
{
  //remove from the inputPath the extension and the period
  const boost::filesystem::path inputPath = boost::filesystem::absolute(input);

  boost::filesystem::path input_name = inputPath.stem();
  boost::filesystem::path directory_to_use(inputPath);
  directory_to_use = directory_to_use.remove_filename();

  //construct the new output path

  boost::filesystem::path outputFile  =
                        directory_to_use /= input_name += "_output";

  //we use replace_extension instead of append newExt the following reason.
  //replace_extension handles the use case of newExt having or not having
  //a leading "." character.
  outputFile.replace_extension(newExt);

  return outputFile.string();
}

//----------------------------------------------------------------------------
void send_jobFailedStatus(  remus::worker::Worker * const w,
                           const remus::worker::Job &j,
                           const std::string& reason )
{
  const remus::proto::JobProgress failure_message(reason);
  remus::proto::JobStatus status(j.id(),failure_message);
  //create a status with a message marks us as IN_PROGRESS, so we need to move
  //to a FAILED state.
  status.markAsFailed();
  w->updateStatus(status);
}

//----------------------------------------------------------------------------
void send_jobProgress( remus::worker::Worker * const w,
                       const remus::worker::Job &j,
                       const std::string& progress )
{
  const remus::proto::JobProgress progress_message(progress);
  remus::proto::JobStatus status(j.id(),progress_message);
  w->updateStatus(status);
}

}

//----------------------------------------------------------------------------
TetGenWorker::TetGenWorker(remus::common::FileHandle const& fhandle,
                           remus::worker::ServerConnection const& connection):
  remus::worker::Worker(
    remus::proto::make_JobRequirements(
      remus::common::make_MeshIOType(remus::meshtypes::PiecewiseLinearComplex(),
                                     remus::meshtypes::Mesh3D()),
      "CMBMeshTetGenWorker",
      fhandle,
      remus::common::ContentFormat::XML),
    connection)
{
}
//----------------------------------------------------------------------------
void TetGenWorker::meshJob()
{
  remus::worker::Job j = this->getJob();
  if (!j.valid())
    {
    detail::send_jobFailedStatus(this,j,"Invalid Job Given to Worker");
    return;
    }

  const remus::proto::JobSubmission& submission = j.submission();

  //verify that we have instance, model_file, and discrete_mesh.
  remus::proto::JobContent rawInstance;
  remus::proto::JobContent modelFilePath;
  remus::proto::JobContent discreteMeshData;

  //todo move get_value into JobSubmission
  bool valid;
  valid = detail::get_value(submission, "instance", rawInstance);
  valid = valid && detail::get_value(submission, "model_file_path", modelFilePath);
  valid = valid && detail::get_value(submission, "discrete_mesh", discreteMeshData);
  if (!j.valid())
    {
    detail::send_jobFailedStatus(this,j,"Invalid Job Submission to Worker");
    return;
    }
  detail::send_jobProgress(this,j,"Parsed Job Submission");


  // 1. Convert the discrete mesh data to the data form that tetgen requires.
  //    this info includes all the points, the cell connections, what
  //    region each cell is part of, the region ids and a point inside each region
  detail::send_jobProgress(this,j,"Constructing TetGen Input Mesh");
  detail::tetgen_wrapper tetInfo = detail::make_tetgenInput(discreteMeshData);


  //2. Parse the attribute system to build the tetgen command line arguments
  detail::send_jobProgress(this,j,"Constructing TetGen Control Flags");
  const std::string tetoptions = detail::make_tetgenFlags(rawInstance);
  std::cout << "tetoptions: " << tetoptions << std::endl;
  tetInfo.parse_behavior(tetoptions);

  detail::send_jobProgress(this,j,"Starting TetGen");
  //finally call tetgen
  bool mesher_ran = tetInfo.tetrahedralize();
  if(!mesher_ran)
    {
    detail::send_jobFailedStatus(this,j,"TetGen crashed while meshing");
    return;
    }
  detail::send_jobProgress(this,j,"TetGen Finished");

  //inspect the output of tetgen, if it doesn't have any tets and points
  //we know that tetgen failed to mesh, but didn't crash
  if(tetInfo.output.numberofpoints == 0 &&
     tetInfo.output.numberoftetrahedra == 0)
    {
    detail::send_jobFailedStatus(this,j,"TetGen failed to create a mesh");
    return;
    }


  //add all output generates triangles to the known face list
  //this is needed so that we can write out a correct bc file
  for (int i = 0; i < tetInfo.output.numberoftrifaces; i++)
    {
    if(tetInfo.output.trifacemarkerlist[i] < 0)
      {
      const detail::Face faceToAdd(tetInfo.output.trifacelist[3 * i],
                                   tetInfo.output.trifacelist[3 * i + 1],
                                   tetInfo.output.trifacelist[3 * i + 2],
                                   tetInfo.output.trifacemarkerlist[i],
                                   -1);
      tetInfo.knownFaces.add(faceToAdd);
      }
    }

  //now take the tetInfo.output and write out a 3dm and bc file.
  //for now we use the information in modelFilePath to create an output
  //file, whose path we will send back as our results
  detail::send_jobProgress(this,j,"Generate Output Mesh File");
  const std::string inputFilepath(modelFilePath.data(),
                                  modelFilePath.dataSize());


  const std::string output3DMFileName = detail::make_outputFile(inputFilepath,
                                                             "3dm");

  const std::string outputBCFileName = detail::make_outputFile(inputFilepath,
                                                             "bc");

  //write out first the 3dm file, and than the bc file
  bool didWrite = detail::write_3dm_file(output3DMFileName,tetInfo.output);
  didWrite = didWrite && detail::write_bc_file(outputBCFileName,
                                               tetInfo.knownFaces,
                                               tetInfo.output);
  if(!didWrite)
    {
    detail::send_jobFailedStatus(this,j,
                                 "Failed to save generated mesh to file");
    return;
    }

  //we send back as a string instead of a remus::common::FileHandle on purpose
  //in the future FileHandle will support reading and transmitting the contents
  //of the file, and for this worker we want to send back the exact path, not
  //the file contents
  remus::proto::JobResult result = remus::proto::make_JobResult(j.id(),
                                                            output3DMFileName);

  //return the results
  detail::send_jobProgress(this,j,"Sending Results");
  this->returnResult(result);
}

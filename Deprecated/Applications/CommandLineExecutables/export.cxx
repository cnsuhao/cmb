/*=========================================================================

Copyright (c) 2013 Kitware Inc. 28 Corporate Drive,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced, distributed,
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
// .NAME export.cxx - Command line export application
// .SECTION Description
// .SECTION See Also


#include <iostream>
#include <string>
#include <vtksys/CommandLineArguments.hxx>
#include <vtksys/SystemTools.hxx>

#include "DefaultExportTemplate.h"
#include "vtkCMBStandaloneMesh.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/Item.h"
#include "smtk/attribute/Manager.h"
#include "smtk/attribute/StringItem.h"
#include "smtk/io/Logger.h"
#include "smtk/io/AttributeReader.h"
#include "smtk/common/ResourceSet.h"
#include "smtk/common/ResourceSetReader.h"
#include "smtkModel.h"
#include "vtkCMBMeshGridRepresentationServer.h"
#include "vtkCMBMeshToModelReadOperator.h"
#include "vtkCMBModelReadOperator.h"
#include "vtkDiscreteModel.h"
#include "vtkDiscreteModelWrapper.h"
#include "vtkFieldData.h"
#include "vtkPolyData.h"
#include "vtkPythonExporter.h"
#include "vtkSmartPointer.h"



//----------------------------------------------------------------------------
// Returns path (string) to python executable, which is needed to initialize exporter correctly
// If not successful, returns empty string
std::string find_python_executable(std::string exec_path, std::string python_exec_arg)
{
  std::string python_path;

  // 1. Check relative path (for installed versions)
  std::string real_exe_path = vtksys::SystemTools::GetRealPath(exec_path.c_str());
  std::string exe_dir = vtksys::SystemTools::GetParentDirectory(real_exe_path.c_str());
  //std::cout << "exe_dir" <<  exe_dir << "\n";

  std::string ext = vtksys::SystemTools::GetExecutableExtension();
  std::string python_exec = std::string("pvpython") + ext;
  // Note: don't use SystemTools::JoinPath() here, because it doesn't insert 1st "/"
  std::string test_path = exe_dir + "/" + python_exec;
  if (vtksys::SystemTools::FileExists(test_path.c_str()))
    {
    python_path = test_path;
    }

  // 2. Check program argument 7
  if (!python_exec_arg.empty())
    {
    if (vtksys::SystemTools::FileExists(python_exec_arg.c_str()))
      {
      python_path = python_exec_arg;
      }
    else
      {
      std::cout << "WARNING: File not found for specified python executable"
                << " (" << python_exec_arg << ")."
                << std::endl;
      }
    }

  // 3. Check for the python executable used at build time
#ifdef CMAKE_PYTHON_EXECUTABLE
  if (python_path.empty())
    {
    test_path = CMAKE_PYTHON_EXECUTABLE;
    if (vtksys::SystemTools::FileExists(test_path.c_str()))
      {
      python_path = test_path;
      }
    }
#endif  // CMAKE_PYTHON_EXECUTABLE

  if (python_path.empty())
    {
    std::cout << "ERROR: Unable to find python executable.\n";
#ifdef CMAKE_PYTHON_VERSION
    std::cout << "       You can try using your system's python, but note that is must be version "
              << CMAKE_PYTHON_VERSION << "\n";
#endif
    }
  else
    {
    std::cout << "Using python executable: " << python_path;
    }
    std::cout << std::endl;

  return python_path;
}


//----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
  // Command line arguments
  std::vector<std::string> analysis_names;
  std::string cmb_model_path;
  std::string grid_path;
  std::string m2m_path;
  std::string output_path;
  std::string python_script_path;
  std::string sim_path;
  std::string python_exec_arg;

  vtksys::CommandLineArguments args;
  args.Initialize(argc, argv);
  args.AddArgument("-a", vtksys::CommandLineArguments::MULTI_ARGUMENT,
    &analysis_names, "Analysis Name(s) to export");
  args.AddArgument("-c", vtksys::CommandLineArguments::SPACE_ARGUMENT,
    &cmb_model_path, "[REQUIRED] CMB model file (*.cmb)");
  args.AddArgument("-g", vtksys::CommandLineArguments::SPACE_ARGUMENT,
    &grid_path, "Analysis Grid file (*.2dm, *.3dm)");
  args.AddArgument("-m", vtksys::CommandLineArguments::SPACE_ARGUMENT,
    &m2m_path, "MeshToModel info file (*.m2m), used with -g option)");
  args.AddArgument("-o", vtksys::CommandLineArguments::SPACE_ARGUMENT,
    &output_path, "Output file");
  args.AddArgument("-p", vtksys::CommandLineArguments::SPACE_ARGUMENT,
    &python_script_path, "[REQUIRED] Python script (*.py)");
  args.AddArgument("-s", vtksys::CommandLineArguments::SPACE_ARGUMENT,
    &sim_path, "Simulation data (attributes) file (*.crf, *.sbi)");
  args.AddArgument("-y", vtksys::CommandLineArguments::SPACE_ARGUMENT,
    &python_exec_arg, "Python executable (path)");

  if (!args.Parse())
    {
    std::cout << args.GetHelp() << std::endl;
    return -1;
    }

  // Check required inputs
  if (cmb_model_path.empty())
    {
    std::cout << "CANNOT RUN: No CMB Model file specified" << "\n"
              << args.GetHelp() << std::endl;
    return -2;
    }

    // Check required inputs
  if (python_script_path.empty())
    {
    std::cout << "CANNOT RUN: No Python script specified" << "\n"
              << args.GetHelp() << std::endl;
    return -2;
    }

  std::string python_exec = find_python_executable(argv[0], python_exec_arg);
  if (python_exec.empty())
    {
    std::cout << "CANNOT RUN: Cannot find Python executable - try -y option" << "\n"
              << args.GetHelp() << std::endl;
    return -3;
    }

  // Read cmb model
  //const char *cmb_path = "/media/ssd/sim/cmb_core/git/CMB/Source/TestingData/test2D.cmb";
  std::cout << "Loading CMBModel: " << cmb_model_path << std::endl;
  vtkDiscreteModelWrapper* modelWrapper = vtkDiscreteModelWrapper::New();
  vtkDiscreteModel* Model = modelWrapper->GetModel();

  vtkSmartPointer<vtkCMBModelReadOperator> cmbReader =
    vtkSmartPointer<vtkCMBModelReadOperator>::New();
  cmbReader->SetFileName(cmb_model_path.c_str());
  cmbReader->Operate(modelWrapper);
  if(cmbReader->GetOperateSucceeded() == false)
    {
    vtkGenericWarningMacro("Could not load file " << cmb_model_path);
    return 1;
    }

  int num_verts = Model->GetNumberOfAssociations(vtkModelVertexType);
  int num_edges = Model->GetNumberOfAssociations(vtkModelEdgeType);
  int num_faces = Model->GetNumberOfAssociations(vtkModelFaceType);
  std::cout << "Model Num verts " << num_verts
            << ", Num edges " << num_edges
            << ", Num faces " << num_faces
            << std::endl;


  // Load analysis mesh and save geometry (vtkPolyData)
  vtkPolyData *gridPolyData = NULL;
  if (!grid_path.empty())
    {
    //const char * mesh_path = "/home/john/projects/cmb/solvers/ADH/CMB3.0/test2D.2dm";
    vtkCMBStandaloneMesh *Mesh = vtkCMBStandaloneMesh::New();
    std::cout << "Loading analysis grid file: " << grid_path << std::endl;
    bool mesh_ok = Mesh->Load(grid_path.c_str());
    std::cout << "OK?: " << mesh_ok << std::endl;
    if (mesh_ok)
      {
     Mesh->Initialize(Model);  // must do this *after* loading mesh file!

     //Generate analysis grid
     vtkCMBMeshGridRepresentationServer *gridRep =
       vtkCMBMeshGridRepresentationServer::New();
     gridRep = vtkCMBMeshGridRepresentationServer::New();
     bool init_ok = gridRep->Initialize(Mesh);
     std::cout << "Initialize GridInfo: " << init_ok << std::endl;
     Model->SetAnalysisGridInfo(gridRep);
     gridPolyData = gridRep->GetRepresentation();
      }
    else
      {
      std::cout << "CANNOT RUN: Error loading analysis grid file " << std::endl;
      return -4;
      }

    // Grids typically need mesh-to-model (m2m) info file
    if (m2m_path.empty())
      {
      std::cout << "WARNING: No mesh-to-model info file specified for input grid" << std::endl;;
      }
    }


  // Load mesh-to-model info file
  if (!m2m_path.empty())
    {
    //m2m_path = "/home/john/projects/cmb/git/cmb-protos/data/test.m2m";
    std::cout << "Loading mesh-to-model info file: " << m2m_path << std::endl;
    vtkSmartPointer<vtkCMBMeshToModelReadOperator> m2mReader =
      vtkSmartPointer<vtkCMBMeshToModelReadOperator>::New();
    m2mReader->SetFileName(m2m_path.c_str());
    m2mReader->Operate(modelWrapper);
    if(m2mReader->GetOperateSucceeded() == false)
      {
      vtkGenericWarningMacro("Could not load file " << m2m_path);
      return 1;
      }

    // The m2mReader overwrites the grid representation, blowing away its geometry.
    // Add that geometry (vtkPolyData) back here.
    vtkCMBMeshGridRepresentationServer *newGridRep =
      vtkCMBMeshGridRepresentationServer::SafeDownCast(Model->GetAnalysisGridInfo());
    if (newGridRep)
      {
      vtkPolyData *newPolyData = newGridRep->GetRepresentation();
      newPolyData->SetPoints(gridPolyData->GetPoints());
      newPolyData->SetPolys(gridPolyData->GetPolys());
      }
    else
      {
      std::cout << "Could not set grid geometry" << std::endl;
      }
    }

  // FIXME: There is no more smtk::model::Model
  // Create smtkModel and apply to Manager
  //smtkModel *smtk_model = new smtkModel();
  //smtk_model->setDiscreteModel(Model);
  //smtk_model->loadGroupItems(vtkModelMaterialType);
  //smtk_model->loadGroupItems(vtkDiscreteModelEntityGroupType);
  //smtk::model::ModelPtr smtk_model_ptr(smtk_model);


  // Load simulation model (attributes)
  smtk::io::AttributeReader reader;
  smtk::io::Logger logger;
  smtk::attribute::ManagerPtr attman(new smtk::attribute::Manager());
  smtk::attribute::ManagerPtr expman(new smtk::attribute::Manager());
  smtk::common::ResourceSet resources;
  if (!sim_path.empty())
    {
    //attman->setRefModel(smtk_model_ptr);
    std::cout << "Loading simulation file: " << sim_path << std::endl;
    std::string ext = vtksys::SystemTools::GetFilenameExtension(sim_path);
    if (ext == ".sbi")
      {
      // Load attribute manager
      bool sim_err = reader.read(*attman, sim_path, logger);
      if (sim_err)
        {
        std::cout << "SIMULATION FILE HAS ERRORS:\n"
                  << logger.convertToString() << std::endl;
        return -5;
        }

      // Initialize default export manager
      bool exp_err = reader.readContents(*expman, defaultExportTemplateString, logger);
      if (exp_err)
        {
        std::cout << "ERRORS building export attributes:\n"
                  << logger.convertToString() << std::endl;
        return -5;
        }
      }
    else if (ext == ".crf")
      {
      // Load file into string
      std::ifstream in(sim_path.c_str());
      if (in)
        {
        unsigned long fsize = vtksys::SystemTools::FileLength(sim_path.c_str());
        std::string contents;
        contents.resize(fsize);
        in.read(&contents[0], contents.size());
        in.close();

        // Parse string to populate ResourceSet with attman & expman
        smtk::common::ResourceSetReader resourceReader;
        std::map<std::string, smtk::common::ResourcePtr> resourceMap;
        resourceMap["simbuilder"] = attman;
        resourceMap["export"] = expman;

        bool hasErrors = resourceReader.readString(contents, resources,
                                             logger, true, &resourceMap);
        if (hasErrors)
          {
          std::cout << "ERROR reading sim file\n"
                    << logger.convertToString()
                    << std::endl;
          return -6;
          }
        }
      }  // else (.crf)
    }  // if (sim_path not empty)

  std::vector<smtk::attribute::AttributePtr> attList;
  expman->findAttributes("ExportSpec", attList);
  smtk::attribute::AttributePtr exportAtt = attList[0];
  smtk::attribute::ItemPtr item;

  if (analysis_names.size() > 0)
    {
    item = exportAtt->find("AnalysisTypes");
    if (item)
      {
      smtk::attribute::StringItemPtr typesItem =
        smtk::dynamic_pointer_cast<smtk::attribute::StringItem>(item);
      //typesItem->setValue(0, analysis_names[0]);
      bool ok = typesItem->setNumberOfValues(analysis_names.size());
      if (!ok)
        {
        std::cout << "ERROR - Cannot set number of analysis types to "
                  << analysis_names.size()
                  << std::endl;
        return -7;
        }
      std::cout << "Exporting analysis type(s):\n";
      for (std::size_t i=0; i<analysis_names.size(); ++i)
        {
        std::string name = analysis_names[i];
        size_t len = name.size();
        // Strip any quotes
        if (('\"' == name[0] && '\"' == name[len-1]) ||
            ('\'' == name[0] && '\'' == name[len-1]))
          {
          name = name.substr(1, len-2);
          }

        typesItem->setValue(i, name);
        std::cout << "  \"" << name << "\"\n";
        }
      std::cout << std::endl;
      }
    }

  // Set output file (path)
  if (output_path != "")
    {
    item = exportAtt->find("OutputFile");
    if (item)
      {
      smtk::attribute::FileItemPtr fileItem =
        smtk::dynamic_pointer_cast<smtk::attribute::FileItem>(item);
      if (fileItem)
        {
        fileItem->setValue(0, output_path);
        }
      }
    }
  std::cout << "\n";


  // Instantiate python exporter and run
  vtkPythonExporter *exporter = vtkPythonExporter::New();
  exporter->SetPythonExecutable(python_exec.c_str());
  std::cout << "Using python script: " << python_script_path << std::endl;
  exporter->SetScript(python_script_path.c_str());

  exporter->Operate(Model, *attman, *expman);
  std::cout << "\n" << "Exporter succeed?: " << exporter->GetOperateSucceeded() << "\n";
  if (exporter->GetOperateSucceeded())
    {
    if (output_path == "")
      {
      std::cout << "Wrote default output" << "\n";
      }
    else
      {
      std::cout << "Wrote " << output_path << "\n";
      }
    }
  else
    {
    std::cout << "Output NOT generated or incomplete" << "\n";
    }
  std::cout << endl;

  return 0;
}

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkPythonExporter.h"

#include <vtkObjectFactory.h>
#include <vtkPythonInterpreter.h>

#include <vtksys/SystemTools.hxx>

#include <smtk/attribute/Attribute.h>
#include <smtk/attribute/Item.h>
#include <smtk/attribute/FileItem.h>
#include <smtk/attribute/System.h>
#include <smtk/model/Manager.h>

#include <smtk/io/AttributeReader.h>
#include <smtk/io/Logger.h>
// #include <smtk/io/SaveJSON.h>

#include <smtk/simulation/ExportSpec.h>

#include <algorithm>
#include <sstream>

#include "vtkModelManagerWrapper.h"

namespace
{
  // Description:
  // Given the attributes serialized into contents, deserialize that
  // data into manager.
  void DeserializeSMTK(const char* contents,
                       smtk::attribute::SystemPtr manager)
  {
    smtk::io::AttributeReader xmlr;
    smtk::io::Logger logger;
    xmlr.readContents(manager, contents, logger);
    std::vector<smtk::attribute::DefinitionPtr> definitions;
    manager->findBaseDefinitions(definitions);
  }
}

vtkStandardNewMacro(vtkPythonExporter);
vtkCxxSetObjectMacro(vtkPythonExporter,ModelManagerWrapper,vtkModelManagerWrapper);

vtkPythonExporter::vtkPythonExporter()
{
  this->OperateSucceeded = 0;
  this->Script = 0;
  this->PythonPath = 0;
  this->PythonExecutable = 0;
  this->ModelManagerWrapper = NULL;
}

vtkPythonExporter::~vtkPythonExporter()
{
  this->SetScript(0);
  this->SetPythonPath(0);
  this->SetPythonExecutable(0);
  this->SetModelManagerWrapper(NULL);
}

void vtkPythonExporter::Operate(vtkModelManagerWrapper* modelWrapper,
                                const char* smtkContents,
                                const char* exportContents)
{
  if(!this->AbleToOperate(modelWrapper))
    {
    this->OperateSucceeded = 0;
    return;
    }

  // NOTE: We need to set the model manager BEFORE deseriazlize, so that
  // all the ModelEntityItems have associated EntityRefs properly initialized.
  // create the attributes from smtkContents
  auto simManager = smtk::attribute::System::create();
  simManager->setRefModelManager(modelWrapper->GetModelManager());
  DeserializeSMTK(smtkContents, simManager);

  auto exportManager = smtk::attribute::System::create();
  exportManager->setRefModelManager(modelWrapper->GetModelManager());
  DeserializeSMTK(exportContents, exportManager);

  this->Operate(modelWrapper->GetModelManager(), simManager, exportManager);
}

void vtkPythonExporter::Operate(vtkModelManagerWrapper* modelWrapper,
                                const char* smtkContents)
{
  if(!this->AbleToOperate(modelWrapper))
    {
    this->OperateSucceeded = 0;
    return;
    }

  // NOTE: We need to set the model manager BEFORE deseriazlize, so that
  // all the ModelEntityItems have associated EntityRefs properly initialized.
  // create the attributes from smtkContents
  // create the attributes from smtkContents
  auto manager = smtk::attribute::System::create();
  manager->setRefModelManager(modelWrapper->GetModelManager());
  DeserializeSMTK(smtkContents, manager);

  // Create empty export manager
  auto exportManager = smtk::attribute::System::create();
  this->Operate(modelWrapper->GetModelManager(), manager, exportManager);
}

template<class IN> std::string to_hex_address(IN* ptr)
{
  std::stringstream ss;
  ss << std::hex << ptr;
  std::string address;
  ss >> address;
  if(address[0] == '0' && (address[1] == 'x' || address[1] =='X'))
  {
    address = address.substr(2);
  }
  return address;
}

void vtkPythonExporter::Operate(smtk::model::ManagerPtr modelMgr,
                                smtk::attribute::SystemPtr manager,
                                smtk::attribute::SystemPtr exportManager)
{

//  std::ofstream json("/Users/yuminyuan/Downloads/hydrafiles/exportModelManager.json");
//  json << smtk::io::SaveJSON::fromModelManager(modelMgr);
//  json.close();

  // Check that we have a python script
  if (!this->GetScript() || strcmp(this->GetScript(),"")==0 )
    {
    vtkWarningMacro("Cannot export - no python script specified");
    this->OperateSucceeded = 0;
    return;
    }

  manager->setRefModelManager(modelMgr);
  exportManager->setRefModelManager(modelMgr);

/*
  smtk::attribute::AttributePtr att = manager->findAttribute("Material-0");
  if(att)
  {
    smtk::model::EntityRefArray entarray = att->associatedModelEntities<smtk::model::EntityRefArray>();
    std::cout << "num of entities " << entarray.size() << std::endl;
    std::cout << "num of associations " << att->associations()->numberOfValues() << std::endl;
  }

  std::ofstream manjson("/Users/yuminyuan/Downloads/hydrafiles/afterManModelManager.json");
  manjson << smtk::io::SaveJSON::fromModelManager(manager->refModelManager());
  manjson.close();

  std::ofstream expjson("/Users/yuminyuan/Downloads/hydrafiles/afterExpModelManager.json");
  expjson << smtk::io::SaveJSON::fromModelManager(exportManager->refModelManager());
  expjson.close();

*/
  // Set python executable if defined
  if (this->PythonExecutable)
    {
      vtkPythonInterpreter::SetProgramName(this->PythonExecutable);
    }

  // Prepend the paths defined in PythonPath to sys.path
  if (this->PythonPath)
    {
    std::string pathscript;
    pathscript += "import sys\n";
    std::vector<vtksys::String> paths;
    paths = vtksys::SystemTools::SplitString(this->PythonPath, ';');
    for (size_t cc=0; cc < paths.size(); cc++)
      {
      if (!paths[cc].empty())
        {
        pathscript += "if not ";
        pathscript += paths[cc];
        pathscript += " in sys.path:\n";
        pathscript += "  sys.path.insert(0, ";
        pathscript += paths[cc];
        pathscript += ")\n";

        vtkPythonInterpreter::RunSimpleString(pathscript.c_str());
        }
      }
    }
  std::string path = vtksys::SystemTools::GetFilenamePath(this->Script);
  if(!path.empty())
    {
    std::string pathscript;
    pathscript += "import sys\n";
    pathscript += "if not ";
    pathscript += '"' + path + '"';
    pathscript += " in sys.path:\n";
    pathscript += "  sys.path.insert(0, ";
    pathscript += '"' + path + '"';
    pathscript += ")\n";
    vtkPythonInterpreter::RunSimpleString(pathscript.c_str());
    }

///***************************************************************///
/// NOTE: we have to figure out something for discrete/cmb session ///
 /*
  // Initialize GridInfo object
  smtk::model::ModelPtr smtkModel = manager.refModel();

 
  PythonExportGridInfo *gridInfoRaw = 0x0;
  if (2 == model->GetModelDimension())
    {
    gridInfoRaw = new PythonExportGridInfo2D(model);
    }
  else if (3 == model->GetModelDimension())
    {
    gridInfoRaw = new PythonExportGridInfo3D(model);
    }
  smtk::shared_ptr< PythonExportGridInfo > gridInfo(gridInfoRaw);
  smtkModel->setGridInfo(gridInfo);


  // Get filename from model
  std::string name = model->GetFileName();
  if (name == "")
    {
    // If empty string, try filename from model's mesh
    const DiscreteMesh& mesh = model->GetMesh();
    name = mesh.GetFileName();
    }
  smtkModel->setNativeModelName(name);
*/
///***************************************************************///

  // Initialize ExportSpec object
  smtk::simulation::ExportSpec spec;
  spec.setSimulationAttributes(manager);
  spec.setExportAttributes(exportManager);
//  spec.setAnalysisGridInfo(gridInfo);

  std::string runscript;
  std::string script = vtksys::SystemTools::GetFilenameWithoutExtension(this->Script);

  // Call the function
  // reload the script if it has already been imported
  runscript += "import sys\n";
  runscript += "if sys.modules.has_key('" + script + "'):\n";
  runscript += "  reload(" + script + ")\n";
  runscript += "else:\n";
  runscript += "  import " + script + "\n";
  runscript += "import smtk\n";

  std::string spec_address = to_hex_address(&spec);

  runscript += "spec = smtk.simulation.ExportSpec._InternalConverterDoNotUse_('" + spec_address +"')\n";
  runscript += script + ".ExportCMB(spec)\n";
  //std::cout << "\nPython script:\n" << runscript << std::endl;
  vtkPythonInterpreter::RunSimpleString(runscript.c_str());

  this->OperateSucceeded = 1;
}

bool vtkPythonExporter::AbleToOperate(vtkModelManagerWrapper* modelWrapper)
{
  if(!modelWrapper)
    {
    vtkErrorMacro("Passed in a null model manager wrapper.");
    return false;
    }
  if(!this->Script)
    {
    vtkErrorMacro("No Python script.");
    return false;
    }
  return true;
}

void vtkPythonExporter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "OperateSucceeded: " << this->OperateSucceeded << endl;
  os << indent << "Script: " << (this->Script ? this->Script : "(NULL)") << endl;
  os << indent << "PythonPath: " << (this->PythonPath ? this->PythonPath : "(NULL)") << endl;
}

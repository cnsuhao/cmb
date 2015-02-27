// cVeg.cpp: implementation of the cVeg class.
//
//////////////////////////////////////////////////////////////////////

#include "cVeg.h"

#include "PathFix.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

cVeg::cVeg()
{
  Initialize();
}

cVeg::~cVeg()
{
  Initialize();
}

void cVeg::Initialize()
{
  model.clear();
  leaf.clear();
  mat.clear();

  nodefile = "";
  ensight_nodefile = "";

  ensight_stomatal.clear();

  met_file = "";
  start_sim_time = 0;
  end_sim_time = 0;

  met_wind_height = 0.0;

  output_mesh = "";
  ensight_output_mesh = "";

  input_flux_file = 0;

  instance.clear();
}


//////////////////////////////////////////////////////////////////////
// Read File
//////////////////////////////////////////////////////////////////////

void cVeg::ReadComment( ifstream& fin )
{
  char inln[200];

  fin.getline(inln,199);  // finish reading the current line
}

void cVeg::ReadModels( ifstream& fin )
{
  int ii, nummodels;
  VegModel vm;

  fin >> nummodels;
  for (ii=0; ii<nummodels; ii++)
  {
    fin >> vm.id >> vm.filename;
    model.push_back(vm);
  }
}

void cVeg::ReadLeafSize( ifstream& fin )
{
  int ii, numleaf;
  VegLeaf vl;

  fin >> numleaf;
  for (ii=0; ii<numleaf; ii++)
  {
    fin >> vl.id >> vl.size;
    leaf.push_back(vl);
  }
}

void cVeg::ReadMaterialCode( ifstream& fin )
{
  int ii, nummc;
  VegMat vm;

  fin >> nummc;
  for (ii=0; ii<nummc; ii++)
  {
    fin >> vm.id >> vm.index;
    mat.push_back(vm);
  }
}

void cVeg::ReadStomatal( ifstream& fin )
{
  string name;

  fin >> name;
  ensight_stomatal.push_back(name);
}

void cVeg::ReadInstance( ifstream& fin )
{
  int ii, numinstance;
  VegInstance vi;

  fin >> numinstance;
  for (ii=0; ii<numinstance; ii++)
  {
    fin >> vi.id >> vi.scl >> vi.rotz >> vi.posx >> vi.posy >> vi.posz;
    instance.push_back(vi);
  }
}

bool cVeg::Read( const char *fname )
{
  ifstream fin(pathfix(fname));  //our input file stream
  string cmd;

  if(!fin)
  {
    cout << "Problem opening Veg file: " << fname <<".\n";
    return false;
  }

  cmd = "";
  while(!fin.eof())
  {
    if (cmd[0]=='#') ReadComment( fin );

    if (cmd=="MODELS") ReadModels( fin );

    if (cmd=="LEAFSIZE") ReadLeafSize( fin );

    if (cmd=="MATERIALCODE") ReadMaterialCode( fin );

    if (cmd=="NODEFILE") fin >> nodefile;

    if (cmd=="ENSIGHT_NODEFILE") fin >> ensight_nodefile;

    if (cmd=="ENSIGHT_STOMATAL") ReadStomatal( fin );

    if (cmd=="MET_FILE") fin >> met_file;

    if (cmd=="START_SIM_TIME") fin >> start_sim_time;

    if (cmd=="END_SIM_TIME") fin >> end_sim_time;

    if (cmd=="MET_WIND_HEIGHT") fin >> met_wind_height;

    if (cmd=="OUTPUT_MESH") fin >> output_mesh;

    if (cmd=="ENSIGHT_OUTPUT_MESH") fin >> ensight_output_mesh;

    if (cmd=="INPUT_FLUX_FILE") fin >> input_flux_file;

    if (cmd=="INSTANCE") ReadInstance( fin );

    fin >> cmd;

  }

  fin.close();
  return true;
}

//////////////////////////////////////////////////////////////////////
// Read File
//////////////////////////////////////////////////////////////////////

void cVeg::WriteComment( ofstream& fout, const char *ostr )
{
  fout << "# " << ostr << "\n";
}

void cVeg::WriteModels( ofstream& fout )
{
  int ii;

  fout << "MODELS " << model.size() << "\n";
  for (ii=0; ii<model.size(); ii++)
  {
    fout << model[ii].id << " " << model[ii].filename << "\n";
  }

}

void cVeg::WriteLeafSize( ofstream& fout )
{
  int ii;

  fout << "LEAF_SIZE " << leaf.size() << "\n";
  for (ii=0; ii<leaf.size(); ii++)
  {
    fout << leaf[ii].id << " " << leaf[ii].size << "\n";
  }
}

void cVeg::WriteMaterialCode( ofstream& fout )
{
  int ii;

  fout << "MATERIAL_CODE " << mat.size() << "\n";
  for (ii=0; ii<mat.size(); ii++)
  {
    fout << mat[ii].id << " " << mat[ii].index << "\n";
  }
}

void cVeg::WriteStomatal( ofstream& fout )
{
  int ii;

  for (ii=0; ii<ensight_stomatal.size(); ii++)
  {
    fout << "ENSIGHT_STOMATAL " << ensight_stomatal[ii] << "\n";
  }
}

void cVeg::WriteInstance( ofstream& fout )
{
  int ii;

  fout << "INSTANCE " << instance.size() << "\n";
  for (ii=0; ii<instance.size(); ii++)
  {
    fout << instance[ii].id << " " << instance[ii].scl << " " << instance[ii].rotz << " " << instance[ii].posx << " " << instance[ii].posy << " " << instance[ii].posz << "\n";
  }
}

void cVeg::Write( const char *fname )
{
  ofstream fout(pathfix(fname));  //our input file stream
  string cmd;

  if(!fout)
  {
    cout << "Problem opening Veg output file: " << fname <<".\n";
    return;
  }

  WriteComment( fout, "From SceneGen 1.0" );

  WriteModels( fout );

  WriteLeafSize( fout );

  WriteMaterialCode( fout );

  fout << "NODEFILE " << nodefile << "\n";

  fout << "ENSIGHT_NODEFILE " << ensight_nodefile << "\n";

  WriteStomatal( fout );

  fout << "MET_FILE " << met_file << "\n";

  fout << "START_SIM_TIME " << start_sim_time << "\n";

  fout << "END_SIM_TIME " << end_sim_time << "\n";

  fout << "MET_WIND_HEIGHT " << met_wind_height << "\n";

  fout << "OUTPUT_MESH " << output_mesh << "\n";

  fout << "ENSIGHT_OUTPUT_MESH " << ensight_output_mesh << "\n";

  fout << "INPUT_FLUX_FILE " << input_flux_file << "\n";

  WriteInstance( fout );

  fout.close();
}

//////////////////////////////////////////////////////////////////////
// SceneGen Specific Routines
//////////////////////////////////////////////////////////////////////
void cVeg::GetInstanceInformation( long iid, string& infile, double& iscl, double& irotz, double& iposx, double& iposy, double& iposz )
{
  long ii;

  for (ii=0; ii<model.size(); ii++)
  {
    if (model[ii].id == instance[iid].id)
    {
      infile = model[ii].filename;
    }
  }

  iscl = instance[iid].scl;
  irotz = instance[iid].rotz;
  iposx = instance[iid].posx;
  iposy = instance[iid].posy;
  iposz = instance[iid].posz;
}

void cVeg::FixInstanceInformation( long iid, double iscl, double irotz, double iposx, double iposy, double iposz )
{
  instance[iid].scl = iscl;
  instance[iid].rotz = irotz;
  instance[iid].posx = iposx;
  instance[iid].posy = iposy;
  instance[iid].posz = iposz;
}

// cVeg.h: interface for the cVeg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(CVEG_H)
#define CVEG_H

#include "gm.h"

#include <vector>
#include <string>
#
#include <fstream>

using namespace std;

typedef struct vegmodeltype
{
  string id;
  string filename;
} VegModel;

typedef struct vegleafsizetype
{
  string id;
  double size;
} VegLeaf;

typedef struct vegmaterialcodetype
{
  string id;
  long index;
} VegMat;

typedef struct veginstancetype
{
  string id;
  double scl;
  double rotz;
  double posx;
  double posy;
  double posz;
} VegInstance;

class cVeg
{
private:
  vector<VegModel> model;
  vector<VegLeaf> leaf;
  vector<VegMat> mat;

  string nodefile;
  string ensight_nodefile;

  vector<string> ensight_stomatal;

  string met_file;
  long start_sim_time;
  long end_sim_time;

  double met_wind_height;

  string output_mesh;
  string ensight_output_mesh;

  long input_flux_file;

  vector<VegInstance> instance;

public:
  cVeg();
  virtual ~cVeg();

  void Initialize();

  bool Read( const char *fname );
  void Write( const char *fname );

  // SceneGen specific
  long GetNumberOfInstances() { return instance.size(); }
  void GetInstanceInformation( long iid, string& infile, double& iscl, double& irotz, double& iposx, double& iposy, double& iposz );
  void FixInstanceInformation( long iid, double iscl, double irotz, double iposx, double iposy, double iposz );

private:
  void ReadComment( ifstream& fin );
  void ReadModels( ifstream& fin );
  void ReadLeafSize( ifstream& fin );
  void ReadMaterialCode( ifstream& fin );
  void ReadStomatal( ifstream& fin );
  void ReadInstance( ifstream& fin );

  void WriteComment( ofstream& fout, const char * );
  void WriteModels( ofstream& fout );
  void WriteLeafSize( ofstream& fout );
  void WriteMaterialCode( ofstream& fout );
  void WriteStomatal( ofstream& fout );
  void WriteInstance( ofstream& fout );

};

#endif // !defined(CVEG_H)

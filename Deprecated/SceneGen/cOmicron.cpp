// cOmicron.cpp: implementation of the cOmicron class.
//
//////////////////////////////////////////////////////////////////////

#include "cOmicron.h"

#include "PathFix.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction (cOmicronCADOrient)
//////////////////////////////////////////////////////////////////////

cOmicronCADOrient::cOmicronCADOrient()
{
  Init();
}

cOmicronCADOrient::~cOmicronCADOrient()
{
  Init();
}

void cOmicronCADOrient::Init()
{
  fname = "";
  trans.assign(0.0, 0.0, 0.0);
  rotz = 0.0;
  roty = 0.0;
  rotx = 0.0;
  scl = 1.0;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction (cOmicron)
//////////////////////////////////////////////////////////////////////

cOmicron::cOmicron()
{
  Initialize();
}

cOmicron::~cOmicron()
{
  Initialize();
}

void cOmicron::Initialize()
{
  surface.Init();

  boundary[0].assign(0.0, 0.0);
  boundary[1].assign(0.0, 0.0);
  boundary[2].assign(0.0, 0.0);
  boundary[3].assign(0.0, 0.0);

  domainbottom = 0.0;
  framespace = 0.0;

  areaconstraint = 0.0025;
  volumeconstraint = 0.00008;
  discconstraint = 0.0008;

  numregions = 0;
  region.clear();
}

bool cOmicron::Read( const char *fname )
{
  char inln[200];
  long ii;
  ifstream fin(pathfix(fname));  //our input file stream

  if(!fin)
  {
    cout << "Problem opening Omicron file.\n";
    return false;
  }

//the first line is our surface
  fin >> surface.fname;
  fin.getline(inln, 199);  //finish the line

//the next line are our coordinates for the boundary window
  fin.getline(inln,199,':');
  fin >> boundary[0][0] >> boundary[0][1]
      >> boundary[1][0] >> boundary[1][1]
      >> boundary[2][0] >> boundary[2][1]
      >> boundary[3][0] >> boundary[3][1];
  fin.getline(inln,199);

//Transforms for the surface
  fin.getline(inln,199, ':');
  fin >> surface.trans[0] >> surface.trans[1] >> surface.trans[2] >>
         surface.rotz >> surface.roty >> surface.rotx >> surface.scl;
  fin.getline(inln,199);

//bottom and factor
  fin.getline(inln,199, ':');
  fin >> domainbottom >> framespace;
  fin.getline(inln,199);

//area constraint
  fin.getline(inln,199, ':');
  fin >> areaconstraint;
  fin.getline(inln,199);

//volume constraint
  fin.getline(inln,199, ':');
  fin >> volumeconstraint;
  fin.getline(inln,199);

//surface disc size
  fin.getline(inln,199, ':');
  fin >> discconstraint;
  fin.getline(inln,199);

//number of regions
  fin.getline(inln,199, ':');
  fin >> numregions;
  fin.getline(inln,199);

  cOmicronCADOrient cco;
  region.resize(numregions, cco);

//the objects themselves
  for (ii=0; ii<numregions; ii++)
  {
    //the first line is our object
    fin >> region[ii].fname;
    fin.getline(inln, 199);  //finish the line

    fin.getline(inln,199, ':');
    fin >> region[ii].trans[0] >> region[ii].trans[1] >> region[ii].trans[2] >>
           region[ii].rotz >> region[ii].roty >> region[ii].rotx >> region[ii].scl;
    fin.getline(inln,199);
  }

  fin.close();

  return true;
}

void cOmicron::Write( const char *fname )
{
  ofstream fout(pathfix(fname));
  long ii;

  if(!fout)
  {
    cout << "Problem opening Omicron output file.\\n";
    return;
  }

  fout.precision(12);  // increased precision for large coordinates

  fout << surface.fname << endl;

  fout << "4 Corners (X_i,Y_i) Coordinates: ";
  fout << boundary[0][0] << " " << boundary[0][1] << " "
       << boundary[1][0] << " " << boundary[1][1] << " "
       << boundary[2][0] << " " << boundary[2][1] << " "
       << boundary[3][0] << " " << boundary[3][1] << "\n";

  fout << "(X_translation,Y_translation,Z_translation,rotz,roty,rotx,scale): ";
  fout << surface.trans[0] << " " << surface.trans[1] << " " << surface.trans[2];
  fout.precision(4);  // lower precision for angles
  fout << " " << surface.rotz << " " << surface.roty << " " << surface.rotx << " "
       << surface.scl << "\n";

  fout << "(BOTTOM of domain, FACTOR spacing on frame): "
       << domainbottom << " " << framespace << "\n";

  fout << "area_constraint: " << areaconstraint << "\n";
  fout << "volume_constraint: " << volumeconstraint << "\n";
  fout << "disc_constraint: " << discconstraint << "\n";

  // find the actual number of regions
  fout << "number_of_regions: " << numregions << "\n";

  for (ii=0; ii<numregions; ii++)
  {
    fout << region[ii].fname << endl;
    fout << "(X_translation,Y_translation,Z_translation,rotz,roty,rotx,scale): ";
    fout.precision(12);  // increased precision for large coordinates
    fout << region[ii].trans[0] << " " << region[ii].trans[1] << " " << region[ii].trans[2];
    fout.precision(4);  // lower precision for angles
    fout << " " << region[ii].rotz << " " << region[ii].roty << " " << region[ii].rotx << " " << region[ii].scl << "\n";
  }

  fout.close();
}

void cOmicron::GetSurfaceData( string& fname, double& posx, double& posy, double& posz,
                               double& rotz, double& roty, double& rotx, double& scl)
{
  fname = surface.fname;
  posx = surface.trans[0];
  posy = surface.trans[1];
  posz = surface.trans[2];
  rotz = surface.rotz;
  roty = surface.roty;
  rotx = surface.rotx;
  scl = surface.scl;
}

void cOmicron::GetBoundaryData( double& x0, double& y0, double& x1, double& y1,
                                double& x2, double& y2, double& x3, double& y3 )
{
  x0 = boundary[0][0]; y0 = boundary[0][1];
  x1 = boundary[1][0]; y1 = boundary[1][1];
  x2 = boundary[2][0]; y2 = boundary[2][1];
  x3 = boundary[3][0]; y3 = boundary[3][1];
}

void cOmicron::GetObjectData( long idx, string& fname, double& posx, double& posy, double& posz,
                              double& rotz, double& roty, double& rotx, double& scl)
{
  fname = region[idx].fname;
  posx = region[idx].trans[0];
  posy = region[idx].trans[1];
  posz = region[idx].trans[2];
  rotz = region[idx].rotz;
  roty = region[idx].roty;
  rotx = region[idx].rotx;
  scl = region[idx].scl;
}


void cOmicron::SetSurfaceData( string fname, double posx, double posy, double posz,
                               double rotz, double roty, double rotx, double scl)
{
  surface.fname = fname;
  surface.trans[0] = posx;
  surface.trans[1] = posy;
  surface.trans[2] = posz;
  surface.rotz = rotz;
  surface.roty = roty;
  surface.rotx = rotx;
  surface.scl = scl;
}

void cOmicron::SetBoundaryData( double x0, double y0, double x1, double y1,
                                double x2, double y2, double x3, double y3, double bottom )
{
  boundary[0][0] = x0; boundary[0][1] = y0;
  boundary[1][0] = x1; boundary[1][1] = y1;
  boundary[2][0] = x2; boundary[2][1] = y2;
  boundary[3][0] = x3; boundary[3][1] = y3;
  domainbottom = bottom;
}

void cOmicron::MakeRegionList( long size )
{
  region.clear();

  numregions = size;

  cOmicronCADOrient cco;
  region.resize(numregions, cco);

}

void cOmicron::SetObjectData( long idx, string fname, double posx, double posy, double posz,
                              double rotz, double roty, double rotx, double scl)
{
  region[idx].fname = fname;
  region[idx].trans[0] = posx;
  region[idx].trans[1] = posy;
  region[idx].trans[2] = posz;
  region[idx].rotz = rotz;
  region[idx].roty = roty;
  region[idx].rotx = rotx;
  region[idx].scl = scl;
}

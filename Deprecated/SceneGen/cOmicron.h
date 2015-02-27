// cOmicron.h: interface for the cOmicron class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(COMICRON_H)
#define COMICRON_H

#include "gm.h"

#include <vector>
#include <string>
#
#include <fstream>

using namespace std;

class cOmicronCADOrient
{
public:
  string fname;
  gmVector3 trans;
  double rotz, rotx, roty;
  double scl; //uniform scale

  //Methods
  cOmicronCADOrient();
  virtual ~cOmicronCADOrient();
  void Init();
};

class cOmicron
{
private:
  gmVector2 boundary[4];
  cOmicronCADOrient surface;

  double domainbottom;
  double framespace;

  double areaconstraint;
  double volumeconstraint;
  double discconstraint;

  long numregions;
  vector<cOmicronCADOrient> region;

public:
  cOmicron();
  virtual ~cOmicron();

  void Initialize();
  void CleanUp();
  bool Read( const char *fname );
  void Write( const char *fname );

  void GetSurfaceData( string& fname, double& posx, double& posy, double& posz,
                       double& rotz, double& roty, double& rotx, double& scl);
  void GetBoundaryData( double& x0, double& y0, double& x1, double& y1,
                        double& x2, double& y2, double& x3, double& y3 );
  double GetBottom( ) { return domainbottom; }
  long NumObjects( ) { return region.size(); }
  void GetObjectData( long idx, string& fname, double& posx, double& posy, double& posz,
                      double& rotz, double& roty, double& rotx, double& scl);

  void SetSurfaceData( string fname, double posx, double posy, double posz,
                       double rotz, double roty, double rotx, double scl);
  void SetBoundaryData( double x0, double y0, double x1, double y1,
                        double x2, double y2, double x3, double y3, double bottom );
  void MakeRegionList( long size );
  void SetObjectData( long idx, string fname, double posx, double posy, double posz,
                      double rotz, double roty, double rotx, double scl);
};

#endif // !defined(COMICRON_H)

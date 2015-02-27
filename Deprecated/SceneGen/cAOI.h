// cAOI.h: interface for the cAOI class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(cAOI_H)
#define cAOI_H

#
#include "OGLMaterial.h"
#include "aabb.h"

#
#
#include <string>
#include <vector>

using namespace std;

class cAOI
{
private:
  string name;
  int numpts;
  vector<gmVector2> bndry;
  double base;
  double height;
  long matid;

public:
  cAOI();
  virtual ~cAOI();
  void Init();

  long Read( ifstream& inpf );
  void Plot( );
  cAABB GetBoundingBox();
  void AddPoint(double x, double y);
  void SetBase(double bs);
  void SetTop(double tp);
  void SetHeights(double bs, double ht);

  int GetNumPoints() { return numpts; }

  void SetMatID( long inmatid ) { matid = inmatid; }
  long GetMatID( ) { return matid; }

  void SetName( const char *nm ) { name = nm; }
  const char *GetName() { return name.c_str(); }
};

#endif // !defined(cAOI_H)

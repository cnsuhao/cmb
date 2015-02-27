// AABB.h:  This file contains the definitions for an axis-aligned bounding box
//                created 10-24-2006
//////////////////////////////////////////////////////////////////////////////////

#if !defined(AABB_H)
#define AABB_H

#include "gm.h"

#
#include <fstream>

using namespace std;

class cAABB
{
private:
  gmVector3 mn;                           //Minimum Value
  gmVector3 mx;                           //Maximum Value

public:
  cAABB();
  virtual ~cAABB();

  void Reset();
  void Adjust( gmVector3 vtx );
  gmVector3 GetMin() { return mn; }
  gmVector3 GetMax() { return mx; }
  gmVector3 GetMid() { return ((mn+mx)*0.5); }
  gmVector3 GetRange() { return (mx-mn); }
  double GetDiameter() { return (mx-mn).length(); }
  bool Inside( gmVector3 vtx );
};

#endif //!defined(AABB_H)

// cSurfaceObjectDefinition.h: interface for the cSurfaceObjectDefinition class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(ObjectDefinition_H)
#define ObjectDefinition_H

#include "SceneGen.h"

#
#
#

#
#
#

#
#

using namespace std;

class cSurfaceObjectDefinition
{
private:
  string name;
  string geomid;
  long matid;

  gmVector3 scl;
  gmMatrix4 rot;  //ALL rotations occur BEFORE repositioning
  gmVector3 pos;

  bool highlighted;
  bool selected;

  bool surfitem;

  COGLMaterial mater;
  COGLMaterial himater;

//  cObjectGeometry objgeom;
  cObjectGeometry *objgeomp;

public:
  cSurfaceObjectDefinition();
  virtual ~cSurfaceObjectDefinition();

//object reading method
  long Read( ifstream& inpf );

//Oriented object creation methods
  void Initialize();
  void ChangePosition( double dx, double dy, double dz );
  void ChangeRotation( gmMatrix4& pre );
  void FixPosition( double dx, double dy, double dz );
  void SetUniformScale( double scale ) { scl[0] = scale; scl[1] = scale; scl[2] = scale; }
  void SetScale( double dx, double dy, double dz)  { scl[0] = dx; scl[1] = dy; scl[2] = dz; }
  double GetUniformScale() { return scl[0]; }
  gmVector3 GetScale() { return scl; }
  void RotationEuler( double yaw, double pitch, double roll );
  void RotationPhiTheta( double phi, double theta);
  void MakeEuler( double& yaw, double& pitch, double& roll );
  double GetRotZ( );
  void GetPos( double& px, double& py, double& pz) { px = pos[0]; py = pos[1]; pz = pos[2]; }
  gmVector3 GetPos() { return pos; }
  void GetOffset( double& dx, double& dy, double& dz );

  bool IsSurfaceItem() { return surfitem; }
  void MakeSurfaceItem() { surfitem = true; }

  void MakeLink( cObjectGeometry *OG ) { objgeomp = OG; name = objgeomp->GetObjectID(); }

  //Display Methods
  void Display( long baselist );
  void Display( long baselist, gmMatrix4 rotd, gmVector3 poso);  //for selected ObjectDefinitions

//Data Limits Methods
  gmVector3 GetMin() { return objgeomp->GetAABB().GetMin(); }
  gmVector3 GetMid() { return objgeomp->GetAABB().GetMid(); }
  gmVector3 GetMax() { return objgeomp->GetAABB().GetMax(); }
  cAABB GetAABB() { return objgeomp->GetAABB(); }
  cAABB GetTransformedAABB();

  gmVector3 FindClosest(gmVector3 closeto);
  const char *GetName();

  void SetHighLight( bool val ) { highlighted = val; }
  void SetSelect( bool val ) { selected = val; }
  bool IsSelected() { return selected; }

  void SetColor( float r, float g, float b );
  void SetHighLightColor( float r, float g, float b );
};

#endif // !defined(ObjectDefinition_H)

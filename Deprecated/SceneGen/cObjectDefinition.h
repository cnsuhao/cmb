// cObjectDefinition.h: interface for the CObjectDefinition class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_OBJECTDEFINITION_H__)
#define _OBJECTDEFINITION_H__

#

#
#include "OGLMaterial.h"
#include "cObjectGeometry.h"

#
#
#

using namespace std;

class cObjectDefinition
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
  bool deleted;

  bool fixxrot, fixyrot, fixzrot;
  bool fixxpos, fixypos, fixzpos;

  bool surfitem;

  COGLMaterial mater;
  COGLMaterial himater;

//  cObjectGeometry objgeom;
  cObjectGeometry *objgeomp;

public:
  cObjectDefinition();
  virtual ~cObjectDefinition();

//object reading method
  long Read( ifstream& inpf );
  long ReadSurfaceItem( ifstream& inpf );

//Oriented object creation methods
  void Initialize();
  void ChangePosition( double dx, double dy, double dz );
  void SetPosition( double dx, double dy, double dz )   { pos[0] = dx; pos[1] = dy; pos[2] = dz; }
  void FixPosition( double dx, double dy, double dz );
  void ChangeRotation( gmMatrix4& pre );
  void SetUniformScale( double scale ) { scl[0] = scale; scl[1] = scale; scl[2] = scale; }
  void SetScale( double dx, double dy, double dz)  { scl[0] = dx; scl[1] = dy; scl[2] = dz; }
  double GetUniformScale() { return scl[0]; }
  gmVector3 GetScale() { return scl; }
  void GetScale( double &x, double &y, double &z) { x = scl[0]; y = scl[1]; z = scl[2]; }
  void RotationEuler( double yaw, double pitch, double roll );
  void RotationPhiTheta( double phi, double theta);
  void MakeEuler( double& yaw, double& pitch, double& roll );
  double GetRotZ( );
  void GetPos( double& px, double& py, double& pz) { px = pos[0]; py = pos[1]; pz = pos[2]; }
  gmVector3 GetPos() { return pos; }
  void GetOffset( double& dx, double& dy, double& dz );

  bool GetFixXRot() { return fixxrot; }
  bool GetFixYRot() { return fixyrot; }
  bool GetFixZRot() { return fixzrot; }
  void SetFixXRot( bool f ) { fixxrot = f; }
  void SetFixYRot( bool f ) { fixyrot = f; }
  void SetFixZRot( bool f ) { fixzrot = f; }

  bool GetFixXPos() { return fixxpos; }
  bool GetFixYPos() { return fixypos; }
  bool GetFixZPos() { return fixzpos; }
  void SetFixXPos( bool f ) { fixxpos = f; }
  void SetFixYPos( bool f ) { fixypos = f; }
  void SetFixZPos( bool f ) { fixzpos = f; }

  bool IsSurfaceItem() { return surfitem; }
  void MakeSurfaceItem() { surfitem = true; }

  void MakeLink( cObjectGeometry *OG ) { objgeomp = OG; name = objgeomp->GetObjectID(); }
  cObjectGeometry * GetLink() { return objgeomp; }

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
  long GetMatID() { return matid; }
  void SetMatID( int BMID ) { matid=BMID; }

  void SetHighLight( bool val ) { highlighted = val; }
  void SetSelect( bool val ) { selected = val; }
  bool IsSelected() { if (!deleted) return selected; else return false;}
  void SetDeleted( bool val ) { deleted = val; }
  bool IsDeleted() { return deleted; }

  void SetColor( float r, float g, float b );
  void SetHighLightColor( float r, float g, float b );

  //for output routines
  void WritePOVRay( ofstream& fout );
  void WriteWavefront( ofstream& fout, long& voff );
  void WriteXML( ofstream &fout );

private:
  long NumVertexes() { return objgeomp->NumNodes(); }
  gmVector3 GetVertex( long ii );
  long NumTriangles() { return objgeomp->NumTris(); }
  void GetTri( long ii, long& idx1, long& idx2, long& idx3 ) { objgeomp->GetTri(ii, idx1, idx2, idx3); }
};

#endif // !defined(_OBJECTDEFINITION_H__)

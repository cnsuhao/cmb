// cObjectGeometry.h: interface for the cObjectGeometry class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_COBJECTGEOMETRY_H__)
#define _COBJECTGEOMETRY_H__

#include <QObject>

#include "CrossGlut.h"

#
#
#

#include "aabb.h"

#include <vector>
#include <string>

using namespace std;

class TriType
{
public:
  long idx[3];
  gmVector3 nrm;

  TriType();
  virtual ~TriType();
};

class cObjectGeometry : public QObject
{
  Q_OBJECT
private:
  string objid;
  string filekeep;
  gmVector3 baseoffset;  //In order to work effectively in OpenGL,
                         //  the object MAY need to be offset
  vector<gmVector3> nodelist;
  vector<gmVector3> nodeclr;

  vector<TriType> trilist;

  bool pt_color;

  long dl_ptid;       //OpenGL display list id for points
  long dl_wfid;       //OpenGL display list id for WireFrame
  long dl_sfid;       //OpenGL display list id for surface-drawing

  cAABB aabb;  //Axis Aligned Bounding Box

public:
  cObjectGeometry();
  virtual ~cObjectGeometry();

//tobj creation methods
  void Initialize();
  void ClearDisplayLists();

//object reading method
  long Read( ifstream& inpf );

//Format Reading Methods
  void ReadData( const char *fname );

  void Read2DM( const char *fname );
  void Read3DM( const char *fname );
  void ReadSTH( const char *fname );
  void ReadPoly( const char *fname );
  void ReadMCPoly( const char *fname );
  void ReadPTS( const char *fname );
  void ReadBinaryPTS( const char *fname );
  void ReadTextPTS( const char *fname );
  void ReadOBJ( const char *fname );

//Format Writing Methods
  void WriteData( const char *fname );

  void Write2DM( const char *fname );
  void WritePoly( const char *fname );
  void WritePTS( const char *fname );
  void WriteTextPTS( const char *fname );
  void WriteBinaryPTS( const char *fname );
  void WriteOBJ( const char *fname );
  void WriteSTL( const char *fname );

//  void WriteXML( const char *fname );
  void WriteXML( ofstream &fout );

//Requests
  bool HasSurface() { return !trilist.empty(); }
//  const char *GetFileName() { return filekeep.c_str(); }
  const char *GetFileName() { return objid.c_str(); }

//Display Methods
  void PlotPoints( long baselist );
  void PlotWireframe( long baselist );
  void PlotSurface( long baselist );

//Data Limits Methods
  gmVector3 GetMin() { return aabb.GetMin(); }
  gmVector3 GetMid() { return aabb.GetMid(); }
  gmVector3 GetMax() { return aabb.GetMax(); }
  cAABB GetAABB() { return aabb; }

  void FixBase( double dx, double dy, double dz);
  gmVector3 GetBaseOffset() { return baseoffset; }

  const char *GetObjectID() { return objid.c_str(); }

  gmVector3 AverageClosest( long numavg, gmVector3 closeto );

  long NumNodes() { return nodelist.size(); }
  gmVector3 GetNode( long idx ) { return nodelist[idx]; }
  long NumTris() { return trilist.size(); }
  void GetTri( long idx, long& nd1, long& nd2, long& nd3 ) { nd1 = trilist[idx].idx[0]; nd2 = trilist[idx].idx[1]; nd3 = trilist[idx].idx[2]; }

signals:
  void setProgressLabel( QString *qs );
  void setProgressMax( int pmax );
  void setProgressVal( int pval );
  void startProgress(char *caption);
  void finishProgress();

private:
  long AddNode( gmVector3 innode, gmVector3 inclr, long idx );
  long AddNode( gmVector3 innode, long idx );
  long AddTri( TriType intri, long idx );
  long AddNode( gmVector3 innode, gmVector3 inclr );
  long AddNode( gmVector3 innode );
  long AddTri( TriType intri );

  void FixNormals( void );
  void RemoveDupTris( void );

  double avgRadius( gmVector3 center, double radius );
  double DistErr( double z1, double z2, double ze );
  void MakeSurface( void );

};

#endif // !defined(_COBJECTGEOMETRY_H__)

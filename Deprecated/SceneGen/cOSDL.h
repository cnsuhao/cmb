// cOSDL.h: interface for the cOSDL class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(COSDL_H__)
#define COSDL_H__

#
#include <QTreeWidget>

#

#
#
#
#include <deque>
#

using namespace std;

#
#include "cAOI.h"
#include "cOutputs.h"

#include "cObjectDefinition.h"
#include "cOmicron.h"
#include "cVeg.h"

#define OSDL_VERSION     1.0

//Global Units
#define GU_KM  0
#define GU_M   1
#define GU_CM  2
#define GU_MM  3
#define GU_FT  4
#define GU_IN  5

#define BASE_SURFACE        0
#define BASE_TESSOBJ        32767
#define BASE_ORIENTTESSOBJ  65534
#define BASE_OCCLUDER       98301
#define BASE_ORIENTOCCLUDER 131068

#define AOI_SURFACE -2
#define AOI_FULL -1

///////////////////////////////////////////////////////////////////////////////////////////////////////
// Undo info and class
///////////////////////////////////////////////////////////////////////////////////////////////////////

#define UNDO_DEL 1
#define UNDO_CSP 2
#define UNDO_CSR 3
#define UNDO_CSRA 4
#define UNDO_CSID 5
#define UNDO_CSS 6
#define UNDO_CSRS 7
#define UNDO_PSPS 8
#define UNDO_PSM 9

class cOSDL_Undo
{
public:

  int op;

  vector<long> objectid;
  vector<long> matid;
  vector<gmVector3> sclid;

  gmVector3 doff;
  gmMatrix4 drot;
  bool xy;
  gmVector3 scale;

  cOSDL_Undo();
  virtual ~cOSDL_Undo();
  void UndoInit();
};

///////////////////////////////////////////////////////////////////////////////////////////////////////
// Undo info and class
///////////////////////////////////////////////////////////////////////////////////////////////////////

class cOSDL : public QObject
{
  Q_OBJECT
private:
  double version;

  bool loading;

  cOutputs outputs;

  //These establish presets
  int GlobalUnits;
  cOmicron OmicronModel;
  cVeg VegModel;

  cAABB aabb;

  vector<cAOI> aoi;
  vector<cObjectGeometry *> oglist;

  cObjectDefinition primary_surface;              // separates the "ground" from "air"
  vector<cObjectDefinition> surfaces;             // provide layers of materials

  vector<cObjectDefinition> tessobj;              // form 3d elements (either meshed or meshable)
  vector<cObjectDefinition> orienttessobj;

  vector<cObjectDefinition> occluder;             // form 2d elements or occlude for visual conditions
  vector<cObjectDefinition> orientoccluder;

  //Providing an UNDO structure
  deque<cOSDL_Undo *> ulist;

public:
  cOSDL();
  virtual ~cOSDL();
  void Init();

  void ClearDisplayLists();
  cAABB GetAABB() { return aabb; }

  // Clean-up Routines
  void cleanObjectGeometry( long idx = -1 );
  void Clean();

  // Plot routines
  void FillTree( QTreeWidget *qtree );
  void PlotScene( long dlbase, gmMatrix4 drot, gmVector3 doff, bool xy);
  void PlotSceneAbout( long dlbase, gmMatrix4 drot, gmVector3 drotabout, gmVector3 doff, bool xy);
  void PlotSceneSelect( long dlbase, gmMatrix4 drot, gmVector3 doff, bool xy);

  // Object selection routines
  bool IDSelected( long isset );
  void ToggleSelectIndex( long isset );
  void SetSelect( long isset, bool bval );
  void SelectInBoundingBox( cAABB tester );
  void DeselectAll();

  // Selected object transformations (undo-able)
  void DeleteSelectedItems( void );
  void ChangeSelectPosition( gmVector3 doff );
  void ChangeSelectRotation( gmMatrix4 drot, bool xy );
  void ChangeSelectRotationAbout( gmVector3 doff, gmMatrix4 drot, bool xy );
  void ChangeSelectMatID( int BMID );
  void ChangeSelectScale( double sclx, double scly, double sclz );
  void ChangeSelectRelativeScale( double sclx, double scly, double sclz );

  void ProjectSelectedToPrimarySurface();
  void ProjectSelectedToMidScene();

  void Undo();

  //setting the area of interest
  void SetAABB( long aoii = AOI_FULL );
  bool IsEmpty() { return (aoi.empty() && oglist.empty()); }

  void SetLoading( bool bv ) { loading = bv; }
  bool IsLoading() { return loading; }

  //setting the global units
  void SetGlobalUnits( int gu ) { GlobalUnits = gu; }
  int GetGlobalUnits() { return (GlobalUnits); }

  // File routines
  bool Read( const char *fname );
  bool WriteXML( const char *fname );

  void ProcessOutputs();

  // Import routines
  void ImportPrimarySurface( const char *fname, double posx, double posy, double posz, int basemat );
  void ImportSurface( const char *fname );
  void ImportTesselatedObject( const string &fileName, double scl, double xpos, double ypos, double zpos, int basemat, bool rotYtoZ );
  void ImportOrientedTesselatedObject( const string &fileName, double scl, double xpos, double ypos, double zpos, int basemat, bool rotYtoZ );
  void ImportOccluder( const string &fileName, double scl, double xpos, double ypos, double zpos, int basemat, bool rotYtoZ );
  void ImportOrientedOccluder( const string &fileName, double scl, double xpos, double ypos, double zpos, int basemat, bool rotYtoZ );

signals:
  void selection( long idx, bool bval );
  void startProgress();

  void setProgressMax( int pmax );
  void setProgressVal( int pval );
  void startProgress( char *caption );
  void finishProgress();

public slots:
  void onSelection( long idx, bool bval );

  void getProgressMax( int pmax );
  void getProgressVal( int pval );
  void startingProgress(char *caption );
  void finishingProgress();

private:
  cObjectGeometry * AddGeometry( string nfname );

  void ReadOmicronFile( const char *fname );
  void ReadVegFile( const char *fname );

  void WriteOmicronFile( const char *fname );
  void WriteVegFile( const char *fname );

  void WritePOVRayFile( const char *fname );
  void WriteWavefrontFile( const char *fname );

  // OSDL Read routines
  bool ReadObjectGeometry( ifstream& fin );
  bool ReadAreaOfInterest( ifstream& fin );
  bool Read2DPolygon( ifstream& fin, cAOI& iaoi );

  bool ObjectGeometryReferenced( long idx );

  void Undo_Del( cOSDL_Undo * );
  void Undo_CSP( cOSDL_Undo * );
  void Undo_CSR( cOSDL_Undo * );
  void Undo_CSRA( cOSDL_Undo * );
  void Undo_CSID( cOSDL_Undo * );
  void Undo_CSS( cOSDL_Undo * );
  void Undo_CSRS( cOSDL_Undo * );
  void Undo_PS( cOSDL_Undo * );
};

#endif // !defined(COSDL_H)

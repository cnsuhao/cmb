// ViewWnd.h: interface for the ViewWnd class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_VIEWWND_H__)
#define _VIEWWND_H__

#
#include <QTimer>

#include "SceneGen.h"

#include "OGLCamera.h"
#include "ViewMenu.h"
#include "PerspControl.h"
#include "OrthoControl.h"
#include "Accelerator.h"
#

#
#
#
#

using namespace std;

////////////////////////////////////////////////////////////////////
// Enumerations
//

enum eviewmode
{
  perspective,
  xyortho,
  xzortho,
  yzortho
};


class ViewWnd : public QGLWidget
{
      Q_OBJECT
private:
  // Pointer to the main data structure
  MainSceneData *msd;

  // Interaction and viewing modes
  enum eviewmode vm;  // our view-mode (perspective or ortho)

  // Menu System Information (in the View Window)
  //ProgramMenu pm;

  ViewMenu vmnu;
  long vwMouseX, vwMouseY;
  bool vwrm;
  long vwbasex, vwbasey;
  int timerid;

  //perspective view info
  COGLCamera syncam;  // Our synthetic camera
  PerspControl sc;
  int pmode;
  CAccelerator accel;
  CAccelerator paccel;

  //ortho view info
  cAABB fullextents;
  cAABB zoomextents;
  double pixscl;
  int omode;
  OrthoControl oc;

  //control information
  float ctlx, ctly;   // Center point
  bool plotcontrols;  // Set to false for screen captures

public:
  ViewWnd(QWidget *parent = 0);

  void SetMSD( MainSceneData *xmsd );

  void Prime( void );
  void SetAccel( void );
  void Refresh() { updateGL(); }
  void ResetView();

  void SetCameraPosition( double x, double y, double z, double heading, double pitch, double yaw);
  void saveScreen( const char *fname );
  void SaveCameraSettings( const char *fname ) { syncam.SaveCameraSettings(fname); }
  void LoadCameraSettings( const char *fname ) { syncam.LoadCameraSettings(fname); }

signals:
  void changeMessage(char *msg);
  void clearStatus();

public slots:
  void extentsChanged( long x );

protected:
  void initializeGL();
  void resizeGL(int width, int height);
  void paintGL();
  void mousePressEvent(QMouseEvent *event);
  void mouseReleaseEvent(QMouseEvent *event);
  void mouseMoveEvent(QMouseEvent *event);
  void timerEvent(QTimerEvent *event);
    //void mouseDoubleClickEvent(QMouseEvent *event);

private:
  void draw(void);
//  void OnTimer();
  void OnMouse(int button, int state, int x, int y);
  void OnPassiveMotion( int x, int y );
  void OnMotion( int x, int y );
  void OnIdle( void );

  void CenterAABB( cAABB aabb );
  void GetMSDFullExtents(cAABB& aabb);
  long ObjectDefinition_Select( int x, int y);

  void vwFixProjection( bool picking );
  void vwOrientObjects( gmMatrix4 &drot, gmVector3 &doff);
  long Pick3D(long pX, long pY);
  void vwPlotZoomBox( void );
  void AspectCorrect( cAABB& aabb );

  void PerspectiveMouse( int button, int state, int x, int y);
  void PerspectivePassiveMotion( int x, int y );
  void PerspectiveMotion( int x, int y);
  void PerspectiveIdle( void );

  void OrthoMouse( int button, int state, int x, int y);
  void OrthoPassiveMotion( int x, int y );
  void OrthoMotion( int x, int y);
  void NotifyGLError( void );

  double ComputePixScale( );
  void PixBound( double whorz, double wvert, double& pbhorz, double& pbvert);
  void OrthoMouseToWorld( int mhorz, int mvert, double& whorz, double& wvert);
  gmVector3 OrthoMouseToWorld( int mhorz, int mvert);
  gmVector3 OrthoScaleMouseToWorld( int mhorz, int mvert);
};

#endif // !defined(_VIEWWND_H__)

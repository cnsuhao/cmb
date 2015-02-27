// OrthoControl.h: interface for the OrthoControl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(ORTHOCONTROL_H)
#define ORTHOCONTROL_H

#include <QGLWidget>
#include "CrossGlut.h"
#include "ControlSettings.h"

#include "gm.h"

//define the possible modes
#define OC_NONE 0
#define OC_SELECT 1
#define OC_ZOOM 2
#define OC_ROTATE 3
#define OC_TRANSLATE 4
#define OC_RESETBUTTON 5
#define OC_PROJECTBUTTON 6
#define OC_CENTERBUTTON 7

class OrthoControl
{
private:
  double drot;
  gmVector3 dpos;

  QGLWidget *drawto;

  float offsetx, offsety; //for plotting menu

  int mode;
  int mcover;
  float costable[73], sintable[73];

  int action;
  long bw, bh, cw, ch;

public:
  OrthoControl();
  virtual ~OrthoControl();

  void SetDrawTo( QGLWidget *pt ) { drawto = pt; }

  void SetOffset( float ox, float oy );
  void SetMode( int nm ) { mode = nm; }
  int GetMode( ) { return(mode); }
  int GetMouse( ) { return(mcover); }

  void SetAction( int act, long ibw, long ibh) { action=act; bw=ibw; bh=ibh; }
  int GetAction() { return action; }
  void SetCurrent( long icw, long ich );
  bool GetActionDelta( long &dw, long &dh);
  bool GetActionEndPoints( long &obw, long &obh, long &ocw, long &och );
  void TranslateAction( double dx, double dy, double dz );

  void PlotDropShadow();
  void Plot();

  void SetAngle( int mx, int my );
  void ResetAngle() { drot=0.0; }

  void SetPos( double px, double py, double pz ) { dpos[0] = px; dpos[1] = py; dpos[2] = pz; }
  void AddPos( double dx, double dy, double dz ) { dpos[0] += dx; dpos[1] += dy; dpos[2] += dz; }
  double GetAngle() { return drot; }
  gmVector3& GetPos() { return dpos; }

  bool MouseOver( int mx, int my);

private:
  bool inCircle( float mx, float my, float cx, float cy, float rad = 10.0 );
  bool inTriangle( float mx, float my, float px1, float py1, float px2, float py2, float px3, float py3 );
  bool inBBox( float mx, float my, float minx, float miny, float maxx, float maxy );

  void PlotModeBorder( double offsetx, double offsety);
  void PlotButtonBorder( double offsetx, double offsety);
  void OutputText(float x, float y, const char *string);
  int GetStringWidth( char *s );
};

#endif // !defined(ORTHOCONTROL_H)

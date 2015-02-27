// PerspControl.h: interface for the PerspControl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(PerspControl_H)
#define PerspControl_H

#include "CrossGlut.h"
#include "ControlSettings.h"

#include <QGLWidget>
#include <math.h>
#include "gm.h"

//define the possible modes
#define SC_NONE 0
#define SC_ROTATE 1
#define SC_DRIVE 2
#define SC_SELECT 3
#define SC_PANLEFT 4
#define SC_PANRIGHT 5
#define SC_PANUP 6
#define SC_PANDOWN 7
#define SC_PANFORE 8
#define SC_PANAFT 9
#define SC_RESET 10

class PerspControl
{
private:
  float offsetx, offsety;

  int mode;
  int mcover;
  float costable[73], sintable[73];

  QGLWidget *drawto;

public:
  PerspControl();
  virtual ~PerspControl();

  void SetDrawTo( QGLWidget *pt ) { drawto = pt; }

  void SetOffset( float ox, float oy );
  void SetMode( int nm ) { mode = nm; }
  int GetMode( ) { return(mode); }
  int GetMouse( ) { return(mcover); }

  void Plot();

  bool MouseOver( int mx, int my);

private:
  void OutputText(float x, float y, const char *string);
  int GetStringWidth( char *s );
  void PlotDropShadow();
  bool inCircle( float mx, float my, float cx, float cy, float rad = 10.0 );
  bool inTriangle( float mx, float my, float px1, float py1, float px2, float py2, float px3, float py3 );
};

#endif // !defined(PerspControl_H)

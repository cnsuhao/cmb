// ViewMenu.h: interface for the ViewMenu class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(VIEWMENU_H)
#define VIEWMENU_H

#include <QGLWidget>

#include "CrossGlut.h"
#include "ControlSettings.h"

//define the possible modes
#define VM_NONE 0
#define VM_PERSPECTIVE 1
#define VM_XYORTHO 2
#define VM_XZORTHO 3
#define VM_YZORTHO 4

class ViewMenu
{
private:
  float offsetx, offsety;

  int mode;
  int mcover;

  QGLWidget *drawto;

  float widthP;
  float widthXY;
  float widthXZ;
  float widthYZ;

public:
  ViewMenu();
  virtual ~ViewMenu();

  void SetDrawTo( QGLWidget *pt ) { drawto = pt; }

  void SetOffset( float ox, float oy );
  void SetMode( int nm ) { mode = nm; }
  int GetMode( ) { return(mode); }
  int GetMouse( ) { return(mcover); }

  void Plot();

  bool MouseOver( int mx, int my);

private:
  bool inBBox( float mx, float my, float minx, float miny, float maxx, float maxy );
  void OutputText( float x, float y, const char *string);
  int GetStringWidth( char *s );

};

#endif // !defined(AFX_VIEWMENU_H__6CC70922_47EE_4496_90E4_263ED54A4139__INCLUDED_)

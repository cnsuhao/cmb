// ViewMenu.cpp: implementation of the ViewMenu class.
//
//////////////////////////////////////////////////////////////////////

#include "ViewMenu.h"

#include <stdio.h>

#include <QFont>
#include <QFontMetrics>
#include <QRect>
#include <QString>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ViewMenu::ViewMenu()
{
  offsetx = offsety = 0.0;
  mode = VM_PERSPECTIVE;
  mcover = VM_NONE;

  // for centering text
  widthP = (float)GetStringWidth("Perspective");
  widthXY = (float)GetStringWidth("XY Ortho");
  widthXZ = (float)GetStringWidth("XZ Ortho");
  widthYZ = (float)GetStringWidth("YZ Ortho");

  drawto = NULL;
}

ViewMenu::~ViewMenu()
{

}

//////////////////////////////////////////////////////////////////////
// Set Values
//////////////////////////////////////////////////////////////////////

void ViewMenu::SetOffset( float ox, float oy)
{
  offsetx = ox;
  offsety = oy;
}

//////////////////////////////////////////////////////////////////////
// Draw Control
//////////////////////////////////////////////////////////////////////

void ViewMenu::OutputText(float x, float y, const char *string)
{
  if (drawto==NULL) return;
  drawto->renderText(x,y, 0.0 ,string);
}

int ViewMenu::GetStringWidth( char *s )
{
  QFont qf;
  QFontMetrics qfm(qf);
  QRect qr;
  QString qs = s;

  int ret;

  qr = qfm.boundingRect(qs);
  ret = qr.width();

  return ret;
}

void ViewMenu::Plot()
{
  int width, height;

  width = drawto->width();
  height = drawto->height();

  // Other drawing has taken place, now to draw the control
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0,width, 0, height);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);

  // Offset Drawing to proper position
  glTranslatef(offsetx, offsety, 0);

  glColor4f( 1.0, 1.0, 1.0, XH_ALPHA );

  // Plot Perspective Button
  if (mode == VM_PERSPECTIVE)
  {
    glColor4f( 1.0, 1.0, 1.0, LO_ALPHA );
    glBegin(GL_POLYGON);
      glVertex2d(0.0, 75.0);
      glVertex2d(0.0, 95.0);
      glVertex2d(100.0, 95.0);
      glVertex2d(100.0, 75.0);
    glEnd();
    glColor4f( 1.0, 1.0, 1.0, XH_ALPHA );
  }

#ifdef DROP_SHADOWS
  glColor4f( 0.0, 0.0, 0.0, XH_ALPHA );
  glBegin(GL_LINE_LOOP);
    glVertex2d(1.0, 74.0);
    glVertex2d(1.0, 94.0);
    glVertex2d(101.0, 94.0);
    glVertex2d(101.0, 74.0);
  glEnd();
  OutputText(98.0-widthP, 79.0, "Perspective");
  glColor4f( 1.0, 1.0, 1.0, XH_ALPHA );
#endif //DROP_SHADOWS

  glBegin(GL_LINE_LOOP);
    glVertex2d(0.0, 75.0);
    glVertex2d(0.0, 95.0);
    glVertex2d(100.0, 95.0);
    glVertex2d(100.0, 75.0);
  glEnd();
  OutputText(97.0-widthP, 80.0, "Perspective");

  // Plot XY Ortho Button
  if (mode == VM_XYORTHO)
  {
    glColor4f( 1.0, 1.0, 1.0, LO_ALPHA );
    glBegin(GL_POLYGON);
      glVertex2d(0.0, 50.0);
      glVertex2d(0.0, 70.0);
      glVertex2d(100.0, 70.0);
      glVertex2d(100.0, 50.0);
    glEnd();
    glColor4f( 1.0, 1.0, 1.0, XH_ALPHA );
  }

#ifdef DROP_SHADOWS
  glColor4f( 0.0, 0.0, 0.0, XH_ALPHA );
  glBegin(GL_LINE_LOOP);
    glVertex2d(1.0, 49.0);
    glVertex2d(1.0, 69.0);
    glVertex2d(101.0, 69.0);
    glVertex2d(101.0, 49.0);
  glEnd();
  OutputText(98.0-widthXY, 54.0, "XY Ortho");
  glColor4f( 1.0, 1.0, 1.0, XH_ALPHA );
#endif //DROP_SHADOWS

  glBegin(GL_LINE_LOOP);
    glVertex2d(0.0, 50.0);
    glVertex2d(0.0, 70.0);
    glVertex2d(100.0, 70.0);
    glVertex2d(100.0, 50.0);
  glEnd();
  OutputText(97.0-widthXY, 55.0, "XY Ortho");

  // Plot XZ Ortho Button
  if (mode == VM_XZORTHO)
  {
    glColor4f( 1.0, 1.0, 1.0, LO_ALPHA );
    glBegin(GL_POLYGON);
      glVertex2d(0.0, 25.0);
      glVertex2d(0.0, 45.0);
      glVertex2d(100.0, 45.0);
      glVertex2d(100.0, 25.0);
    glEnd();
    glColor4f( 1.0, 1.0, 1.0, XH_ALPHA );
  }

#ifdef DROP_SHADOWS
  glColor4f( 0.0, 0.0, 0.0, XH_ALPHA );
  glBegin(GL_LINE_LOOP);
    glVertex2d(1.0, 24.0);
    glVertex2d(1.0, 44.0);
    glVertex2d(101.0, 44.0);
    glVertex2d(101.0, 24.0);
  glEnd();
  OutputText(98.0-widthXZ, 29.0, "XZ Ortho");
  glColor4f( 1.0, 1.0, 1.0, XH_ALPHA );
#endif //DROP_SHADOWS

  glBegin(GL_LINE_LOOP);
    glVertex2d(0.0, 25.0);
    glVertex2d(0.0, 45.0);
    glVertex2d(100.0, 45.0);
    glVertex2d(100.0, 25.0);
  glEnd();
  OutputText(97.0-widthXZ, 30.0, "XZ Ortho");

  // Plot YZ Ortho Button
  if (mode == VM_YZORTHO)
  {
    glColor4f( 1.0, 1.0, 1.0, LO_ALPHA );
    glBegin(GL_POLYGON);
      glVertex2d(0.0, 0.0);
      glVertex2d(0.0, 20.0);

      glVertex2d(100.0, 20.0);
      glVertex2d(100.0, 0.0);
    glEnd();
    glColor4f( 1.0, 1.0, 1.0, XH_ALPHA );
  }

#ifdef DROP_SHADOWS
  glColor4f( 0.0, 0.0, 0.0, XH_ALPHA );
  glBegin(GL_LINE_LOOP);
    glVertex2d(1.0, -1.0);
    glVertex2d(1.0, 19.0);
    glVertex2d(101.0, 19.0);
    glVertex2d(101.0, -1.0);
  glEnd();
  OutputText(98.0-widthYZ, 4.0, "YZ Ortho");
  glColor4f( 1.0, 1.0, 1.0, XH_ALPHA );
#endif //DROP_SHADOWS

  glBegin(GL_LINE_LOOP);
    glVertex2d(0.0, 0.0);
    glVertex2d(0.0, 20.0);
    glVertex2d(100.0, 20.0);
    glVertex2d(100.0, 0.0);
  glEnd();
  OutputText(97.0-widthYZ, 5.0, "YZ Ortho");

// Clean Up
  glDisable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);
}

//////////////////////////////////////////////////////////////////////
// Use Control
//////////////////////////////////////////////////////////////////////

bool ViewMenu::inBBox( float mx, float my, float minx, float miny, float maxx, float maxy )
{
  if ( (mx >= minx) && (mx <= maxx) && (my >= miny) && (my <= maxy) ) return true;
  return false;
}


bool ViewMenu::MouseOver( int mx, int my )
{
  float cmx, cmy;
  int mval;
  bool ret;

  cmx = (float)mx - offsetx;
  cmy = (float)my - offsety;

  mval = VM_NONE;

  // Check Perspective
  if (inBBox(cmx, cmy, 0.0, 75.0, 100.0, 95.0)) mval = VM_PERSPECTIVE;

  // Check XYOrtho
  if (inBBox(cmx, cmy, 0.0, 50.0, 100.0, 70.0)) mval = VM_XYORTHO;

  // Check XZOrtho
  if (inBBox(cmx, cmy, 0.0, 25.0, 100.0, 45.0)) mval = VM_XZORTHO;

  // Check YZOrtho
  if (inBBox(cmx, cmy, 0.0, 0.0, 100.0, 20.0)) mval = VM_YZORTHO;

  ret = (mval != mcover);
  if (ret) mcover = mval;

  return (ret);
}

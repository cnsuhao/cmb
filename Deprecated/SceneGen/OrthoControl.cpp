// OrthoControl.cpp: implementation of the OrthoControl class.
//
//////////////////////////////////////////////////////////////////////

#include "OrthoControl.h"

#include <stdio.h>
#include <math.h>

#define SCALE 3.0

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

OrthoControl::OrthoControl()
{
  drawto = NULL;

  drot = 0.0;
  dpos.assign(0.0, 0.0, 0.0);

  offsetx = offsety = 0.0;
  mode = OC_ZOOM;
  mcover = OC_NONE;

  for (int ii = 0; ii<73; ii++)
  {
    costable[ii] = cos((double)ii*5.0*gmDEGTORAD);
    sintable[ii] = sin((double)ii*5.0*gmDEGTORAD);
  }

  action = OC_NONE;
  bw = bh = 0;
  cw = ch = 0;
}

OrthoControl::~OrthoControl()
{

}

//////////////////////////////////////////////////////////////////////
// Set Values
//////////////////////////////////////////////////////////////////////

void OrthoControl::SetOffset( float ox, float oy)
{
  offsetx = ox;
  offsety = oy;
}

//////////////////////////////////////////////////////////////////////
// Draw Control
//////////////////////////////////////////////////////////////////////
void OrthoControl::OutputText(float x, float y, const char *string)
{
  if (drawto==NULL) return;
  drawto->renderText(x,y, 0.0 ,string);
}

int OrthoControl::GetStringWidth( char *s )
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

void OrthoControl::PlotModeBorder( double offsetx, double offsety)
{
    glVertex2d(0.0*SCALE+offsetx, 1.0*SCALE+offsety);
    glVertex2d(0.0*SCALE+offsetx, 9.0*SCALE+offsety);
    glVertex2d(1.0*SCALE+offsetx, 9.0*SCALE+offsety);
    glVertex2d(1.0*SCALE+offsetx, 10.0*SCALE+offsety);
    glVertex2d(9.0*SCALE+offsetx, 10.0*SCALE+offsety);
    glVertex2d(9.0*SCALE+offsetx, 9.0*SCALE+offsety);
    glVertex2d(10.0*SCALE+offsetx, 9.0*SCALE+offsety);
    glVertex2d(10.0*SCALE+offsetx, 1.0*SCALE+offsety);
    glVertex2d(9.0*SCALE+offsetx, 1.0*SCALE+offsety);
    glVertex2d(9.0*SCALE+offsetx, 0.0*SCALE+offsety);
    glVertex2d(1.0*SCALE+offsetx, 0.0*SCALE+offsety);
    glVertex2d(1.0*SCALE+offsetx, 1.0*SCALE+offsety);
}

void OrthoControl::PlotButtonBorder( double offsetx, double offsety)
{
    glVertex2d(0.0*SCALE+offsetx, 1.0*SCALE+offsety);
    glVertex2d(0.0*SCALE+offsetx, 5.0*SCALE+offsety);
    glVertex2d(1.0*SCALE+offsetx, 6.0*SCALE+offsety);
    glVertex2d(5.0*SCALE+offsetx, 6.0*SCALE+offsety);
    glVertex2d(6.0*SCALE+offsetx, 5.0*SCALE+offsety);
    glVertex2d(6.0*SCALE+offsetx, 1.0*SCALE+offsety);
    glVertex2d(5.0*SCALE+offsetx, 0.0*SCALE+offsety);
    glVertex2d(1.0*SCALE+offsetx, 0.0*SCALE+offsety);
}

void OrthoControl::PlotDropShadow()
{
  long ii;

  glColor4f( 0.0, 0.0, 0.0, HI_ALPHA );

  // *** Plot Action Mode at top center of screen
  int width, height;
  float xc, yc;
  char outstr[20];

  width = drawto->width();
  height = drawto->height();
  switch( mode )
  {
  case OC_ZOOM:
    sprintf(outstr,"ZOOM");
    break;
  case OC_SELECT:
    sprintf(outstr,"SELECT");
    break;
  case OC_ROTATE:
    sprintf(outstr,"ROTATE");
    break;
  case OC_TRANSLATE:
    sprintf(outstr,"TRANSLATE");
    break;
  default:
    break;
  }
  yc = (float)height - 15.0;
  xc = (float)(width-GetStringWidth(outstr))*0.5;
  OutputText( xc+1, yc+1, outstr );

  // Offset Drawing to proper position
  glPushMatrix();

  glTranslatef(offsetx+1, offsety+1, 0);

//*** Plot TRANSLATE Symbol ***
  //Border
  glBegin(GL_LINE_LOOP);
    PlotModeBorder(0.0, 0.0);
  glEnd();

  glBegin(GL_LINES);
    //cross hairs
    glVertex2d(5.0*SCALE, 1.0*SCALE);
    glVertex2d(5.0*SCALE, 9.0*SCALE);
    glVertex2d(1.0*SCALE, 5.0*SCALE);
    glVertex2d(9.0*SCALE, 5.0*SCALE);
    //down arrow
    glVertex2d(5.0*SCALE, 1.0*SCALE);
    glVertex2d(4.0*SCALE, 2.0*SCALE);
    glVertex2d(5.0*SCALE, 1.0*SCALE);
    glVertex2d(6.0*SCALE, 2.0*SCALE);
    //up arrow
    glVertex2d(5.0*SCALE, 9.0*SCALE);
    glVertex2d(4.0*SCALE, 8.0*SCALE);
    glVertex2d(5.0*SCALE, 9.0*SCALE);
    glVertex2d(6.0*SCALE, 8.0*SCALE);
    //left arrow
    glVertex2d(1.0*SCALE, 5.0*SCALE);
    glVertex2d(2.0*SCALE, 4.0*SCALE);
    glVertex2d(1.0*SCALE, 5.0*SCALE);
    glVertex2d(2.0*SCALE, 6.0*SCALE);
    //right arrow
    glVertex2d(9.0*SCALE, 5.0*SCALE);
    glVertex2d(8.0*SCALE, 4.0*SCALE);
    glVertex2d(9.0*SCALE, 5.0*SCALE);
    glVertex2d(8.0*SCALE, 6.0*SCALE);
  glEnd();

  glBegin(GL_LINE_LOOP);
    for (ii=0; ii < 72; ii++) glVertex2d((costable[ii]+5.0)*SCALE,(sintable[ii]+5.0)*SCALE);
  glEnd();

//*** Plot ROTATE Symbol ***
  //Border
  glBegin(GL_LINE_LOOP);
    PlotModeBorder(0.0*SCALE, 11.0*SCALE);
  glEnd();

  glBegin(GL_LINES);
    //cross hairs
    glVertex2d(5.0*SCALE, 14.0*SCALE);
    glVertex2d(5.0*SCALE, 18.0*SCALE);
    glVertex2d(3.0*SCALE, 16.0*SCALE);
    glVertex2d(7.0*SCALE, 16.0*SCALE);
  glEnd();
  glBegin(GL_LINE_LOOP);
    //arrow
    glVertex2d(5.0*SCALE, 20.0*SCALE);
    glVertex2d(6.0*SCALE, 19.0*SCALE);
    glVertex2d(4.0*SCALE, 19.0*SCALE);
  glEnd();
  glBegin(GL_LINE_LOOP);
    for (ii=0; ii < 72; ii++) glVertex2d((costable[ii]+5.0)*SCALE,(sintable[ii]+16.0)*SCALE);
  glEnd();
  glBegin(GL_LINE_LOOP);
    for (ii=0; ii < 72; ii++) glVertex2d((costable[ii]*3.0+5.0)*SCALE,(sintable[ii]*3.0+16.0)*SCALE);
  glEnd();
  glBegin(GL_LINE_LOOP);
    for (ii=0; ii < 72; ii++) glVertex2d((costable[ii]*4.0+5.0)*SCALE,(sintable[ii]*4.0+16.0)*SCALE);
  glEnd();

//*** Plot SELECT Symbol ***
  //Border
  glBegin(GL_LINE_LOOP);
    PlotModeBorder(0.0*SCALE, 22.0*SCALE);
  glEnd();

  glBegin(GL_LINES);
    //cross hairs
    glVertex2d(5.0*SCALE, 25.0*SCALE);
    glVertex2d(5.0*SCALE, 29.0*SCALE);
    glVertex2d(3.0*SCALE, 27.0*SCALE);
    glVertex2d(7.0*SCALE, 27.0*SCALE);
    //area select
    glVertex2d(2.0*SCALE, 24.0*SCALE);
    glVertex2d(2.0*SCALE, 27.0*SCALE);
    glVertex2d(2.0*SCALE, 24.0*SCALE);
    glVertex2d(5.0*SCALE, 24.0*SCALE);

    glVertex2d(2.0*SCALE, 30.0*SCALE);
    glVertex2d(8.0*SCALE, 30.0*SCALE);
    glVertex2d(8.0*SCALE, 30.0*SCALE);
    glVertex2d(8.0*SCALE, 24.0*SCALE);

  glEnd();
  glBegin(GL_LINE_LOOP);
    for (ii=0; ii < 72; ii++) glVertex2d((costable[ii]+5.0)*SCALE,(sintable[ii]+27.0)*SCALE);
  glEnd();

//*** Plot ZOOM Symbol ***
  //Border
  glBegin(GL_LINE_LOOP);
    PlotModeBorder(0.0*SCALE, 33.0*SCALE);
  glEnd();

  glBegin(GL_LINES);
    //zoom box
    glVertex2d(3.0*SCALE, 38.0*SCALE);
    glVertex2d(3.0*SCALE, 37.0*SCALE);
    glVertex2d(3.0*SCALE, 37.0*SCALE);
    glVertex2d(4.0*SCALE, 37.0*SCALE);

    glVertex2d(3.0*SCALE, 39.0*SCALE);
    glVertex2d(5.0*SCALE, 39.0*SCALE);
    glVertex2d(5.0*SCALE, 39.0*SCALE);
    glVertex2d(5.0*SCALE, 37.0*SCALE);

    //handle
    glVertex2d(8.0*SCALE, 35.0*SCALE);
    glVertex2d(4.707107*SCALE, 38.707107*SCALE);

  glEnd();
  glBegin(GL_LINE_LOOP);
    for (ii=0; ii < 72; ii++) glVertex2d((costable[ii]*2.0+4.0)*SCALE,(sintable[ii]*2.0+38.0)*SCALE);
  glEnd();

//*** Plot CENTER Symbol ***
  //Border
  glBegin(GL_LINE_LOOP);
    PlotButtonBorder(11.0*SCALE, 23.0*SCALE);
  glEnd();

  glBegin(GL_LINES);
    glVertex2d(13.0*SCALE, 24.0*SCALE);
    glVertex2d(13.0*SCALE, 25.0*SCALE);
    glVertex2d(13.0*SCALE, 25.0*SCALE);
    glVertex2d(12.0*SCALE, 25.0*SCALE);

    glVertex2d(15.0*SCALE, 24.0*SCALE);
    glVertex2d(15.0*SCALE, 25.0*SCALE);
    glVertex2d(15.0*SCALE, 25.0*SCALE);
    glVertex2d(16.0*SCALE, 25.0*SCALE);

    glVertex2d(13.0*SCALE, 28.0*SCALE);
    glVertex2d(13.0*SCALE, 27.0*SCALE);
    glVertex2d(13.0*SCALE, 27.0*SCALE);
    glVertex2d(12.0*SCALE, 27.0*SCALE);

    glVertex2d(15.0*SCALE, 28.0*SCALE);
    glVertex2d(15.0*SCALE, 27.0*SCALE);
    glVertex2d(15.0*SCALE, 27.0*SCALE);
    glVertex2d(16.0*SCALE, 27.0*SCALE);
  glEnd();

//*** Plot PROJECT Symbol ***
  //Border
  glBegin(GL_LINE_LOOP);
    PlotButtonBorder(11.0*SCALE, 30.0*SCALE);
  glEnd();

  glBegin(GL_LINES);
    glVertex2d(12.0*SCALE, 33.0*SCALE);
    glVertex2d(16.0*SCALE, 33.0*SCALE);

    glVertex2d(14.0*SCALE, 34.0*SCALE);
    glVertex2d(15.0*SCALE, 35.0*SCALE);
    glVertex2d(14.0*SCALE, 34.0*SCALE);
    glVertex2d(13.0*SCALE, 35.0*SCALE);

    glVertex2d(14.0*SCALE, 32.0*SCALE);
    glVertex2d(15.0*SCALE, 31.0*SCALE);
    glVertex2d(14.0*SCALE, 32.0*SCALE);
    glVertex2d(13.0*SCALE, 31.0*SCALE);
  glEnd();

//*** Plot RESET Symbol ***
  //Border
  glBegin(GL_LINE_LOOP);
    PlotButtonBorder(11.0*SCALE, 37.0*SCALE);
  glEnd();

  glBegin(GL_LINE_LOOP);
    glVertex2d(12.0*SCALE, 38.0*SCALE);
    glVertex2d(12.0*SCALE, 42.0*SCALE);
    glVertex2d(16.0*SCALE, 42.0*SCALE);
    glVertex2d(16.0*SCALE, 38.0*SCALE);
  glEnd();
  glBegin(GL_LINES);
    glVertex2d(12.0*SCALE, 38.0*SCALE);
    glVertex2d(13.0*SCALE, 39.0*SCALE);

    glVertex2d(12.0*SCALE, 42.0*SCALE);
    glVertex2d(13.0*SCALE, 41.0*SCALE);

    glVertex2d(16.0*SCALE, 42.0*SCALE);
    glVertex2d(15.0*SCALE, 41.0*SCALE);

    glVertex2d(16.0*SCALE, 38.0*SCALE);
    glVertex2d(15.0*SCALE, 39.0*SCALE);
  glEnd();

//*** DONE ***
  glPopMatrix();
}

void OrthoControl::Plot()
{
  int ii;
  int width, height;
  float xc, yc;
  char outstr[80];

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

  // *** Given mode, plot bounding box, line, or rotation compass
  switch( action )
  {
  case OC_ZOOM:
  case OC_SELECT:
    glColor4f( 1.0, 0.8, 0.1, XH_ALPHA);

    glBegin(GL_LINE_LOOP);
      glVertex2f( (float) bw, (float)bh );
      glVertex2f( (float) bw, (float)ch );
      glVertex2f( (float) cw, (float)ch );
      glVertex2f( (float) cw, (float)bh );
    glEnd();
    break;
  case OC_ROTATE:
    glColor4f( 1.0, 0.8, 0.1, XH_ALPHA );
    //boundary disk plotting
    //glBegin(GL_LINE_LOOP);
    //  for (ii=0; ii < 72; ii++) glVertex2d((costable[ii]*50.0)+bw,(sintable[ii]*50.0)+bh);
    //glEnd();
    glBegin(GL_LINE_LOOP);
      for (ii=0; ii < 72; ii++) glVertex2d((costable[ii]*55.0)+bw,(sintable[ii]*55.0)+bh);
    glEnd();
    //direction triangle plotting
    float crot, srot;
    float crotp, srotp;
    float crotm, srotm;
    crot = cos((drot+90.0)*gmDEGTORAD);
    srot = sin((drot+90.0)*gmDEGTORAD);
    crotp = cos((drot+95.0)*gmDEGTORAD);
    srotp = sin((drot+95.0)*gmDEGTORAD);
    crotm = cos((drot+85.0)*gmDEGTORAD);
    srotm = sin((drot+85.0)*gmDEGTORAD);
    glBegin(GL_TRIANGLES);
      glVertex2f( (crot*55)+bw, (srot*55)+bh );
      glVertex2f( (crotp*45)+bw, (srotp*45)+bh );
      glVertex2f( (crotm*45)+bw, (srotm*45)+bh );
    glEnd();
    // center crosshair plotting
    glBegin(GL_LINES);
      glVertex2f( (float)bw+5.0, (float)bh );
      glVertex2f( (float)bw-5.0, (float)bh );
      glVertex2f( (float)bw, (float)bh+5.0 );
      glVertex2f( (float)bw, (float)bh-5.0 );
    glEnd();
    break;
/*
  case OC_TRANSLATE:
    glColor4f( 1.0, 0.5, 0.1, 0.75 );
    glBegin(GL_LINES);
      glVertex2f( (float) bw, (float)bh );
      glVertex2f( (float) cw, (float)ch );
    glEnd();
    break;
*/
  default:
    break;
  }
#ifdef DROP_SHADOWS
  PlotDropShadow();
#endif

  // *** Plot Action Mode at top center of screen
  glColor4f( 1.0, 1.0, 1.0, XH_ALPHA );
  switch( mode )
  {
  case OC_ZOOM:
    sprintf(outstr,"ZOOM");
    break;
  case OC_SELECT:
    sprintf(outstr,"SELECT");
    break;
  case OC_ROTATE:
    sprintf(outstr,"ROTATE");
    break;
  case OC_TRANSLATE:
    sprintf(outstr,"TRANSLATE");
    break;
  default:
    break;
  }
  yc = (float)height - 15.0;
  xc = (float)(width-GetStringWidth(outstr))*0.5;
  OutputText( xc, yc, outstr );

  // Offset Drawing to proper position
  glTranslatef(offsetx, offsety, 0);

  glColor4f( 1.0, 1.0, 1.0, HI_ALPHA );
//*** Plot TRANSLATE Symbol ***
  //Border
  glBegin(GL_LINE_LOOP);
    PlotModeBorder(0.0, 0.0);
  glEnd();

  glBegin(GL_LINES);
    //cross hairs
    glVertex2d(5.0*SCALE, 1.0*SCALE);
    glVertex2d(5.0*SCALE, 9.0*SCALE);
    glVertex2d(1.0*SCALE, 5.0*SCALE);
    glVertex2d(9.0*SCALE, 5.0*SCALE);
    //down arrow
    glVertex2d(5.0*SCALE, 1.0*SCALE);
    glVertex2d(4.0*SCALE, 2.0*SCALE);
    glVertex2d(5.0*SCALE, 1.0*SCALE);
    glVertex2d(6.0*SCALE, 2.0*SCALE);
    //up arrow
    glVertex2d(5.0*SCALE, 9.0*SCALE);
    glVertex2d(4.0*SCALE, 8.0*SCALE);
    glVertex2d(5.0*SCALE, 9.0*SCALE);
    glVertex2d(6.0*SCALE, 8.0*SCALE);
    //left arrow
    glVertex2d(1.0*SCALE, 5.0*SCALE);
    glVertex2d(2.0*SCALE, 4.0*SCALE);
    glVertex2d(1.0*SCALE, 5.0*SCALE);
    glVertex2d(2.0*SCALE, 6.0*SCALE);
    //right arrow
    glVertex2d(9.0*SCALE, 5.0*SCALE);
    glVertex2d(8.0*SCALE, 4.0*SCALE);
    glVertex2d(9.0*SCALE, 5.0*SCALE);
    glVertex2d(8.0*SCALE, 6.0*SCALE);
  glEnd();

  glBegin(GL_LINE_LOOP);
    for (ii=0; ii < 72; ii++) glVertex2d((costable[ii]+5.0)*SCALE,(sintable[ii]+5.0)*SCALE);
  glEnd();

  if (mode == OC_TRANSLATE)
  {
    glColor4f( 1.0, 1.0, 1.0, LO_ALPHA );
    glBegin(GL_POLYGON);
      PlotModeBorder(0.0, 0.0);
    glEnd();
    glColor4f( 1.0, 1.0, 1.0, HI_ALPHA );
  }

//*** Plot ROTATE Symbol ***
  //Border
  glBegin(GL_LINE_LOOP);
    PlotModeBorder(0.0*SCALE, 11.0*SCALE);
  glEnd();

  glBegin(GL_LINES);
    //cross hairs
    glVertex2d(5.0*SCALE, 14.0*SCALE);
    glVertex2d(5.0*SCALE, 18.0*SCALE);
    glVertex2d(3.0*SCALE, 16.0*SCALE);
    glVertex2d(7.0*SCALE, 16.0*SCALE);
  glEnd();
  glBegin(GL_LINE_LOOP);
    //arrow
    glVertex2d(5.0*SCALE, 20.0*SCALE);
    glVertex2d(6.0*SCALE, 19.0*SCALE);
    glVertex2d(4.0*SCALE, 19.0*SCALE);
  glEnd();
  glBegin(GL_LINE_LOOP);
    for (ii=0; ii < 72; ii++) glVertex2d((costable[ii]+5.0)*SCALE,(sintable[ii]+16.0)*SCALE);
  glEnd();
  glBegin(GL_LINE_LOOP);
    for (ii=0; ii < 72; ii++) glVertex2d((costable[ii]*3.0+5.0)*SCALE,(sintable[ii]*3.0+16.0)*SCALE);
  glEnd();
  glBegin(GL_LINE_LOOP);
    for (ii=0; ii < 72; ii++) glVertex2d((costable[ii]*4.0+5.0)*SCALE,(sintable[ii]*4.0+16.0)*SCALE);
  glEnd();

  if (mode == OC_ROTATE)
  {
    glColor4f( 1.0, 1.0, 1.0, LO_ALPHA );
    glBegin(GL_POLYGON);
      PlotModeBorder(0.0*SCALE, 11.0*SCALE);
    glEnd();
    glColor4f( 1.0, 1.0, 1.0, HI_ALPHA );
  }

//*** Plot SELECT Symbol ***
  //Border
  glBegin(GL_LINE_LOOP);
    PlotModeBorder(0.0*SCALE, 22.0*SCALE);
  glEnd();

  glBegin(GL_LINES);
    //cross hairs
    glVertex2d(5.0*SCALE, 25.0*SCALE);
    glVertex2d(5.0*SCALE, 29.0*SCALE);
    glVertex2d(3.0*SCALE, 27.0*SCALE);
    glVertex2d(7.0*SCALE, 27.0*SCALE);
    //area select
    glVertex2d(2.0*SCALE, 24.0*SCALE);
    glVertex2d(2.0*SCALE, 27.0*SCALE);
    glVertex2d(2.0*SCALE, 24.0*SCALE);
    glVertex2d(5.0*SCALE, 24.0*SCALE);

    glVertex2d(2.0*SCALE, 30.0*SCALE);
    glVertex2d(8.0*SCALE, 30.0*SCALE);
    glVertex2d(8.0*SCALE, 30.0*SCALE);
    glVertex2d(8.0*SCALE, 24.0*SCALE);

  glEnd();
  glBegin(GL_LINE_LOOP);
    for (ii=0; ii < 72; ii++) glVertex2d((costable[ii]+5.0)*SCALE,(sintable[ii]+27.0)*SCALE);
  glEnd();

  if (mode == OC_SELECT)
  {
    glColor4f( 1.0, 1.0, 1.0, LO_ALPHA );
    glBegin(GL_POLYGON);
      PlotModeBorder(0.0*SCALE, 22.0*SCALE);
    glEnd();
    glColor4f( 1.0, 1.0, 1.0, HI_ALPHA );
  }

//*** Plot ZOOM Symbol ***
  //Border
  glBegin(GL_LINE_LOOP);
    PlotModeBorder(0.0*SCALE, 33.0*SCALE);
  glEnd();

  glBegin(GL_LINES);
    //zoom box
    glVertex2d(3.0*SCALE, 38.0*SCALE);
    glVertex2d(3.0*SCALE, 37.0*SCALE);
    glVertex2d(3.0*SCALE, 37.0*SCALE);
    glVertex2d(4.0*SCALE, 37.0*SCALE);

    glVertex2d(3.0*SCALE, 39.0*SCALE);
    glVertex2d(5.0*SCALE, 39.0*SCALE);
    glVertex2d(5.0*SCALE, 39.0*SCALE);
    glVertex2d(5.0*SCALE, 37.0*SCALE);

    //handle
    glVertex2d(8.0*SCALE, 35.0*SCALE);
    glVertex2d(4.707107*SCALE, 38.707107*SCALE);

  glEnd();
  glBegin(GL_LINE_LOOP);
    for (ii=0; ii < 72; ii++) glVertex2d((costable[ii]*2.0+4.0)*SCALE,(sintable[ii]*2.0+38.0)*SCALE);
  glEnd();

  if (mode == OC_ZOOM)
  {
    glColor4f( 1.0, 1.0, 1.0, LO_ALPHA );
    glBegin(GL_POLYGON);
      PlotModeBorder(0.0*SCALE, 33.0*SCALE);
    glEnd();
    glColor4f( 1.0, 1.0, 1.0, HI_ALPHA );
  }

//*** Plot CENTER Symbol ***
  //Border
  glBegin(GL_LINE_LOOP);
    PlotButtonBorder(11.0*SCALE, 23.0*SCALE);
  glEnd();

  glBegin(GL_LINES);
    glVertex2d(13.0*SCALE, 24.0*SCALE);
    glVertex2d(13.0*SCALE, 25.0*SCALE);
    glVertex2d(13.0*SCALE, 25.0*SCALE);
    glVertex2d(12.0*SCALE, 25.0*SCALE);

    glVertex2d(15.0*SCALE, 24.0*SCALE);
    glVertex2d(15.0*SCALE, 25.0*SCALE);
    glVertex2d(15.0*SCALE, 25.0*SCALE);
    glVertex2d(16.0*SCALE, 25.0*SCALE);

    glVertex2d(13.0*SCALE, 28.0*SCALE);
    glVertex2d(13.0*SCALE, 27.0*SCALE);
    glVertex2d(13.0*SCALE, 27.0*SCALE);
    glVertex2d(12.0*SCALE, 27.0*SCALE);

    glVertex2d(15.0*SCALE, 28.0*SCALE);
    glVertex2d(15.0*SCALE, 27.0*SCALE);
    glVertex2d(15.0*SCALE, 27.0*SCALE);
    glVertex2d(16.0*SCALE, 27.0*SCALE);
  glEnd();
//  glBegin(GL_LINE_LOOP);
//    for (ii=0; ii < 72; ii++) glVertex2d((costable[ii]*0.5+14.0)*SCALE,(sintable[ii]*0.5+26.0)*SCALE);
//  glEnd();

  if (mcover == OC_CENTERBUTTON)
  {
    glColor4f( 1.0, 1.0, 1.0, LO_ALPHA );
    glBegin(GL_POLYGON);
      PlotButtonBorder(11.0*SCALE, 23.0*SCALE);
    glEnd();
    glColor4f( 1.0, 1.0, 1.0, HI_ALPHA );
  }

//*** Plot PROJECT Symbol ***
  //Border
  glBegin(GL_LINE_LOOP);
    PlotButtonBorder(11.0*SCALE, 30.0*SCALE);
  glEnd();

  glBegin(GL_LINES);
    glVertex2d(12.0*SCALE, 33.0*SCALE);
    glVertex2d(16.0*SCALE, 33.0*SCALE);

    glVertex2d(14.0*SCALE, 34.0*SCALE);
    glVertex2d(15.0*SCALE, 35.0*SCALE);
    glVertex2d(14.0*SCALE, 34.0*SCALE);
    glVertex2d(13.0*SCALE, 35.0*SCALE);

    glVertex2d(14.0*SCALE, 32.0*SCALE);
    glVertex2d(15.0*SCALE, 31.0*SCALE);
    glVertex2d(14.0*SCALE, 32.0*SCALE);
    glVertex2d(13.0*SCALE, 31.0*SCALE);
  glEnd();

  if (mcover == OC_PROJECTBUTTON)
  {
    glColor4f( 1.0, 1.0, 1.0, LO_ALPHA );
    glBegin(GL_POLYGON);
      PlotButtonBorder(11.0*SCALE, 30.0*SCALE);
    glEnd();
    glColor4f( 1.0, 1.0, 1.0, HI_ALPHA );
  }

//*** Plot RESET Symbol ***
  //Border
  glBegin(GL_LINE_LOOP);
    PlotButtonBorder(11.0*SCALE, 37.0*SCALE);
  glEnd();

  glBegin(GL_LINE_LOOP);
    glVertex2d(12.0*SCALE, 38.0*SCALE);
    glVertex2d(12.0*SCALE, 42.0*SCALE);
    glVertex2d(16.0*SCALE, 42.0*SCALE);
    glVertex2d(16.0*SCALE, 38.0*SCALE);
  glEnd();
  glBegin(GL_LINES);
    glVertex2d(12.0*SCALE, 38.0*SCALE);
    glVertex2d(13.0*SCALE, 39.0*SCALE);

    glVertex2d(12.0*SCALE, 42.0*SCALE);
    glVertex2d(13.0*SCALE, 41.0*SCALE);

    glVertex2d(16.0*SCALE, 42.0*SCALE);
    glVertex2d(15.0*SCALE, 41.0*SCALE);

    glVertex2d(16.0*SCALE, 38.0*SCALE);
    glVertex2d(15.0*SCALE, 39.0*SCALE);
  glEnd();

  if (mcover == OC_RESETBUTTON)
  {
    glColor4f( 1.0, 1.0, 1.0, LO_ALPHA );
    glBegin(GL_POLYGON);
      PlotButtonBorder(11.0*SCALE, 37.0*SCALE);
    glEnd();
    glColor4f( 1.0, 1.0, 1.0, HI_ALPHA );
  }

// Clean Up
  glDisable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);
}

//////////////////////////////////////////////////////////////////////
// Use Control
//////////////////////////////////////////////////////////////////////

bool OrthoControl::inCircle( float mx, float my, float cx, float cy, float rad )
{
  float ds, sr;

  ds = ((cx-mx)*(cx-mx)) + (cy-my)*(cy-my);
  sr = rad*rad;

  if (ds < sr) return true;
  return false;
}

bool OrthoControl::inTriangle( float mx, float my, float px1, float py1, float px2, float py2, float px3, float py3 )
{
  float fab, fca, fbc;

  fab = ((my-py1)*(px2-px1)) - ((mx-px1)*(py2-py1));
  fca = ((my-py3)*(px1-px3)) - ((mx-px3)*(py1-py3));
  fbc = ((my-py2)*(px3-px2)) - ((mx-px2)*(py3-py2));

  if ( ( (fab * fbc) > 0.0 ) && ( (fbc * fca) > 0.0 ) ) return true;
  return false;
}

bool OrthoControl::inBBox( float mx, float my, float minx, float miny, float maxx, float maxy )
{
  if ( (mx >= minx) && (mx <= maxx) && (my >= miny) && (my <= maxy) ) return true;
  return false;
}

bool OrthoControl::MouseOver( int mx, int my )
{
  float cmx, cmy;
  int mval;
  bool ret;

  cmx = (float)mx - offsetx;
  cmy = (float)my - offsety;

  mval = OC_NONE;

  // Check Translate Symbol
  if (inBBox(cmx, cmy, 0.0*SCALE, 0.0*SCALE, 10.0*SCALE, 10.0*SCALE)) mval = OC_TRANSLATE;

  // Check Rotate Symbol
  if (inBBox(cmx, cmy, 0.0*SCALE, 11.0*SCALE, 10.0*SCALE, 21.0*SCALE)) mval = OC_ROTATE;

  // Check Select Symbol
  if (inBBox(cmx, cmy, 0.0*SCALE, 22.0*SCALE, 10.0*SCALE, 32.0*SCALE)) mval = OC_SELECT;

  // Check Zoom Symbol
  if (inBBox(cmx, cmy, 0.0*SCALE, 33.0*SCALE, 10.0*SCALE, 43.0*SCALE)) mval = OC_ZOOM;

  // Check Center Button Symbol
  if (inBBox(cmx, cmy, 11.0*SCALE, 23.0*SCALE, 16.0*SCALE, 29.0*SCALE)) mval = OC_CENTERBUTTON;

  // Check Project Button Symbol
  if (inBBox(cmx, cmy, 11.0*SCALE, 30.0*SCALE, 16.0*SCALE, 36.0*SCALE)) mval = OC_PROJECTBUTTON;

  // Check Reset Button Symbol
  if (inBBox(cmx, cmy, 11.0*SCALE, 37.0*SCALE, 16.0*SCALE, 43.0*SCALE)) mval = OC_RESETBUTTON;


  ret = (mval != mcover);
  if (ret) mcover = mval;

  return (ret);
}

void OrthoControl::SetAngle( int mx, int my )
{
  double cmx, cmy;
  double ra, da;

  cmx = ((double)mx - offsetx) - 50.0;
  cmy = ((double)my - offsety) - 50.0;

  ra = atan2(cmx, cmy);
  da = 360.0 - (ra * gmRADTODEG);

  drot = da;

}

void OrthoControl::SetCurrent( long icw, long ich )
{
  cw=icw;
  ch=ich;

  if (action==OC_ROTATE)
  {
    double cmx, cmy;
    double ra, da;

    cmx = (double)(cw - bw);
    cmy = (double)(ch - bh);

    ra = atan2(cmx, cmy);
    da = -(ra * gmRADTODEG);
    if (da<0.0) da+=360.0;

    drot = da;
  }
}

bool OrthoControl::GetActionDelta( long &odw, long &odh )
{
  odw = cw - bw;
  odh = ch - bh;

  if ((odw==0)&&(odh==0)) return false;

  return true;
}

bool OrthoControl::GetActionEndPoints( long &obw, long &obh, long &ocw, long &och )
{
  long odw, odh;

  obw = bw;
  obh = bh;
  ocw = cw;
  och = ch;

  odw = cw - bw;
  odh = ch - bh;

  if ((odw==0)&&(odh==0)) return false;

  return true;
}

void OrthoControl::TranslateAction( double dx, double dy, double dz )
{
  if (action!=OC_TRANSLATE) return;

  dpos[0] = dx;
  dpos[1] = dy;
  dpos[2] = dz;
}

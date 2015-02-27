// PerspControl.cpp: implementation of the PerspControl class.
//
//////////////////////////////////////////////////////////////////////

#include "PerspControl.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PerspControl::PerspControl()
{
  drawto = NULL;

  offsetx = offsety = 0.0;
  mode = SC_ROTATE;
  mcover = SC_NONE;

  for (int ii = 0; ii<73; ii++)
  {
    costable[ii] = 10.0*cos((double)ii*5.0*gmDEGTORAD);
    sintable[ii] = 10.0*sin((double)ii*5.0*gmDEGTORAD);
  }
}

PerspControl::~PerspControl()
{

}


//////////////////////////////////////////////////////////////////////
// Set Values
//////////////////////////////////////////////////////////////////////

void PerspControl::SetOffset( float ox, float oy)
{
  offsetx = ox;
  offsety = oy;
}

//////////////////////////////////////////////////////////////////////
// Draw Control
//////////////////////////////////////////////////////////////////////
void PerspControl::OutputText(float x, float y, const char *string)
{
  if (drawto==NULL) return;
  drawto->renderText(x,y, 0.0 ,string);
}

int PerspControl::GetStringWidth( char *s )
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

void PerspControl::PlotDropShadow()
{
  long ii;

  int width, height;
  float xc, yc;
  char outstr[20];

  //Show the mode center screen
  width = drawto->width();
  height = drawto->height();
  switch( mode )
  {
  case SC_ROTATE:
    sprintf(outstr,"ROTATE ABOUT");
    break;
  case SC_DRIVE:
    sprintf(outstr,"DRIVE");
    break;
  case SC_SELECT:
    sprintf(outstr,"SELECT");
    break;
  default:
    break;
  }
  yc = (float)height - 15.0;
  xc = (float)(width-GetStringWidth(outstr))*0.5;
  glColor4f( 0.0, 0.0, 0.0, HI_ALPHA );
  OutputText( xc+1, yc+1, outstr );

  glPushMatrix();

    // Offset Drawing to proper position
  glTranslatef(offsetx+1, offsety-1, 0);

  glColor4f( 0.0, 0.0, 0.0, HI_ALPHA );

  // Plot Select Symbol
    //plot the circle
  glBegin(GL_LINES);
    glVertex2d(5.0, 90.0);
    glVertex2d(15.0, 90.0);

    glVertex2d(10.0, 85.0);
    glVertex2d(10.0, 95.0);
  glEnd();
  glBegin(GL_LINE_LOOP);
    for (ii=0; ii < 72; ii++) glVertex2d(costable[ii]+10.0,sintable[ii]+90.0);
  glEnd();

  // Plot Rotate About Symbol
  glBegin(GL_LINES);
    glVertex2d(6.0, 10.0);
    glVertex2d(10.0, 10.0);

    glVertex2d(10.0, 6.0);
    glVertex2d(10.0, 10.0);

    glVertex2d(1.0, 1.0);
    glVertex2d(10.0, 10.0);
  glEnd();
  glBegin(GL_LINE_LOOP);
    for (ii=0; ii < 72; ii++) glVertex2d(costable[ii]+10.0,sintable[ii]+10.0);
  glEnd();

  // Plot Drive Symbol
  glBegin(GL_LINES);
    glVertex2d(65.0, 10.0);
    glVertex2d(75.0, 15.0);

    glVertex2d(65.0, 10.0);
    glVertex2d(75.0, 5.0);
  glEnd();
  glBegin(GL_LINE_LOOP);
    for (ii=68; ii<72; ii++) glVertex2d(costable[ii]+65.0,sintable[ii]+10.0);
    for (ii=0; ii<5; ii++) glVertex2d(costable[ii]+65.0,sintable[ii]+10.0);
  glEnd();

  glBegin(GL_LINE_LOOP);
    for (ii=0; ii < 72; ii++) glVertex2d(costable[ii]+70.0,sintable[ii]+10.0);
  glEnd();

  // Plot Pan Left
  glBegin(GL_LINE_LOOP);
    glVertex2d(0.0, 50.0);
    glVertex2d(15.0, 40.0);
    glVertex2d(15.0, 60.0);
  glEnd();

  // Plot Pan Right
  glBegin(GL_LINE_LOOP);
    glVertex2d(80.0, 50.0);
    glVertex2d(65.0, 60.0);
    glVertex2d(65.0, 40.0);
  glEnd();

  // Plot Pan Up
  glBegin(GL_LINE_LOOP);
    glVertex2d(40.0, 90.0);
    glVertex2d(30.0, 75.0);
    glVertex2d(50.0, 75.0);
  glEnd();

  // Plot Pan Down
  glBegin(GL_LINE_LOOP);
    glVertex2d(40.0, 10.0);
    glVertex2d(50.0, 25.0);
    glVertex2d(30.0, 25.0);
  glEnd();

  // Plot pan fore
  glBegin(GL_LINE_STRIP);
    glVertex2d(31.34, 55.0);
    glVertex2d(25.0, 55.0);
    glVertex2d(40.0, 70.0);
    glVertex2d(55.0, 55.0);
    glVertex2d(48.66, 55.0);
  glEnd();

  // Plot Center
  glBegin(GL_LINE_STRIP);
    for (ii=66; ii<72; ii++) glVertex2d(costable[ii]+40.0,sintable[ii]+50.0);
    for (ii=0; ii<45; ii++) glVertex2d(costable[ii]+40.0,sintable[ii]+50.0);
  glEnd();

  // Plot Pan Aft
  glBegin(GL_LINE_LOOP);
    glVertex2d(40.0, 30.0);
    glVertex2d(25.0, 45.0);
    glVertex2d(55.0, 45.0);
  glEnd();

  // Plot Zoom Control

  glPopMatrix();
}

void PerspControl::Plot()
{
  int ii;
  int width, height;
  float xc, yc;
  char outstr[20];

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


#ifdef DROP_SHADOWS
  PlotDropShadow();
#endif

  //Show the mode center screen
  switch( mode )
  {
  case SC_ROTATE:
    sprintf(outstr,"ROTATE ABOUT");
    break;
  case SC_DRIVE:
    sprintf(outstr,"DRIVE");
    break;
  case SC_SELECT:
    sprintf(outstr,"SELECT");
    break;
  default:
    break;
  }
  yc = (float)height - 15.0;
  xc = (float)(width-GetStringWidth(outstr))*0.5;
  glColor4f( 1.0, 1.0, 1.0, XH_ALPHA );
  OutputText( xc, yc, outstr );

  // Offset Drawing to proper position
  glTranslatef(offsetx, offsety, 0);

  glColor4f( 1.0, 1.0, 1.0, HI_ALPHA );

  // Plot Select Symbol
    //plot the circle
  glBegin(GL_LINES);
    glVertex2d(5.0, 90.0);
    glVertex2d(15.0, 90.0);

    glVertex2d(10.0, 85.0);
    glVertex2d(10.0, 95.0);
  glEnd();
  glBegin(GL_LINE_LOOP);
    for (ii=0; ii < 72; ii++) glVertex2d(costable[ii]+10.0,sintable[ii]+90.0);
  glEnd();

  if (mode == SC_SELECT)
  {
    glColor4f( 1.0, 1.0, 1.0, LO_ALPHA );
    glBegin(GL_POLYGON);
      for (ii=0; ii < 72; ii++) glVertex2d(costable[ii]+10.0,sintable[ii]+90.0);
    glEnd();
    glColor4f( 1.0, 1.0, 1.0, HI_ALPHA );
  }

  // Plot Rotate About Symbol
  glBegin(GL_LINES);
    glVertex2d(6.0, 10.0);
    glVertex2d(10.0, 10.0);

    glVertex2d(10.0, 6.0);
    glVertex2d(10.0, 10.0);

    glVertex2d(1.0, 1.0);
    glVertex2d(10.0, 10.0);
  glEnd();
  glBegin(GL_LINE_LOOP);
    for (ii=0; ii < 72; ii++) glVertex2d(costable[ii]+10.0,sintable[ii]+10.0);
  glEnd();

  if (mode == SC_ROTATE)
  {
    glColor4f( 1.0, 1.0, 1.0, LO_ALPHA );
    glBegin(GL_POLYGON);
      for (ii=0; ii < 72; ii++) glVertex2d(costable[ii]+10.0,sintable[ii]+10.0);
    glEnd();
    glColor4f( 1.0, 1.0, 1.0, HI_ALPHA );
  }

  // Plot Drive Symbol
  glBegin(GL_LINES);
    glVertex2d(65.0, 10.0);
    glVertex2d(75.0, 15.0);

    glVertex2d(65.0, 10.0);
    glVertex2d(75.0, 5.0);
  glEnd();
  glBegin(GL_LINE_LOOP);
    for (ii=68; ii<72; ii++) glVertex2d(costable[ii]+65.0,sintable[ii]+10.0);
    for (ii=0; ii<5; ii++) glVertex2d(costable[ii]+65.0,sintable[ii]+10.0);
  glEnd();

  glBegin(GL_LINE_LOOP);
    for (ii=0; ii < 72; ii++) glVertex2d(costable[ii]+70.0,sintable[ii]+10.0);
  glEnd();

  if (mode == SC_DRIVE)
  {
    glColor4f( 1.0, 1.0, 1.0, LO_ALPHA );
    glBegin(GL_POLYGON);
      for (ii=0; ii < 72; ii++) glVertex2d(costable[ii]+70.0,sintable[ii]+10.0);
    glEnd();
    glColor4f( 1.0, 1.0, 1.0, HI_ALPHA );
  }


  // Plot Pan Left
  glBegin(GL_LINE_LOOP);
    glVertex2d(0.0, 50.0);
    glVertex2d(15.0, 40.0);
    glVertex2d(15.0, 60.0);
  glEnd();

  if (mcover == SC_PANLEFT)
  {
    glColor4f( 1.0, 1.0, 1.0, LO_ALPHA );
    glBegin(GL_POLYGON);
      glVertex2d(0.0, 50.0);
      glVertex2d(15.0, 40.0);
      glVertex2d(15.0, 60.0);
    glEnd();
    glColor4f( 1.0, 1.0, 1.0, HI_ALPHA );
  }


  // Plot Pan Right
  glBegin(GL_LINE_LOOP);
    glVertex2d(80.0, 50.0);
    glVertex2d(65.0, 60.0);
    glVertex2d(65.0, 40.0);
  glEnd();

  if (mcover == SC_PANRIGHT)
  {
    glColor4f( 1.0, 1.0, 1.0, LO_ALPHA );
    glBegin(GL_POLYGON);
      glVertex2d(80.0, 50.0);
      glVertex2d(65.0, 60.0);
      glVertex2d(65.0, 40.0);
    glEnd();
    glColor4f( 1.0, 1.0, 1.0, HI_ALPHA );
  }


  // Plot Pan Up
  glBegin(GL_LINE_LOOP);
    glVertex2d(40.0, 90.0);
    glVertex2d(30.0, 75.0);
    glVertex2d(50.0, 75.0);
  glEnd();

  if (mcover == SC_PANUP)
  {
    glColor4f( 1.0, 1.0, 1.0, LO_ALPHA );
    glBegin(GL_POLYGON);
      glVertex2d(40.0, 90.0);
      glVertex2d(30.0, 75.0);
      glVertex2d(50.0, 75.0);
    glEnd();
    glColor4f( 1.0, 1.0, 1.0, HI_ALPHA );
  }

  // Plot Pan Down
  glBegin(GL_LINE_LOOP);
    glVertex2d(40.0, 10.0);
    glVertex2d(50.0, 25.0);
    glVertex2d(30.0, 25.0);
  glEnd();

  if (mcover == SC_PANDOWN)
  {
    glColor4f( 1.0, 1.0, 1.0, LO_ALPHA );
    glBegin(GL_POLYGON);
      glVertex2d(40.0, 10.0);
      glVertex2d(50.0, 25.0);
      glVertex2d(30.0, 25.0);
    glEnd();
    glColor4f( 1.0, 1.0, 1.0, HI_ALPHA );
  }

  // Plot pan fore
  glBegin(GL_LINE_STRIP);
    glVertex2d(31.34, 55.0);
    glVertex2d(25.0, 55.0);
    glVertex2d(40.0, 70.0);
    glVertex2d(55.0, 55.0);
    glVertex2d(48.66, 55.0);
  glEnd();

  if (mcover == SC_PANFORE)
  {
    glColor4f( 1.0, 1.0, 1.0, LO_ALPHA );
    glBegin(GL_TRIANGLE_FAN);
      glVertex2d(25.0, 55.0);
      glVertex2d(40.0, 70.0);
      for (ii = 18; ii < 31; ii++) glVertex2d(costable[ii]+40.0,sintable[ii]+50.0);
    glEnd();
    glBegin(GL_TRIANGLE_FAN);
      glVertex2d(55.0, 55.0);
      for (ii = 6; ii < 19; ii++) glVertex2d(costable[ii]+40.0,sintable[ii]+50.0);
      glVertex2d(40.0, 70.0);
    glEnd();
    glColor4f( 1.0, 1.0, 1.0, HI_ALPHA );
  }

  // Plot Center
  glBegin(GL_LINE_STRIP);
    for (ii=66; ii<72; ii++) glVertex2d(costable[ii]+40.0,sintable[ii]+50.0);
    for (ii=0; ii<45; ii++) glVertex2d(costable[ii]+40.0,sintable[ii]+50.0);
  glEnd();

  if (mcover == SC_RESET)
  {
    glColor4f( 1.0, 1.0, 1.0, LO_ALPHA );
    glBegin(GL_POLYGON);
      for (ii=66; ii<72; ii++) glVertex2d(costable[ii]+40.0,sintable[ii]+50.0);
      for (ii=0; ii<45; ii++) glVertex2d(costable[ii]+40.0,sintable[ii]+50.0);
    glEnd();
    glColor4f( 1.0, 1.0, 1.0, HI_ALPHA );
  }

  // Plot Pan Aft
  glBegin(GL_LINE_LOOP);
    glVertex2d(40.0, 30.0);
    glVertex2d(25.0, 45.0);
    glVertex2d(55.0, 45.0);
  glEnd();

  if (mcover == SC_PANAFT)
  {
    glColor4f( 1.0, 1.0, 1.0, LO_ALPHA );
    glBegin(GL_POLYGON);
      glVertex2d(40.0, 30.0);
      glVertex2d(25.0, 45.0);
      glVertex2d(55.0, 45.0);
    glEnd();
    glColor4f( 1.0, 1.0, 1.0, HI_ALPHA );
  }

  // Plot Zoom Control

// Clean Up
  glDisable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);
}

//////////////////////////////////////////////////////////////////////
// Use Control
//////////////////////////////////////////////////////////////////////

bool PerspControl::inCircle( float mx, float my, float cx, float cy, float rad )
{
  float ds, sr;

  ds = ((cx-mx)*(cx-mx)) + (cy-my)*(cy-my);
  sr = rad*rad;

  if (ds < sr) return true;
  return false;
}

bool PerspControl::inTriangle( float mx, float my, float px1, float py1, float px2, float py2, float px3, float py3 )
{
  float fab, fca, fbc;

  fab = ((my-py1)*(px2-px1)) - ((mx-px1)*(py2-py1));
  fca = ((my-py3)*(px1-px3)) - ((mx-px3)*(py1-py3));
  fbc = ((my-py2)*(px3-px2)) - ((mx-px2)*(py3-py2));

  if ( ( (fab * fbc) > 0.0 ) && ( (fbc * fca) > 0.0 ) ) return true;
  return false;
}

bool PerspControl::MouseOver( int mx, int my )
{
  float cmx, cmy;
  int mval;
  bool ret;

  cmx = (float)mx - offsetx;
  cmy = (float)my - offsety;

  mval = SC_NONE;

  // Check Select Symbol
  if (inCircle(cmx, cmy, 10.0, 90.0)) mval = SC_SELECT;

  // Check Rotate About Symbol
  if (inCircle(cmx, cmy, 10.0, 10.0)) mval = SC_ROTATE;

  // Check Drive Symbol
  if (inCircle(cmx, cmy, 70.0, 10.0)) mval = SC_DRIVE;

  // Check Pan Left Symbol
  if (inTriangle(cmx, cmy, 0.0, 50.0, 15.0, 40.0, 15.0, 60.0)) mval = SC_PANLEFT;

  // Check Pan Right Symbol
  if (inTriangle(cmx, cmy, 80.0, 50.0, 65.0, 60.0, 65.0, 40.0)) mval = SC_PANRIGHT;

  // Check Pan Up Symbol
  if (inTriangle(cmx, cmy, 40.0, 90.0, 30.0, 75.0, 50.0, 75.0)) mval = SC_PANUP;

  // Check Pan Down Symbol
  if (inTriangle(cmx, cmy, 40.0, 10.0, 50.0, 25.0, 30.0, 25.0)) mval = SC_PANDOWN;

  // Check Pan Fore Symbol
  if (inTriangle(cmx, cmy, 25.0, 55.0, 40.0, 70.0, 55.0, 55.0)) mval = SC_PANFORE;

  // Check RESET Symbol
  if (inCircle(cmx, cmy, 40.0, 50.0)) mval = SC_RESET;

  // Check Pan Aft Symbol
  if (inTriangle(cmx, cmy, 40.0, 30.0, 25.0, 45.0, 55.0, 45.0)) mval = SC_PANAFT;

  // Check Zoom Symbol

  ret = (mval != mcover);
  if (ret) mcover = mval;

  return (ret);
}

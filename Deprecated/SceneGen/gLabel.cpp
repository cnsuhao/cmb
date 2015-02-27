// gLabel.cpp: implementation of the gLabel class.
//
//////////////////////////////////////////////////////////////////////

#include "gLabel.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

gLabel::gLabel()
{
  caption = "";
  posx = posy = 0;
  wd = ht = 15;
  parentid = -1;
  visible = true;

  bgcolor[0] = 0.7;
  bgcolor[1] = 0.7;
  bgcolor[2] = 0.7;
  bgcolor[3] = 0.0;

  fgcolor[0] = 0.0;
  fgcolor[1] = 0.0;
  fgcolor[2] = 0.0;
  fgcolor[3] = 0.0;
}

gLabel::~gLabel()
{

}

//////////////////////////////////////////////////////////////////////
// Methods
//////////////////////////////////////////////////////////////////////
void gLabel::OnDraw(void)
{
  double w1, h1;
  //*** background color ***
  glClearColor(bgcolor[0], bgcolor[1], bgcolor[2], bgcolor[3]);

  glClear(GL_COLOR_BUFFER_BIT);

  //border
  glColor4f(fgcolor[0], fgcolor[1], fgcolor[2], fgcolor[3]);
  w1=wd-1.0;
  h1=ht-1.0;
  glBegin(GL_LINE_LOOP);
    glVertex2d(0.0, 0.0);
    glVertex2d(w1, 0.0);
    glVertex2d(w1, h1);
    glVertex2d(0.0, h1);
  glEnd();

  //text
  w1 = 2.0;
  h1 = 3.0;

  glColor4f(fgcolor[0], fgcolor[1], fgcolor[2], fgcolor[3]);
  outputtext( (int)w1, (int)h1, caption.c_str() );

  glutSwapBuffers();
}

void gLabel::OnReshape(int w, int h)
{
  if (w < 15) w = 15;
  if (h < 15) h = 15;
  wd = w;
  ht = h;
  glViewport(0, 0, wd, ht);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(-0.5, (double)wd-0.5, -0.5, (double)ht-0.5);
  glMatrixMode(GL_MODELVIEW);
  glutPostRedisplay();
}

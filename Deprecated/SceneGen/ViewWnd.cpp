// ViewWnd.cpp: implementation of the ViewWnd class.
//
//////////////////////////////////////////////////////////////////////

#include <QtGui>
#include <QtOpenGL>
#include <QPixmap>
#include <QString>

#include "ViewWnd.h"

#define PIXTOL 0.0000000001
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ViewWnd::ViewWnd(QWidget *parent)
    : QGLWidget(parent)
{
  setFormat(QGLFormat(QGL::DoubleBuffer | QGL::DepthBuffer));
  setMouseTracking(true);

  vm = perspective;
  plotcontrols = true;
}

void ViewWnd::initializeGL()
{
  glClearColor(0.6, 0.55, 0.5, 0.0);

  //initialize msd values
  msd->DLbase = glGenLists(60);

  GLfloat ltAmbient[]={0.2f, 0.2f, 0.2f, 1.0f};

  GLfloat ltDiffuse0[]={0.8f, 0.8f, 0.8f, 1.0f};
	GLfloat ltDiffuse1[]={0.4f, 0.2f, 0.2f, 1.0f};
	GLfloat ltDiffuse2[]={0.2f, 0.2f, 0.4f, 1.0f};

	GLfloat ltSpecular[]={0.0f, 0.0f, 0.0f, 0.0f};

  GLfloat lt0Pos[]={0.0f, 0.0f, 1.0, 1.0f};
	GLfloat lt1Pos[]={1.0, 0.0f, 1.0, 0.0f};
	GLfloat lt2Pos[]={-1.0, 0.0f, 1.0, 0.0f};

// Initialize OpenGL depths and shading model (GL_SMOOTH needs vertex normals)
  glClearDepth(1.0f);
  glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);

// Set the size of drawn points
  glPointSize(1.0);

//*** Polygon Offset ***
	glPolygonOffset(1.0, 0.0);

//*** Line Width ***
	glLineWidth(2.0);

//*** LIGHT MODEL DEFINITION ***
// enable two-sided lighting
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
// ambient color
  glLightfv(GL_LIGHT0,GL_AMBIENT,ltAmbient);
  glLightfv(GL_LIGHT1,GL_AMBIENT,ltAmbient);
  glLightfv(GL_LIGHT2,GL_AMBIENT,ltAmbient);
// diffuse color
	glLightfv(GL_LIGHT0,GL_DIFFUSE,ltDiffuse0);
	glLightfv(GL_LIGHT1,GL_DIFFUSE,ltDiffuse1);
	glLightfv(GL_LIGHT2,GL_DIFFUSE,ltDiffuse2);
// specular component
	glLightfv(GL_LIGHT0,GL_SPECULAR,ltSpecular);
	glLightfv(GL_LIGHT1,GL_SPECULAR,ltSpecular);
	glLightfv(GL_LIGHT2,GL_SPECULAR,ltSpecular);
// position
	glLightfv(GL_LIGHT0,GL_POSITION,lt0Pos);
  glLightfv(GL_LIGHT1,GL_POSITION,lt1Pos);
	glLightfv(GL_LIGHT2,GL_POSITION,lt2Pos);
// Enable Lights
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHT1);
  glEnable(GL_LIGHT2);

//*** Default State ***
  glEnable(GL_LIGHTING);
  glEnable(GL_TEXTURE_1D);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_BLEND);

//*** Fix for glScale ***
  glEnable(GL_NORMALIZE);

  msd->osdl.ClearDisplayLists();

  //CenterAABB(msd->osdl.GetAABB());
}

void ViewWnd::SetMSD( MainSceneData *xmsd )
{
  msd = xmsd;
}

void ViewWnd::Prime()
{
  SetAccel();
  oc.SetDrawTo(this);
  sc.SetDrawTo(this);
  vmnu.SetDrawTo(this);
}

void ViewWnd::ResetView()
{
  CenterAABB(msd->osdl.GetAABB());
  updateGL();
}

//////////////////////////////////////////////////////////////////////
// Callbacks
//////////////////////////////////////////////////////////////////////
void ViewWnd::mousePressEvent(QMouseEvent *event)
{
  int bt;
  int st;
  int x, y;

  bt = (int) event->button();
  st = (int) event->buttons() | event->modifiers();
  x = event->x();
  y = height()- 1 - event->y();

  OnMouse(bt, st, x, y);
}

void ViewWnd::mouseReleaseEvent(QMouseEvent *event)
{
  int bt;
  int st;
  int x, y;

  bt = (int) event->button();
  st = (int) event->buttons() | event->modifiers();
  x = event->x();
  y = height()- 1 - event->y();

  OnMouse(bt, st, x, y);
}

void ViewWnd::mouseMoveEvent(QMouseEvent *event)
{
  int x, y;

  x = event->x();
  y = height()- 1 - event->y();

  if (event->buttons()==Qt::NoButton)
  {
    OnPassiveMotion(x, y);
  }
  else
  {
    OnMotion(x, y);
  }

  if (vm!=perspective)
  {
    long ibx, iby, icx, icy;
    double bx, by, cx, cy, drot;
    char msg[200];
    int act = oc.GetAction();

    switch (act)
    {
    case OC_ZOOM:
    case OC_SELECT:
      oc.GetActionEndPoints(ibx, iby, icx, icy);
      OrthoMouseToWorld(ibx,iby,bx,by);
      OrthoMouseToWorld(icx,icy,cx,cy);
      sprintf(msg,"<%lf,%lf> - <%lf,%lf>", bx,by, cx,cy);
      emit changeMessage(msg);
      break;
    case OC_TRANSLATE:
      oc.GetActionEndPoints(ibx, iby, icx, icy);
      OrthoMouseToWorld(ibx,iby,bx,by);
      OrthoMouseToWorld(icx,icy,cx,cy);
      sprintf(msg,"Translating <%lf,%lf> - <%lf,%lf>    Delta <%lf,%lf>", bx, by, cx, cy, cx-bx, cy-by);
      emit changeMessage(msg);
      break;
    case OC_ROTATE:
      drot = oc.GetAngle();
      sprintf(msg,"Rotating %lf degrees", drot);
      emit changeMessage(msg);
      break;
    default:
      OrthoMouseToWorld(x,y,bx,by);
      sprintf(msg,"<%lf,%lf>", bx,by);
      emit changeMessage(msg);
      break;
    }

  }
}

void ViewWnd::OnMouse(int button, int state, int x, int y)
{
  string tag;

  if (vmnu.GetMouse() != VM_NONE)
  {
    vmnu.SetMode(vmnu.GetMouse());
    vwrm = false;
    switch (vmnu.GetMouse())
    {
    case VM_PERSPECTIVE:
      vm = perspective;
      CenterAABB(msd->osdl.GetAABB());
      break;
    case VM_XYORTHO:
      vm = xyortho;
      GetMSDFullExtents(fullextents);
      zoomextents = fullextents;
      pixscl = ComputePixScale();
      break;
    case VM_XZORTHO:
      vm = xzortho;
      GetMSDFullExtents(fullextents);
      zoomextents = fullextents;
      pixscl = ComputePixScale();
      break;
    case VM_YZORTHO:
      vm = yzortho;
      GetMSDFullExtents(fullextents);
      zoomextents = fullextents;
      pixscl = ComputePixScale();
      break;
    }
    emit clearStatus();
    updateGL();
  }
  else
  {
    if (vm == perspective) PerspectiveMouse( button, state, x, y );
    else                       OrthoMouse( button, state, x, y );
  }
}

void ViewWnd::OnPassiveMotion( int x, int y )
{
  if (!vwrm)
  {
    if (vmnu.MouseOver(x, y) == VM_NONE)
    {
      if (vm == perspective) PerspectivePassiveMotion( x, y );
      else OrthoPassiveMotion( x, y );
    }
  }
  else
  {
    if (vm == perspective) PerspectivePassiveMotion( x, y );
    else OrthoPassiveMotion( x, y );
  }
}

void ViewWnd::OnMotion( int x, int y )
{
  if (vm == perspective) PerspectiveMotion( x, y );
  else                       OrthoMotion( x, y );
}

void ViewWnd::OnIdle( void )
{
  if (vm == perspective) PerspectiveIdle();
}

void ViewWnd::paintGL(void)
{
  // clear the window
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  draw();
}

void ViewWnd::NotifyGLError( void )
{
  // Error check - highly recommended in OpenGL literature
  const GLubyte *errString;
  GLenum ec;

  ec = glGetError();
  if (ec != GL_NO_ERROR)
  {
     errString = gluErrorString(ec);
     printf("OpenGL Error: %s\n", errString);
  }

}

void ViewWnd::draw(void)
{
  glColor3f(0.3, 0.3, 0.3);
  // Do different things based on the projection space we are in,
  //   and the mode selected for that projection
  vwFixProjection(false);

  long bw, bh, cw, ch;
  gmMatrix4 drot;
  gmVector3 doff, drotabout;
  vwOrientObjects( drot, doff );

  oc.GetActionEndPoints(bw, bh, cw, ch);
  drotabout = OrthoMouseToWorld( bw, bh );

  //msd->osdl.PlotScene( msd->DLbase, drot, doff, (vm == xyortho));
  msd->osdl.PlotSceneAbout( msd->DLbase, drot, drotabout, doff, (vm == xyortho));

  if (plotcontrols)
  {
    if (vm == perspective) sc.Plot();
    else
    {
      oc.Plot();
    }

    vmnu.Plot();
  }

  NotifyGLError();

}

void ViewWnd::resizeGL(int w, int h)
{
  glViewport(0, 0, width(), height());
  syncam.SetViewAspect( (float)width() / (float)height() );
  GetMSDFullExtents(fullextents);
  //zoomextents = fullextents;
  AspectCorrect(zoomextents);
  pixscl = ComputePixScale();

  sc.SetOffset((float)width()-110.0, (float)height()-110.0 );
  oc.SetOffset((float)10.0, (float)height()-139.0 );
  vmnu.SetOffset(10.0f, 10.0f );
}

//////////////////////////////////////////////////////////////////////
// procedures
//////////////////////////////////////////////////////////////////////

void ViewWnd::CenterAABB( cAABB aabb )
{
  //fix the perspective view
  gmVector3 mid, ctr;
  double dr, dc;

  mid = aabb.GetMid();
  dr = aabb.GetDiameter() * 0.5;
  /* based on 45 degree field of view - distance to view and fill the screen */
  dc = dr * 2.414213566;

  syncam.Reset();
  syncam.SetViewAngle(45.0);
  syncam.SetViewAspect((float)width()/(float)height());
  syncam.SetViewNearPlane(dc-dr);
  syncam.SetViewFarPlane(dc+dr);
  syncam.SetUpDirection(0.0, 1.0, 0.0);
  syncam.MoveTo(mid[0], mid[1], mid[2] + dc );
  syncam.SetFocalDistance(dc);

  GetMSDFullExtents(fullextents);
  zoomextents = fullextents;
  pixscl = ComputePixScale();
}

void ViewWnd::SetCameraPosition( double x, double y, double z, double heading, double pitch, double yaw)
{
  //fix the perspective view
  gmVector3 mid, ctr;
  double dr, dc;
  cAABB aabb;

  aabb = msd->osdl.GetAABB();
  mid = aabb.GetMid();
  dr = aabb.GetDiameter() * 0.5;
  //*** based on 45 degree field of view - distance to view and fill the screen ***
  dc = dr * 2.414213566;

  syncam.Reset();
  syncam.SetViewAngle(45.0);
  syncam.SetViewAspect((float)width()/(float)height());
  syncam.SetViewNearPlane(dc-dr);
  syncam.SetViewFarPlane(dc+dr);
  syncam.SetUpDirection(0.0, 0.0, 1.0);
  syncam.SetFocalDistance(dc);

  syncam.MoveTo(x, y, z );  // set location

  syncam.Rotate(0.0, -90.0, 0.0 );  // look along x-axis to start
  syncam.Rotate(90.0, 0.0, 0.0 );   // with Z being up

  syncam.Rotate(360-heading, 0, 0 );  // set rotations in order
  syncam.Rotate(0, pitch, 0 );
  syncam.Rotate(0, 0, yaw );

//  GetMSDFullExtents(fullextents);
//  zoomextents = fullextents;
}

void ViewWnd::GetMSDFullExtents(cAABB& aabb)
{
//  long ii;
  cAABB taabb;
  gmVector3 mid, range, adj;

  aabb.Reset();
  aabb = msd->osdl.GetAABB();
  //FW - possibly get extents of ALL the data

  //pad for some space on the sides in ortho view
  mid = aabb.GetMid();
  range = (aabb.GetRange() * 0.5) * 1.05;
  adj = mid - range;
  aabb.Adjust(adj);
  adj = mid + range;
  aabb.Adjust(adj);

  AspectCorrect(aabb);
}

//******************************************
// This approach uses OpenGL pick commands
//******************************************

void ViewWnd::vwFixProjection( bool picking )
{
  cAABB aabb;
  gmVector3 vmid, vrng, vpos;
  double dc;

//  glutSetWindow(ViewWindow);
  switch(vm)
  {
  case perspective:
    aabb = fullextents;
    vmid = aabb.GetMid();
    dc = aabb.GetDiameter() * 0.5;
    syncam.ComputeNearAndFar(vmid[0], vmid[1], vmid[2], dc );
    if (picking)
      syncam.PickPerspective();
    else
      syncam.FixFocalParameters();
    syncam.Fix();
    break;

  case xyortho:
    aabb = zoomextents;
    vrng = aabb.GetRange() * 0.5;
    vmid = aabb.GetMid();
    if (!picking)
    {
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
    }
    glOrtho( -vrng[0], vrng[0], -vrng[1], vrng[1], -vrng[2], vrng[2]);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslated( -vmid[0], -vmid[1], -vmid[2]);
    break;

  case xzortho: //ZOOM XZORTHO
    aabb = zoomextents;
    vrng = aabb.GetRange() * 0.5;
    vmid = aabb.GetMid();
    if (!picking)
    {
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
    }
    glOrtho( -vrng[0], vrng[0], -vrng[2], vrng[2], -vrng[1], vrng[1]);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotated(-90.0, 1.0, 0.0, 0.0);
    glTranslated( -vmid[0], -vmid[1], -vmid[2]);
    break;

  case yzortho:
    aabb = zoomextents;
    vrng = aabb.GetRange() * 0.5;
    vmid = aabb.GetMid();
    if (!picking)
    {
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
    }
    glOrtho( -vrng[1], vrng[1], -vrng[2], vrng[2], -vrng[0], vrng[0]);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotated(-90.0, 1.0, 0.0, 0.0);
    glRotated(-90.0, 0.0, 0.0, 1.0);
    glTranslated( -vmid[0], -vmid[1], -vmid[2]);
    break;

  }
}

void ViewWnd::vwOrientObjects( gmMatrix4 &drot, gmVector3 &doff)
{
  gmMatrix4 moff, m_id;
  gmVector3 axis;
  double dang;

  moff = moff.identity();
  m_id = m_id.identity();
  dang = oc.GetAngle();
  switch (vm)
  {
  case xyortho:
    axis.assign(0.0, 0.0, 1.0);
    moff = moff.rotate( 360.0 - dang,axis);
    break;
  case xzortho:
    axis.assign(0.0, 1.0, 0.0);
    moff = moff.rotate( dang,axis);
    break;
  case yzortho:
    axis.assign(1.0, 0.0, 0.0);
    moff = moff.rotate( 360.0 - dang,axis);
    break;
  case perspective:
    break;
  }

  axis = oc.GetPos();

  drot = moff;
  doff = axis;

// finished
}

long ViewWnd::Pick3D(long pX, long pY)
{

#define PICKBUFSIZE 1000

  GLuint sel[PICKBUFSIZE], hits;
  GLint vp[4];

  long ClosestIndex;
  GLuint ClosestIndexNear;
  GLuint ClosestIndexFar;

  glGetIntegerv(GL_VIEWPORT, vp);

  glSelectBuffer(PICKBUFSIZE, sel);
  glRenderMode(GL_SELECT);

  glInitNames();
  glPushName(0);

  glMatrixMode(GL_PROJECTION);
  //glPushMatrix();
    glLoadIdentity();
    gluPickMatrix( (double) pX, (double) (vp[3] - pY), 1.0, 1.0, vp);

  //plotting code
  // Do different things based on the projection space we are in,
  //   and the mode selected for that projection
    vwFixProjection(true);

    gmMatrix4 drot;
    gmVector3 doff;
    vwOrientObjects( drot, doff );
    msd->osdl.PlotSceneSelect( msd->DLbase, drot, doff, (vm == xyortho));

  //glPopMatrix();
  hits = glRenderMode(GL_RENDER);

  //cout << "PICK3D:  Hits=" << hits << "\n";

  ClosestIndex = -1;
  ClosestIndexNear = 0xFFFFFFFF;
  ClosestIndexFar = 0xFFFFFFFF;

  if (hits != 0)
  {
    GLuint bufcnt = 0;

    for (GLuint iloop = 0; iloop < hits; iloop++)
    {
      GLuint numnames = sel[bufcnt++];
      GLuint dnear = sel[bufcnt++];
      GLuint dfar = sel[bufcnt++];
      long firstname = sel[bufcnt++];

      //cout << "PICK3D: near=" << dnear << "  far=" << dfar << "  numnames=" << numnames << "  firstname=" << firstname << "\n";
      if (dnear < ClosestIndexNear)
      {
        ClosestIndex = firstname;
        ClosestIndexNear = dnear;
        ClosestIndexFar = dfar;
      }
      long ancnames;
      for (GLuint ix = 1; ix < numnames; ix++)
      {
        ancnames = sel[bufcnt++];  //Clears ancillary names
      }
    }
  }

  return ClosestIndex;
}

void ViewWnd::vwPlotZoomBox()
{

  // Other drawing has taken place, now to draw the control
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0,width(), height(), 0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);

  glColor4f( 1.0, 1.0, 0.0, 0.35 );

  glBegin(GL_LINE_LOOP);
    glVertex2f((float) vwbasex, (float) vwbasey);
    glVertex2f((float) vwbasex, (float) vwMouseY);
    glVertex2f((float) vwMouseX, (float) vwMouseY);
    glVertex2f((float) vwMouseX, (float) vwbasey);
  glEnd();

  // Clean Up
  glDisable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);
}

void ViewWnd::AspectCorrect( cAABB& aabb )
{
  // assumes 1-to-1 pixel aspect
  //fix the ortho view to be aspect correct with the monitor, based on ortho view mode
  gmVector3 midpt, deltapt, adjpt;
  float sclhorz, sclvert;

  midpt = aabb.GetMid();
  deltapt = aabb.GetRange();

  switch (vm)
  {
  case xyortho:
    sclhorz = (float)width() /  deltapt[0];
    sclvert = (float)height() /  deltapt[1];
    if (sclhorz < sclvert)
    {
      deltapt[1] = deltapt[1] * (sclvert/sclhorz);
    }
    else
    {
      deltapt[0] = deltapt[0] * (sclhorz/sclvert);
    }
    deltapt *= 0.5;
    adjpt = midpt-deltapt;
    aabb.Adjust(adjpt);
    adjpt = midpt+deltapt;
    aabb.Adjust(adjpt);
    break;
  case xzortho:
    sclhorz = (float)width() / (float) deltapt[0];
    sclvert = (float)height() / (float) deltapt[2];
    if (sclhorz < sclvert)
    {
      deltapt[2] = deltapt[2] * (sclvert/sclhorz);
    }
    else
    {
      deltapt[0] = deltapt[0] * (sclhorz/sclvert);
    }
    deltapt *= 0.5;
    adjpt = midpt-deltapt;
    aabb.Adjust(adjpt);
    adjpt = midpt+deltapt;
    aabb.Adjust(adjpt);
    break;
  case yzortho:
    sclhorz = (float)width() / (float) deltapt[1];
    sclvert = (float)height() / (float) deltapt[2];
    if (sclhorz < sclvert)
    {
      deltapt[2] = deltapt[2] * (sclvert/sclhorz);
    }
    else
    {
      deltapt[1] = deltapt[1] * (sclhorz/sclvert);
    }
    deltapt *= 0.5;
    adjpt = midpt-deltapt;
    aabb.Adjust(adjpt);
    adjpt = midpt+deltapt;
    aabb.Adjust(adjpt);
    break;
  default:
    break;
  }
}

void ViewWnd::SetAccel()
{
  gmVector3 rng;
  double mindim, maxdim;

  rng = msd->osdl.GetAABB().GetRange();
  maxdim = rng.length();
  mindim = rng[0];
  if (mindim > rng[1]) mindim = rng[1];
  if (mindim > rng[2]) mindim = rng[2];

  accel.SetAccel( mindim*0.0001 );
  accel.SetTop( maxdim * 0.5 );

  paccel.SetAccel( mindim*0.01 );
  paccel.SetTop( maxdim * 0.5 );

}

/************************
 * Perspective Routines *
 ************************/
long ViewWnd::ObjectDefinition_Select( int x, int y)
{
  return 0;
}

void ViewWnd::PerspectiveMouse( int button, int state, int x, int y)
{
  int sc_mode;
  long select_id;
  int mstate = state & button;

  sc_mode = sc.GetMouse();

  switch (sc_mode)
  {
  case SC_NONE:
    if (button == Qt::LeftButton)
    {
      if (mstate != 0)
      {
        if (sc.GetMode()!=SC_SELECT)
        {
          vwrm = true;
        }
        else
        {
          vwrm = false;
        }
      }
      else
      {
        vwrm = false;

        if (mstate == 0)
        {
          if (sc.GetMode()==SC_SELECT)
          {
            //find corresponding object and toggle
            select_id = Pick3D(x,height()-1-y);
            //msd->osdl.ToggleSelectIndex(select_id);
            if (!(state & Qt::ControlModifier)) msd->osdl.DeselectAll();
            msd->osdl.SetSelect(select_id,true);
            updateGL();
            //glutSetWindow( msd->TextWindow );
          }
        }
      }
    }

    if (button == Qt::RightButton)
    {
      if (mstate != 0)
      {
        vwrm = true;
      }
      else
      {
        vwrm = false;
      }
    }

    vwMouseX = x;
    vwMouseY = y;
    break;

  case SC_RESET:
    if (button == Qt::LeftButton)
    {
      CenterAABB(msd->osdl.GetAABB());
      updateGL();
    }
    break;

  case SC_PANLEFT:
  case SC_PANRIGHT:
  case SC_PANUP:
  case SC_PANDOWN:
  case SC_PANFORE:
  case SC_PANAFT:
    if (button == Qt::LeftButton)
    {
      if (mstate != 0)
      {
        pmode = sc_mode;
        timerid = startTimer(30);
      }
      else
      {
        pmode = SC_NONE;
        killTimer(timerid);
      }
      paccel.Restart();
      updateGL();
    }
    break;

  case SC_SELECT:
  case SC_ROTATE:
  case SC_DRIVE:
    if (button == Qt::LeftButton)
    {
      if (mstate != 0)
      {
        sc.SetMode(sc_mode);
        updateGL();
      }
    }
    break;

  }
}

void ViewWnd::PerspectivePassiveMotion( int x, int y )
{
  if (sc.MouseOver(x, y))
    updateGL();
}

void ViewWnd::PerspectiveMotion( int x, int y)
{
  long deltax, deltay;

  if (vwrm)
  {
      deltax = x - vwMouseX;
      deltay = -(y - vwMouseY);
      if(sc.GetMode() == SC_DRIVE)
      {
        syncam.Rotate(deltax*0.5, deltay*0.5, 0.0);
      }
      else
      {
        syncam.RotateAboutFocalPoint(deltax*0.5, deltay*0.5, 0.0);
      }
      vwMouseX = x;
      vwMouseY = y;

    updateGL();
  }
}

void ViewWnd::PerspectiveIdle()
{
  float vel;

  vel = paccel.Next();
  switch (pmode)
  {
  case SC_PANLEFT:
    syncam.Move(0.0, vel, 0.0);
    break;
  case SC_PANRIGHT:
    syncam.Move(0.0, -vel, 0.0);
    break;
  case SC_PANUP:
    syncam.Move(0.0, 0.0, -vel);
    break;
  case SC_PANDOWN:
    syncam.Move(0.0, 0.0, vel);
    break;
  case SC_PANFORE:
    syncam.Move(vel, 0.0, 0.0);
    break;
  case SC_PANAFT:
    syncam.Move(-vel, 0.0, 0.0);
    break;
  }
  updateGL();
}

/*************************
 * OrthoGraphic Routines *
 *************************/
double ViewWnd::ComputePixScale()
{
  gmVector3 dpt;

  dpt = zoomextents.GetRange();
  //Get coordinate scale (should be square pixels)
  double pixwd = dpt[0]/(double)width();
  if (pixwd < PIXTOL) return PIXTOL;
  double sclwd = 100000.0;
  while (sclwd>pixwd) sclwd/=10.0;

  return sclwd;
}

void ViewWnd::PixBound( double whorz, double wvert, double& pbhorz, double& pbvert)
{
  pbhorz = floor(whorz / pixscl) * pixscl;
  pbvert = floor(wvert / pixscl) * pixscl;
}

void ViewWnd::OrthoMouseToWorld( int mhorz, int mvert, double& whorz, double& wvert)
{
  gmVector3 dpt, doff;

  dpt = zoomextents.GetRange();
  doff = zoomextents.GetMin();
  switch (vm)
  {
  case xyortho:
    whorz = doff[0] + (dpt[0] * ((float)mhorz / (float)width() ));
    wvert = doff[1] + (dpt[1] * ((float)mvert / (float)height() ));
    break;
  case xzortho:
    whorz = doff[0] + (dpt[0] * ((float)mhorz / (float)width() ));
    wvert = doff[2] + (dpt[2] * ((float)mvert / (float)height() ));
    break;
  case yzortho:
    whorz = doff[1] + (dpt[1] * ((float)mhorz / (float)width() ));
    wvert = doff[2] + (dpt[2] * ((float)mvert / (float)height() ));
    break;
  default:
    break;
  }
  PixBound(whorz, wvert, whorz, wvert);
}

gmVector3 ViewWnd::OrthoMouseToWorld( int mhorz, int mvert)
{
  gmVector3 dpt, doff, ret;

  dpt = zoomextents.GetRange();
  doff = zoomextents.GetMin();
  switch (vm)
  {
  case xyortho:
    ret[0] = doff[0] + (dpt[0] * ((float)mhorz / (float)width() ));
    ret[1] = doff[1] + (dpt[1] * ((float)mvert / (float)height() ));
    ret[2] = 0.0;
    break;
  case xzortho:
    ret[0] = doff[0] + (dpt[0] * ((float)mhorz / (float)width() ));
    ret[1] = 0.0;
    ret[2] = doff[2] + (dpt[2] * ((float)mvert / (float)height() ));
    break;
  case yzortho:
    ret[0] = 0.0;
    ret[1] = doff[1] + (dpt[1] * ((float)mhorz / (float)width() ));
    ret[2] = doff[2] + (dpt[2] * ((float)mvert / (float)height() ));
    break;
  default:
    break;
  }
  return ret;
}

gmVector3 ViewWnd::OrthoScaleMouseToWorld( int mhorz, int mvert)
{
  gmVector3 dpt, doff, ret;

  dpt = zoomextents.GetRange();
//  doff = zoomextents.GetMin();
  switch (vm)
  {
  case xyortho:
    ret[0] = (dpt[0] * ((float)mhorz / (float)width() ));
    ret[1] = (dpt[1] * ((float)mvert / (float)height() ));
    ret[2] = 0.0;
    break;
  case xzortho:
    ret[0] = (dpt[0] * ((float)mhorz / (float)width() ));
    ret[1] = 0.0;
    ret[2] = (dpt[2] * ((float)mvert / (float)height() ));
    break;
  case yzortho:
    ret[0] = 0.0;
    ret[1] = (dpt[1] * ((float)mhorz / (float)width() ));
    ret[2] = (dpt[2] * ((float)mvert / (float)height() ));
    break;
  default:
    break;
  }
  return ret;
}

/*  BCW - design code */
void ViewWnd::OrthoMouse( int button, int state, int x, int y)
{
  int oc_m_over, oc_m_mode;

  long bw, bh, cw, ch;
  gmVector3 bpt, dpt, doff, ep1, ep2;
  cAABB newextents;

  int mstate = state & button;

  static long select_under;
  static bool prev_state;

  oc_m_over = oc.GetMouse();
  oc_m_mode = oc.GetMode();

  switch (oc_m_over)
  {
// *** Respond to mode command ***
  case OC_NONE:
    switch (oc_m_mode)
    {
    case OC_ZOOM:
        if (mstate != 0)
        {
          vwrm=true;
          oc.SetAction(OC_ZOOM, x, y);
        }
        else
        {
          vwrm=false;
          oc.SetCurrent(x,y);
          if (oc.GetActionEndPoints(bw, bh, cw, ch))
          {
            ep1 = OrthoMouseToWorld( bw, bh );
            ep2 = OrthoMouseToWorld( cw, ch );
            // Set the extents to the full range of the perpendicular axis to the view
            dpt = zoomextents.GetRange();
            doff = zoomextents.GetMin();
            switch(vm)
            {
            case xyortho:
              ep1[2] = doff[2];
              ep2[2] = doff[2]+dpt[2];
              break;
            case xzortho:
              ep1[1] = doff[1];
              ep2[1] = doff[1]+dpt[1];
              break;
            case yzortho:
              ep1[0] = doff[0];
              ep2[0] = doff[0]+dpt[0];
              break;
            default:
              break;
            }
            newextents.Reset();
            newextents.Adjust(ep1);
            newextents.Adjust(ep2);
            zoomextents = newextents;
            AspectCorrect(zoomextents);
            pixscl = ComputePixScale();
          }
          oc.SetAction(OC_NONE, 0, 0);
          updateGL();
        }
      break;
    case OC_SELECT:
        // bounding box select
        if (mstate != 0)
        {
            vwrm = true;
            oc.SetAction(OC_SELECT, x, y);
        }
        else
        {
          vwrm=false;
          oc.SetCurrent(x,y);
          if (oc.GetActionEndPoints(bw, bh, cw, ch))
          {
            ep1 = OrthoMouseToWorld( bw, bh );
            ep2 = OrthoMouseToWorld( cw, ch );
            // Set the extents to the full range of the perpendicular axis to the view
            dpt = zoomextents.GetRange();
            doff = zoomextents.GetMin();
            switch(vm)
            {
            case xyortho:
              ep1[2] = doff[2];
              ep2[2] = doff[2]+dpt[2];
              break;
            case xzortho:
              ep1[1] = doff[1];
              ep2[1] = doff[1]+dpt[1];
              break;
            case yzortho:
              ep1[0] = doff[0];
              ep2[0] = doff[0]+dpt[0];
              break;
            default:
              break;
            }
            newextents.Reset();
            newextents.Adjust(ep1);
            newextents.Adjust(ep2);
            if (!(state & Qt::ControlModifier)) msd->osdl.DeselectAll();
//            msd->osdl.DeselectAll();
            msd->osdl.SelectInBoundingBox(newextents);
          }
          oc.SetAction(OC_NONE, 0, 0);
          updateGL();
        }
      break;
    case OC_ROTATE:
        if (mstate != 0)
        {
          select_under = Pick3D(x,height()-1-y);
          prev_state = msd->osdl.IDSelected(select_under);
          msd->osdl.SetSelect(select_under,true);
          vwrm = true;
          oc.SetAction(OC_ROTATE, x, y);
          updateGL();
        }
        else
        {
          //Get rotation from click spot (allow accept or not)

          oc.SetCurrent(x,y);

          gmVector3 dvec;
          gmMatrix4 drot;
          double dang;

          oc.GetActionEndPoints(bw, bh, cw, ch);
          doff = OrthoMouseToWorld( bw, bh );

          drot = drot.identity();
          dang = oc.GetAngle();
          switch (vm)
          {
          case xyortho:
            dvec.assign(0.0, 0.0, 1.0);
            drot = drot.rotate( 360.0 - dang, dvec);
            break;
          case xzortho:
            dvec.assign(0.0, 1.0, 0.0);
            drot = drot.rotate( dang, dvec);
            break;
          case yzortho:
            dvec.assign(1.0, 0.0, 0.0);
            drot = drot.rotate( 360.0 - dang, dvec);
            break;
          default:
            break;
          }

//          msd->osdl.ChangeSelectRotation( drot, (vm==xyortho) );
          msd->osdl.ChangeSelectRotationAbout( doff, drot, (vm==xyortho) );
          oc.ResetAngle();

          msd->osdl.SetSelect(select_under,prev_state);
          vwrm = false;
          oc.SetAction(OC_NONE, 0, 0);
          updateGL();
        }
      break;
    case OC_TRANSLATE:
        if (mstate != 0)
        {
          select_under = Pick3D(x,height()-1-y);
          prev_state = msd->osdl.IDSelected(select_under);
          msd->osdl.SetSelect(select_under,true);
          vwrm = true;
          oc.SetAction(OC_TRANSLATE, x, y);
        }
        else
        {
          //Get translation from click spot (allow accept or not)
          oc.SetCurrent(x,y);
          long dw, dh;
          oc.GetActionDelta(dw, dh);
          ep1 = OrthoScaleMouseToWorld( dw, dh );
//          oc.TranslateAction(ep1[0], ep1[1], ep1[2]);
//          ep2 = oc.GetPos();
          msd->osdl.ChangeSelectPosition( ep1 );
          oc.SetPos(0.0, 0.0, 0.0);

          msd->osdl.SetSelect(select_under,prev_state);
          vwrm = false;
          oc.SetAction(OC_NONE, 0, 0);
          updateGL();
        }
      break;
    }
    break;
// *** Set mode ***
  case OC_ZOOM:
  case OC_SELECT:
  case OC_ROTATE:
  case OC_TRANSLATE:
    if (button == Qt::LeftButton)
    {
      if (mstate == 0)
      {
        oc.SetMode(oc_m_over);
        updateGL();
      }
    }
    break;
// *** action buttons ***
  case OC_RESETBUTTON:
    if (button == Qt::LeftButton)
    {
      if (mstate == 0)
      {
        zoomextents = fullextents;
        pixscl = ComputePixScale();
        updateGL();
      }
    }
    break;
  case OC_PROJECTBUTTON:
    if (button == Qt::LeftButton)
    {
      if (mstate == 0)
      {
        msd->osdl.ProjectSelectedToPrimarySurface();
        updateGL();
      }
    }
    break;

  case OC_CENTERBUTTON:
    if (button == Qt::LeftButton)
    {
      if (mstate == 0)
      {
        msd->osdl.ProjectSelectedToMidScene();
        updateGL();
      }
    }
    break;
  }
}

void ViewWnd::OrthoPassiveMotion( int x, int y )
{
  if (oc.MouseOver(x, y))
      updateGL();
}

void ViewWnd::OrthoMotion( int x, int y)
{
  gmVector3 newpos;
  long dw, dh;

  oc.SetCurrent(x,y);
  oc.GetActionDelta(dw, dh);
  newpos = OrthoScaleMouseToWorld( dw, dh );
  oc.TranslateAction(newpos[0], newpos[1], newpos[2]);
  updateGL();
}


void ViewWnd::timerEvent(QTimerEvent *event)
{
  OnIdle();
}

void ViewWnd::extentsChanged( long x )
{
  msd->osdl.SetAABB(x);
  CenterAABB(msd->osdl.GetAABB());
  updateGL();
}

void ViewWnd::saveScreen( const char *fname )
{
  QPixmap rp;
  QString fn(fname);

  plotcontrols = false;
  rp = renderPixmap();
  rp.save(fname,"BMP");
  plotcontrols = true;
}

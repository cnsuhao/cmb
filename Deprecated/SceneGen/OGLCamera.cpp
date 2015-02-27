// OGLCamera.cpp: implementation of the COGLCamera class.
//
//////////////////////////////////////////////////////////////////////

#include "OGLCamera.h"
#include <math.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COGLCamera::COGLCamera()
{
  Reset();
}

COGLCamera::~COGLCamera()
{

}

void COGLCamera::Reset()
{
  SetIdentity();

  m_pos.assign(0.0, 0.0, 0.0);

  m_fovy=60.0f;
  m_aspect=4.0f/3.0f;
  m_near=1.0f;
  m_far=2.0f;

  m_focaldistance=m_far-m_near;
  m_up.assign(0.0, 1.0, 0.0);

  m_datachanged=true;
}

void COGLCamera::SetIdentity()
{
  m_ViewMatrix = m_ViewMatrix.identity();
}

void COGLCamera::GetPosition( double *px, double *py, double *pz)
{
  *px = -m_pos[0];
  *py = -m_pos[1];
  *pz = -m_pos[2];
}

void COGLCamera::GetPosition( gmVector3 *posvec )
{
  (*posvec)[0] = -m_pos[0];
  (*posvec)[1] = -m_pos[1];
  (*posvec)[2] = -m_pos[2];
}

void COGLCamera::SetUpDirection( double px, double py, double pz)
{
  m_up.assign(px,py,pz);
  m_up.normalize();
}

void COGLCamera::SetUpDirection( gmVector3 *vec )
{
  m_up.assign((*vec)[0],(*vec)[1],(*vec)[2]);
  m_up.normalize();
}

void COGLCamera::MoveTo(double px, double py, double pz)
{
  m_pos.assign(-px,-py,-pz);
  m_datachanged=true;
}

void COGLCamera::MoveTo( gmVector3 *vec)
{
  m_pos.assign(-(*vec)[0],-(*vec)[1],-(*vec)[2]);
  m_datachanged=true;
}

void COGLCamera::Move(double fore, double horiz, double vert)
{
  double dx, dy, dz;

  if (fore==0.0){
    if(horiz==0.0){
      if(vert==0.0){
        return;
      }
    }
  }

  dx = horiz*m_ViewMatrix[0][0] + vert*m_ViewMatrix[0][1] + fore*m_ViewMatrix[0][2];
  dy = horiz*m_ViewMatrix[1][0] + vert*m_ViewMatrix[1][1] + fore*m_ViewMatrix[1][2];
  dz = horiz*m_ViewMatrix[2][0] + vert*m_ViewMatrix[2][1] + fore*m_ViewMatrix[2][2];

  m_pos[0] = m_pos[0] + dx;  m_pos[1] = m_pos[1] + dy;  m_pos[2] = m_pos[2] + dz;
  m_datachanged=true;
}

void COGLCamera::Move( gmVector3 *vec )
{
  double dx, dy, dz;
  double fore, horiz, vert;

  fore = (*vec)[0];
  horiz = (*vec)[1];
  vert = (*vec)[2];

  if (fore==0.0){
    if(horiz==0.0){
      if(vert==0.0){
        return;
      }
    }
  }

  dx = horiz*m_ViewMatrix[0][0] + vert*m_ViewMatrix[0][1] + fore*m_ViewMatrix[0][2];
  dy = horiz*m_ViewMatrix[1][0] + vert*m_ViewMatrix[1][1] + fore*m_ViewMatrix[1][2];
  dz = horiz*m_ViewMatrix[2][0] + vert*m_ViewMatrix[2][1] + fore*m_ViewMatrix[2][2];

  m_pos[0] = m_pos[0] + dx;  m_pos[1] = m_pos[1] + dy;  m_pos[2] = m_pos[2] + dz;
  m_datachanged=true;
}

void COGLCamera::Rotate(double yaw, double pitch, double roll)
{
  double mogl[16];

  if (yaw==0.0){
    if(pitch==0.0){
      if(roll==0.0){
        return;
      }
    }
  }
  glPushMatrix();
  glLoadIdentity();

#ifdef MT4_COLUMNOR  //order matters
  if (yaw!=0.0) glRotated(yaw,0.0,1.0,0.0);
  if (pitch!=0.0) glRotated(pitch,1.0,0.0,0.0);
  if (roll!=0.0) glRotated(roll,0.0,0.0,1.0);

  m_ViewMatrix.MakeOpenGL( mogl );
  glMultMatrixd(mogl);
#else
  m_ViewMatrix.MakeOpenGL( mogl );
  glMultMatrixd(mogl);

  if (yaw!=0.0) glRotated(yaw,0.0,1.0,0.0);
  if (pitch!=0.0) glRotated(pitch,1.0,0.0,0.0);
  if (roll!=0.0) glRotated(roll,0.0,0.0,1.0);
#endif
  glGetDoublev(GL_MODELVIEW_MATRIX,mogl);
  m_ViewMatrix.FromOpenGL( mogl );
  glPopMatrix();
  m_datachanged=true;
}

void COGLCamera::RotateAboutFocalPoint(double yaw, double pitch, double roll)
{
  double mogl[16];
  double deltax, deltay, deltaz;

  if (yaw==0.0){
    if(pitch==0.0){
      if(roll==0.0){
        return;
      }
    }
  }

  // Find the rotation point
  deltax = m_pos[0] + (m_focaldistance * m_ViewMatrix[0][2]);
  deltay = m_pos[1] + (m_focaldistance * m_ViewMatrix[1][2]);
  deltaz = m_pos[2] + (m_focaldistance * m_ViewMatrix[2][2]);

  // Rotate the m_ViewMatrix using premultiplication in the OpenGL buffers
  glPushMatrix();
  glLoadIdentity();

#ifdef MT4_COLUMNOR  //order matters
  if (yaw!=0.0) glRotated(yaw,0.0,1.0,0.0);
  if (pitch!=0.0) glRotated(pitch,1.0,0.0,0.0);
  if (roll!=0.0) glRotated(roll,0.0,0.0,1.0);

  m_ViewMatrix.MakeOpenGL( mogl );
  glMultMatrixd(mogl);
#else
  m_ViewMatrix.MakeOpenGL( mogl );
  glMultMatrixd(mogl);

  if (yaw!=0.0) glRotated(yaw,0.0,1.0,0.0);
  if (pitch!=0.0) glRotated(pitch,1.0,0.0,0.0);
  if (roll!=0.0) glRotated(roll,0.0,0.0,1.0);
#endif

  glGetDoublev(GL_MODELVIEW_MATRIX,mogl);
  m_ViewMatrix.FromOpenGL( mogl );
  glPopMatrix();

  // The camera point is m_focaldistance away from the rotation point along the line of sight
  m_pos.assign(deltax - (m_focaldistance*m_ViewMatrix[0][2]),
               deltay - (m_focaldistance*m_ViewMatrix[1][2]),
               deltaz - (m_focaldistance*m_ViewMatrix[2][2]));
  m_datachanged=true;
}

void COGLCamera::Fix()
{
  double mogl[16];

  if (m_datachanged){
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
#ifdef MT4_COLUMNOR  //order matters
    m_ViewMatrix.MakeOpenGL(mogl);
    glMultMatrixd( mogl );
    glTranslated(m_pos[0], m_pos[1], m_pos[2]);
#else
    glTranslated(m_pos[0], m_pos[1], m_pos[2]);
    m_ViewMatrix.MakeOpenGL(mogl);
    glMultMatrixd( mogl );
#endif
    m_datachanged=false;
  }
}

void COGLCamera::SetViewAngle(double fovy)
{
  if (fovy!=m_fovy){
    m_fovy=fovy;
    FixFocalParameters();
  }

}

void COGLCamera::SetViewAspect(double asp)
{
  if (asp!=m_aspect){
    m_aspect=asp;
    FixFocalParameters();
  }
}

void COGLCamera::SetViewNearPlane(double nr)
{
  if (nr!=m_near){
    if (nr>0.01f) m_near=nr;
    else         m_near=0.01f;

    FixFocalParameters();
  }
}

void COGLCamera::SetViewFarPlane(double fr)
{
  if (fr!=m_far) {
    if (fr>0.02f) m_far=fr;
    else        m_far=0.02f;
    FixFocalParameters();
  }
}

void COGLCamera::FixFocalParameters()
{
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(m_fovy, m_aspect, m_near, m_far);
  glMatrixMode(GL_MODELVIEW);
  m_datachanged=true;
}

void COGLCamera::PickPerspective()
{
  gluPerspective(m_fovy, m_aspect, m_near, m_far);
  m_datachanged=true;
  glMatrixMode(GL_MODELVIEW);
}

void COGLCamera::SetFocalDistance(double fd)
{
  if (fd!=m_focaldistance){
    m_focaldistance=fd;
  }
}

double COGLCamera::GetFocalDistance()
{
  return(m_focaldistance);
}

// *** These routines allow the programmer to save and restore a view ***
void COGLCamera::SaveCameraSettings( const char *filename )
{
  ofstream ofile( filename );

  if(!ofile)
  {
    cout << "Cannot save camera settings!\n";
    return;
  }

  ofile << m_ViewMatrix << endl;
  ofile << m_pos << endl;
  ofile << m_focaldistance << endl;
  ofile << m_fovy << endl;
  ofile << m_aspect << endl;
  ofile << m_near << " " << m_far << endl;

  ofile.close();
}

void COGLCamera::LoadCameraSettings( const char *filename )
{
  ifstream ifile( filename );

  if(!ifile)
  {
    cout << "Cannot load camera settings!\n";
    return;
  }

  ifile >> m_ViewMatrix;
  ifile >> m_pos;
  ifile >> m_focaldistance;
  ifile >> m_fovy;
  ifile >> m_aspect;
  ifile >> m_near >> m_far;

  ifile.close();

  FixFocalParameters();
}

// Overloaded for binary files already opened
void COGLCamera::SaveCameraSettings( std::ostream& ofile )
{
  ofile << m_ViewMatrix << endl;
  ofile << m_pos << endl;
  ofile << m_focaldistance << endl;
  ofile << m_fovy << endl;
  ofile << m_aspect << endl;
  ofile << m_near << " " << m_far << endl;
}

void COGLCamera::LoadCameraSettings( std::istream& ifile )
{
  ifile >> m_ViewMatrix;
  ifile >> m_pos;
  ifile >> m_focaldistance;
  ifile >> m_fovy;
  ifile >> m_aspect;
  ifile >> m_near >> m_far;

  FixFocalParameters();
}

void COGLCamera::Level()
{
	//Weeble on the up-vector (required for drive mode)
	gmVector3 n, v, u;
  double dotprod;

	n.assign( m_ViewMatrix[0][2],
		        m_ViewMatrix[1][2],
					  m_ViewMatrix[2][2]);
	n.normalize();

	dotprod= dot( n, m_up );

	v.assign( m_up[0] - dotprod * n[0],
            m_up[1] - dotprod * n[1],
						m_up[2] - dotprod * n[2]);
	v.normalize();

	u = cross( v, n );

	m_ViewMatrix[0][0] = u[0];
	m_ViewMatrix[1][0] = u[1];
	m_ViewMatrix[2][0] = u[2];

	m_ViewMatrix[0][1] = v[0];
	m_ViewMatrix[1][1] = v[1];
	m_ViewMatrix[2][1] = v[2];

	m_ViewMatrix[0][2] = n[0];
	m_ViewMatrix[1][2] = n[1];
	m_ViewMatrix[2][2] = n[2];
}

// *** Helpers for Picking and Selecting ***
void COGLCamera::GetViewVector(long screenx, long screeny,
                               double *Xpos, double *Ypos, double *Zpos,
                               double *dX, double *dY, double *dZ)
{

  GLint vp[4];
  double zdist;

  gmVector3 camvec, retvec;
  gmMatrix4 imtx;

  // Set Camera Position
  *Xpos = -m_pos[0];
  *Ypos = -m_pos[1];
  *Zpos = -m_pos[2];

  // Get the number of pixels, field of view, and camera position (in world coords)
  glGetIntegerv(GL_VIEWPORT, vp);

  // Compute distance to viewplane (based on pixels and field of view in the y direction)
  zdist = ((double)vp[3] * 0.5f) / (double)tan((m_fovy * 0.5f) * gmDEGTORAD);

  // Create a vector (in camera space) from camera to pixel on screen
  camvec.assign(( ((double)vp[2] * 0.5f)-(double)screenx),
                ( ((double)vp[3] * 0.5f) - (double)(vp[3]-screeny)), zdist);

  // Get the camera matrix and invert
  imtx = m_ViewMatrix;
  imtx = imtx.inverse();

  // Using the inverse ViewMatrix determine the ray direction
  retvec = imtx.transform( camvec );
  retvec.normalize();

  *dX = -retvec[0];
  *dY = -retvec[1];
  *dZ = -retvec[2];
}


void COGLCamera::ComputeNearAndFar(double x, double y, double z, double radius ) //bounding sphere of data
{
  gmVector3 cpt(x,y,z), retcpt;
  double dist;

  GetPosition( &retcpt );
  retcpt = retcpt - cpt;
  dist = retcpt.length();
  m_near = dist - radius;
  if (m_near < 0.1 ) m_near = 0.1;
  m_far = dist + radius;
  if (m_far < 0.2 ) m_far = 0.2;
}

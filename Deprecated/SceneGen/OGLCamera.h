// OGLCamera.h: interface for the COGLCamera class.
//
//////////////////////////////////////////////////////////////////////
#ifndef __OGLCamera_H_

#define __OGLCamera_H_

#include "CrossGlut.h"

#include "gm.h"

#
#include <fstream>

using namespace std;

class COGLCamera
{
private:
  gmMatrix4 m_ViewMatrix;                    //Camera orientation (Normalized)
  gmVector3 m_pos;                           //Camera position
  gmVector3 m_up;                            //Camera down direction (for leveling code)
  double m_focaldistance;                     //Camera focal distance (along orientation line)
  double m_fovy;                              //Camera field of view (vertical)
  double m_aspect;                            //Camera aspect (horizontal/vertical)
  double m_near, m_far;                       //Camera near and far clipping planes
  int m_datachanged;                         //Camera data has changed since the last plot.

public:
  COGLCamera();
  virtual ~COGLCamera();

  // *** Re-establish the original viewing coordinates ***
  void Reset();

  // *** Get information from the camera about conditions ***
  void GetPosition( double *px, double *py, double *pz );
  void GetPosition( gmVector3 *posvec );
  void SetUpDirection( double px, double py, double pz );
  void SetUpDirection( gmVector3 *posvec );
  double GetFocalDistance();
  double GetFieldOfView() { return m_fovy; }
  double GetViewAspect() { return m_aspect; }

  // *** Location and orientation information ***
  void MoveTo(double posx, double posy, double posz);
  void MoveTo( gmVector3 *vec );
  void Move(double fore, double horiz, double vert);
  void Move( gmVector3 *vec );

  void Rotate(double yaw, double pitch, double roll);
  void RotateAboutFocalPoint(double yaw, double pitch, double roll);

  void SetIdentity();

  // *** Called before drawing, this routine tells OpenGL what we need ***
  void Fix();

  // *** Establish a view frustum ***
  void SetViewAngle(double fovy);
  void SetViewAspect(double asp);
  void SetViewNearPlane(double nr);
  void SetViewFarPlane(double fr);
  void SetFocalDistance(double fd);
  void ComputeNearAndFar(double x, double y, double z, double radius ); //bounding sphere of data

  // *** Helpers for Picking and Selecting ***
  void GetViewVector(long screenx, long screeny,          //Helpful for ray-tracing.
                     double *Xpos, double *Ypos, double *Zpos, //Given a screen coord. get the
                     double *dX, double *dY, double *dZ);      // world ray from eye
  // *** File access ***
  void SaveCameraSettings( const char *filename );
  void LoadCameraSettings( const char *filename );

  void SaveCameraSettings( std::ostream& ofile );
  void LoadCameraSettings( std::istream& ifile );

  void PickPerspective();
  void Level();

  void FixFocalParameters();

};

#endif

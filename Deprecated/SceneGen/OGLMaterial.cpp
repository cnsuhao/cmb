// OGLMaterial.cpp: implementation of the COGLMaterial class.
//
//////////////////////////////////////////////////////////////////////

#include "OGLMaterial.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COGLMaterial::COGLMaterial()
{
  m_Face = GL_FRONT;

  m_Shininess = 0.0f;

  m_Ambient[0] = 1.0f;
  m_Ambient[1] = 1.0f;
  m_Ambient[2] = 1.0f;
  m_Ambient[3] = 1.0f;

  m_Diffuse[0] = 1.0f;
  m_Diffuse[1] = 1.0f;
  m_Diffuse[2] = 1.0f;
  m_Diffuse[3] = 1.0f;

  m_Specular[0] = 0.0f;
  m_Specular[1] = 0.0f;
  m_Specular[2] = 0.0f;
  m_Specular[3] = 0.0f;

  m_Emission[0] = 0.0f;
  m_Emission[1] = 0.0f;
  m_Emission[2] = 0.0f;
  m_Emission[3] = 0.0f;

}

COGLMaterial::~COGLMaterial()
{

}

//////////////////////////////////////////////////////////////////////
// Setting Values
//////////////////////////////////////////////////////////////////////

void COGLMaterial::SetFace( int face )
{
  if (face == GL_FRONT) m_Face = face;
  if (face == GL_BACK) m_Face = face;
  if (face == GL_FRONT_AND_BACK) m_Face = face;
}

void COGLMaterial::SetShininess( float sh )
{
  m_Shininess = sh;
}

void COGLMaterial::SetAmbient( float ar, float ag, float ab, float aa )
{
  m_Ambient[0] = ar;
  m_Ambient[1] = ag;
  m_Ambient[2] = ab;
  m_Ambient[3] = aa;
}

void COGLMaterial::SetDiffuse( float ar, float ag, float ab, float aa)
{
  m_Diffuse[0] = ar;
  m_Diffuse[1] = ag;
  m_Diffuse[2] = ab;
  m_Diffuse[3] = aa;
}

void COGLMaterial::SetSpecular( float ar, float ag, float ab, float aa)
{
  m_Specular[0] = ar;
  m_Specular[1] = ag;
  m_Specular[2] = ab;
  m_Specular[3] = aa;
}

void COGLMaterial::SetEmission( float ar, float ag, float ab, float aa)
{
  m_Emission[0] = ar;
  m_Emission[1] = ag;
  m_Emission[2] = ab;
  m_Emission[3] = aa;
}

//////////////////////////////////////////////////////////////////////
// Setting OpenGL
//////////////////////////////////////////////////////////////////////

void COGLMaterial::Fix()
{
  glMaterialfv(m_Face, GL_SHININESS, &m_Shininess);
  glMaterialfv(m_Face, GL_AMBIENT, m_Ambient);
  glMaterialfv(m_Face, GL_DIFFUSE, m_Diffuse);
  glMaterialfv(m_Face, GL_SPECULAR, m_Specular);
  glMaterialfv(m_Face, GL_EMISSION, m_Emission);
}

// OGLMaterial.h: interface for the COGLMaterial class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(OGLMATERIAL_H)
#define OGLMATERIAL_H

#include "CrossGlut.h"

class COGLMaterial
{
private:
  GLint   m_Face;

  GLfloat m_Shininess;
  GLfloat m_Ambient[4];
  GLfloat m_Diffuse[4];
  GLfloat m_Specular[4];
  GLfloat m_Emission[4];
public:
  COGLMaterial();
  virtual ~COGLMaterial();

  void SetFace( int face );
  void SetShininess( float sh );
  void SetAmbient( float ar, float ag, float ab, float aa = 1.0f );
  void SetDiffuse( float ar, float ag, float ab, float aa = 1.0f );
  void SetSpecular( float ar, float ag, float ab, float aa = 1.0f );
  void SetEmission( float ar, float ag, float ab, float aa = 1.0f );
  void GetDiffuse( float& ar, float& ag, float& ab, float& aa ) { ar=m_Diffuse[0]; ag=m_Diffuse[1]; ab=m_Diffuse[2]; aa=m_Diffuse[3]; }

  void Fix();
};

#endif // !defined(OGLMATERIAL_H)

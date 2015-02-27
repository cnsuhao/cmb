// CrossGLUT.h:  This file fixes GLUT for multiplatform compilation
//                created 03-07-2007
//////////////////////////////////////////////////////////////////////////////////

#if !defined(CROSSGLUT_H)
#define CROSSGLUT_H

#include "CrossPlatform.h"

// The Windows GL/gl.h depends on having windows.h included first:
#ifdef _WIN32
  #include <windows.h>
#endif

#ifdef MAC_COMPILE
  #include <OpenGL/gl.h>
  #include <OpenGL/glu.h>
#else
  #include <GL/gl.h>
  #include <GL/glu.h>
#endif

/////////////////////////////////////////////////////
// Finished
/////////////////////////////////////////////////////

#endif // !defined(CROSSGLUT_H)

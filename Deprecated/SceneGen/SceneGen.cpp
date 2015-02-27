
/*******************************************************************
 *									                                               *
 * 	 SceneGen main code - stes up glut and the callbacks and menus *
 *                                                                 *
 *******************************************************************/

/*
 *  Oct 05 2006
 */

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "SceneGen.h"

#include "BaseWnd.h"
#include "ListWnd.h"
#include "gVertScroll.h"

#include "ViewWnd.h"
#include "gWndMan.h"

////////////////////////////////////////////////////////////////////
// Global Variables
//
MainSceneView msv;  //our structure for the program and it's menus

int main(int argc, char **argv)
{
  string infname;

  if (argc < 2)
  {
    printf("Usage:  %s <OSDfile>\n", argv[0]);
    //exit(0);
  }
  infname.assign(argv[1]);

  //Initialize GLUT and the primary windows
  glutInit(&argc, argv);

  glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);

  BaseWnd *BaseWindow;

  BaseWindow = new BaseWnd;
  BaseWindow->SetPosition(64,64);
  BaseWindow->SetDimensions(1024,768);
  BaseWindow->SetCaption("SceneGen");
  int gid;
  gid = AddWindow(BaseWindow);
  BaseWindow->Init();

  //initialize msv values
  msv.DLbase = glGenLists(60);
  //Get Initial Data
  cout << "Reading OSDL File\n";
  msv.osdl.Read(infname.c_str());

  cout << "Finished Reading OSDL File\n";
  //testing the write option
  BaseWindow->Prime();

  //Start processing
  glutMainLoop();

  return 0;             /* ANSI C requires main to return int. */
}

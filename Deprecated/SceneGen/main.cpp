#include <QApplication>

#include <QGLWidget>
#include "SceneGenMainWnd.h"

using namespace std;

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  if (!QGLFormat::hasOpenGL()) {
      cerr << "This system has no OpenGL support" << endl;
      return 1;
  }

  //Get Initial Data
  SceneGenMainWnd sgmw;

  if (argc > 1)
  {
    string infname(argv[1]);

    sgmw.loadFile(infname);
  }

  sgmw.resize(1024, 768);
  sgmw.show();

  return app.exec();
}

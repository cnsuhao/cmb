#ifndef SCENEGENMAINWINDOW_H
#define SCENEGENMAINWINDOW_H

#include <QMainWindow>
#include <QProgressBar>
#include <QLabel>

#include "SceneGen.h"

class QAction;
class ViewComb;

class SceneGenMainWnd : public QMainWindow
{
    Q_OBJECT

public:
  SceneGenMainWnd();
  bool NewData();
  bool loadFile(const string &fileName);
  void undo();
  void deleteSelectedItems();
  void importSurface( const string &fileName );
  void importTessObj( const string &fileName, double scl, double xpos, double ypos, double zpos, int basemat, bool rotYtoZ );
  void importOrientTessObj( const string &fileName, double scl, double xpos, double ypos, double zpos, int basemat, bool rotYtoZ );
  void importOccluder( const string &fileName, double scl, double xpos, double ypos, double zpos, int basemat, bool rotYtoZ );
  void importOrientOccluder( const string &fileName, double scl, double xpos, double ypos, double zpos, int basemat, bool rotYtoZ );

signals:
  void changeExtents( long x );
  void setProgressVal( int ii );

private slots:
  void OpenOSDL();
  void NewOSDL();
  void SaveXML();
  void SaveAnc();
  void Exit();

  void SetBMID();
  void SetScale();

  void zoomSurface();
  void zoomFULL();

  void undo_slot();
  void deleteSelectedItems_slot();
  void importSurface_slot();
  void importTessObj_slot();
  void importOrientTessObj_slot();
  void importOccluder_slot();
  void importOrientOccluder_slot();

  void startingProgress( char *caption );
  void finishingProgress();
  void updateProgress( int ii );

  void displayMessage( char *msg );
  void clearStatusBar();

  void ProcessVidScript();
  void SetCamera();
  void ImageCapture();
  void CaptureCamera();
  void RestoreCamera();

private:
    void createActions();
    void createMenus();
    void createToolbars();
    void createStatusBar();
    QString strippedName(const QString &fullFileName);

    MainSceneData *msd;
    ViewComb *vcmb;

    QMenu *fileMenu;
    QAction *OpenOSDL_Action;
    QAction *NewOSDL_Action;
    QAction *SaveXML_Action;
    QAction *SA_Action;
    QAction *Exit_Action;

    QToolBar *fileToolBar;

    QMenu *editMenu;
    QAction *SetBMID_Action;
    QAction *SetScale_Action;
    QAction *Delete_Action;
    QAction *Undo_Action;

    QToolBar *editToolBar;

    QMenu *viewMenu;
    QAction *ZSE_Action;
    QAction *ZFULL_Action;
    QAction *SCRIPT_Action;
    QAction *SETCAM_Action;
    QAction *CAPCAM_Action;
    QAction *RESCAM_Action;
    QAction *ICAP_Action;

    QMenu *importMenu;
    QAction *IPS_Action;
    QAction *IPTO_Action;
    QAction *IPOTO_Action;
    QAction *IPO_Action;
    QAction *IPOO_Action;

    QStatusBar *statusbar;
    QProgressBar *SB_Progress;
    QLabel *SB_Label;
};

#endif

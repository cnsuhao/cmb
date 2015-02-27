#include <QtGui>

#include "SceneGenMainWnd.h"
#include "ViewComb.h"
#include "dlgBaseMatID.h"
#include "dlgScale.h"
#include "dlgCameraLocation.h"
#include "dlgObjectImport.h"
#include "dlgNew.h"
#include "PathFix.h"

SceneGenMainWnd::SceneGenMainWnd()
{
  msd = new MainSceneData;
  vcmb = new ViewComb;

  msd->osdl.FillTree(vcmb->GetTree());

  vcmb->SetMSD(msd);
  setCentralWidget(vcmb);

  vcmb->Prime();

  createActions();
  createMenus();
  createToolbars();
  createStatusBar();
}

///////////////////////////////////////////////////////////////////
// Public Methods
///////////////////////////////////////////////////////////////////
bool SceneGenMainWnd::NewData()
{
    msd->osdl.Init();

    QString fname;
    int scl, basemat;
    double posx, posy, posz;

    dlgNew *DN = new dlgNew();
    if (DN->exec())
    {
      if (DN->getResults(fname, scl, posx, posy, posz, basemat) )
      {
        setCursor(Qt::WaitCursor);
        string fn = fname.toAscii().constData();
        msd->osdl.ImportPrimarySurface(fn.c_str(), posx, posy, posz, basemat);
        msd->osdl.SetGlobalUnits(scl);

        setCursor(Qt::ArrowCursor);
      }
    }
    delete DN;

    vcmb->SetAccel();
    vcmb->SetMSD(msd);

    msd->osdl.FillTree(vcmb->GetTree());
    vcmb->ResetView();
    return true;
}

bool SceneGenMainWnd::loadFile(const string &fileName)
{
    msd->osdl.Init();
//    msd->osdl.Read(fileName.data());
    if (!msd->osdl.Read(fileName.c_str()) ) {
        statusBar()->showMessage(tr("Loading canceled"), 2000);
        return false;
    }

    vcmb->SetAccel();
    vcmb->SetMSD(msd);

    msd->osdl.FillTree(vcmb->GetTree());
    vcmb->ResetView();

    //vcmb->SetCameraPosition(13.35, 6.54, 10.42, 85.12, 20);
    return true;
}

void SceneGenMainWnd::deleteSelectedItems( )
{
  msd->osdl.DeleteSelectedItems();
  msd->osdl.FillTree(vcmb->GetTree());
  //vcmb->ResetView();
}

void SceneGenMainWnd::undo( )
{
  msd->osdl.Undo();
  msd->osdl.FillTree(vcmb->GetTree());
  //vcmb->ResetView();
}

void SceneGenMainWnd::importSurface( const string &fileName )
{
  msd->osdl.ImportSurface(fileName.c_str());

  vcmb->SetMSD(msd);

  msd->osdl.FillTree(vcmb->GetTree());
  vcmb->ResetView();
}

void SceneGenMainWnd::importTessObj( const string &fileName, double scl, double xpos, double ypos, double zpos, int basemat, bool rotYtoZ )
{
  msd->osdl.ImportTesselatedObject(fileName.c_str(), scl, xpos, ypos, zpos, basemat, rotYtoZ);

  vcmb->SetMSD(msd);

  msd->osdl.FillTree(vcmb->GetTree());
  vcmb->ResetView();
}

void SceneGenMainWnd::importOrientTessObj( const string &fileName, double scl, double xpos, double ypos, double zpos, int basemat, bool rotYtoZ )
{
  msd->osdl.ImportOrientedTesselatedObject(fileName.c_str(), scl, xpos, ypos, zpos, basemat, rotYtoZ);

  vcmb->SetMSD(msd);

  msd->osdl.FillTree(vcmb->GetTree());
  vcmb->ResetView();
}

void SceneGenMainWnd::importOccluder( const string &fileName, double scl, double xpos, double ypos, double zpos, int basemat, bool rotYtoZ )
{
  msd->osdl.ImportOccluder(fileName.c_str(), scl, xpos, ypos, zpos, basemat, rotYtoZ);

  vcmb->SetMSD(msd);

  msd->osdl.FillTree(vcmb->GetTree());
  vcmb->ResetView();
}

void SceneGenMainWnd::importOrientOccluder( const string &fileName, double scl, double xpos, double ypos, double zpos, int basemat, bool rotYtoZ )
{
  msd->osdl.ImportOrientedOccluder(fileName.c_str(), scl, xpos, ypos, zpos, basemat, rotYtoZ);

  vcmb->SetMSD(msd);

  msd->osdl.FillTree(vcmb->GetTree());
  vcmb->ResetView();
}

///////////////////////////////////////////////////////////////////
// Private Methods
///////////////////////////////////////////////////////////////////
void SceneGenMainWnd::createActions()
{
    OpenOSDL_Action = new QAction(tr("&Open OSDL"), this);
    OpenOSDL_Action->setIcon(QIcon(":/set/48x48-open.png"));
    OpenOSDL_Action->setShortcut(tr("Ctrl+O"));
    OpenOSDL_Action->setStatusTip(tr("Load OSDL file"));
    connect(OpenOSDL_Action, SIGNAL(triggered()), this, SLOT(OpenOSDL()));

    NewOSDL_Action = new QAction(tr("&New"), this);
    NewOSDL_Action->setIcon(QIcon(":/set/48x48-new.png"));
    NewOSDL_Action->setShortcut(tr("Ctrl+N"));
    NewOSDL_Action->setStatusTip(tr("Clear Data"));
    connect(NewOSDL_Action, SIGNAL(triggered()), this, SLOT(NewOSDL()));

    SaveXML_Action = new QAction(tr("&Save OSDL XML"), this);
//    SaveXML_Action->setIcon(QIcon(":/set/48x48-open.png"));
    SaveXML_Action->setShortcut(tr("Ctrl+L"));
    SaveXML_Action->setStatusTip(tr("Save OSDL XML file"));
    connect(SaveXML_Action, SIGNAL(triggered()), this, SLOT(SaveXML()));

    SA_Action = new QAction(tr("&Export and Exit"), this);
    SA_Action->setIcon(QIcon(":/set/48x48-save.png"));
    SA_Action->setShortcut(tr("Ctrl+S"));
    SA_Action->setStatusTip(tr("Export Omega, Vegetation, Wavefront, and POV-Ray files"));
    connect(SA_Action, SIGNAL(triggered()), this, SLOT(SaveAnc()));

    Exit_Action = new QAction(tr("E&xit..."), this);
    Exit_Action->setIcon(QIcon(":/set/48x48-exit.png"));
    Exit_Action->setShortcut(tr("Ctrl+X"));
    Exit_Action->setStatusTip(tr("Finish using this extraordinary program"));
    connect(Exit_Action, SIGNAL(triggered()), this, SLOT(Exit()));

    SetBMID_Action = new QAction(tr("Base Material ID..."), this);
//    SetBMID_Action->setIcon(QIcon(":/images/open.png"));
//    SetBMID_Action->setShortcut(tr("Ctrl+X"));
    SetBMID_Action->setStatusTip(tr("Set the base material ID for selected items"));
    connect(SetBMID_Action, SIGNAL(triggered()), this, SLOT(SetBMID()));

    SetScale_Action = new QAction(tr("Scale..."), this);
//    SetScale_Action->setIcon(QIcon(":/images/open.png"));
//    SetScale_Action->setShortcut(tr("Ctrl+X"));
    SetScale_Action->setStatusTip(tr("Set the scale factor for selected items"));
    connect(SetScale_Action, SIGNAL(triggered()), this, SLOT(SetScale()));

    Undo_Action = new QAction(tr("Undo"), this);
    Undo_Action->setIcon(QIcon(":/set/48x48-undo.png"));
//    Undo_Action->setShortcut(Qt::Key_Delete);
    Undo_Action->setStatusTip(tr("Undo last action"));
    connect(Undo_Action, SIGNAL(triggered()), this, SLOT(undo_slot()));

    Delete_Action = new QAction(tr("Delete selected"), this);
    Delete_Action->setIcon(QIcon(":/set/48x48-cut.png"));
    Delete_Action->setShortcut(Qt::Key_Delete);
    Delete_Action->setStatusTip(tr("Deletes all selected items"));
    connect(Delete_Action, SIGNAL(triggered()), this, SLOT(deleteSelectedItems_slot()));

    ZSE_Action = new QAction(tr("Zoom Surface Extents"), this);
//    ZSE_Action->setIcon(QIcon(":/images/new.png"));
//    ZSE_Action->setShortcut(tr("Ctrl+X"));
    ZSE_Action->setStatusTip(tr("Zoom to the defining surface extents"));
    connect(ZSE_Action, SIGNAL(triggered()), this, SLOT(zoomSurface()));

    ZFULL_Action = new QAction(tr("Zoom Full"), this);
//    ZFULL_Action->setIcon(QIcon(":/images/new.png"));
//    ZFULL_Action->setShortcut(tr("Ctrl+X"));
    ZFULL_Action->setStatusTip(tr("Zoom to the full area of interest region"));
    connect(ZFULL_Action, SIGNAL(triggered()), this, SLOT(zoomFULL()));

    SETCAM_Action = new QAction(tr("Set Camera..."), this);
//    SETCAM_Action->setIcon(QIcon(":/images/new.png"));
//    SETCAM_Action->setShortcut(tr("Ctrl+X"));
    SETCAM_Action->setStatusTip(tr("Set the camera position and orientation (relative to the surface)"));
    connect(SETCAM_Action, SIGNAL(triggered()), this, SLOT(SetCamera()));

    CAPCAM_Action = new QAction(tr("Capture Camera"), this);
//    CAPCAM_Action->setIcon(QIcon(":/images/new.png"));
//    CAPCAM_Action->setShortcut(tr("Ctrl+X"));
    CAPCAM_Action->setStatusTip(tr("Capture the current camera position and orientation"));
    connect(CAPCAM_Action, SIGNAL(triggered()), this, SLOT(CaptureCamera()));

    RESCAM_Action = new QAction(tr("Restore Camera"), this);
//    RESCAM_Action->setIcon(QIcon(":/images/new.png"));
//    RESCAM_Action->setShortcut(tr("Ctrl+X"));
    RESCAM_Action->setStatusTip(tr("Restore the captured camera position and orientation"));
    connect(RESCAM_Action, SIGNAL(triggered()), this, SLOT(RestoreCamera()));

    ICAP_Action = new QAction(tr("Image Capture"), this);
//    ZFULL_Action->setIcon(QIcon(":/images/new.png"));
//    ZFULL_Action->setShortcut(tr("Ctrl+X"));
    ICAP_Action->setStatusTip(tr("Capture the current view of the data"));
    connect(ICAP_Action, SIGNAL(triggered()), this, SLOT(ImageCapture()));

    SCRIPT_Action = new QAction(tr("Process Camera Script..."), this);
//    SCRIPT_Action->setIcon(QIcon(":/images/new.png"));
//    SCRIPT_Action->setShortcut(tr("Ctrl+X"));
    SCRIPT_Action->setStatusTip(tr("Processes a script of camera shots"));
    connect(SCRIPT_Action, SIGNAL(triggered()), this, SLOT(ProcessVidScript()));

    IPS_Action = new QAction(tr("Ancillary Surface"), this);
//    IPS_Action->setIcon(QIcon(":/images/new.png"));
//    IPS_Action->setShortcut(tr("Ctrl+X"));
    IPS_Action->setStatusTip(tr("Import an ancillary surface for the scene"));
    connect(IPS_Action, SIGNAL(triggered()), this, SLOT(importSurface_slot()));

    IPTO_Action = new QAction(tr("Tesselated Object"), this);
//    IPTO_Action->setIcon(QIcon(":/images/new.png"));
//    IPTO_Action->setShortcut(tr("Ctrl+X"));
    IPTO_Action->setStatusTip(tr("Import a tesselated object for the scene"));
    connect(IPTO_Action, SIGNAL(triggered()), this, SLOT(importTessObj_slot()));

    IPOTO_Action = new QAction(tr("Oriented Tesselated Object"), this);
//    IPOTO_Action->setIcon(QIcon(":/images/new.png"));
//    IPOTO_Action->setShortcut(tr("Ctrl+X"));
    IPOTO_Action->setStatusTip(tr("Import an oriented tesselated object for the scene"));
    connect(IPOTO_Action, SIGNAL(triggered()), this, SLOT(importOrientTessObj_slot()));

    IPO_Action = new QAction(tr("Occluder"), this);
//    IPO_Action->setIcon(QIcon(":/images/new.png"));
//    IPO_Action->setShortcut(tr("Ctrl+X"));
    IPO_Action->setStatusTip(tr("Import an occluder for the scene"));
    connect(IPO_Action, SIGNAL(triggered()), this, SLOT(importOccluder_slot()));

    IPOO_Action = new QAction(tr("Oriented Occluder"), this);
//    IPOO_Action->setIcon(QIcon(":/images/new.png"));
//    IPOO_Action->setShortcut(tr("Ctrl+X"));
    IPOO_Action->setStatusTip(tr("Import an oriented occluder for the scene"));
    connect(IPOO_Action, SIGNAL(triggered()), this, SLOT(importOrientOccluder_slot()));

    connect(vcmb, SIGNAL(changeMessage(char *)), this, SLOT(displayMessage(char *)));
    connect(vcmb, SIGNAL(clearStatus()), this, SLOT(clearStatusBar()));

    connect(this, SIGNAL(changeExtents(long)), vcmb, SLOT(extentsChanged(long)));
}

void SceneGenMainWnd::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(OpenOSDL_Action);
    fileMenu->addAction(NewOSDL_Action);
    fileMenu->addAction(SaveXML_Action);
    fileMenu->addSeparator();
    fileMenu->addAction(SA_Action);
    fileMenu->addAction(Exit_Action);

    editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(SetBMID_Action);
    editMenu->addAction(SetScale_Action);
    editMenu->addSeparator();
    editMenu->addAction(Delete_Action);
    editMenu->addSeparator();
    editMenu->addAction(Undo_Action);

    viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(ZSE_Action);
    viewMenu->addAction(ZFULL_Action);
    viewMenu->addSeparator();
    viewMenu->addAction(SETCAM_Action);
    viewMenu->addSeparator();
    viewMenu->addAction(CAPCAM_Action);
    viewMenu->addAction(RESCAM_Action);
    viewMenu->addSeparator();
    viewMenu->addAction(ICAP_Action);
    viewMenu->addAction(SCRIPT_Action);

    importMenu = menuBar()->addMenu(tr("&Import"));
    //importMenu->addAction(IPS_Action);
    importMenu->addAction(IPTO_Action);
    importMenu->addAction(IPOTO_Action);
    importMenu->addAction(IPO_Action);
    importMenu->addAction(IPOO_Action);
}

void SceneGenMainWnd::createToolbars()
{
    fileToolBar = addToolBar(tr("&File"));
    fileToolBar->addAction(NewOSDL_Action);
    fileToolBar->addAction(OpenOSDL_Action);
    fileToolBar->addAction(SA_Action);
    fileToolBar->addAction(Exit_Action);

    editToolBar = addToolBar(tr("&Edit"));
    editToolBar->addAction(Delete_Action);
    editToolBar->addAction(Undo_Action);
}

void SceneGenMainWnd::createStatusBar()
{
    statusbar = new QStatusBar;
    setStatusBar(statusbar);
    connect(&msd->osdl, SIGNAL(startProgress(char *)), this, SLOT(startingProgress(char *)));
    connect(&msd->osdl, SIGNAL(finishProgress()), this, SLOT(finishingProgress()));
}


QString SceneGenMainWnd::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

///////////////////////////////////////////////////////////////////
// Slots
///////////////////////////////////////////////////////////////////
void SceneGenMainWnd::OpenOSDL()
{
  QString fname = QFileDialog::getOpenFileName(this,tr("Open OSDL"), "*.osd.txt", tr("OSDL Files (*.osd.txt)"));

  if (!fname.isEmpty())
  {
    setCursor(Qt::WaitCursor);
    QFileInfo qfi( fname );
    QString pname = qfi.path();
    QDir::setCurrent(pname);
    printf("  OpenOSDL:  %s %s\n", fname.toAscii().constData(), pname.toAscii().constData() );
    string fn = fname.toAscii().constData();
    loadFile(fn);
    setCursor(Qt::ArrowCursor);
  }
}

void SceneGenMainWnd::NewOSDL()
{
  NewData();
}

void SceneGenMainWnd::SaveXML()
{
  QString fname = QFileDialog::getSaveFileName(this,tr("Save OSDL"), "*.osd", tr("OSDL Files (*.osd)"));

  if (!fname.isEmpty())
  {
    setCursor(Qt::WaitCursor);
    QFileInfo qfi( fname );
    QString pname = qfi.path();
    QDir::setCurrent(pname);
    printf("  OpenOSDL:  %s %s\n", fname.toAscii().constData(), pname.toAscii().constData() );
    string fn = fname.toAscii().constData();
    msd->osdl.WriteXML(fn.c_str());
    setCursor(Qt::ArrowCursor);
  }
}

void SceneGenMainWnd::SaveAnc()
{
  msd->osdl.ProcessOutputs();
  close();
}

void SceneGenMainWnd::Exit()
{
  msd->osdl.WriteXML("OSDL.XML");
  close();
}

void SceneGenMainWnd::displayMessage( char *msg )
{
  statusBar()->showMessage(tr(msg), 0);
}

void SceneGenMainWnd::clearStatusBar()
{
  statusBar()->clearMessage();
}
void SceneGenMainWnd::zoomSurface()
{
  emit changeExtents( AOI_SURFACE );
}

void SceneGenMainWnd::zoomFULL()
{
  emit changeExtents( AOI_FULL );
}

void SceneGenMainWnd::deleteSelectedItems_slot()
{
  deleteSelectedItems();
}

void SceneGenMainWnd::undo_slot()
{
  undo();
}

void SceneGenMainWnd::importSurface_slot()
{
  QString fname = QFileDialog::getOpenFileName(this,tr("Import Surface"), "*.*", tr("Surface Files (*.pts *.2dm *.3dm)"));

  if (!fname.isEmpty())
  {
    setCursor(Qt::WaitCursor);
    string fn = fname.toAscii().constData();
    importSurface(fn);
    setCursor(Qt::ArrowCursor);
  }
}

void SceneGenMainWnd::importTessObj_slot()
{
  QString fname;
  double scl;
  double posx, posy, posz;
  int basemat;
  bool rotYtoZ;

  dlgObjectImport *DOI = new dlgObjectImport();
  DOI->setTitle(tr("Import Tessellated Object"));
  DOI->setFileAttributes("Import Tessellated Object", "TessObj Files (*.2dm, *.3dm, *.obj)");
  DOI->setGlobalUnits(1);
  if (DOI->exec())
  {
    if (DOI->getResults(fname, scl, posx, posy, posz, basemat, rotYtoZ) )
    {
      setCursor(Qt::WaitCursor);
      string fn = fname.toAscii().constData();
      importTessObj(fn, scl, posx, posy, posz, basemat, rotYtoZ);
      setCursor(Qt::ArrowCursor);
    }
  }
  delete DOI;
}

void SceneGenMainWnd::importOrientTessObj_slot()
{
  QString fname;
  double scl;
  double posx, posy, posz;
  int basemat;
  bool rotYtoZ;

  dlgObjectImport *DOI = new dlgObjectImport();
  DOI->setTitle(tr("Import Oriented Tessellated Object"));
  DOI->setFileAttributes("Import Oriented Tessellated Object", "TessObj Files (*.2dm, *.3dm, *.obj)");
  DOI->setGlobalUnits(1);
  if (DOI->exec())
  {
    if (DOI->getResults(fname, scl, posx, posy, posz, basemat, rotYtoZ) )
    {
      setCursor(Qt::WaitCursor);
      string fn = fname.toAscii().constData();
      importOrientTessObj(fn, scl, posx, posy, posz, basemat, rotYtoZ);
      setCursor(Qt::ArrowCursor);
    }
  }
  delete DOI;
}

void SceneGenMainWnd::importOccluder_slot()
{
  QString fname;
  double scl;
  double posx, posy, posz;
  int basemat;
  bool rotYtoZ;

  dlgObjectImport *DOI = new dlgObjectImport();
  DOI->setTitle(tr("Import Occluder"));
  DOI->setFileAttributes("Import Occluder", "Occluder Files (*.2dm, *.3dm, *.obj)");
  DOI->setGlobalUnits(1);
  if (DOI->exec())
  {
    if (DOI->getResults(fname, scl, posx, posy, posz, basemat, rotYtoZ) )
    {
      setCursor(Qt::WaitCursor);
      string fn = fname.toAscii().constData();
      importOccluder(fn, scl, posx, posy, posz, basemat, rotYtoZ);
      setCursor(Qt::ArrowCursor);
    }
  }
  delete DOI;
}

void SceneGenMainWnd::importOrientOccluder_slot()
{
  QString fname;
  double scl;
  double posx, posy, posz;
  int basemat;
  bool rotYtoZ;

  dlgObjectImport *DOI = new dlgObjectImport();
  DOI->setTitle(tr("Import Oriented Occluder"));
  DOI->setFileAttributes("Import Oriented Occluder", "Occluder Files (*.2dm, *.3dm, *.obj)");
  DOI->setGlobalUnits(1);
  if (DOI->exec())
  {
    if (DOI->getResults(fname, scl, posx, posy, posz, basemat, rotYtoZ) )
    {
      setCursor(Qt::WaitCursor);
      string fn = fname.toAscii().constData();
      importOrientOccluder(fn, scl, posx, posy, posz, basemat, rotYtoZ);
      setCursor(Qt::ArrowCursor);
    }
  }
  delete DOI;
}

// Progress slots
void SceneGenMainWnd::startingProgress( char *caption )
{
  SB_Label = new QLabel( caption );

  SB_Progress = new QProgressBar();
  SB_Progress->setRange(0,10000);

  statusbar->addWidget(SB_Label);
  statusbar->addWidget(SB_Progress);
  connect(&msd->osdl, SIGNAL(setProgressMax(int)), SB_Progress, SLOT(setMaximum(int)));
  connect(&msd->osdl, SIGNAL(setProgressVal(int)), this, SLOT(updateProgress(int)));
  connect(this, SIGNAL(setProgressVal(int)), SB_Progress, SLOT(setValue(int)));
}

void SceneGenMainWnd::finishingProgress()
{
  statusbar->removeWidget(SB_Label);
  statusbar->removeWidget(SB_Progress);

  delete SB_Label;
  delete SB_Progress;
}

void SceneGenMainWnd::updateProgress( int ii )
{
  msd->osdl.SetLoading(true);
  qApp->processEvents(); //QEventLoop::ExcludeUserInputEvents);
  emit setProgressVal(ii);
  msd->osdl.SetLoading(false);
}

void SceneGenMainWnd::ImageCapture()
{
  QDir qd;

  QString fname = QFileDialog::getSaveFileName(this,tr("Image Capture"), "*.bmp", tr("BMP Files (*.bmp)"));

  if (!fname.isEmpty())
  {
    setCursor(Qt::WaitCursor);
    QString pname = qd.filePath(fname);
    QDir::setCurrent(pname);
    string fn = fname.toAscii().constData();
    vcmb->saveScreen(fname.toAscii().constData());
    setCursor(Qt::ArrowCursor);
  }
}

void SceneGenMainWnd::SetCamera()
{
  double X;
  double Y;
  double Z;
  double Heading;
  double Pitch;
  double Yaw;

  dlgCameraLocation *DCL = new dlgCameraLocation();

  if (DCL->exec())
  {
    DCL->getResults( X, Y, Z, Heading, Pitch, Yaw );
    vcmb->SetCameraPosition( X, Y, Z, Heading, Pitch, Yaw );
  }
  delete DCL;
}

void SceneGenMainWnd::CaptureCamera()
{
  vcmb->SaveCameraSettings(".\\CamCap.cam");
}

void SceneGenMainWnd::RestoreCamera()
{
  vcmb->LoadCameraSettings(".\\CamCap.cam");
}

void SceneGenMainWnd::ProcessVidScript()
{
  QDir qd;

  string basename;
  char compname[80];
  int numframes;
  double xpos, ypos, zpos, heading, pitch, yaw;

  QString fname = QFileDialog::getOpenFileName(this,tr("Script Capture"), "*.spt", tr("script Files (*.spt)"));

  if (!fname.isEmpty())
  {
    setCursor(Qt::WaitCursor);
    QString pname = qd.filePath(fname);
    QDir::setCurrent(pname);
    string fn = fname.toAscii().constData();

    ifstream fin(pathfix(fn.c_str()));  //our input file stream
    fin >> basename >> numframes;
    for (int ii=0; ii<numframes; ii++)
    {
      fin >> xpos >> ypos >> zpos >> heading >> pitch >> yaw;
      vcmb->SetCameraPosition( xpos, ypos, zpos, heading, pitch, yaw );
      sprintf(compname,"%s%04d.bmp",basename.c_str(),ii+1);
      vcmb->saveScreen(pathfix(compname));
    }

    setCursor(Qt::ArrowCursor);
  }
}

void SceneGenMainWnd::SetBMID()
{
  int BMID = 0;
  dlgBaseMatID *BMIDD = new dlgBaseMatID();

  if (BMIDD->exec())
  {
    BMID = BMIDD->getBaseMatID();
    msd->osdl.ChangeSelectMatID( BMID );
    msd->osdl.FillTree(vcmb->GetTree());
  }
  delete BMIDD;
}

void SceneGenMainWnd::SetScale()
{
  double SCL = 0;
  dlgScale *DSCL = new dlgScale();

  if (DSCL->exec())
  {
    SCL = DSCL->getResults();
    msd->osdl.ChangeSelectRelativeScale( SCL, SCL, SCL );  //uniform scale (at present)
  }
  delete DSCL;
}

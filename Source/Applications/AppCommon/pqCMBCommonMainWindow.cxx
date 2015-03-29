/*=========================================================================

  Program:   CMB
  Module:    pqCMBCommonMainWindow.cxx

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
=========================================================================*/
#include "pqCMBCommonMainWindow.h"
#include "ui_qtCMBMainWindow.h"

#include "pqCMBCommonMainWindowCore.h"

#include <QDir>
#include <QMessageBox>
#include <QScrollArea>
#include <QShortcut>
#include <QtDebug>
#include <QToolButton>
#include <QToolBar>
#include <QLabel>
#include <QComboBox>
#include <QTimer>

#include "pqApplicationCore.h"
#include "pqDisplayColorWidget.h"
#include "vtkMapper.h"
#include "pqObjectBuilder.h"
#include "pqOutputPort.h"
#include "pqDataRepresentation.h"
#include "pqPipelineSource.h"
#include "pqPropertyLinks.h"
#include "pqRecentFilesMenu.h"
#include "pqRenderView.h"
#include "pqDisplayRepresentationWidget.h"
#include "pqCMBRubberBandHelper.h"
#include "pqScalarBarRepresentation.h"

#include "pqScalarsToColors.h"
#include "pqSelectionManager.h"
#include "pqServer.h"
#include "pqServerConnectReaction.h"
#include "pqServerDisconnectReaction.h"
#include "pqServerResource.h"
#include "pqSetName.h"
#include "pqSMAdaptor.h"
#include "pqSMProxy.h"
#include "pqViewMenuManager.h"
#include "pqWaitCursor.h"
#include "pqHelpReaction.h"
#include "pqEditCameraReaction.h"
#include "pqTimerLogReaction.h"

#include "vtkObjectFactory.h"
#include "vtkProcessModule.h"
#include "vtkSmartPointer.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMProxyManager.h"
#include "vtkSMRenderViewProxy.h"
#include "vtkSMSourceProxy.h"
#include "vtkVariant.h"
#include "vtkMapper.h"

#include "qtCMBAboutDialog.h"
#include "qtCMBHelpDialog.h"
#include "pqHelpReaction.h"
#include "pqCoreUtilities.h"
#include "qtCMBColorDialog.h"
#include "pqPipelineRepresentation.h"

#include "pqProxyWidget.h"
#include "pqSaveScreenshotReaction.h"
#include "pqTestingReaction.h"
#include "pqTimerLogReaction.h"

#include "vtkPVRenderViewSettings.h"
#include "vtkPVGeneralSettings.h"
#include "pqLoadStateReaction.h"
#include "pqSaveStateReaction.h"
#include "pqManagePluginsReaction.h"

#include "pqCMBLoadDataReaction.h"
#include "pqPluginIOBehavior.h"
#include "qtCMBPanelsManager.h"
#include "pqActiveObjects.h"
#include "pqProxyWidget.h"
#include "vtkCommand.h"
#include "pqCMBColorMapWidget.h"

class cmbRecentFilesMenu : public pqRecentFilesMenu
{
  QPointer<pqCMBCommonMainWindowCore> Core;
public:
  cmbRecentFilesMenu(QMenu& menu, QObject* p=0) :
    pqRecentFilesMenu(menu, p)
  {
  };

  void setCore(pqCMBCommonMainWindowCore* core)
    { this->Core = core; }

  virtual bool open(
    pqServer* server, const pqServerResource& resource) const
    {
    if (this->Core && server && !resource.path().isEmpty())
      {
      QFileInfo fInfo(resource.path());

      QString readerGroup = resource.data("modelmanager");
      QString readerName = resource.data("readoperator");

      if ((!readerName.isEmpty() && !readerGroup.isEmpty()) /* ||
        fInfo.suffix().toLower() == "cmb" */)
        {
        QStringList files;
        files << resource.path();
        this->Core->onFileOpen(files);
        return true;
        }
      }
    return pqRecentFilesMenu::open(server, resource);
    }
};

class pqCMBCommonMainWindow::vtkInternal
{
public:
  vtkInternal(QWidget* /*parent*/) :
    RecentFilesMenu(0)
    {
    this->DockPanelViewMenu=0;
    }

  ~vtkInternal()
    {
    delete this->DockPanelViewMenu;
    }

  Ui::qtCMBMainWindow UI;
  cmbRecentFilesMenu* RecentFilesMenu;
  pqViewMenuManager* DockPanelViewMenu;

  pqPropertyLinks GUILinker;

  QList<pqOutputPort*> LastSelectionPorts;

  QPointer<pqServerConnectReaction> ServerConnectReaction;
  QPointer<pqServerDisconnectReaction> ServerDisconnectReaction;
  QPointer<pqHelpReaction> HelpReaction;
  QPointer<pqEditCameraReaction> EditCameraReaction;
  QPointer<pqTimerLogReaction> TimerLogReaction;
  QPointer<pqCMBLoadDataReaction> LoadDataReaction;
  QPointer<qtCMBPanelsManager> panelsManager;
  QPointer<pqProxyWidget> displayPanel;
  QPointer<pqCMBColorMapWidget> ColorEditor;

};

//----------------------------------------------------------------------------
pqCMBCommonMainWindow::pqCMBCommonMainWindow():
SelectionShortcut(NULL),
ResetCameraShortcut(NULL),
Internal(new vtkInternal(this))
{
  this->MainWindowCore = NULL;
  this->Internal->UI.setupUi(this);
  this->Internal->RecentFilesMenu = new
    cmbRecentFilesMenu(*this->Internal->UI.menuRecentFiles, this);

  pqApplicationCore* core = pqApplicationCore::instance();

  QObject::connect(this->Internal->UI.action_Exit,
    SIGNAL(triggered()), core, SLOT(quit()));

  QObject::connect(this->Internal->UI.action_Help,
    SIGNAL(triggered()), this, SLOT(onHelpHelp()));

  QObject::connect(this->Internal->UI.action_About,
    SIGNAL(triggered()), this, SLOT(onHelpAbout()));

  this->Internal->ServerConnectReaction = new pqServerConnectReaction(
    this->Internal->UI.actionServerConnect);
  this->Internal->ServerDisconnectReaction = new pqServerDisconnectReaction(
    this->Internal->UI.actionServerDisconnect);

  QObject::connect(
    &pqActiveObjects::instance(), SIGNAL(viewChanged(pqView*)),
    this, SLOT(onViewChanged()));

  // Setup some standard shortcuts
  this->getMainDialog()->action_Close->setShortcuts(QKeySequence::Close);
  this->getMainDialog()->action_Exit->setShortcuts(QKeySequence::Quit);
  this->getMainDialog()->action_Help->setShortcuts(QKeySequence::HelpContents);
  this->getMainDialog()->action_Open_File->setShortcuts(QKeySequence::Open);
  this->getMainDialog()->action_Save_As->setShortcuts(QKeySequence::SaveAs);
  this->getMainDialog()->action_Save_Data->setShortcuts(QKeySequence::Save);
  this->getMainDialog()->actionNew->setShortcuts(QKeySequence::New);
  this->getMainDialog()->actionRedo->setShortcuts(QKeySequence::Redo);
  this->getMainDialog()->actionUndo->setShortcuts(QKeySequence::Undo);

}

//----------------------------------------------------------------------------
pqCMBCommonMainWindow::~pqCMBCommonMainWindow()
{
  delete this->Internal;

  if (this->SelectionShortcut)
    {
    delete this->SelectionShortcut;
    }

  if (this->ResetCameraShortcut)
    {
    delete this->ResetCameraShortcut;
    }
}

//----------------------------------------------------------------------------
qtCMBPanelsManager* pqCMBCommonMainWindow::panelsManager()
{
  if(!this->Internal->panelsManager)
    {
    this->Internal->panelsManager = new qtCMBPanelsManager(this);
    }
  return this->Internal->panelsManager;
}

//----------------------------------------------------------------------------
pqProxyWidget* pqCMBCommonMainWindow::displayPanel(vtkSMProxy* repProxy)
{
  if(this->Internal->displayPanel &&
    this->Internal->displayPanel->proxy() != repProxy)
    {
    delete this->Internal->displayPanel;
    this->Internal->displayPanel = NULL;
    }

  if(!this->Internal->displayPanel && repProxy)
    {
    this->Internal->displayPanel = new pqProxyWidget(repProxy, this);
    this->Internal->displayPanel->setView(this->MainWindowCore->activeRenderView());
    this->Internal->displayPanel->setApplyChangesImmediately(true);
//    pqActiveObjects::instance().disconnect(this->Internal->displayPanel);
    }

  return this->Internal->displayPanel;
}

//----------------------------------------------------------------------------
pqCMBColorMapWidget* pqCMBCommonMainWindow::colorEditor(QWidget* p)
{
  if(!this->Internal->ColorEditor)
    {
    this->Internal->ColorEditor = new pqCMBColorMapWidget(p);
    }
  return this->Internal->ColorEditor;
}

//-----------------------------------------------------------------------------
QDockWidget* pqCMBCommonMainWindow::createDockWidget (QMainWindow* mw,
  QWidget* content, const std::string& title,
  Qt::DockWidgetArea dockarea, QDockWidget* lastdw)
{
  QDockWidget* dw = new QDockWidget(mw);
  QWidget* container = new QWidget();
  container->setObjectName("dockscrollWidget");
  container->setSizePolicy(QSizePolicy::Preferred,
    QSizePolicy::Expanding);

  QScrollArea* s = new QScrollArea(dw);
  s->setWidgetResizable(true);
  s->setFrameShape(QFrame::NoFrame);
  s->setObjectName("scrollArea");
  s->setWidget(container);

  QVBoxLayout* vboxlayout = new QVBoxLayout(container);
  vboxlayout->setMargin(0);
  vboxlayout->addWidget(content);

  QString dockTitle(title.c_str());
  dw->setWindowTitle(dockTitle);
  dw->setObjectName(dockTitle.append("dockWidget"));
  dw->setWidget(s);
  mw->addDockWidget(dockarea,dw);
  if(lastdw)
    {
    mw->tabifyDockWidget(lastdw, dw);
    }
  return dw;
}

//----------------------------------------------------------------------------
void pqCMBCommonMainWindow::appendDatasetNameToTitle(const QString& strTitle)
{
  QString currentTitle = QApplication::applicationName().append(
    " ").append(QApplication::applicationVersion());

  QString newTitle = strTitle.isEmpty() ? currentTitle :
    currentTitle.append(" ( ").append(strTitle).append(" )");
  this->setWindowTitle(newTitle);
}
//----------------------------------------------------------------------------
void pqCMBCommonMainWindow::onViewChanged()
{
  if(this->MainWindowCore && this->MainWindowCore->activeRenderView())
    {
    this->setCentralWidget(
      this->MainWindowCore->activeRenderView()->widget());
    // Allow multiple representation selection
    this->MainWindowCore->activeRenderView()->
      setUseMultipleRepresentationSelection(true);

    QObject::connect(this->MainWindowCore->activeRenderView(),
      SIGNAL(multipleSelected(QList<pqOutputPort*>)),
      this, SLOT(onViewSelected(QList<pqOutputPort*>)));

    this->Internal->EditCameraReaction = new pqEditCameraReaction(
      this->Internal->UI.actionAdjustCamera,
      this->MainWindowCore->activeRenderView());

    vtkPVRenderViewSettings::GetInstance()->SetOutlineThreshold(50);
    vtkPVRenderViewSettings::GetInstance()->SetResolveCoincidentTopology(
      vtkPVRenderViewSettings::ZSHIFT);
    vtkPVRenderViewSettings::GetInstance()->SetZShift(0.002);
    vtkPVRenderViewSettings::GetInstance()->SetUseDisplayLists(1);

//    pqPipelineRepresentation::setUnstructuredGridOutlineThreshold(2.5);
//    pqServer::setCoincidentTopologyResolutionModeSetting(
//      VTK_RESOLVE_POLYGON_OFFSET);
//    pqServer::setGlobalImmediateModeRenderingSetting(false);
//  pqServer::setCoincidentTopologyResolutionModeSetting(
//    VTK_RESOLVE_SHIFT_ZBUFFER);
//  pqServer::setZShiftSetting(0.002);


//    this->updateViewPositions();
    }
}
//----------------------------------------------------------------------------
void pqCMBCommonMainWindow::onLockViewSize(bool lock)
{
  if (!lock)
    {
    this->centralWidget()->setMaximumSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
    }
  else
    {
    this->centralWidget()->setMaximumSize(QSize(300, 300));
    }
}

//----------------------------------------------------------------------------
void pqCMBCommonMainWindow::initMainWindowCore()
{
  if(!this->MainWindowCore)
    {
    return;
    }

  // Create builtin server connection.
  //pqApplicationCore::instance()->getObjectBuilder()->blockSignals(true);
  pqServer* server = pqApplicationCore::instance()->getObjectBuilder()
    ->createServer(pqServerResource("builtin:"));
  //pqApplicationCore::instance()->getObjectBuilder()->blockSignals(false);
  this->MainWindowCore->onServerCreationFinished(server);

  this->Internal->DockPanelViewMenu =
    new pqViewMenuManager(this, this->Internal->UI.menuShow);

  this->Internal->UI.faceParametersDock->setEnabled(false);

  QObject::connect(
    this->MainWindowCore, SIGNAL(enableExternalProcesses(bool)),
    this, SLOT(onEnableExternalProcesses(bool)));

  this->Internal->RecentFilesMenu->setCore(this->MainWindowCore);

//  QObject::connect(this->Internal->UI.action_Open_File,
//    SIGNAL(triggered()), this->MainWindowCore, SLOT(onFileOpen()));
  QObject::connect(this->Internal->UI.action_Save_Data,
    SIGNAL(triggered()), this->MainWindowCore, SLOT(onSaveData()));

  QObject::connect(this->Internal->UI.action_Save_As,
    SIGNAL(triggered()), this->MainWindowCore, SLOT(onSaveAsData()));

  QObject::connect(this->Internal->UI.action_Close,
    SIGNAL(triggered()), this->MainWindowCore, SLOT(onCloseData()));


  //<addaction name="actionToolsDumpWidgetNames" />
  new pqTestingReaction(this->Internal->UI.menu_Tools->addAction("Record Test...")
    << pqSetName("actionToolsRecordTest"),
    pqTestingReaction::RECORD);
  new pqTestingReaction(this->Internal->UI.menu_Tools->addAction("Play Test...")
    << pqSetName("actionToolsPlayTest"),
    pqTestingReaction::PLAYBACK,Qt::QueuedConnection);
  QObject::connect(this->Internal->UI.actionLock_View_Size,
    SIGNAL(toggled(bool)), this, SLOT(onLockViewSize(bool)));
//  new pqSaveScreenshotReaction(this->Internal->UI.actionSaveScreenshot);
  QObject::connect(this->Internal->UI.actionSaveScreenshot,
    SIGNAL(triggered()), this->MainWindowCore, SLOT(onSaveScreenshot()));

//  new pqTestingReaction(this->Internal->UI.menu_Tools->addAction("Lock View Size")
//    << pqSetName("actionTesting_Window_Size"),
//    pqTestingReaction::LOCK_VIEW_SIZE);
//  new pqTestingReaction(this->Internal->UI.menu_Tools->addAction("Lock View Size Custom...")
//    << pqSetName("actionTesting_Window_Size_Custom"),
//    pqTestingReaction::LOCK_VIEW_SIZE_CUSTOM);
  this->Internal->UI.menu_Tools->addSeparator();
  new pqTimerLogReaction(this->Internal->UI.menu_Tools->addAction("Timer Log")
    << pqSetName("actionToolsTimerLog"));
  QAction* action = this->Internal->UI.menu_Tools->addAction("&Output Window")
    << pqSetName("actionToolsOutputWindow");
  QObject::connect(action, SIGNAL(triggered()),
    pqApplicationCore::instance(),
    SLOT(showOutputWindow()));
  this->Internal->UI.menu_Tools->addSeparator();

#ifdef BUILD_SHARED_LIBS
  // Add support for importing plugins only if ParaView was built shared.
  new pqManagePluginsReaction(this->Internal->UI.menu_Tools->addAction("Manage Plugins...") <<
    pqSetName("actionManage_Plugins"));
#else
  QAction* action2 = this->Internal->UI.menu_Tools->addAction("Manage Plugins...");
  action2->setEnabled(false);
  action2->setToolTip(
    "Use BUILD_SHARED:ON while compiling to enable plugin support.");
  action2->setStatusTip(action2->toolTip());
#endif

  QAction* ssaction = this->Internal->UI.menu_Tools->addAction("Save State (Debug)")
    << pqSetName("actionToolsSaveState");
//  QAction* lsaction = this->Internal->UI.menu_Tools->addAction("Load State (Debug)")
//    << pqSetName("actionToolsLoadState");
  new pqSaveStateReaction(ssaction);
//  new pqLoadStateReaction(lsaction);

  QObject::connect(this->Internal->UI.actionSettings,
    SIGNAL(triggered()), this->MainWindowCore, SLOT(onEditSettings()));

  QObject::connect(
    this->Internal->UI.actionReset_Camera, SIGNAL(triggered()),
    this->MainWindowCore, SLOT(resetCamera()));
  QObject::connect(
    this->Internal->UI.actionView_Positive_X, SIGNAL(triggered()),
    this->MainWindowCore, SLOT(resetViewDirectionPosX()));
  QObject::connect(
    this->Internal->UI.actionView_Negative_X, SIGNAL(triggered()),
    this->MainWindowCore, SLOT(resetViewDirectionNegX()));
  QObject::connect(
    this->Internal->UI.actionView_Positive_Y, SIGNAL(triggered()),
    this->MainWindowCore, SLOT(resetViewDirectionPosY()));
  QObject::connect(
    this->Internal->UI.actionView_Negative_Y, SIGNAL(triggered()),
    this->MainWindowCore, SLOT(resetViewDirectionNegY()));
  QObject::connect(
    this->Internal->UI.actionView_Positive_Z, SIGNAL(triggered()),
    this->MainWindowCore, SLOT(resetViewDirectionPosZ()));
  QObject::connect(
    this->Internal->UI.actionView_Negative_Z, SIGNAL(triggered()),
    this->MainWindowCore, SLOT(resetViewDirectionNegZ()));
  // Set up Center Axes toolbar.
  QObject::connect(
    this->Internal->UI.actionShowCenterAxes, SIGNAL(toggled(bool)),
    this->MainWindowCore, SLOT(setCenterAxesVisibility(bool)));
  QObject::connect(
    this->Internal->UI.actionResetCenter, SIGNAL(triggered()),
    this->MainWindowCore,
    SLOT(resetCenterOfRotationToCenterOfCurrentData()));
  QObject::connect(
    this->Internal->UI.actionPickCenter, SIGNAL(toggled(bool)),
    this->MainWindowCore,
    SLOT(pickCenterOfRotation(bool)));
  QObject::connect(this->MainWindowCore, SIGNAL(disableAxisChange()),
                   this, SLOT(disableAxisChange()));
  QObject::connect(this->MainWindowCore, SIGNAL(enableAxisChange()),
                   this, SLOT(enableAxisChange()));

  //QObject::connect(
  //  this->MainWindowCore, SIGNAL(enableShowCenterAxis(bool)),
  //  this, SLOT(onShowCenterAxisChanged(bool)), Qt::QueuedConnection);
  QObject::connect(
    this->MainWindowCore, SIGNAL(enableResetCenter(bool)),
    this->Internal->UI.actionResetCenter, SLOT(setEnabled(bool)));
  QObject::connect(
    this->MainWindowCore, SIGNAL(enablePickCenter(bool)),
    this->Internal->UI.actionPickCenter, SLOT(setEnabled(bool)));
  QObject::connect(
    this->MainWindowCore, SIGNAL(pickingCenter(bool)),
    this->Internal->UI.actionPickCenter, SLOT(setChecked(bool)));

  QObject::connect(this->Internal->UI.actionLinkCenterWithFocal,
    SIGNAL(toggled(bool)), this->MainWindowCore, SLOT(linkCenterWithFocalPoint(bool)));

  // setup some statusBar stuff
  // this->MainWindowCore->setupProcessBar(this->statusBar());
  this->onViewChanged();
  this->MainWindowCore->setupCameraManipulationModeBox(
    this->Internal->UI.toolBar_View );

  //setup universal shortcuts for all the suite applications
  //these all must come after we call onViewChanged!

  //setup the selection shortcut. Note it will only fire
  //when the central widget ( 3d render window ) has focus
  this->SelectionShortcut = new QShortcut(QKeySequence(tr("S")),
    this->centralWidget());
  this->SelectionShortcut->setContext(Qt::WidgetShortcut);
  QObject::connect(this->SelectionShortcut, SIGNAL(activated()),
    this,SLOT(onSelectionShortcutActivated()));

  //setup the 3d render window reset camera shortcut.
  //note that it will only fire when the 3d render window has focus
  this->ResetCameraShortcut = new QShortcut(Qt::Key_Space,
    this->centralWidget());
  this->ResetCameraShortcut->setContext(Qt::WidgetShortcut);
  QObject::connect(this->ResetCameraShortcut, SIGNAL(activated()),
    this->MainWindowCore, SLOT(resetCamera()));

  vtkPVGeneralSettings::GetInstance()->SetScalarBarMode(
    vtkPVGeneralSettings::MANUAL_SCALAR_BARS);

  // new plugin io behavior to add more readers from plugin
  this->Internal->LoadDataReaction =
    new pqCMBLoadDataReaction(this->Internal->UI.action_Open_File);
  this->Internal->LoadDataReaction->setPluginIOBehavior(
    new pqPluginIOBehavior(this));
  if(this->MainWindowCore->programDirectory())
    {
    this->Internal->LoadDataReaction->setProgramDirectory(
      this->MainWindowCore->programDirectory());
    }

  // Set up a callback to before further intialization once the application
  // event loop starts.
  QTimer::singleShot(100, this->MainWindowCore, SLOT(applicationInitialize()));
}

//----------------------------------------------------------------------------
Ui::qtCMBMainWindow* pqCMBCommonMainWindow::getMainDialog()
{
  return &this->Internal->UI;
}

//----------------------------------------------------------------------------
QList<pqOutputPort*> &pqCMBCommonMainWindow::getLastSelectionPorts()
{
  return this->Internal->LastSelectionPorts;
}

//----------------------------------------------------------------------------
pqCMBLoadDataReaction* pqCMBCommonMainWindow::loadDataReaction()
{
  return this->Internal->LoadDataReaction;
}

//----------------------------------------------------------------------------
void pqCMBCommonMainWindow::addControlPanel(QWidget* panel)
{
  if(panel)
    {
    // panel->setParent(this->Internal->UI.dockWidgetContents_2);
    this->Internal->UI.dockWidgetContents_2->layout()->addWidget(panel);
    }
}

//----------------------------------------------------------------------------
void pqCMBCommonMainWindow::updateEnableState(bool data_loaded)
{
  this->Internal->UI.action_Save_Data->setEnabled(data_loaded);
  this->Internal->UI.action_Save_As->setEnabled(data_loaded);
  this->Internal->UI.action_Close->setEnabled(data_loaded);

  this->Internal->UI.faceParametersDock->setEnabled(data_loaded);
  this->Internal->UI.toolBar_Selection->setEnabled(data_loaded);
}

//----------------------------------------------------------------------------
// THIS PROB. SHOULD BE REMOVED!
void pqCMBCommonMainWindow::onViewSelected(QList<pqOutputPort*> opports)
{
  this->Internal->LastSelectionPorts.clear();
  for(int i=0; i<opports.count(); i++)
    {
    this->Internal->LastSelectionPorts.append(opports.value(i));
    }
}

//-----------------------------------------------------------------------------
void pqCMBCommonMainWindow::clearGUI()
{
  this->updateEnableState(false);
}

//-----------------------------------------------------------------------------
bool pqCMBCommonMainWindow::compareView(const QString& ReferenceImage,
  double Threshold,
  ostream& Output,
  const QString& TempDirectory)
{
  return this->MainWindowCore->compareView(
    ReferenceImage, Threshold, Output, TempDirectory);
}

//----------------------------------------------------------------------------
void pqCMBCommonMainWindow::setupZoomToBox()
{
  QObject::connect(this->MainWindowCore->renderViewSelectionHelper(),
                   SIGNAL(selectionModeChanged(int)),
                   this, SLOT(onSelectionModeChanged(int)));
  QObject::connect(
                   this->MainWindowCore->renderViewSelectionHelper(),
                   SIGNAL(enableZoom(bool)),
                   this->Internal->UI.actionZoomToBox, SLOT(setEnabled(bool)));
  QObject::connect(
                   this->Internal->UI.actionZoomToBox, SIGNAL(triggered()),
                   this->MainWindowCore->renderViewSelectionHelper(), SLOT(beginZoom()));
}

//----------------------------------------------------------------------------
// Update "Select" action with the status of the rubber band.
void pqCMBCommonMainWindow::onSelectionModeChanged(int mode)
{
  if (mode == pqCMBRubberBandHelper::INTERACT)
    {
    if(this->Internal->UI.action_Select->isChecked())
      {
      this->updateSelection();
      this->Internal->UI.action_Select->setChecked(false);
      }
    }
  else if(mode != pqCMBRubberBandHelper::ZOOM)
    {
    this->Internal->UI.action_Select->setChecked(true);
    }

  this->Internal->UI.actionZoomToBox->setChecked(
    mode == pqCMBRubberBandHelper::ZOOM);

}

//----------------------------------------------------------------------------
void pqCMBCommonMainWindow::onEnableMenuItems(bool state)
{
  // File menu
  this->getMainDialog()->action_Open_File->setEnabled(state);
  this->getMainDialog()->action_Close->setEnabled(state);
  this->getMainDialog()->action_Save_Data->setEnabled(state);
  this->getMainDialog()->action_Save_As->setEnabled(state);
  this->getMainDialog()->menuRecentFiles->setEnabled(state);
}

//----------------------------------------------------------------------------
void pqCMBCommonMainWindow::initProjectManager( )
{
  this->MainWindowCore->initProjectManager();
}

//----------------------------------------------------------------------------
void pqCMBCommonMainWindow::onSelectionShortcutActivated( )
{
  if (this->getMainDialog()->action_Select->isEnabled() )
    {
    //trigger ignores if an action is enabled, so we only fire
    //if the application is allowing selection
    this->getMainDialog()->action_Select->trigger();
    }
}

void pqCMBCommonMainWindow::disableAxisChange()
{
  bool dis = true;
  this->Internal->UI.actionView_Positive_X->setDisabled(dis);
  this->Internal->UI.actionView_Negative_X->setDisabled(dis);
  this->Internal->UI.actionView_Positive_Y->setDisabled(dis);
  this->Internal->UI.actionView_Negative_Y->setDisabled(dis);
  this->Internal->UI.actionView_Positive_Z->setDisabled(dis);
  this->Internal->UI.actionView_Negative_Z->setDisabled(dis);
}

void pqCMBCommonMainWindow::enableAxisChange()
{
  bool dis = false;
  this->Internal->UI.actionView_Positive_X->setDisabled(dis);
  this->Internal->UI.actionView_Negative_X->setDisabled(dis);
  this->Internal->UI.actionView_Positive_Y->setDisabled(dis);
  this->Internal->UI.actionView_Negative_Y->setDisabled(dis);
  this->Internal->UI.actionView_Positive_Z->setDisabled(dis);
  this->Internal->UI.actionView_Negative_Z->setDisabled(dis);
}

//----------------------------------------------------------------------------
void pqCMBCommonMainWindow::showHelpPage(const QString& url)
{
  pqHelpReaction::showHelp(url);
}

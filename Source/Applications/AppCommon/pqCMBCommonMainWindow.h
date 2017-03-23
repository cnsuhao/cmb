//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqCMBCommonMainWindow
// .SECTION Description
// The main window for the application.

#ifndef __pqCMBCommonMainWindow_h
#define __pqCMBCommonMainWindow_h

#include "cmbAppCommonExport.h"
#include <QMainWindow>
#include <QList>
#include <vtkIOStream.h>
#include "cmbSystemConfig.h"

class pqOutputPort;
class pqCMBCommonMainWindowCore;
class QShortcut;
class pqCMBLoadDataReaction;
class qtCMBPanelsManager;
class pqProxyWidget;
class pqCMBColorMapWidget;
class vtkSMProxy;
class QDockWidget;

namespace Ui { class qtCMBMainWindow; }
namespace smtk {namespace io { class Logger; }}

class CMBAPPCOMMON_EXPORT pqCMBCommonMainWindow : public QMainWindow
{
  Q_OBJECT
public:
  pqCMBCommonMainWindow();
  ~pqCMBCommonMainWindow() override;

  virtual bool compareView(const QString& ReferenceImage, double Threshold, ostream& Output, const QString& TempDirectory);
  void addControlPanel(QWidget* panel);
  Ui::qtCMBMainWindow* getMainDialog();

  QList<pqOutputPort*> &getLastSelectionPorts();
  void appendDatasetNameToTitle(const QString& strTitle);
  pqCMBLoadDataReaction* loadDataReaction();

  qtCMBPanelsManager* panelsManager();
  pqProxyWidget* displayPanel(vtkSMProxy* repProxy);
  pqCMBColorMapWidget* colorEditor(QWidget* p);

public slots:
  // Description:
  // Method to handle what to do when the user
  // Presses the "S" key
  virtual void onSelectionShortcutActivated();

  virtual void onViewChanged();
  // Description:
  // Locks the view size for testing.
  virtual void onLockViewSize(bool);

  // Description:
  // Updates the log window
  void updateLog(const smtk::io::Logger&);

protected slots:

  // Description:
  // open About dialog
  virtual void onHelpAbout(){}
  virtual void onHelpHelp(){}
  virtual void showHelpPage(const QString& url);

  void onViewSelected(QList<pqOutputPort*> opports);
  void disableAxisChange();
  void enableAxisChange();

  // Description:
  // Create a Ruler Dialog
  void createRulerDialog();

  //Description:
  // Update the render view when axes grid visibilty is on
  void checkVisibilityAndUpdateRenderView();

  // Description
  // Create a AxesGrid Configuration Dialog
  void createAxesGridConfigurationDialog();

  // Description:
  // Called when starting and external process (to disable starting another)
  // and when completing an external process (to reenable)
  virtual void onEnableExternalProcesses(bool /*state*/){}

  virtual void onEnableMenuItems(bool state);

  // Description:
  // For UI tests recording on Mac, we don't want the menu in the native mode.
  virtual void onRecordTest();
  virtual void onRecordTestStopped();

  virtual void loadMultiFilesStart(){}
  virtual void loadMultiFilesStop(){}

  // Description:
  // Reaction to changing the camera interaction mode to either 2D or 3D
  // If mode is true the camera is in 2D mode - if false it is 3D
  void onCameraInteractionModeChangeTo2D(bool);
  // Description:
  // Reaction to locking and unlocking the camera interaction mode.
  void onEnableCameraInteractionModeChanges(bool);
protected:
  // Description:
  // Updates the enable state of various menus.
  virtual void updateEnableState(bool data_loaded);

  virtual void clearGUI();
  virtual void updateSelection(){}

  // Description
  // Initializes the application.
  virtual void initMainWindowCore();

  void initProjectManager();
  virtual QDockWidget* initPVColorEditorDock();
  virtual void initInspectorDock() ;

  pqCMBCommonMainWindowCore* MainWindowCore;

  QShortcut *SelectionShortcut;
  QShortcut *ResetCameraShortcut;
  bool m_isExiting;

private:
  pqCMBCommonMainWindow(const pqCMBCommonMainWindow&); // Not implemented.
  void operator=(const pqCMBCommonMainWindow&); // Not implemented.


  class vtkInternal;
  vtkInternal* Internal;
};

#endif

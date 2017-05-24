//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef _pqCMBCommonMainWindowCore_h
#define _pqCMBCommonMainWindowCore_h

#include "cmbAppCommonExport.h"
#include "cmbSystemConfig.h"
#include "pqRenderViewBase.h"
#include "pqVariableType.h"
#include "qtCMBProjectServerManager.h"

#include <smtk/SharedPtr.h>
#include <vtkIOStream.h>

#include <QList>
#include <QObject>
#include <QPointer>
#include <QWidget>
#include <memory>

#include <string>

#include <remus/proto/Job.h>
#include <remus/proto/JobResult.h>

class pqCMBDisplayProxyEditor;
class pqCMBPreviewDialog;
class pqReaderFactory;

class pqActionGroupInterface;
class pqDockWindowInterface;
class pqGenericViewModule;
//class pqMultiView;
class pqObjectInspectorDriver;
class pqOutputPort;
class pqPipelineSource;
class pqDataRepresentation;
class pqPlotViewModule;
class pqProxy;
class pqRenderView;
class pqCMBRubberBandHelper;
class pqSelectionManager;
class pqServer;
class pqServerManagerModelItem;
class pqToolsMenu;
class pqUndoStack;
class pqView;
class pqViewContextMenuManager;

class qtArcWidget;
class pqSettings;
class qtCMBApplicationOptions;
class qtCMBApplicationOptionsDialog;
class qtCMBMeshingMonitor;
class pqCMBRenderLog;

class QAction;
class QDockWidget;
class QIcon;
class QImage;
class QMenu;
class QPoint;
class QSize;
class QStatusBar;
class QToolBar;
class QWidget;

namespace smtk
{
namespace extension
{
class qtSelectionManager;
typedef smtk::shared_ptr<smtk::extension::qtSelectionManager> qtSelectionManagerPtr;
}
}

class CMBAPPCOMMON_EXPORT pqCMBCommonMainWindowCore : public QObject
{
  Q_OBJECT

public:
  pqCMBCommonMainWindowCore(QWidget* parent);
  ~pqCMBCommonMainWindowCore() override;

  QWidget* parentWidget() const;

  /// Returns a multi-view widget which can be embedded in the UI
  //pqViewManager& multiViewManager();
  /// Returns the selection manager, which handles interactive selection
  pqSelectionManager* pvSelectionManager();

  /// Returns the qtSelectionManager which handles selection between smtk
  // and CMB
  smtk::extension::qtSelectionManagerPtr smtkSelectionManager() const;

  /// Returns the selection and pick helper used for 3D views.
  pqCMBRubberBandHelper* renderViewSelectionHelper() const;

  /// Setup a variable-selection toolbar
  //virtual void setVariableToolbar(QToolBar* parent);
  /// Setup a representation-selection toolbar
  //virtual void setRepresentationToolbar(QToolBar* parent);

  /// Setup a progress bar, attaching it to the given status bar
  virtual void setupProgressBar(QStatusBar* parent);

  virtual void setupAppearanceEditor(QWidget* aFrame);
  virtual void setAppearanceEditor(pqCMBDisplayProxyEditor* displayEditor);
  pqCMBDisplayProxyEditor* getAppearanceEditor();
  QWidget* getAppearanceEditorContainer();

  /** Compares the contents of the window with the given reference image,
  returns true iff they "match" within some tolerance */
  virtual bool compareView(
    const QString& ReferenceImage, double Threshold, ostream& Output, const QString& TempDirectory);

  /// Call this once all of your slots/signals are connected, to
  /// set the initial state of GUI components
  virtual void initializeStates();

  /// returns the active source.
  pqPipelineSource* getActiveSource();

  /// returns the active server.
  pqServer* getActiveServer() const;

  pqRenderView* activeRenderView() const;

  virtual void removeActiveSource();

  // This will create a source with the given xmlname on the active server.
  // On success returns
  // pqPipelineSource for the source proxy. The actual creation is delegated
  // to pqObjectBuilder instance. Using this method will optionally,
  // create a display for the source in the active render window (if both
  // the active window is indeed on the active server. The created source
  // becomes the active source.
  pqPipelineSource* createSourceOnActiveServer(const QString& xmlname);

  // Returns the view context menu manager. If the manager is not
  // created, a new one will be created and returned.
  pqViewContextMenuManager* getViewContextMenuManager();

  /// Returns the undo stack used for the application.
  pqUndoStack* getApplicationUndoStack() const;

  /// Asks the user to make a new server connection, if none exists.
  bool makeServerConnectionIfNoneExists();

  /// Asks the user for a new connection (even if a server connection
  /// already exists.
  bool makeServerConnection();

  /// Get/Set ProcessExecDirectory
  const QString& getProcessExecDirectory() const;
  void setProcessExecDirectory(QString execPath);

  /// some convenient methods
  pqCMBPreviewDialog* previewDialog();

  // return source of appended inputs; caller resposibility to destroy
  pqPipelineSource* getAppendedSource(QList<pqOutputPort*>& inputs);

  //Returns the enum of this program, so we can get information
  //from the project manager
  qtCMBProjectServerManager::PROGRAM getProgramKey() { return ProgramKey; }

  // Description:
  // Create a arc widget, and also return the orthoplane it is on.
  // Use qtArcWidget for ContourWidget related stuffs
  qtArcWidget* createPqContourWidget(int& orthoplane);
  void deleteContourWidget(qtArcWidget* arcWidget);
  qtArcWidget* createContourWidgetFromSource(
    int orthoplane, double projPos, vtkSMSourceProxy* source);

  /// Get the common cmb application settings option.
  qtCMBApplicationOptions* cmbAppOptions();
  qtCMBApplicationOptionsDialog* appSettingsDialog();

  void setDisplayRepresentation(pqDataRepresentation* rep);
  const char* programDirectory();

  /// Return true if the camera is in 2D mode
  bool isUsing2DCameraInteraction() const;

  /// Return the Render Log
  pqCMBRenderLog* renderLog();

signals:
  void enableVariableToolbar(bool);
  void enableResetCenter(bool);
  void enablePickCenter(bool);
  void enableShowCenterAxis(bool);
  void enableFileSaveScreenshot(bool);
  void pickingCenter(bool);

  /** \todo Hide these private implementation details */
  void postAccept();

  //returns when a job has finished
  void remusCompletedNormally(remus::proto::Job);

  void enableExternalProcesses(bool);

  void disableAxisChange();
  void enableAxisChange();

  // Signal fired if the camera interaction controls
  // were enabled (true) or disabled (false)
  void enableCameraInteractionModeChanged(bool);
  // Signal fired if the camera interaction mode was changed
  // to 2D (true) or 3D (false)
  void cameraInteractionModeChangedTo2D(bool);

public slots:

  /// Called when a new server is connected.
  virtual void onServerCreationFinished(pqServer* server);

  /// Creates a builtin connection, if no connection
  /// currently exists.
  void makeDefaultConnectionIfNoneExists();

  virtual void onFileOpen(const QStringList& /*files*/) {}

  virtual void onToolsManageLinks();
  virtual void onSaveScreenshot();
  virtual void onHelpEnableTooltips(bool enabled = true);

  // Called to show the settings dialog.
  // Subclass should override this method to add app-specific options.
  // This superclass method will add app-common options to the dialog
  // See also, applyAppSettings
  virtual void onEditSettings();

  // Camera slots.
  void resetCamera();
  void getCameraInfo(double focalPt[3], double position[3], double viewDirection[3],
    double& distance, double viewUp[3], double& parallelScale);
  void linkCenterWithFocalPoint(bool);
  void updateFocalPointWithCenter();

  // Description:
  // Set the camera interaction mode to 2D or 3D
  void set2DCameraInteraction();
  void set3DCameraInteraction();
  // Description:
  // Controls the camera mode stack. pushCameraInteraction pushes the current
  // camera mode onto the stack while popCameraInteraction sets the camera mode
  // based on the top of the stack.  The stack is then popped.  If the stack is
  // empty when popCameraInteraction is called then the camera mode is not changed
  // and the method returns false.  void pushCameraInteraction();
  void pushCameraInteraction();
  bool popCameraInteraction();
  // Description:
  // These methods lock and unlock the camera interaction mode UI.
  void enableCameraInteractionModeChanges(bool);

  static void getViewCameraInfo(pqRenderView* view, double focalPt[3], double position[3],
    double viewDirection[3], double& distance, double viewUp[3], double& parallelScale);

  void resetViewDirectionPosX();
  void resetViewDirectionNegX();
  void resetViewDirectionPosY();
  void resetViewDirectionNegY();
  void resetViewDirectionPosZ();
  void resetViewDirectionNegZ();
  void resetViewDirection(
    double look_x, double look_y, double look_z, double up_x, double up_y, double up_z);
  void updateCameraPositionDueToModeChange();

  // This option is used for testing. Sets the maximum size for
  // all render windows. When size.isEmpty() is true,
  // it resets the maximum bounds on the render windows.
  void setMaxRenderWindowSize(const QSize& size);
  void enableTestingRenderWindowSize(bool enable);

  // Resets the center of rotation to the center of the active
  // source in the active view.
  virtual void resetCenterOfRotationToCenterOfCurrentData();

  // Zoom to the selection blocks.
  // return true if selections are valid
  virtual bool zoomToSelection();

  // Next mouse press in 3D window sets the center of rotation to
  // the corresponding world coordinates.
  void pickCenterOfRotation(bool begin);
  void pickCenterOfRotationFinished(double x, double y, double z);
  void setCenterOfRotation(double x, double y, double z);

  // Set center axes visibility on active render view.
  void setCenterAxesVisibility(bool visible);

  // Set show axes grid in render view
  void setShowAxisGrid(bool show);

  // Set the enable state for main window excepting some widgets marked as
  // non-blockable. Non-blockable widgets are registered with the
  // pqProgressManager.
  void setSelectiveEnabledState(bool);

  // Description:
  // Closes the currently opened solid.
  virtual void onRemovingServer(pqServer*) { this->onCloseData(); }
  virtual void onCloseData() {}

  // Description:
  // Saves the data (geometry + region and material IDs) in a file.
  virtual void onSaveData() {}
  virtual void onSaveAsData() {}

  //virtual void onRubberBandSelect(bool) {}

  void requestRender();
  bool checkForPreviewDialog();

  // Setup a area for displaying mouse position
  void setupMousePositionDisplay(QStatusBar* toolbar);

  // Description
  // Responses to events to control the mouse position text
  void enterRenderView();
  void leaveRenderView();
  void updateMousePositionText();

  // Zoom onto the selected object
  virtual void zoomOnSelection() {}

  virtual void onPreviewAccepted() {}
  virtual void onPreviewRejected() {}

  /// Called when the active view in the pqActiveView singleton changes.
  void onActiveViewChanged(pqView* view);

  /// Called when a new reader is created by the GUI.
  /// We add the reader to the recent files menu.
  virtual void onReaderCreated(
    pqPipelineSource* vtkNotUsed(reader), const QString& vtkNotUsed(filename))
  {
  }

  /// This method is called once after the application event loop
  /// begins. This is where we process certain command line options
  /// such as --data, --server etc.
  virtual void applicationInitialize();

  /// Show the camera dialog for the active view module
  void showCameraDialog(pqView*);

  /// Shows message boxes for server timeout warnings.
  void fiveMinuteTimeoutWarning();
  void finalTimeoutWarning();

  /// Called when a new view is created from pqObjectBuilder
  void onViewCreated(pqView*);

  /// launches an instance of the remus server on the machine that
  /// the current paraview server is located
  virtual void launchLocalMeshingService();

  //return the remus servers endpoint information as string
  //the string form will be tcp://ip.address:port
  //Since we are passing this as a string instead of the actual type
  //we lose out on the ability to use inproc connection type.
  //Requires that launchLocalRemusServer has been called beforehand.
  //If no server has been created will return std::string();
  QString meshingServiceEndpoint() const;

  //return the remus monitor so that applications can state what
  //jobs to monitor, and than connect slots to the signals that
  //the monitor emits.
  //Note Returns NULL if we haven't called launchLocalRemusServer
  //or a remote remus server hasn't been set
  QPointer<qtCMBMeshingMonitor> meshServiceMonitor();

  virtual void closeData();
  virtual void saveData(const QString& vtkNotUsed(filename)) {}
  virtual void showStatusMessage(const QString& strMessage);

  bool getExistingFileName(const QString filters, const QString title, QString& selectedFile);

  pqServerManagerModelItem* getActiveObject() const;

  virtual void initProjectManager();

  // Description:
  // Subclass should override this method to load app-specific
  // settings, and this Superclass method will load common settings.
  // See Also, onEditSettings()
  virtual void applyAppSettings();

protected slots:
  virtual void loadProgramFile();

protected:
  virtual void updateViewPositions();
  void updateContourLoop(vtkSMProxy* implicitLoop, qtArcWidget* arcWidget);
  bool getContourNormal(double normal[3], qtArcWidget* arcWidget);
  bool getContourProjectionNormal(int& projNormal, qtArcWidget* arcWidget);
  bool getContourProjectionPosition(double& position, qtArcWidget* arcWidget);
  qtArcWidget* createDefaultArcWidget();
  void setArcPlane(qtArcWidget* arcWidget, int orthoPlane, double projpos);

  virtual void buildDefaultBehaviors(QObject* parent_widget);

  //Subclass this method to alter the default behavior
  virtual void buildRenderWindowContextMenuBehavior(QObject* parent_widget);

  /// Event filter callback.
  bool eventFilter(QObject* caller, QEvent* e) override;

  qtCMBProjectServerManager::PROGRAM ProgramKey;
  smtk::extension::qtSelectionManagerPtr qtSelectionMgr;

private:
  class vtkInternal;
  vtkInternal* const Internal;

  // Helper to initialize Python environment. This doesn't initialize Python
  // but simply sets up the environment so when Python is initialized, it can
  // find ParaView modules. This does nothing is not build with Python support.
  bool InitializePythonEnvironment();
};

#endif // !_pqCMBCommonMainWindowCore_h

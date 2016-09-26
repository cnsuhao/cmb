//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqCMBSceneBuilderMainWindow
// .SECTION Description
// The main window for the application.

#ifndef __pqCMBSceneBuilderMainWindow_h
#define __pqCMBSceneBuilderMainWindow_h

#include "pqCMBCommonMainWindow.h"
#include <QVariant>
#include <QTreeWidgetItem>
#include "cmbSystemConfig.h"

class pqOutputPort;
class vtkDataSet;
class vtkIntArray;
class pqPipelineSource;
class pqCMBSceneTree;
class pqCMBSceneNode;
class pqCMBSceneBuilderMainWindowCore;

class pqCMBSceneBuilderMainWindow : public pqCMBCommonMainWindow
{
  typedef pqCMBCommonMainWindow Superclass;
  Q_OBJECT

public:
  pqCMBSceneBuilderMainWindow();
  ~pqCMBSceneBuilderMainWindow() override;

public slots:

  void onSceneLoaded();
  void onSceneSaved();
  void onSelectGlyph(bool checked);
  void onSelectionModeChanged(int) override;
  void setToolbarsEnabled(bool enable);

protected slots:
  // Description:
  // Updates the enable state of various menus.
  void updateEnableState();

  // Description:
  // open About dialog
  void onHelpAbout() override;
  void onHelpHelp() override;

  // For processing selections via the Scene Tree's nodes
  void onSceneNodeSelected(const QList<pqCMBSceneNode *> *unselected,
                           const QList<pqCMBSceneNode *> *newlySelected);

  // For processing name changes via the Scene Tree's nodes
  void onSceneNodeNameChanged(pqCMBSceneNode *node);

  // Description:
  // Called when starting and external process (to disable starting another)
  // and when completing an external process (to reenable)
  void onEnableExternalProcesses(bool state) override;

  // Description:
  // Called when the Surface Cell Selection is triggered
  void onRubberBandSelect(bool checked);

  // Description:
  // Called to lock and unlock the program from a modal state
  void onEnableMenuItems(bool state) override;

protected:
  // Description

  void clearGUI() override;
  void updateSelection() override;
  void setupMenuActions();
  pqCMBSceneBuilderMainWindowCore* getThisCore();

  // Initializes the application.
  virtual void initializeApplication();

private:
  pqCMBSceneBuilderMainWindow(const pqCMBSceneBuilderMainWindow&); // Not implemented.
  void operator=(const pqCMBSceneBuilderMainWindow&); // Not implemented.

  class vtkInternal;
  vtkInternal* Internal;
  pqCMBSceneTree *Tree;

};

#endif




//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqCMBGeologyBuilderMainWindow
// .SECTION Description
// The main window for the application.

#ifndef __pqCMBGeologyBuilderMainWindow_h
#define __pqCMBGeologyBuilderMainWindow_h

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
class pqCMBGeologyBuilderMainWindowCore;

class pqCMBGeologyBuilderMainWindow : public pqCMBCommonMainWindow
{
  typedef pqCMBCommonMainWindow Superclass;
  Q_OBJECT

public:
  pqCMBGeologyBuilderMainWindow();
  virtual ~pqCMBGeologyBuilderMainWindow();

public slots:

  void onSceneLoaded();
  void onSceneSaved();
  void onSelectGlyph(bool checked);
  virtual void onSelectionModeChanged(int);

protected slots:
  // Description:
  // Updates the enable state of various menus.
  void updateEnableState();

  // Description:
  // open About dialog
  void onHelpAbout();
  void onHelpHelp();

  // For processing selections via the Scene Tree's nodes
  void onSceneNodeSelected(const QList<pqCMBSceneNode *> *unselected,
                           const QList<pqCMBSceneNode *> *newlySelected);

  // For processing name changes via the Scene Tree's nodes
  void onSceneNodeNameChanged(pqCMBSceneNode *node);

  // Description:
  // Called when starting and external process (to disable starting another)
  // and when completing an external process (to reenable)
  void onEnableExternalProcesses(bool state);

  // Description:
  // Called when the Surface Cell Selection is triggered
  void onRubberBandSelect(bool checked);

  // Description:
  // Called to lock and unlock the program from a modal state
  virtual void onEnableMenuItems(bool state);

protected:
  // Description

  virtual void clearGUI();
  virtual void updateSelection();
  void setupMenuActions();
  pqCMBGeologyBuilderMainWindowCore* getThisCore();

  // Initializes the application.
  virtual void initializeApplication();
  virtual void initGeoToolbar();

private:
  pqCMBGeologyBuilderMainWindow(const pqCMBGeologyBuilderMainWindow&); // Not implemented.
  void operator=(const pqCMBGeologyBuilderMainWindow&); // Not implemented.

  class vtkInternal;
  vtkInternal* Internal;
  pqCMBSceneTree *Tree;
};

#endif

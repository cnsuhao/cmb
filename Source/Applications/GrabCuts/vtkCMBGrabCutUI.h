//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef vtkCMBGrabCutUI_h
#define vtkCMBGrabCutUI_h

#include <vtkSmartPointer.h>

#include <QMainWindow>

// Forward Qt class declarations
class Ui_grabCuts;

class vtkCMBGrabCutUI : public QMainWindow
{
  Q_OBJECT
public:
  friend class vtkCMBGrabLeftMousePressCallback;
  friend class vtkCMBGrabMouseMoveCallback;
  friend class vtkCMBGrabLeftMouseReleasedCallback;
  // Constructor/Destructor
  vtkCMBGrabCutUI();
  ~vtkCMBGrabCutUI() override;

public slots:

  virtual void slotExit();
  void open();
  void saveVTP();
  void saveMask();
  void run();
  void clear();
  void pointSize(int i);
  void numberOfIterations(int j);
  void showPossibleLabel(bool b);
  void setTransparency(int t);
  void setDrawMode(int m);
  void setFGFilterSize(int f);
  void setBGFilterSize(int b);

private:

  class Internal;
  Internal *internal;
  // Designer form
  Ui_grabCuts *ui;
};

#endif
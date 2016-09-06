#ifndef vtkCMBGrabCutUI_H
#define vtkCMBGrabCutUI_H

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
  ~vtkCMBGrabCutUI();

public slots:

  virtual void slotExit();
  void open();
  void saveVTP();
  void saveMask();
  void run();
  void pointSize(int i);
  void numberOfIterations(int j);
  void showPossibleLabel(bool b);
  void setTransparency(int t);
  void setDrawMode(int m);

private:

  class Internal;
  Internal *internal;
  // Designer form
  Ui_grabCuts *ui;
};

#endif
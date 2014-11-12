/*=========================================================================

 Program:   Visualization Toolkit
 Module:    $RCSfile: qtSMTKModelPanel.h,v $

 =========================================================================*/
// .NAME qtSMTKModelPanel - Custom object panel for vtkSMTKModelReader
// .SECTION Description
#include <QDockWidget>

#include "smtk/common/UUID.h"

class ModelManager;

namespace smtk {
 namespace attribute {
  class qtFileItem;
 }
}

class qtSMTKModelPanel : public QDockWidget
{
  Q_OBJECT
public:
  qtSMTKModelPanel(ModelManager* mmgr, QWidget* p);
  ~qtSMTKModelPanel();

public slots:
  /// Called if the user accepts pending modifications
  void onDataUpdated();
  void updateTreeSelection();
  void clearUI();

protected slots:
  void selectEntities(const smtk::common::UUIDs& ids);
  void onFileItemCreated(smtk::attribute::qtFileItem* fileItem);
  void onLaunchFileBrowser();

private:
  class qInternal;
  qInternal* Internal;

};

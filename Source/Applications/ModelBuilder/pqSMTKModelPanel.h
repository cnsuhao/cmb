/*=========================================================================

 Program:   Visualization Toolkit
 Module:    $RCSfile: pqSMTKModelPanel.h,v $

 =========================================================================*/
// .NAME pqSMTKModelPanel - Custom object panel for vtkSMTKModelReader
// .SECTION Description
#include <QDockWidget>

#include "smtk/model/EntityRef.h"

class pqCMBModelManager;
class vtkObject;
class pqDataRepresentation;
class vtkSMIntVectorProperty;
class vtkSMDoubleMapProperty;

namespace smtk {
 namespace attribute {
  class qtFileItem;
 }
}

class pqSMTKModelPanel : public QDockWidget
{
  Q_OBJECT
public:
  pqSMTKModelPanel(pqCMBModelManager* mmgr, QWidget* p);
  ~pqSMTKModelPanel();

  pqCMBModelManager* modelManager();

  void setBlockVisibility(pqDataRepresentation* rep,
    const QList<unsigned int>& indices, bool visible);
  void setBlockColor(pqDataRepresentation* rep,
    const QList<unsigned int>& indices, const QColor&);
  void showOnlyBlocks(pqDataRepresentation* rep,
    const QList<unsigned int>& indices);
  void showAllBlocks(pqDataRepresentation* rep);

public slots:
  /// Called if the user accepts pending modifications
  void onDataUpdated();
  void updateTreeSelection();
  void clearUI();
//  void linkRepresentations();

protected slots:
  void selectEntities(const smtk::model::EntityRefs& entities);
  void onFileItemCreated(smtk::attribute::qtFileItem* fileItem);
  void onLaunchFileBrowser();
//  void propertyChanged(
//    vtkObject* caller, unsigned long, void*);
//  void linkRepresentation(pqDataRepresentation *representation);
//  void updateEntityVisibility(vtkSMIntVectorProperty* ivp,
//                              pqDataRepresentation* representation);
//  void updateEntityColor(vtkSMDoubleMapProperty* dmp,
//                         pqDataRepresentation* representation);

private:
  class qInternal;
  qInternal* Internal;

};

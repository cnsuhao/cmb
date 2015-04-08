/*=========================================================================

 Program:   Visualization Toolkit
 Module:    $RCSfile: pqSMTKModelPanel.h,v $

 =========================================================================*/
// .NAME pqSMTKModelPanel - Custom object panel for vtkSMTKModelReader
// .SECTION Description
#include <QDockWidget>

#include "smtk/model/EntityRef.h"
#include "smtk/extension/qt/qtMeshSelectionItem.h" // for qtMeshSelectionItem::MeshListUpdateType

class vtkObject;
class pqCMBModelManager;
class pqDataRepresentation;
class pqOutputPort;
class vtkSMIntVectorProperty;
class vtkSMDoubleMapProperty;
class cmbSMTKModelInfo;

namespace smtk {
  namespace attribute {
    class qtFileItem;
    class qtMeshSelectionItem;
    class qtModelEntityItem;
  }
}
namespace smtk {
  namespace model {
    class qtModelView;
  }
}

class pqSMTKModelPanel : public QDockWidget
{
  Q_OBJECT
public:
  pqSMTKModelPanel(pqCMBModelManager* mmgr, QWidget* p);
  ~pqSMTKModelPanel();

  pqCMBModelManager* modelManager();
  smtk::model::qtModelView* modelView();

  void setBlockVisibility(pqDataRepresentation* rep,
    const QList<unsigned int>& indices, bool visible);
  void setBlockColor(pqDataRepresentation* rep,
    const QList<unsigned int>& indices, const QColor&);
  void showOnlyBlocks(pqDataRepresentation* rep,
    const QList<unsigned int>& indices);
  void showAllBlocks(pqDataRepresentation* rep);

  void addMeshSelectionOperation(
    smtk::attribute::qtMeshSelectionItem* meshItem,
    const std::string& opName, const smtk::common::UUID& uuid);
  void setCurrentMeshSelectionItem(
    smtk::attribute::qtMeshSelectionItem* meshItem);
  void startMeshSelectionOperation(
    const QList<pqOutputPort*> &);

public slots:
  /// Called if the user accepts pending modifications
  void resetUI();
  void clearUI();
  void updateTreeSelection();
  void onEntitiesExpunged(const smtk::model::EntityRefs& expungedEnts);
  void requestEntityAssociation(
    smtk::attribute::qtModelEntityItem* entItem);
  void requestEntitySelection(const smtk::common::UUIDs& uuids);

protected slots:
  void selectEntityRepresentations(const smtk::model::EntityRefs& entities);
  void onFileItemCreated(smtk::attribute::qtFileItem* fileItem);
  void onLaunchFileBrowser();
  void onModelEntityItemCreated(
    smtk::attribute::qtModelEntityItem* entItem);
  void onRequestEntityAssociation();
  void updateMeshSelection(
    const smtk::attribute::MeshSelectionItemPtr&, cmbSMTKModelInfo*);
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

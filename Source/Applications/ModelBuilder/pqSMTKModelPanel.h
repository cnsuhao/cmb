//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqSMTKModelPanel - Custom object panel for vtkSMTKModelReader
// .SECTION Description
#include <QDockWidget>

#include "smtk/model/EntityRef.h"
#include "smtk/extension/qt/qtMeshSelectionItem.h" // for qtMeshSelectionItem::MeshListUpdateType
#include "smtk/PublicPointerDefs.h"
#include "smtk/mesh/MeshSet.h"

class vtkObject;
class pqCMBModelManager;
class pqDataRepresentation;
class pqOutputPort;
class vtkSMIntVectorProperty;
class vtkSMDoubleMapProperty;
class pqSMTKModelInfo;
class pqPipelineSource;
class vtkPVInformation;

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

  void setBlockVisibility(
    const smtk::common::UUID& sessionid,
    const smtk::common::UUIDs& entids,
    const smtk::mesh::MeshSets& meshes, bool visible);
  void setBlockColor(
    const smtk::common::UUID& sessionid,
    const smtk::common::UUIDs& entids,
    const smtk::mesh::MeshSets& meshes, const QColor&);

  void addMeshSelectionOperation(
    smtk::attribute::qtMeshSelectionItem* meshItem,
    const std::string& opName, const smtk::common::UUID& uuid);
  void setCurrentMeshSelectionItem(
    smtk::attribute::qtMeshSelectionItem* meshItem);
  void startMeshSelectionOperation(
    const QList<pqOutputPort*> &);
  void resetMeshSelectionItems();

public slots:
  /// Called if the user accepts pending modifications
  void resetUI();
  void clearUI();
  void updateTreeSelection();
  void onEntitiesExpunged(const smtk::model::EntityRefs& expungedEnts);
  void requestEntityAssociation(
    smtk::attribute::qtModelEntityItem* entItem);
  void requestEntitySelection(const smtk::common::UUIDs& uuids);
  void cancelOperation(const smtk::model::OperatorPtr&);

protected slots:
  void selectEntityRepresentations(const smtk::model::EntityRefs& entities);
  void selectMeshRepresentations(const smtk::mesh::MeshSets& );
  void onFileItemCreated(smtk::attribute::qtFileItem* fileItem);
  void onLaunchFileBrowser();
  void onModelEntityItemCreated(
    smtk::attribute::qtModelEntityItem* entItem);
  void onRequestEntityAssociation();
  void updateMeshSelection(
    const smtk::attribute::MeshSelectionItemPtr&, pqSMTKModelInfo*);
  void gatherSelectionInfo(pqPipelineSource* source,
                           vtkPVInformation* pvInfo,
                           smtk::common::UUIDs& uuids,
                           smtk::mesh::MeshSets& meshes);

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

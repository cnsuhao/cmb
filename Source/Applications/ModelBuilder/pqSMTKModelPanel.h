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

#include "smtk/PublicPointerDefs.h"
#include "smtk/extension/qt/qtMeshSelectionItem.h" // for qtMeshSelectionItem::MeshListUpdateType
#include "smtk/mesh/MeshSet.h"
#include "smtk/model/DescriptivePhrase.h" // for selectPropertyRepresentations
#include "smtk/model/EntityRef.h"

#include <QPointer>
class vtkObject;
class pqCMBModelManager;
class pqDataRepresentation;
class pqOutputPort;
class vtkSMIntVectorProperty;
class vtkSMDoubleMapProperty;
class pqSMTKModelInfo;
class pqPipelineSource;
class vtkPVInformation;

namespace smtk
{
namespace extension
{
class qtFileItem;
class qtMeshSelectionItem;
class qtModelEntityItem;
class qtModelView;
enum class SelectionModifier;
}
}

class pqSMTKModelPanel : public QDockWidget
{
  Q_OBJECT
public:
  pqSMTKModelPanel(pqCMBModelManager* mmgr, QWidget* p);
  ~pqSMTKModelPanel() override;

  pqCMBModelManager* modelManager();
  smtk::extension::qtModelView* modelView();
  /// when entity color is modified, call this function so that selected entities
  /// block would be set to invisible(face, edge and vertex) so that selection
  /// color would be properly rendered.
  /// when starting a new selection, turn on selected entities' block.
  bool changeSelEntitiesBlockVisibility(bool status);

  virtual std::string selectionSourceName() { return this->m_selectionSourceName; }

  void setBlockVisibility(const smtk::common::UUID& sessionid, const smtk::common::UUIDs& entids,
    const smtk::mesh::MeshSets& meshes, bool visible);
  void setBlockColor(const smtk::common::UUID& sessionid, const smtk::common::UUIDs& entids,
    const smtk::mesh::MeshSets& meshes, const QColor&);

  void addMeshSelectionOperation(smtk::extension::qtMeshSelectionItem* meshItem,
    const std::string& opName, const smtk::common::UUID& uuid);
  void setCurrentMeshSelectionItem(smtk::extension::qtMeshSelectionItem* meshItem);
  void startMeshSelectionOperation(const QList<pqOutputPort*>&);
  void resetMeshSelectionItems();

signals:
  void sendSelectionsFromRenderWindowToSelectionManager(const smtk::model::EntityRefs& selEntities,
    const smtk::mesh::MeshSets& selMeshes, const smtk::model::DescriptivePhrases& DesPhrases,
    const smtk::extension::SelectionModifier modifierFlag,
    const std::string& incomingSelectionSource);

public slots:
  /// Called if the user accepts pending modifications
  void resetUI();
  void clearUI();
  void updateTreeSelection();
  void onEntitiesExpunged(const smtk::model::EntityRefs& expungedEnts);
  void requestEntityAssociation(smtk::extension::qtModelEntityItem* entItem);
  void requestEntitySelection(const smtk::common::UUIDs& uuids);
  void cancelOperation(const smtk::model::OperatorPtr&);
  bool removeClosedSession(const smtk::model::SessionRef& sref);
  /// meshPanel also uses this function to update render view
  void onSelectionChangedUpdateRenderView(const smtk::model::EntityRefs& selEntites,
    const smtk::mesh::MeshSets& selMeshes, const smtk::model::DescriptivePhrases& DesPhrases,
    const std::string& incomingSelectionSource);

protected slots:
  void selectEntityRepresentations(const smtk::model::EntityRefs& entities);
  void selectMeshRepresentations(const smtk::mesh::MeshSets&);
  void onFileItemCreated(smtk::extension::qtFileItem* fileItem);
  bool onLaunchFileBrowser();
  void onModelEntityItemCreated(smtk::extension::qtModelEntityItem* entItem);
  void onRequestEntityAssociation();
  void updateMeshSelection(const smtk::attribute::MeshSelectionItemPtr&, pqSMTKModelInfo*);
  void gatherSelectionInfo(pqPipelineSource* source, vtkPVInformation* pvInfo,
    smtk::common::UUIDs& uuids, smtk::mesh::MeshSets& meshes);

private:
  class qInternal;
  qInternal* Internal;
  std::string m_selectionSourceName;
};

#include "SplitFaceOperator.h"

#include "Bridge.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/DoubleItem.h"
#include "smtk/attribute/ModelEntityItem.h"

#include "smtk/model/Operator.h"
#include "smtk/model/ModelEntity.h"
#include "smtk/model/Manager.h"

#include "vtkDiscreteModelWrapper.h"
#include "vtkModel.h"
#include "vtkModelFace.h"
#include "vtkModelItem.h"
#include "vtkModelEntity.h"

#include "SplitFaceOperator_xml.h"

using namespace smtk::model;

namespace cmbsmtk {
  namespace cmb {

SplitFaceOperator::SplitFaceOperator()
{
}

bool SplitFaceOperator::ableToOperate()
{
  smtk::model::ModelEntity model;
  return
    this->ensureSpecification() &&
    // The SMTK model must be valid
    (model = this->specification()->findModelEntity("model")->value().as<ModelEntity>()).isValid() &&
    // The CMB model must exist:
    this->cmbBridge()->findModel(model.entity()) &&
    // The CMB face to split must be valid
    this->fetchCMBFaceId() >= 0
    ;
}

OperatorResult SplitFaceOperator::operateInternal()
{
  Bridge* bridge = this->cmbBridge();

  // Translate SMTK inputs into CMB inputs
  this->m_op->SetFeatureAngle(
    this->specification()->findDouble("feature angle")->value());

  this->m_op->SetId(this->fetchCMBFaceId()); // "face to split"

  vtkDiscreteModelWrapper* modelWrapper =
    bridge->findModel(
      this->specification()->findModelEntity("model")->value().entity());

  this->m_op->Operate(modelWrapper);
  bool ok = this->m_op->GetOperateSucceeded();
  OperatorResult result =
    this->createResult(
      ok ?  OPERATION_SUCCEEDED : OPERATION_FAILED);

  if (ok)
    {
    // TODO: Read list of new Faces created by split and
    //       use the bridge to translate them and store
    //       them in the OperatorResult (well, a subclass).
    smtk::model::ManagerPtr store = this->manager();

    // First, get rid of the old face that we split.
    smtk::util::UUID faceUUID =
      this->specification()->findModelEntity("face to split")->value().entity();
    store->erase(faceUUID);

    // Now re-add it (it will have new edges)
    vtkModelItem* item = bridge->entityForUUID(faceUUID);
    Cursor c = bridge->addCMBEntityToManager(faceUUID, item, store, true);

    vtkIdTypeArray* newFaceIds = this->m_op->GetCreatedModelFaceIDs();
    for (vtkIdType i = 0; i <= newFaceIds->GetMaxId(); ++i)
      {
      vtkIdType faceId = newFaceIds->GetValue(i);
      vtkModelFace* face = dynamic_cast<vtkModelFace*>(
        modelWrapper->GetModelEntity(vtkModelFaceType, faceId));
      faceUUID = bridge->findOrSetEntityUUID(face);
      Cursor c = bridge->addCMBEntityToManager(faceUUID, face, store, true);
      /*
      this->m_op->SetCurrentNewFaceId(faceId);
      vtkIdTypeArray* spvrt = this->m_op->GetSplitEdgeVertIds();
      vtkIdTypeArray* nwvrt = this->m_op->GetCreatedModelEdgeVertIDs();
      vtkIdTypeArray* loops = this->m_op->GetFaceEdgeLoopIDs();
      */
      }
    }

  return result;
}

Bridge* SplitFaceOperator::cmbBridge() const
{
  return dynamic_cast<Bridge*>(this->bridge());
}

int SplitFaceOperator::fetchCMBFaceId() const
{
  vtkModelItem* item =
    this->cmbBridge()->entityForUUID(
      this->specification()->findModelEntity(
        "face to split")->value().entity());
  vtkModelEntity* face = dynamic_cast<vtkModelEntity*>(item);
  if (face)
    return face->GetUniquePersistentId();

  return -1;
}

  } // namespace cmb
} // namespace cmbsmtk

smtkImplementsModelOperator(
  cmbsmtk::cmb::SplitFaceOperator,
  cmb_split_face,
  "split face",
  SplitFaceOperator_xml,
  cmbsmtk::cmb::Bridge);

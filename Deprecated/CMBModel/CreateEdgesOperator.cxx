#include "CreateEdgesOperator.h"

#include "Bridge.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/ModelEntityItem.h"

#include "smtk/model/ModelEntity.h"
#include "smtk/model/Operator.h"

#include "vtkModelItem.h"
#include "vtkModelEntity.h"

#include "CreateEdgesOperator_xml.h"

using namespace smtk::model;

namespace cmbsmtk {
  namespace cmb {

CreateEdgesOperator::CreateEdgesOperator()
{
}

bool CreateEdgesOperator::ableToOperate()
{
  smtk::model::ModelEntity model;
  return
    this->ensureSpecification() &&
    // The SMTK model must be valid
    (model = this->specification()->findModelEntity("model")->value().as<ModelEntity>()).isValid() &&
    // The CMB model must exist:
    this->cmbBridge()->findModel(model.entity())
    ;
}

OperatorResult CreateEdgesOperator::operateInternal()
{
  Bridge* bridge = this->cmbBridge();

  vtkDiscreteModelWrapper* modelWrapper =
    bridge->findModel(
      this->specification()->findModelEntity("model")->value().entity());
  if (!modelWrapper)
    {
    return this->createResult(OPERATION_FAILED);
    }

  this->m_op->Operate(modelWrapper);
  bool ok = this->m_op->GetOperateSucceeded();
  OperatorResult result =
    this->createResult(
      ok ?  OPERATION_SUCCEEDED : OPERATION_FAILED);

  // TODO: Read list of new Edges created and
  //       use the bridge to translate them and store
  //       them in the OperatorResult (well, a subclass).

  return result;
}

Bridge* CreateEdgesOperator::cmbBridge() const
{
  return dynamic_cast<Bridge*>(this->bridge());
}

  } // namespace cmb
} // namespace cmbsmtk

smtkImplementsModelOperator(
  cmbsmtk::cmb::CreateEdgesOperator,
  cmb_create_edges,
  "create edges",
  CreateEdgesOperator_xml,
  cmbsmtk::cmb::Bridge);

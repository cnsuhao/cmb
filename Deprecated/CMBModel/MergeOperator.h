#ifndef __cmbsmtk_cmb_MergeOperator_h
#define __cmbsmtk_cmb_MergeOperator_h

#include "smtk/model/Operator.h"
#include "vtkMergeOperator.h"
#include "vtkNew.h"

namespace cmbsmtk {
  namespace cmb {

class Bridge;

/**\brief Merge adjacent cells into a single cell.
  *
  * The source and target cells are merged, with the result being stored in target.
  * All boundaries of the source are either modified (so that they now refer to
  * the target) or are deleted (because they served as a boundary between the source
  * and target).
  *
  * The source and target must be adjacent and have the same parametric dimension.
  */
class VTKCMBDISCRETEMODEL_EXPORT MergeOperator : public smtk::model::Operator
{
public:
  smtkTypeMacro(MergeOperator);
  smtkCreateMacro(MergeOperator);
  smtkSharedFromThisMacro(Operator);
  smtkDeclareModelOperator();

  virtual bool ableToOperate();

protected:
  MergeOperator();
  virtual smtk::model::OperatorResult operateInternal();
  Bridge* cmbBridge() const;
  int fetchCMBCellId(const std::string& parameterName) const;

  vtkNew<vtkMergeOperator> m_op;
};

  } // namespace cmb
} // namespace cmbsmtk

#endif // __cmbsmtk_cmb_MergeOperator_h

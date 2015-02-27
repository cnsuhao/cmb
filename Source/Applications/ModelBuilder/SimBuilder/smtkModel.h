/*=========================================================================

Copyright (c) 1998-2012 Kitware Inc. 28 Corporate Drive,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced, distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
=========================================================================*/
// .NAME smtkModel.h -
// .SECTION Description
// .SECTION See Also

#ifndef __smtkModel_h
#define __smtkModel_h

#include "smtk/model/Model.h"
#include "smtk/model/Item.h"
#include "cmbSystemConfig.h"

class smtkModelInternals;
class vtkDiscreteModel;
class vtkModelEntity;
class vtkSMProxy;

class smtkModel : public smtk::model::Model
{
  public:
    smtkModel();
    virtual ~smtkModel();

    vtkDiscreteModel* discreteModel();
    void setDiscreteModel(vtkDiscreteModel* discreteModel);
    vtkSMProxy* modelWrapper();
    void setModelWrapper(vtkSMProxy* modelwrapper);

    // Description:
    // Given a vtkDiscreteModel group type (specified in vtkDiscreteModel.h)
    // and a model entity dimension, return the SMTK mask
    // for SMTK groups. WARNING: This assumes we are NOT dealing with
    // mixed model types that have 2D and 3D domains.
    virtual smtk::model::MaskType convertGroupTypeToMask(
      int grouptype, int modelDimension);
    virtual void loadGroupItems(int grouptype);

    virtual void updateGroupItems(int groupType);


  virtual smtk::model::ItemPtr createModelItem(vtkModelEntity* entity);
  virtual smtk::model::ItemPtr createModelItem(
      const std::string &name, int id,
      smtk::model::Item::Type itemType);

  virtual smtk::model::GroupItemPtr createModelGroupFromCMBGroup(int groupId);
  virtual bool deleteModelGroup(int id);

    static bool convertEntityType(int modelType,
      smtk::model::Item::Type& enType);

    void updateModelItemName(int entId, std::string& entName);
    void removeModelItem(int entId);

  protected:

  private:
    smtkModelInternals *Internals;

};

#endif /* __smtkModel_h */

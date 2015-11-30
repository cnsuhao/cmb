//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME pqSMTKUIHelper -
// .SECTION Description - Helper for smtk::attribute::qtFileItem
//

#ifndef __pqSMTKUIHelper_h
#define __pqSMTKUIHelper_h

#include "cmbSystemConfig.h"

#include "smtk/extension/qt/qtFileItem.h"
#include "smtk/extension/qt/qtModelEntityItem.h"
#include "smtk/extension/qt/qtModelView.h"
#include "smtk/attribute/DirectoryItem.h"
#include "smtk/attribute/DirectoryItemDefinition.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/FileItemDefinition.h"
#include "smtk/attribute/ModelEntityItem.h"
#include "smtk/attribute/ModelEntityItemDefinition.h"

#include "pqServer.h"
#include "pqFileDialog.h"

#include <QString>
#include <QWidget>
#include <QLineEdit>
#include <QObject>

//BTX
namespace pqSMTKUIHelper
{
  //----------------------------------------------------------------------------
  static void process_smtkFileItemRequest(
    smtk::attribute::qtFileItem* fileItem,
    pqServer* server, QWidget* parent_Widget = NULL)
  {
    if(!fileItem)
      {
      return;
      }

    QString filters;
    bool existingFile=true;
    if (!fileItem->isDirectory())
      {
      smtk::attribute::FileItemPtr fItem =
        smtk::dynamic_pointer_cast<smtk::attribute::FileItem>(fileItem->getObject());
      const smtk::attribute::FileItemDefinition *fItemDef =
        dynamic_cast<const smtk::attribute::FileItemDefinition*>(fItem->definition().get());
      filters = fItemDef->getFileFilters().c_str();
      existingFile = fItemDef->shouldExist();
      }
    else
      {
      smtk::attribute::DirectoryItemPtr dItem =
        smtk::dynamic_pointer_cast<smtk::attribute::DirectoryItem>(fileItem->getObject());
      const smtk::attribute::DirectoryItemDefinition *dItemDef =
        dynamic_cast<const smtk::attribute::DirectoryItemDefinition*>(dItem->definition().get());
      existingFile = dItemDef->shouldExist();
      }

    QString title = fileItem->isDirectory() ? QObject::tr("Select Directory:") :
      QObject::tr("Select File:");
    pqFileDialog file_dialog(server,
       parent_Widget,
       title,
       QString(),
       filters);

    if (fileItem->isDirectory())
      {
      file_dialog.setFileMode(pqFileDialog::Directory);
      }
    else if (existingFile)
      {
      file_dialog.setFileMode(pqFileDialog::ExistingFile);
      }
    else
      {
      file_dialog.setFileMode(pqFileDialog::AnyFile);
      }
    file_dialog.setObjectName("SimBuilder Select File Dialog");
    file_dialog.setWindowModality(Qt::WindowModal);
    if (file_dialog.exec() == QDialog::Accepted)
      {
      QStringList files = file_dialog.getSelectedFiles();
      fileItem->setInputValue(files[0]);
      }

  }

  //----------------------------------------------------------------------------
  static void process_smtkModelEntityItemSelectionRequest(
    smtk::attribute::qtModelEntityItem* entityItem,
    smtk::model::qtModelView* modelView)
  {
    if(!entityItem || !modelView)
      {
      return;
      }

    smtk::attribute::ModelEntityItemPtr eItem =
      smtk::dynamic_pointer_cast<smtk::attribute::ModelEntityItem>(
      entityItem->getObject());
    const smtk::attribute::ModelEntityItemDefinition *eItemDef =
      dynamic_cast<const smtk::attribute::ModelEntityItemDefinition*>(
      eItem->definition().get());

    smtk::model::EntityRefs selentityrefs;
    // search current selection
    modelView->currentSelectionByMask(selentityrefs, eItemDef->membershipMask());
    // if current selection does not match the item mask, search the parents.
    if(selentityrefs.size() == 0)
      modelView->currentSelectionByMask(selentityrefs,
        eItemDef->membershipMask(), true);
    if(selentityrefs.size() > 0)
      entityItem->associateEntities(selentityrefs);
  }

}


#include "pqPipelineSource.h"
#include "vtkNew.h"
#include "vtkSMSourceProxy.h"
#include "vtkPVSelectionInformation.h"
#include "vtkSelection.h"
#include "vtkSelectionNode.h"
#include "vtkUnsignedIntArray.h"
#include "vtkSMPropertyHelper.h"
#include <QList>

namespace pqCMBSelectionHelperUtil
{

static vtkSelectionNode* gatherSelectionNode(
    pqPipelineSource* source,
    vtkPVSelectionInformation* selInfo)
  {
    vtkSMSourceProxy* smSource = vtkSMSourceProxy::SafeDownCast(source->getProxy());
    vtkSMSourceProxy* selSource = smSource->GetSelectionInput(0);
    selSource->UpdatePipeline();

    selSource->GatherInformation(selInfo);
    if(selInfo->GetSelection() &&
      selInfo->GetSelection()->GetNumberOfNodes())
      {
      return selInfo->GetSelection()->GetNode(0);
      }
    return NULL;
  }

static int fillSelectionIdList(
  QList<unsigned int> & result,
  vtkUnsignedIntArray* blockIds, vtkSMPropertyHelper* selIDs)
{
  int total = 0;
  if(blockIds)
    {
    for(vtkIdType ui=0;ui<blockIds->GetNumberOfTuples();ui++)
      {
      unsigned int block_id = blockIds->GetValue(ui);
      result.push_back(block_id);
      }
    total += blockIds->GetNumberOfTuples();
    }

  if(selIDs)
    {
    unsigned int count = selIDs->GetNumberOfElements();
    // [composite_index, process_id, index]
    for (unsigned int cc=0; cc < (count/3); cc++)
      {
      unsigned int block_id = selIDs->GetAsInt(3*cc);
      result.push_back(block_id);
      }
    total += count/3;
    }
  return total;
}


}

//ETX
#endif

// .NAME cmbSMTKUIHelper -
// .SECTION Description - Helper for smtk::attribute::qtFileItem
//

#ifndef __cmbSMTKUIHelper_h
#define __cmbSMTKUIHelper_h

#include "cmbSystemConfig.h"

#include "smtk/extension/qt/qtFileItem.h"
#include "smtk/attribute/DirectoryItem.h"
#include "smtk/attribute/DirectoryItemDefinition.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/FileItemDefinition.h"

#include "pqServer.h"
#include "pqFileDialog.h"

#include <QString>
#include <QWidget>
#include <QLineEdit>
#include <QObject>

//BTX
namespace cmbSMTKUIHelper
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
    QLineEdit* lineEdit =  static_cast<QLineEdit*>(
      fileItem->property("DataItem").value<void *>());
    if(!lineEdit)
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
      lineEdit->setText(files[0]);
      fileItem->onInputValueChanged();
      }

  }
}
//ETX
#endif

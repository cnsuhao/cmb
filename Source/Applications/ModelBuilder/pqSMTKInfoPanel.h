//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef _pqSMTKInfoPanel_h
#define _pqSMTKInfoPanel_h

#include <QPointer>
#include <QWidget>

#include "smtk/PublicPointerDefs.h"
#include "smtk/extension/qt/qtMeshSelectionItem.h"
#include "smtk/mesh/MeshSet.h"
#include "smtk/model/DescriptivePhrase.h"
#include "smtk/model/EntityRef.h"

class pqCMBModelManager;
class pqOutputPort;
class QTreeWidgetItem;
class vtkEventQtSlotConnect;
class vtkPVDataInformation;

namespace smtk
{
namespace extension
{
class qtSelectionManager;
}
}

/**
* Widget which provides information about an output port of a source proxy
*/
class pqSMTKInfoPanel : public QWidget
{
  Q_OBJECT
public:
  /**
  * constructor
  */
  pqSMTKInfoPanel(QPointer<pqCMBModelManager> mmgr, QWidget* p = 0);
  /**
  * destructor
  */
  ~pqSMTKInfoPanel() override;

  /**
  * get the proxy for which properties are displayed
  */
  pqOutputPort* getOutputPort();

  QPointer<pqCMBModelManager> modelManager();

  void updateModel(QPointer<pqCMBModelManager> mmgr);

public slots:
  /**
  * TODO: have this become automatic instead of relying on
  * the accept button in case another client modifies the pipeline.
  */
  void updateInformation();

  /**
  * Set the display whose properties we want to edit.
  */
  void setOutputPort(pqOutputPort* outputport);

protected slots:
  void onCurrentItemChanged(QTreeWidgetItem* item);
  void onSelectionChangedUpdateInfoPanel(const smtk::model::EntityRefs&,
    const smtk::mesh::MeshSets&, const smtk::model::DescriptivePhrases&);

private:
  /**
  * builds the composite tree structure.
  */
  QTreeWidgetItem* fillCompositeInformation(
    vtkPVDataInformation* info, QTreeWidgetItem* parent = 0);

  void fillDataInformation(vtkPVDataInformation* info);

private:
  QPointer<pqCMBModelManager> ModelManager;
  QPointer<pqOutputPort> OutputPort;
  vtkEventQtSlotConnect* VTKConnect;
  class pqUi;
  pqUi* Ui;
};

#endif

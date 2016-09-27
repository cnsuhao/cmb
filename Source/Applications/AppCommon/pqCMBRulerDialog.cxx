//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "pqCMBRulerDialog.h"

#include "pqActiveObjects.h"
#include "pqProxyWidget.h"
#include "pqObjectBuilder.h"
#include "pqRenderView.h"
#include "pqPVApplicationCore.h"
#include "vtkSMProxyManager.h"
#include "vtkSMProperty.h"
#include "vtkSMSourceProxy.h"

#include <QDialog>
#include <QVBoxLayout>

//-----------------------------------------------------------------------------
/// constructor
pqCMBRulerDialog::pqCMBRulerDialog(QWidget* p)
  : QDialog(p)
{
  // Create the render dialog
  this->setObjectName("pqCMBRulerDialog");
  this->setWindowTitle("Ruler (switch to main window to apply change)");
  // Get the rendering view
  pqRenderView* view = qobject_cast<pqRenderView*>(pqActiveObjects::instance().activeView());
  // Connect the rendering window with pipeline source
  pqObjectBuilder* const builder = pqPVApplicationCore::instance()->getObjectBuilder();
  this->RulerSource = builder->createSource("sources","Ruler", view->getServer());

  // Create widget for this source
  this->RulerWidget = new pqProxyWidget(this->RulerSource->getSourceProxy());
  bool applyChanges = true;
  this->RulerWidget->setApplyChangesImmediately(applyChanges);
  this->RulerWidget->setView(view);
  this->RulerWidget->filterWidgets();
  QVBoxLayout* RulerLayout = new QVBoxLayout(this);
  RulerLayout->addWidget(this->RulerWidget);
  this->RulerWidget->updatePanel();


}

//-----------------------------------------------------------------------------
/// destructor
pqCMBRulerDialog::~pqCMBRulerDialog()
{
  pqObjectBuilder* const builder = pqPVApplicationCore::instance()->getObjectBuilder();
  builder->destroy(this->RulerSource);
  delete this->RulerWidget;
}

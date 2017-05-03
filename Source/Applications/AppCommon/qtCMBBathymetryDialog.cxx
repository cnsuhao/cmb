//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME Represents a dialog for texturing objects into SceneGen.
// .SECTION Description
// .SECTION Caveats

#include "qtCMBBathymetryDialog.h"

#include "./Scene/Core/pqCMBSceneNodeIterator.h"
#include "pqCMBSceneNode.h"
#include "pqCMBSceneTree.h"
#include "pqCMBTexturedObject.h"
#include "ui_qtCMBBathymetryDialog.h"
//#include <pqFileDialog.h>
//#include <QFileInfo>
#include "pqPipelineSource.h"
#include "qtCMBSceneObjectImporter.h"
#include <QDoubleValidator>
#include <QLineEdit>
#include <QStackedWidget>
#include <QTreeWidget>

int qtCMBBathymetryDialog::manageBathymetry(pqCMBSceneNode* node)
{
  if (!node || node->isTypeNode())
  {
    return 0;
  }

  if (dynamic_cast<pqCMBTexturedObject*>(node->getDataObject()) == NULL)
  {
    return 0;
  }
  qtCMBBathymetryDialog importer(node);
  return importer.exec();
}

qtCMBBathymetryDialog::qtCMBBathymetryDialog(pqCMBSceneTree* sceneTree)
  : Status(-1)
  , Node(0)
  , SceneTree(sceneTree)
{
  this->initUI();
}

qtCMBBathymetryDialog::qtCMBBathymetryDialog(pqCMBSceneNode* n)
  : Status(-1)
  , Node(n)
{
  if (!n || !n->getDataObject())
  {
    return;
  }
  pqCMBTexturedObject* object = dynamic_cast<pqCMBTexturedObject*>(n->getDataObject());
  if (!object)
  {
    return;
  }
  this->SceneTree = this->Node->getTree();
  this->initUI();
}

qtCMBBathymetryDialog::~qtCMBBathymetryDialog()
{
  if (this->BathymetryDialog)
  {
    delete BathymetryDialog;
  }
  if (this->MainDialog)
  {
    delete MainDialog;
  }
}

int qtCMBBathymetryDialog::exec()
{
  this->MainDialog->setModal(true);
  this->MainDialog->show();
  this->MainDialog->exec();
  return this->Status;
}

void qtCMBBathymetryDialog::accept()
{
  if (!this->bathymetrySourceObject())
  {
    return;
  }

  if (this->Node)
  {
    pqCMBTexturedObject* object = dynamic_cast<pqCMBTexturedObject*>(this->Node->getDataObject());
    object->applyBathymetry(this->bathymetrySourceObject(),
      this->BathymetryDialog->ElevationRadius->text().toDouble(), this->useHighElevationLimit(),
      this->highElevationLimit(), this->useLowElevationLimit(), this->lowElevationLimit());
  }
  else
  {
    // Whoever using this dialog will get bathymetrySourceObject() and elevationRadius.
  }
  this->MainDialog->close();
  this->Status = 1;
}

void qtCMBBathymetryDialog::cancel()
{
  this->Status = 0;
}

void qtCMBBathymetryDialog::displaySourceImporter()
{
  bool randomPlacement, translateBasedOnView, useTextureConstraint, useGlyphs;
  int count, glyphPlaybackOption;
  QMap<pqCMBSceneNode*, int> constraints;
  QString glyphPlaybackFilename;
  bool enableRandomPlacement = false, enableTextureConstraintPlacement = false;
  bool useGlyphPlayback = false;
  pqCMBSceneNode* node = qtCMBSceneObjectImporter::importNode(this->SceneTree->getRoot(),
    enableRandomPlacement, enableTextureConstraintPlacement, &randomPlacement,
    &translateBasedOnView, &count, constraints, useGlyphs, useTextureConstraint, useGlyphPlayback,
    glyphPlaybackOption, glyphPlaybackFilename);
  if (!node || (node->isTypeNode() && node->getChildren().size() == 0))
  {
    return;
  }

  pqCMBSceneObjectBase* sObj =
    node->isTypeNode() ? node->getChildren()[0]->getDataObject() : node->getDataObject();
  this->BathymetryDialog->BathymetrySources->addItem(node->getName());
  QVariant vdata;
  vdata.setValue(static_cast<void*>(sObj));
  int idx = this->BathymetryDialog->BathymetrySources->count() - 1;
  this->BathymetryDialog->BathymetrySources->setCurrentIndex(idx);
  this->BathymetryDialog->BathymetrySources->setItemData(idx, vdata);
}

pqCMBSceneObjectBase* qtCMBBathymetryDialog::bathymetrySourceObject() const
{
  int n = this->BathymetryDialog->BathymetrySources->currentIndex();
  pqCMBSceneObjectBase* sObj = static_cast<pqCMBSceneObjectBase*>(
    this->BathymetryDialog->BathymetrySources->itemData(n).value<void*>());
  return sObj;
}

double qtCMBBathymetryDialog::elevationRadius()
{
  return this->BathymetryDialog->ElevationRadius->text().toDouble();
}

double qtCMBBathymetryDialog::highElevationLimit()
{
  return this->BathymetryDialog->highElevation->text().toDouble();
}

double qtCMBBathymetryDialog::lowElevationLimit()
{
  return this->BathymetryDialog->lowElevation->text().toDouble();
}

bool qtCMBBathymetryDialog::useHighElevationLimit()
{
  return this->BathymetryDialog->checkBoxHighLimit->isChecked();
}

bool qtCMBBathymetryDialog::useLowElevationLimit()
{
  return this->BathymetryDialog->checkBoxLowLimit->isChecked();
}

bool qtCMBBathymetryDialog::applyOnlyToVisibleMeshes()
{
  return this->BathymetryDialog->OnlyApplyToVisibleMesh->isChecked();
}

void qtCMBBathymetryDialog::removeBathymetry()
{
  if (this->Node)
  {
    pqCMBTexturedObject* object = dynamic_cast<pqCMBTexturedObject*>(this->Node->getDataObject());
    object->unApplyBathymetry();
    this->Node->getTree()->sceneObjectChanged();
  }
  else
  {
  }
  this->MainDialog->close();
  this->Status = 2;
}

void qtCMBBathymetryDialog::initUI()
{
  if (!this->SceneTree)
  {
    return;
  }
  if (!this->SceneTree->getRoot())
  {
    this->SceneTree->createRoot("Scene");
  }

  this->ModelAndMeshMode = false;

  this->MainDialog = new QDialog(this->SceneTree->getWidget());
  QDoubleValidator* validator = new QDoubleValidator(this->MainDialog);
  this->BathymetryDialog = new Ui::qtCMBBathymetryDialog;
  this->BathymetryDialog->setupUi(MainDialog);

  // Prep the line edit widgets
  //validator->setRange(0.001, VTK_DOUBLE_MAX);
  validator->setBottom(0.0);
  this->BathymetryDialog->ElevationRadius->setValidator(validator);
  this->BathymetryDialog->ElevationRadius->setText("1.0");

  pqCMBTexturedObject* tObject =
    this->Node ? dynamic_cast<pqCMBTexturedObject*>(this->Node->getDataObject()) : NULL;

  pqCMBSceneNode* node;
  SceneObjectNodeIterator iter(this->SceneTree->getRoot());
  iter.addObjectTypeFilter(pqCMBSceneObjectBase::Points);
  iter.addObjectTypeFilter(pqCMBSceneObjectBase::UniformGrid);
  iter.addObjectTypeFilter(pqCMBSceneObjectBase::GroundPlane);
  iter.addObjectTypeFilter(pqCMBSceneObjectBase::Faceted);

  int idx = 0, currentIdx = -1;
  while ((node = iter.next()))
  {
    this->BathymetryDialog->BathymetrySources->addItem(node->getName());
    QVariant vdata;
    vdata.setValue(static_cast<void*>(node->getDataObject()));
    this->BathymetryDialog->BathymetrySources->setItemData(idx, vdata);
    if (tObject && currentIdx == -1 && tObject->getBathymetrySource() &&
      tObject->getBathymetrySource() == node->getDataObject())
    {
      currentIdx = idx;
    }
    idx++;
  }

  if (currentIdx >= 0)
  {
    this->BathymetryDialog->BathymetrySources->setCurrentIndex(currentIdx);
  }
  QDoubleValidator* eleValidator = new QDoubleValidator(this->MainDialog);
  this->BathymetryDialog->lowElevation->setValidator(eleValidator);
  this->BathymetryDialog->lowElevation->setText("0.0");
  this->BathymetryDialog->highElevation->setValidator(eleValidator);
  this->BathymetryDialog->highElevation->setText("0.0");

  bool enable = (!tObject || (tObject && tObject->getBathymetrySource())) ? true : false;
  this->BathymetryDialog->RemoveBathymetryButton->setEnabled(enable);

  this->setMeshAndModelMode(this->ModelAndMeshMode);
  this->initConnections();
}

void qtCMBBathymetryDialog::initConnections()
{
  QObject::connect(this->BathymetryDialog->RemoveBathymetryButton, SIGNAL(clicked()), this,
    SLOT(removeBathymetry()));
  QObject::connect(this->MainDialog, SIGNAL(accepted()), this, SLOT(accept()));
  QObject::connect(this->MainDialog, SIGNAL(rejected()), this, SLOT(cancel()));
  QObject::connect(this->BathymetryDialog->FileBrowserButton, SIGNAL(clicked()), this,
    SLOT(displaySourceImporter()));
}

void qtCMBBathymetryDialog::setMeshAndModelMode(bool isModel)
{
  this->ModelAndMeshMode = isModel;
  this->BathymetryDialog->OnlyApplyToVisibleMesh->setVisible(isModel);
}

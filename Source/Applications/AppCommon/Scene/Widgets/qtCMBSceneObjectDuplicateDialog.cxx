/*=========================================================================

  Program:   CMB
  Module:    qtCMBSceneObjectDuplicateDialog.cxx

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
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
// .NAME Represents a dialog for importing objects into SceneGen.
// .SECTION Description
// .SECTION Caveats

#include "qtCMBSceneObjectDuplicateDialog.h"

#include "ui_qtCMBSceneObjectDuplicate.h"
#include "qtCMBPlacementConstraintWidget.h"
#include "pqCMBSceneObjectBase.h"
#include "pqCMBSceneTree.h"
#include "pqCMBSceneNode.h"

//-----------------------------------------------------------------------------
int  qtCMBSceneObjectDuplicateDialog::getCopyInfo(pqCMBSceneNode *parent,
                bool enableGlyphOption, bool enableTextureConstraintOption,
                bool &useGlyphPlayback, QMap<pqCMBSceneNode*, int> &constraints,
                bool &okToUseGlyphs, bool &useTextureConstraint, int &glyphPlaybackOption,
                QString &glyphPlaybackFileName)
{
  qtCMBSceneObjectDuplicateDialog copyDialog(parent, enableGlyphOption,
    enableTextureConstraintOption, useGlyphPlayback);
  return copyDialog.exec(constraints, okToUseGlyphs, useTextureConstraint,
                         useGlyphPlayback, glyphPlaybackOption, glyphPlaybackFileName);
}

//-----------------------------------------------------------------------------
qtCMBSceneObjectDuplicateDialog::
qtCMBSceneObjectDuplicateDialog(pqCMBSceneNode *node, bool enableGlyphOption,
                                  bool enableTextureConstraintOption, bool &useGlyphPlayback) : Count(0)
{
  this->Parent = node;
  MainDialog = new QDialog();

  this->CopyDialog = new Ui::qtCMBSceneObjectDuplicate;
  this->CopyDialog->setupUi(MainDialog);

  this->PlacementWidget = new qtCMBPlacementConstraintWidget(this->Parent,
    this->CopyDialog->MainFrame);
  this->CopyDialog->MainFrame->layout()->addWidget(this->PlacementWidget);
  this->PlacementWidget->enableGlyphOption(enableGlyphOption);
  this->PlacementWidget->enableTextureConstraintOption(enableTextureConstraintOption);
  if(useGlyphPlayback)
    {
    this->PlacementWidget->showGlyphPlaybackGroupBox(true);
    }
  else
    {
    this->PlacementWidget->showGlyphPlaybackGroupBox(false);
    }
  QObject::connect(this->MainDialog, SIGNAL(accepted()), this, SLOT(accept()));
  QObject::connect(this->MainDialog, SIGNAL(rejected()), this, SLOT(cancel()));
}

//-----------------------------------------------------------------------------
qtCMBSceneObjectDuplicateDialog::~qtCMBSceneObjectDuplicateDialog()
{
  if(this->PlacementWidget)
    {
    delete this->PlacementWidget;
    }
  if (this->CopyDialog)
    {
    delete CopyDialog;
    }
  if (this->MainDialog)
    {
    delete MainDialog;
    }
}
//-----------------------------------------------------------------------------
int qtCMBSceneObjectDuplicateDialog::exec(QMap<pqCMBSceneNode*, int> &constraints,
                                            bool &okToUseGlyphs,
                                            bool &useTextureConstraint,
                                            bool &useGlyphPlayback,
                                            int &glyphPlaybackOption,
                                            QString &glyphPlaybackFileName)
{
  this->MainDialog->setModal(true);
  this->MainDialog->show();
  this->MainDialog->exec();

  if (!this->Count)
    {
    return 0;
    }
  constraints = this->PlacementWidget->getSelectedConstraints();
  okToUseGlyphs = this->PlacementWidget->useGlyphs();
  useTextureConstraint = this->PlacementWidget->useTextureConstraint();
  if(useGlyphPlayback)
    {
    glyphPlaybackOption = this->PlacementWidget->getGlyphPlaybackOption();
    glyphPlaybackFileName = this->PlacementWidget->getGlyphPlaybackFilename();
    }
  else
    {
    glyphPlaybackOption = -1;
    glyphPlaybackFileName = QString("");
    }
  return this->Count;
}
//-----------------------------------------------------------------------------
void qtCMBSceneObjectDuplicateDialog::accept()
{
  this->MainDialog->hide();
  this->Count = this->PlacementWidget->getPlacementCount();
}
//-----------------------------------------------------------------------------
void qtCMBSceneObjectDuplicateDialog::cancel()
{
  this->Count = 0;
}
//-----------------------------------------------------------------------------

/*=========================================================================

  Program:   CMB
  Module:    qtCMBSceneObjectDuplicateDialog.h

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
// .NAME qtCMBSceneObjectDuplicateDialog - imports an object into the scene.
// .SECTION Description
// .SECTION Caveats


#ifndef __qtCMBSceneObjectDuplicateDialog_h
#define __qtCMBSceneObjectDuplicateDialog_h

#include "cmbAppCommonExport.h"
#include <QObject>
#include <QStringList>
#include <vector>
#include "cmbSystemConfig.h"

class pqCMBSceneNode;
class pqCMBSceneObjectBase;
class QDialog;
class qtCMBPlacementConstraintWidget;

namespace Ui
{
  class qtCMBSceneObjectDuplicate;
};

class CMBAPPCOMMON_EXPORT qtCMBSceneObjectDuplicateDialog : public QObject
{
  Q_OBJECT

public:
  // returns the number of copies
  static int getCopyInfo(pqCMBSceneNode *parent,
                         bool enableGlyphOption,
                         bool enableTextureConstraintOption,
                         bool &useGlyphPlayback,
                         QMap<pqCMBSceneNode *, int> &constraints,
                         bool &okToUseGlyphs,
                         bool &useTextureConstraint,
                         int &glyphPlaybackOption,
                         QString &glyphPlaybackFileName);

protected slots:
  void accept();
  void cancel();

protected:
  qtCMBSceneObjectDuplicateDialog(pqCMBSceneNode *n, bool enableGylphOption,
    bool enableTextureConstraintOption, bool &useGlyphPlayback);
  qtCMBSceneObjectDuplicateDialog():
    CopyDialog(NULL), MainDialog(NULL), Parent(NULL), Count(0)
    {}
  virtual ~qtCMBSceneObjectDuplicateDialog();
  int exec(QMap<pqCMBSceneNode*, int> &constraints,
           bool &okToUseGlyphs,
           bool &useTextureConstraint,
           bool &useGlyphPlayback,
           int &glyphPlaybackOption,
           QString &glyphPlaybackFileName);

  Ui::qtCMBSceneObjectDuplicate *CopyDialog;
  QDialog *MainDialog;
  pqCMBSceneNode *Parent;

  qtCMBPlacementConstraintWidget* PlacementWidget;
  int Count;
};

#endif /* __qtCMBSceneObjectDuplicateDialog_h */

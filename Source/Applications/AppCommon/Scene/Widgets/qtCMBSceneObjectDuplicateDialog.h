//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
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

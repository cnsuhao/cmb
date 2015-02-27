/*=========================================================================

  Program:   CMB
  Module:    qtCMBPlacementConstraintWidget.h

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
// .NAME qtCMBPlacementConstraintWidget - The scene control panel widget.
// .SECTION Description
//  This class is a GUI panel widget to control the operations to the scene.
//  The main component of the panel is a tree structure to represent the scene.
// .SECTION Caveats

#ifndef __qtCMBPlacementConstraintWidget_h
#define __qtCMBPlacementConstraintWidget_h

#include "cmbAppCommonExport.h"
#include <QWidget>
#include "cmbSystemConfig.h"
class QTableWidgetItem;

namespace Ui { class qtObjectPlacementConstraint; }

class qtCMBPlacementConstraintWidgetInternal;
class pqCMBSceneNode;

class CMBAPPCOMMON_EXPORT qtCMBPlacementConstraintWidget : public QWidget
{
  Q_OBJECT

public:
  qtCMBPlacementConstraintWidget(pqCMBSceneNode* n, QWidget* parent=0);
  virtual ~qtCMBPlacementConstraintWidget();

  Ui::qtObjectPlacementConstraint* getWidget();
  const QMap <pqCMBSceneNode*, int>& getSelectedConstraints() const;
  int getPlacementCount();
  void updateConstraintTable();
  void enableGlyphOption(bool mode);
  void enableTextureConstraintOption(bool mode);
  bool useGlyphs() const;
  bool useTextureConstraint() const;
  void showGlyphPlaybackGroupBox(bool show) const;
  int getGlyphPlaybackOption() const;
  QString getGlyphPlaybackFilename() const;
  bool fileExists(QString fileName) const;

signals:

public slots:

private slots:
  void onTableSelectionChanged();
  void onItemChanged(QTableWidgetItem*);
  void displayGlyphPlaybackFileBrowser();
  void filesSelected(const QList<QStringList> &files);
  void checkGlyphPlaybackFile(int option);

private:
  qtCMBPlacementConstraintWidgetInternal* Internal;
  void setupConstraintTable();
  void addTableRow(pqCMBSceneNode* scenenode);

  pqCMBSceneNode* parentNode;

};
#endif

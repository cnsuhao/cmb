//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
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

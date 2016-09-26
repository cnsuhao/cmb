//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtCMBSceneObjectImporter - imports an object into the scene.
// .SECTION Description
// .SECTION Caveats


#ifndef __qtCMBSceneObjectImporter_h
#define __qtCMBSceneObjectImporter_h

#include "cmbAppCommonExport.h"
#include <QObject>
#include <QStringList>
#include <vector>
#include "cmbSystemConfig.h"

class pqCMBSceneNode;
class pqCMBSceneObjectBase;
class QDialog;
class qtCMBPlacementConstraintWidget;
class pqPipelineSource;
class QIntValidator;
class pqFileDialogModel;
class QProgressDialog;

namespace Ui
{
  class qtCMBSceneObjectImport;
};

class CMBAPPCOMMON_EXPORT qtCMBSceneObjectImporter : public QObject
{
  Q_OBJECT

public:
  static pqCMBSceneNode *importNode(pqCMBSceneNode *parent, bool AllowRandomOption,
                                  bool allowTextureConstraintPlacement,
                                  bool *randomPlacement, bool *translateBasedOnView,
                                  int *count, QMap<pqCMBSceneNode*, int> &constraints,
                                  bool &useGlyphs, bool &useTextureConstraint,
                                  bool &useGlyphPlayback,
                                  int &glyphPlaybackOption,
                                  QString &glyphPlaybackFilename);

  void setRandomPlacementOption(bool mode);

protected slots:
  void accept();
  void cancel();
  void changeObjectType();
  void changeGeometricType();
  void displayFileBrowser();
  void filesSelected(const QStringList &files);
  void updateRowExtents();
  void updateColumnExtents();
  void updateDialog();
  void updateProgress(const QString&, int progress);

protected:
  qtCMBSceneObjectImporter(pqCMBSceneNode *n);
  qtCMBSceneObjectImporter():
    ImportDialog(NULL), MainDialog(NULL), Node(NULL), Parent(NULL)
    {}
  ~qtCMBSceneObjectImporter() override;
  void assignUnits(pqCMBSceneObjectBase *obj);
  void setupObjectTypes();
  void updateDEMExtents();
  pqCMBSceneNode *exec(bool *randomPlacement,
                     bool allowTextureConstraintPlacement,
                     bool *translateBasedOnView, int *count,
                     QMap<pqCMBSceneNode*, int> &constraints,
                     bool &useGlyphs,
                     bool &useTextureConstraint,
                     bool &useGlyphPlayback,
                     QString &glyphPlaybackFileName,
                     int &glyphPlaybackOption);

  void importLIDARFile(const QString &fileName);
  void importLASFile(const QString &fileName);
  void importUniformGrid(const QString &fileName);
  void importShapeFile(const QString &fileName);
  void importSolidMesh(const QString &filename);
  void importBorFile(const QString &filename);
  int computeApproximateRepresentingFloatDigits(double min, double max);
  bool userRequestsDoubleData(double bounds[6]);

  pqCMBSceneNode *createObjectNode(pqCMBSceneObjectBase *obj, const char *name,
    pqCMBSceneNode *parentNode);

  Ui::qtCMBSceneObjectImport *ImportDialog;
  QDialog *MainDialog;
  pqCMBSceneNode *Node, *Parent;
  qtCMBPlacementConstraintWidget* PlacementWidget;
  int ObjectType;
  QIntValidator *MinRowValidator;
  QIntValidator *MaxRowValidator;
  QIntValidator *MinColumnValidator;
  QIntValidator *MaxColumnValidator;
  pqFileDialogModel *FileValidator;
  QProgressDialog *Progress;
  int ColumnExtents[2];
  int RowExtents[2];
  QString CurrentFileName;
};

#endif /* __qtCMBSceneObjectImporter_h */

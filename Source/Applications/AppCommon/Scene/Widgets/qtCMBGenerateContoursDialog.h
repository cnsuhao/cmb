//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME qtCMBGenerateContoursDialog - .
// .SECTION Description
// .SECTION Caveats

#ifndef __qtCMBGenerateContoursDialog_h
#define __qtCMBGenerateContoursDialog_h

#include "cmbAppCommonExport.h"
#include "cmbSystemConfig.h"
#include <QDialog>
#include <QDoubleValidator>

class pqCMBSceneNode;
class pqCMBUniformGrid;
class pqCMBEnumPropertyWidget;
class qtCMBProgressWidget;
class pqDataRepresentation;
class pqRenderView;
class pqPipelineSource;
class QDoubleValidator;
class QIntValidator;
class QProgressDialog;

namespace Ui
{
class qtCMBGenerateContoursDialog;
};

class CMBAPPCOMMON_EXPORT qtCMBGenerateContoursDialog : public QDialog
{
  Q_OBJECT
public:
  qtCMBGenerateContoursDialog(
    pqCMBSceneNode* parentNode, QWidget* parent = NULL, Qt::WindowFlags flags = 0);
  ~qtCMBGenerateContoursDialog() override;

  int exec();

protected slots:
  void generateContours();
  void onCreateContourNodes();
  void onCancel();
  void updateProgress(const QString&, int progress);
  void updateContourButtonStatus();

  void onOpacityChanged(int opacity);

protected:
  void setupProgressBar(QWidget* progressWidget);
  void disableWhileProcessing();

  Ui::qtCMBGenerateContoursDialog* InternalWidget;

  QDialog* MainDialog;
  pqCMBSceneNode* ParentNode;
  pqCMBEnumPropertyWidget* RepresentationWidget;
  pqRenderView* RenderView;
  pqCMBUniformGrid* Grid;
  pqDataRepresentation* ContourRepresentation;
  pqPipelineSource* ContourSource;
  pqPipelineSource* CleanPolyLines;
  QString ProgressMessage;
  QString ImageNodeName;
  qtCMBProgressWidget* ProgressWidget;
  bool ProgressMessagesMustMatch;
  double ContourValue;
  double MinimumLineLength;
  bool UseRelativeLineLength;
  QDoubleValidator* ContourValidator;
  QProgressDialog* Progress;
};

//need a sublcass validator, since QDoubleValidator is really shitty
class CMBAPPCOMMON_EXPORT InternalDoubleValidator : public QDoubleValidator
{
  Q_OBJECT
public:
  InternalDoubleValidator(QObject* parent);
  void fixup(QString& input) const override;
};

#endif

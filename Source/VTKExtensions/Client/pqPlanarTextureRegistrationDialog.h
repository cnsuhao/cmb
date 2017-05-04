//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqPlanarTextureRegistrationDialog - manages the node's texture information.
// .SECTION Description
// .SECTION Caveats

#ifndef __pqPlanarTextureRegistrationDialog_h
#define __pqPlanarTextureRegistrationDialog_h

#include "cmbSystemConfig.h"
#include "vtkCMBClientModule.h" // For export macro
#include <QObject>
#include <QPointer>
#include <QStringList>

class pqDataRepresentation;
class pqPipelineSource;
class pqPropertyLinks;
class pqRenderView;
class pqView;
class pqServer;
class vtkSMNewWidgetRepresentationProxy;

class QDialog;

namespace Ui
{
class qtPlanarTextureRegistrationDialog;
};

class VTKCMBCLIENT_EXPORT pqPlanarTextureRegistrationDialog : public QObject
{
  Q_OBJECT

public:
  pqPlanarTextureRegistrationDialog(
    pqServer* server, pqRenderView* view, const QString& strTitle, QWidget* parent);
  ~pqPlanarTextureRegistrationDialog() override;

  void initializeTexture(const double bounds[6], const QStringList& textureFiles,
    const QString& currentTextureFile, const double regPoints[12], int numRegPoints,
    bool pluginPanel = false);

  int getNumberOfRegistrationPoints();
  void getRegistrationPoints(double pnts[12]);
  const QString& getCurrentImageFileName() { return this->currentImageFileName; }
  void updateTextureList(const QStringList& textureFiles, const QString& currentTextureFile);

  QDialog* getMainDialog();
  pqRenderView* getImageRenderView();
  pqRenderView* currentView();
  pqPropertyLinks* getLinks();
  void showButtons(bool showing);
  pqPipelineSource* getTextureImageSource() { return this->textureImageSource; }

signals:
  void removeCurrentTexture();
  void registerCurrentTexture(
    const QString& filename, int numberOfRegistrationPoints, double* points);
  void dialogModified();

public slots:
  int exec();
  void apply();

protected slots:
  void accept();
  void cancel();
  void displayFileBrowser();
  void filesSelected(const QStringList& files);
  void registrationModeChanged(int mode);
  void applyTexture();
  void removeTexture();
  void displayImage(const QString& filename);
  void updateAllViews();
  void destroyingView(pqView*);

protected:
  int Status;
  QDialog* MainDialog;
  Ui::qtPlanarTextureRegistrationDialog* TextureDialog;
  pqServer* CurrentServer;
  pqRenderView* CurrentView;
  double CurrentBounds[6];
  bool PV_PLUGIN_USE;

  QPointer<pqRenderView> imageRenderView;
  pqDataRepresentation* imageDataRepresentation;
  pqPipelineSource* textureImageSource;
  pqPropertyLinks* Links;
  QString currentImageFileName;
  double RegistrationPoints[12];
  QStringList TextureFiles;

  vtkSMNewWidgetRepresentationProxy* widgetI1;
  vtkSMNewWidgetRepresentationProxy* widgetI2;
  vtkSMNewWidgetRepresentationProxy* widgetI3;
  vtkSMNewWidgetRepresentationProxy* widgetS1;
  vtkSMNewWidgetRepresentationProxy* widgetS2;
  vtkSMNewWidgetRepresentationProxy* widgetS3;

  void PlaceWidgets(bool defaultPosition = true);
  vtkSMNewWidgetRepresentationProxy* setupPointWidget(pqServer* server, pqRenderView* view);
  void updatePointWidget(vtkSMNewWidgetRepresentationProxy* widget, QList<QVariant> center,
    bool visibility = true, bool enable = true);
  void getRegistrationPointPair(
    int i, double xy[2], double st[2], const double regPoints[12]) const;
  void removeSWidgets();
  void removeIWidgets();
};

#endif /* __pqPlanarTextureRegistrationDialog_h */

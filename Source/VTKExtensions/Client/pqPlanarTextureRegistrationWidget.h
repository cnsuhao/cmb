//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef pqPlanarTextureRegistrationWidget_h
#define pqPlanarTextureRegistrationWidget_h

#include "vtkCMBClientModule.h" // For export macro

#include "pqComponentsModule.h"
#include "pqPropertyWidget.h"
#include "pqSMProxy.h"

#include "pqPlanarTextureRegistrationDialog.h"

class vtkSMProperty;
class pqDataRepresentation;
class pqProxyWidgetDialog;

class VTKCMBCLIENT_EXPORT pqPlanarTextureRegistrationWidget : public pqPropertyWidget
{
  Q_OBJECT
  typedef pqPropertyWidget Superclass;

public:
  pqPlanarTextureRegistrationWidget(vtkSMProxy* proxy, vtkSMProperty* smproperty, QWidget* parent = 0);
  virtual ~pqPlanarTextureRegistrationWidget();

protected:
  vtkSMProperty* getTextureProperty();
  pqDataRepresentation* getRepresentation();
  void updateTextureList();
  void fetchTextureFiles(QStringList & textureFiles);

  /// Called if the user accepts pending modifications
  virtual void apply();
  /// Called if the user rejects pending modifications
  virtual void reset();

 private slots:
  /// Called to update the enable state for certain widgets.
  void updateEnableState();

  void setTextureMap(const QString& filename,
    int numberOfRegistrationPoints, double *points);
  void unsetTextureMap();

  /// Forces a reload of the widget. Generally one does not need to call this
  /// method explicitly.
  void proxyRegistered(const QString& group);
  void proxyUnRegistered( const QString& group, const QString&, vtkSMProxy* proxy);

private:
  Q_DISABLE_COPY(pqPlanarTextureRegistrationWidget)

  class pqImplementation;
  pqImplementation* const Implementation;

  QString PropertyName;
};

#endif

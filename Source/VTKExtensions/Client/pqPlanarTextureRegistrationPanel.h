//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef _pqPlanarTextureRegistrationPanel_h
#define _pqPlanarTextureRegistrationPanel_h

#include "pqPropertyWidget.h"
#include "cmbSystemConfig.h"

class vtkSMProperty;
class pqDataRepresentation;

/// Custom panel for the planar texture registration filter
class pqPlanarTextureRegistrationPanel : public pqPropertyWidget
{
  typedef pqPropertyWidget Superclass;

  Q_OBJECT

public:
  pqPlanarTextureRegistrationPanel(vtkSMProxy* pxy, vtkSMPropertyGroup*, QWidget* p=0);
  ~pqPlanarTextureRegistrationPanel() override;

  void apply() override;
  void reset() override;

protected:
  vtkSMProperty* getTextureProperty();
  pqDataRepresentation* getRepresentation();
  void updateTextureList();
  void fetchTextureFiles(QStringList & textureFiles);

private slots:

  void setTextureMap(const QString& filename,
    int numberOfRegistrationPoints, double *points);
  void unsetTextureMap();

  /// Forces a reload of the widget. Generally one does not need to call this
  /// method explicitly.
  void proxyRegistered(const QString& group);
  void proxyUnRegistered( const QString& group, const QString&, vtkSMProxy* proxy);
private:
  class pqImplementation;
  pqImplementation* const Implementation;
};

#endif

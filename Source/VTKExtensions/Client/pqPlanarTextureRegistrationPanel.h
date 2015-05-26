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

#include "pqObjectPanel.h"
#include "cmbSystemConfig.h"

class vtkSMProperty;
class pqDataRepresentation;

/// Custom panel for the planar texture registration filter
class pqPlanarTextureRegistrationPanel : public pqObjectPanel
{
  typedef pqObjectPanel Superclass;

  Q_OBJECT

public:
  pqPlanarTextureRegistrationPanel(pqProxy* proxy, QWidget* p);
  ~pqPlanarTextureRegistrationPanel();

public slots:

protected:
  vtkSMProperty* getTextureProperty();
  pqDataRepresentation* getRepresentation();
  void updateTextureList();
  void fetchTextureFiles(QStringList & textureFiles);

private slots:
  /// Called if the user accepts pending modifications
  virtual void onAccepted();
  /// Called if the user rejects pending modifications
  virtual void onRejected();

  /// Called if the user accepts pending modifications
  //void onAccepted();
  /// Called if the user rejects pending modifications
  //void onRejected();
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
  class pqImplementation;
  pqImplementation* const Implementation;
};

#endif

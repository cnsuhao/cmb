/*=========================================================================

  Program:   CMB
  Module:    pqCMBModelEntity.h

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
// .NAME pqCMBModelEntity - a client side CMB model entity object.
// .SECTION Description
// .SECTION Caveats

#ifndef __pqCMBModelEntity_h
#define __pqCMBModelEntity_h

#include <QObject>
#include <QList>

#include "vtkModelEntity.h"
#include "vtkWeakPointer.h"
#include "vtkSmartPointer.h"
#include "cmbSystemConfig.h"

class pqCMBTreeItem;
class pqCMBModel;
class pqPipelineSource;
class pqDataRepresentation;
class pqRenderView;
class pqServer;
class vtkSMProxy;
class vtkSMSourceProxy;
class vtkDiscreteLookupTable;
class vtkCMBModelEntityMesh;
class vtkSMPropertyLink;
class vtkSMProperty;

class  pqCMBModelEntity : public QObject
{
  Q_OBJECT

public:
  pqCMBModelEntity();
  virtual ~pqCMBModelEntity();

  // Description:
  // Add/remove a widget that is associated with this object
  virtual void addModelWidget(pqCMBTreeItem* widget);
  virtual void removeModelWidget(pqCMBTreeItem* widget);

  // Decscription:
  // Get or clear all the widgets assocated with this object
  const QList<pqCMBTreeItem*>& getModelWidgets()
    {return this->Widgets;}
  virtual void clearWidgets();

  // Description:
  // Get/Set the actual model entity that this object is representing.
  virtual vtkModelEntity* getModelEntity()
    {return this->ModelEntity;}
  virtual void setModelEntity(vtkModelEntity* entity);
  virtual vtkIdType getUniqueEntityId()
    {return this->UniqueEntityId;}

  enum EntityColorByMode
  {
    ColorByNone      = 0,
    ColorByEntity    = 1,
    ColorByDomain    = 2,
    ColorByMaterial  = 3,
    ColorByBCS       = 4,
    ColorByAttribute = 5
  };
  // Description:
  // Get the representation color mode set by "ModelFaceColorMode" property
  //virtual int getColorMode();
  virtual void colorByColorMode(vtkDiscreteLookupTable*, int /*colorMode*/){;}

  // Description:
  // Get databounds of the entity
  bool getBounds(double bounds[6]) const;

  // Description:
  // Visibility related methods
  virtual void setVisibility(int visible);
  static void setRepresentationVisibility(
    pqDataRepresentation*, int visible);
  int getVisibility();
  int meshRepresentationVisible();
  void setPickable(int pickable);
  int getPickable();

  // Description:
  // Color related methods
  void setColor(float RGBA[4])
    { this->setColor(RGBA[0], RGBA[1], RGBA[2], RGBA[3]); }
  void setColor(double r, double g, double b, double a);
  double* getColor();
  void getColor(float RGBA[4]);


  // Description:
  // Selection related methods
  void setHighlight( int highlight);
  virtual void select();
  virtual void deselect();
  bool hasSelectionInput();
  void setSelectionInput(vtkSMSourceProxy*);

  // Description:
  // Set the mesh entity
  void setMeshEntity(
    vtkCMBModelEntityMesh* entMesh);
  vtkCMBModelEntityMesh* getMeshEntity();
  void updateMeshEntity(pqCMBModel* model);
  void setEdgeMeshPointsVisibility(int visible);
  void setEntityMeshVisibility(int visible)
  { this->setRepresentationVisibility(
    this->MeshRepresentation, visible); }
  void applyMeshBathymetry(vtkSMProxy* bathymetrySource, double eleRaius,
    bool useHighLimit, double eleHigh, bool useLowLimit, double eleLow);
  void unapplyMeshBathymetry();
  void synchronizeMeshRepresentation(vtkSMProperty *changedProp, const char* propertyName);
  void setMeshRepresentationType(const char* strType);

  // Description:
  // Some convenient methods
  pqPipelineSource * getSource() const;
  pqDataRepresentation *getRepresentation() const;
  void updateRepresentation(vtkSMProxy* modelWrapper);
  void setLODMode(int mode, bool updateRep);

  void setRepresentationColor(double color[3], double opacity=1.0)
    { this->setRepresentationColor(color[0], color[1],color[2], opacity);  }
  void setRepresentationColor(double r, double g, double b, double opacity=1.0);
  vtkSMProxy* getModelWrapper(){return this->ModelWrapper;}
  void setModelWrapper(vtkSMProxy* modelWrapper)
    {this->ModelWrapper = modelWrapper;}
  void setMeshRepresentationColor(double color[3])
    { this->setMeshRepresentationColor(color[0], color[1],color[2]);  }
  void setMeshRepresentationColor(double r, double g, double b);
  virtual bool getMeshRepresentationColor(double color[4]) const;
  virtual pqDataRepresentation* getMeshRepresentation()
  { return this->MeshRepresentation;}
  virtual pqPipelineSource * getMeshSource() const;

protected:

  // Description:
  // convenient methods
  virtual void init();

  void getColor(vtkModelEntity* modelEntity, double *rgb,
    double &opacity, vtkDiscreteLookupTable* lut);
  pqPipelineSource* getMeshBathymetryFilter();

  // Description:
  // some internal ivars
  vtkModelEntity* ModelEntity;
  QList<pqCMBTreeItem*> Widgets;

  // Description:
  // Some ivars
  pqPipelineSource* PolyProvider;
  pqDataRepresentation* Representation;

  // This id is the same as the ModelEntity Id, and is required
  // because ModelEntity could be removed from the model, but
  // we may still need to identify pqCMBModelEntity
  vtkIdType UniqueEntityId;

  // Reference to the mesh entity
  vtkWeakPointer<vtkCMBModelEntityMesh> MeshEntity;
  pqPipelineSource* MeshPolyProvider;
  pqDataRepresentation* MeshRepresentation;
  pqPipelineSource* MeshBathymetryFilter;
  int MeshEdgePointsVisible;
  QString MeshFaceRepresentationType;


  vtkSMProxy* ModelWrapper;
};

#endif /* __pqCMBModelEntity_h */

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqCMBSceneObjectBase - represents a 3D SceneGen object.
// .SECTION Description
// .SECTION Caveats


#ifndef __pqCMBSceneObjectBase_h
#define __pqCMBSceneObjectBase_h

#include "cmbAppCommonExport.h"
#include <QObject>
#include <QPointer>
#include "vtkSMProxy.h"
#include "vtkSmartPointer.h"
#include "cmbSceneUnits.h"
#include "pqVariableType.h"
#include "cmbSystemConfig.h"
#include "pqDataRepresentation.h"

class pqPipelineSource;
class pqDataRepresentation;
class pqRenderView;
class pqServer;
class vtkPoints;
class vtkSMSourceProxy;
class vtkTransform;
class vtkBoundingBox;

class  CMBAPPCOMMON_EXPORT pqCMBSceneObjectBase
{

public:
  enum enumObjectType
  {
    Unknown,
    Points,
    VOI,
    Line,
    Arc,
    GroundPlane,
    Faceted,
    Glyph,
    UniformGrid,
    Polygon,
    GeneralCone,
    SolidMesh,
    GeoBoreHole,
    GeoCrossSection
  };

  enum enumSurfaceType
  {
    Other,
    Solid,
    TIN
  };

  pqCMBSceneObjectBase();
  pqCMBSceneObjectBase(pqPipelineSource *source);
  virtual ~pqCMBSceneObjectBase();

  pqPipelineSource * getSource() const;
  virtual pqPipelineSource * getSelectionSource() const;
  virtual void setSelectionInput(vtkSMSourceProxy *selectionInput);

  virtual vtkSMSourceProxy *getSelectionInput() const;

  pqDataRepresentation * getRepresentation() const;

  virtual pqCMBSceneObjectBase *duplicate(pqServer *server, pqRenderView *view,
                                    bool updateRep = true) = 0;

  virtual enumObjectType getType() const = 0;
  std::string getTypeAsString() const;

  virtual bool isDefaultConstrained() const{return false;}

  virtual void setVisibility(bool mode);

  void setUnits(cmbSceneUnits::Enum unitType);
  cmbSceneUnits::Enum getUnits() const
    { return this->Units;}

  void clearConstraints();

  virtual void getAreaStats(double* areaStats);
  virtual void getGeometryBounds(double* geoBounds) const;
  virtual void getPolySideStats(double* polySide);
  virtual double getSurfaceArea();
  virtual vtkIdType getNumberOfPoints();
  virtual vtkIdType getNumberOfPolygons();

  bool isSnapTarget() const
    {return this->IsSnapTarget;}
  void setIsSnapTarget(bool mode)
    {this->IsSnapTarget = mode;}

  bool isFullyConstrained() const
    {return this->IsFullyConstrained;}
  void setIsFullyConstrained(bool mode)
    {this->IsFullyConstrained = mode;}

  std::string getUserDefinedType() const
    {return this->UserDefinedType;}
  void setUserDefinedType(const char *type)
    {this->UserDefinedType = type;}

  bool isXTransConstrain() const
    {return this->Constraints[0];}
  void setXTransConstraint(bool mode)
    {this->setConstraint(0, mode);}

  bool isYTransConstrain() const
    {return this->Constraints[1];}
  void setYTransConstraint(bool mode)
    {this->setConstraint(1, mode);}

  bool isZTransConstrain() const
    {return this->Constraints[2];}
  void setZTransConstraint(bool mode)
    {this->setConstraint(2, mode);}

  bool isXRotationConstrain() const
    {return this->Constraints[3];}
  void setXRotationConstraint(bool mode)
    {this->setConstraint(3, mode);}

  bool isYRotationConstrain() const
    {return this->Constraints[4];}
  void setYRotationConstraint(bool mode)
    {this->setConstraint(4, mode);}

  bool isZRotationConstrain() const
    {return this->Constraints[5];}
  void setZRotationConstraint(bool mode)
    {this->setConstraint(5, mode);}

  bool isIsotropicScalingConstrain() const
    {return this->Constraints[6];}
  void setIsotropicScalingConstraint(bool mode)
    {this->setConstraint(6, mode);}

  bool isXScalingConstrain() const
    {return this->Constraints[7];}
  void setXScalingConstraint(bool mode)
    {this->setConstraint(7, mode);}

  bool isYScalingConstrain() const
    {return this->Constraints[8];}
  void setYScalingConstraint(bool mode)
    {this->setConstraint(8, mode);}

  bool isZScalingConstrain() const
    {return this->Constraints[9];}
  void setZScalingConstraint(bool mode)
    {this->setConstraint(9, mode);}

  static void getCameraFocalPoint(pqRenderView *view, double pos[3]);

  virtual void getColor(double color[4]) const;
  virtual void setColor(double color[4], bool updateRep = true);
  ///Description:
  /// Returns the Scene Bounds of the Objects Representation.
  /// Note: This includes any transformations of the actor
  virtual void getBounds(double bounds[6]) const;
  virtual void getBounds(vtkBoundingBox* bb) const;
  ///Description:
  /// Returns the Bounds of the data.
  virtual void getDataBounds(double bounds[6]) const;
  void getPosition(double pos[3]) const;
  void setPosition(double pos[3], bool updateRep = true);
  void getOrientation(double ori[3]) const;
  void setOrientation(double ori[3], bool updateRep = true);
  void getScale(double scale[3]) const;
  void setScale(double scale[3], bool updateRep = true);
  void getOrigin(double origin[3]) const;
  void setOrigin(double origin[3], bool updateRep = true);
  void getTransform(vtkTransform *transform) const;
  void setTransform(vtkTransform *transform, bool updateRep = true);
  void setPickable(bool mode);
  int getPickable();
  virtual void applyTransform(double scaleDelta[3],
                              double orientationDelta[3],
                              double translationDelta[3]);

  virtual void updateRepresentation();
  void getClosestPoint(const double p[3], double cp[3]);
  void setLODMode(int mode, bool updateRep = true);
  void zoomOnObject();

  virtual void updatePolyDataStats();

  virtual void setMarkedForDeletion();
  virtual void unsetMarkedForDeletion();

  static pqCMBSceneObjectBase::enumObjectType convertStringToType(const char *);
  static std::string convertTypeToString( pqCMBSceneObjectBase::enumObjectType t);
  static pqCMBSceneObjectBase::enumSurfaceType convertStringToSurfaceType(const char *);
  static std::string convertSurfaceTypeToString( pqCMBSceneObjectBase::enumSurfaceType t);

  static void DoRepresentationCallback(vtkObject* vtk_obj, unsigned long event,
                           void* client_data, void* call_data);

protected:
  void setupSceneObject();

  void setSource(pqPipelineSource *source);
  virtual void setRepresentation(pqDataRepresentation *rep);

  virtual void duplicateInternals(pqCMBSceneObjectBase *obj);
  inline void updateVectorProperty(const char* name, double *v,
    const int &size, const bool &updateRep);
  void setConstraint(int i, bool mode);
  void updateTransform() const; //will modify Transform and TransformNeedsUpdate

  QPointer<pqPipelineSource> Source;
  QPointer<pqPipelineSource> ClosestPointFilter;
  QPointer<pqPipelineSource> PolyDataStatsFilter;
  std::string UserDefinedType;
  cmbSceneUnits::Enum Units;
  bool Constraints[10];
  bool IsFullyConstrained;
  bool IsSnapTarget;

private:
  //The representation needs to be private as we have to force
  //everybody to use SetRepresentation so that the callbacks for getting the correct
  //display bounds are correct
  QPointer<pqDataRepresentation> Representation;

  //Holds all the information for cache the Representation Bounds
  //Including the cached transform matrix and the vtkCallbacks
  class pqCMBSceneObjectBaseInternal;
  pqCMBSceneObjectBaseInternal *Internal;

};

#endif /* __pqCMBSceneObjectBase_h */

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME CmbScenePolyline - represents a contour object.
// .SECTION Description
// .SECTION Caveats

#ifndef __CmbScenePolyline_h
#define __CmbScenePolyline_h


#include "cmbAppCommonExport.h"
#include "pqCMBSceneObjectBase.h"
#include "cmbSystemConfig.h"
#include <set>

class pqRenderView;
class pqServer;
class vtkSMOutputPort;
class pqPipelineSource;
class pqDataRepresentation;
class vtkSMNewWidgetRepresentationProxy;
class vtkSMSourceProxy;
class vtkPVArcInfo;
class vtkIdTypeArray;
class pqCMBPolygon;

class  CMBAPPCOMMON_EXPORT pqCMBArc : public pqCMBSceneObjectBase
{
  typedef pqCMBSceneObjectBase Superclass;
public:
  //Description: Default constructor that than
  //needs createArc called on once its input for arc shape is created
  pqCMBArc();

  //Description: Constructor that takes in the proxy
  //to call createArc on.
  pqCMBArc(vtkSMSourceProxy *proxy);

  ~pqCMBArc() override;

  //Description:
  //The server side arc has already been created, we just
  //need to create the client side object and the representation.
  //this called after a split operation
  //Note: This doesn't change to make sure the arcId exists on the
  //server so watch out!
  virtual bool createArc(const vtkIdType& arcId);

  //Description:
  //Creates the server side arc from the widget poly data that is passed in
  //this method can only be called once per cmbArc.
  //returns true if it created the arc
  virtual bool createArc(vtkSMNewWidgetRepresentationProxy *widget);

  //Description:
  //Edit this arc representation with the widget proxy passed in
  //this will update the arc state when it is done
  virtual bool editArc(vtkSMNewWidgetRepresentationProxy *widget);

  //Description:
  //Given a selection object find the middle point and use
  //that as the new selection for the arc. This is used when
  //determine the start and end points for editing an arc, when
  //the user want to edit on a section of the arc.
  virtual bool findPickPoint(vtkSMOutputPort *port);

  //Description:
  //Update the server side arc with the new widget proxy shape
  //returns true if it could update the arc
  //filles the passed in vtkIdTypeArray with arcs that need to be created
  virtual bool updateArc(vtkSMNewWidgetRepresentationProxy *widget,
                         vtkIdTypeArray *newlyCreatedArcIds);

  //Description:
  //Update the server with a connection between this arc
  //and the arc that is being passed in
  //Returns the id of the created arc. Will return -1 if we can't connect
  //the arcs
  virtual vtkIdType autoConnect(const vtkIdType& secondArcId);

  pqCMBSceneObjectBase::enumObjectType getType() const override;
  bool isDefaultConstrained() const override{return true;}
  pqCMBSceneObjectBase *duplicate(pqServer *server, pqRenderView *view,
                                    bool updateRep = true) override;
  vtkPVArcInfo* getArcInfo();

  vtkIdType getArcId(){return ArcId;}

  bool isClosedLoop();
  int getClosedLoop();

  bool isUsedByPolygons() const
    { return this->PolygonsUsingArc.size() > 0; }

  const std::set< pqCMBPolygon* >& polygonsUsingArc() const
    { return this->PolygonsUsingArc; }

  void inheritPolygonRelationships(pqCMBArc* parent);

  int getPlaneProjectionNormal(){return PlaneProjectionNormal;}
  double getPlaneProjectionPosition(){return PlaneProjectionPosition;}

  void setPlaneProjectionNormal(const int &norm)
      {PlaneProjectionNormal=norm;}
  void setPlaneProjectionPosition(const double &pos)
      {PlaneProjectionPosition=pos;}

  // Overwrite the databounds
  void getDataBounds(double bounds[6]) const override
    {this->getBounds(bounds);}

  void setSelectionInput(vtkSMSourceProxy *selectionInput) override;
  virtual void select();
  virtual void deselect();

  void getColor(double color[4]) const override;
  void setColor(double color[4], bool updateRep = true) override;

  void setMarkedForDeletion() override;
  void unsetMarkedForDeletion() override;

  void arcIsModified();
  void updateRepresentation() override;
protected:
  //make pqCMBPolygon a friend class so that we can have
  //arcs and polygons refresh each other if needed
  friend class pqCMBPolygon;

  void addPolygon(pqCMBPolygon* poly);
  void removePolygon(pqCMBPolygon* poly);

  vtkPVArcInfo *ArcInfo;

  //Description:
  //Creates the server side arc from the proxy poly data that is passed in
  //this method can only be called once per cmbArc.
  //returns true if it created the arc
  virtual bool createArc(vtkSMSourceProxy *proxy);

  void setRepresentation(pqDataRepresentation *rep) override;
  void updatePlaneProjectionInfo(vtkSMNewWidgetRepresentationProxy *widget);

  // Indicates the projection normal as lying along the
  // XAxis, YAxis, ZAxis, or Oblique. For X, Y, and Z axes,
  // the projection normal is assumed to be anchored at
  // (0,0,0)
  int             PlaneProjectionNormal; // for the bounded plane

  // Indicates a distance from the origin of the projection
  // normal where the project plane will be placed
  double          PlaneProjectionPosition;

  double origColor[4];
  double selColor[4];

  vtkIdType ArcId;

  std::set<pqCMBPolygon*> PolygonsUsingArc;

};
#endif

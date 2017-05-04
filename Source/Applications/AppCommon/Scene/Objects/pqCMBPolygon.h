//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqCMBPolygon - represents a collection of contours that make
// a polygon
// .SECTION Description
// .SECTION Caveats

#ifndef __pqCMBPolygon_h
#define __pqCMBPolygon_h

#include "cmbAppCommonExport.h"
#include "cmbSystemConfig.h"
#include "pqCMBTexturedObject.h"
#include <set>
#include <vector>

class pqRenderView;
class pqCMBSceneNode;
class pqCMBArc;

class CMBAPPCOMMON_EXPORT pqCMBPolygon : public pqCMBTexturedObject
{
  typedef pqCMBSceneObjectBase Superclass;

public:
  pqCMBPolygon(double minAngle, double edgeLength, std::vector<pqCMBSceneNode*>& inputNodes);
  ~pqCMBPolygon() override;

  bool isDefaultConstrained() const override { return true; }

  pqCMBSceneObjectBase::enumObjectType getType() const override;
  pqCMBSceneObjectBase* duplicate(
    pqServer* server, pqRenderView* view, bool updateRep = true) override;

  bool writeToFile();
  std::string getFileName() const;
  void setFileName(std::string& filename);

  bool isValidPolygon() const { return ValidPolygon; }

  //returns the min angle used to mesh this polygon
  //a min angle of zero means min angle was disabled
  double getMinAngle() const { return MinAngle; }

  //returns the edge length used to mesh this polygon
  //an edge legth of zero means the edge length was disabled
  double getEdgeLength() const { return EdgeLength; }

  const std::set<pqCMBArc*>& arcsUsedByPolygon() const { return this->InputArcs; }

  void updateRepresentation() override;

protected:
  //remesh the polygon, if the input arcs have changed.
  //returns if the polygon is valid
  bool remesh();

  //make pqCMBPolygon a friend class so that we can have
  //arcs and polygons refresh each other if needed
  friend class pqCMBArc;

  void addArc(pqCMBArc* arc);
  void removeArc(pqCMBArc* arc);
  void arcIsDirty(pqCMBArc* arc);

  double MinAngle;
  double EdgeLength;
  bool ValidPolygon;
  std::set<pqCMBArc*> InputArcs;
  bool NeedsRemesh;
  std::string FileName;
};
#endif /* __pqCMBPolygon_h */

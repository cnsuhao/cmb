/*=========================================================================

  Program:   CMB
  Module:    pqCMBPolygon.h

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
// .NAME pqCMBPolygon - represents a collection of contours that make
// a polygon
// .SECTION Description
// .SECTION Caveats

#ifndef __pqCMBPolygon_h
#define __pqCMBPolygon_h


#include "cmbAppCommonExport.h"
#include "pqCMBTexturedObject.h"
#include "cmbSystemConfig.h"
#include <set>
#include <vector>

class pqRenderView;
class pqCMBSceneNode;
class pqCMBArc;

class  CMBAPPCOMMON_EXPORT pqCMBPolygon : public pqCMBTexturedObject
{
  typedef pqCMBSceneObjectBase Superclass;
public:
  pqCMBPolygon(double minAngle, double edgeLength,
    std::vector<pqCMBSceneNode*> &inputNodes);
  virtual ~pqCMBPolygon();

  virtual bool isDefaultConstrained() const{return true;}

  virtual pqCMBSceneObjectBase::enumObjectType getType() const;
  virtual pqCMBSceneObjectBase *duplicate(pqServer *server, pqRenderView *view,
                                    bool updateRep = true);

  bool writeToFile();
  std::string getFileName() const;
  void setFileName(std::string &filename);

  bool isValidPolygon() const {return ValidPolygon;}

  //returns the min angle used to mesh this polygon
  //a min angle of zero means min angle was disabled
  double getMinAngle() const { return MinAngle; }

  //returns the edge length used to mesh this polygon
  //an edge legth of zero means the edge length was disabled
  double getEdgeLength() const { return EdgeLength; }

  const std::set<pqCMBArc*>& arcsUsedByPolygon() const
    { return this->InputArcs; }

  virtual void updateRepresentation();

protected:
  //remesh the polygon, if the input arcs have changed.
  //returns if the polygon is valid
  bool remesh();

  //make pqCMBPolygon a friend class so that we can have
  //arcs and polygons refresh each other if needed
  friend class pqCMBArc;

  void addArc( pqCMBArc* arc );
  void removeArc( pqCMBArc* arc );
  void arcIsDirty( pqCMBArc* arc );

  double MinAngle;
  double EdgeLength;
  bool ValidPolygon;
  std::set<pqCMBArc*> InputArcs;
  bool NeedsRemesh;
  std::string FileName;

};
#endif /* __pqCMBPolygon_h */

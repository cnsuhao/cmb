//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "pqCMBLine.h"

#include <pqRenderView.h>
#include <pqServer.h>
#include <vtkSMInputProperty.h>
#include <vtkSMIntVectorProperty.h>
#include <vtkSMNewWidgetRepresentationProxy.h>
#include <vtkSMPropertyHelper.h>
#include <vtkSMRepresentationProxy.h>
#include <vtkSMSourceProxy.h>

#include "smtk/extension/paraview/widgets/qtLineWidget.h"

//-----------------------------------------------------------------------------
pqCMBLine::pqCMBLine() : pqCMBSceneObjectBase()
{
  this->LineWidget = NULL;
}

//-----------------------------------------------------------------------------
pqCMBLine::~pqCMBLine() { delete this->LineWidget; }

//-----------------------------------------------------------------------------
void pqCMBLine::getDefaultBounds(
  pqRenderView* theView, double bounds[6])
{
  if(!theView)
    {
    return;
    }
  double focal[3];
  pqCMBSceneObjectBase::getCameraFocalPoint(theView, focal);
  bounds[0]=focal[0]-0.5;bounds[2]=focal[1]-0.5;bounds[4]=focal[2]-0.5;
  bounds[1]=focal[0]+0.5;bounds[3]=focal[1]+0.5;bounds[5]=focal[2]+0.5;
}

//-----------------------------------------------------------------------------
pqCMBLine::pqCMBLine(pqCMBSceneObjectBase* refObj,
                           pqServer *server,
                           pqRenderView *view,
                           bool updateRep)
{
  double bounds[6];
  pqCMBLine::getDefaultBounds(view, bounds);
  if (refObj && refObj->getType() != pqCMBSceneObjectBase::Line)
    {
    refObj->getBounds(bounds);
    }

  double pos1[3], pos2[3];
  pos1[0]=bounds[0];pos1[1]=bounds[2];pos1[2]=bounds[4];
  pos2[0]=bounds[1];pos2[1]=bounds[3];pos2[2]=bounds[5];

  this->initialize(pos1, pos2, server, view, updateRep);
}

//-----------------------------------------------------------------------------
pqCMBLine::pqCMBLine(double point1[3],
                           double point2[3],
                           pqServer *server,
                           pqRenderView *view,
                           bool updateRep)
{
  this->initialize(point1, point2, server, view, updateRep);
}
//-----------------------------------------------------------------------------
void pqCMBLine::initialize(double point1[3],
                              double point2[3],
                              pqServer *server,
                              pqRenderView *view,
                              bool /*updateRep*/)
{
  this->LineWidget = new qtLineWidget();
  this->LineWidget->setObjectName("pqCMBLineWidget");
  this->LineWidget->setView(view);
  this->LineWidget->setPoints(point1, point2);
  this->UserDefinedType = "Line";
}

//-----------------------------------------------------------------------------
void pqCMBLine::setVisibility(bool mode)
{
  if (this->LineWidget) {
    this->LineWidget->setEnableInteractivity(mode);
    }
}

//-----------------------------------------------------------------------------
void pqCMBLine::setSelectionInput(
  vtkSMSourceProxy *selectionInput)
{
  if(!selectionInput)
    {
    this->deselect();
    }
  else
    {
    this->select();
    }
}

//-----------------------------------------------------------------------------
void pqCMBLine::select()
{
  this->LineWidget->emphasize();
  this->LineWidget->setVisible(true);
}

//-----------------------------------------------------------------------------

void pqCMBLine::deselect()
{
  this->LineWidget->setVisible(false);
  this->LineWidget->deemphasize();
}

//-----------------------------------------------------------------------------
int pqCMBLine::getPointPosition( int pointIdx,
  double &x, double &y, double &z)
{
  if(!this->LineWidget)
    {
    return 0;
    }
  if(pointIdx != 1 && pointIdx != 2)
    {
    return 0;
    }

    double p1[3], p2[3];
    this->LineWidget->points(p1, p2);

    /// looking at old code, seems like pointIdx is 1-based.
    if (pointIdx == 1) {
      x = p1[0];
      y = p1[1];
      z = p1[2];
    } else {
      x = p2[0];
      y = p2[1];
      z = p2[2];
    }
    return 1;
}

//-----------------------------------------------------------------------------
pqCMBSceneObjectBase *pqCMBLine::duplicate(pqServer *server,
                                                  pqRenderView *view,
                                                  bool updateRep)
{
  double point1[3], point2[3];
  this->getPoint1Position(point1[0], point1[1], point1[2]);
  this->getPoint2Position(point2[0], point2[1], point2[2]);
  pqCMBLine *nobj = new pqCMBLine(point1, point2, server, view, updateRep);
  nobj->UserDefinedType = this->UserDefinedType;
  double color[4];
  this->getColor(color);
  nobj->setColor(color);
  return nobj;
}

//-----------------------------------------------------------------------------
void pqCMBLine::getColor(double color[4]) const
{
  QColor qcolor = this->LineWidget->color();
  color[0] = qcolor.redF();
  color[1] = qcolor.greenF();
  color[2] = qcolor.blueF();
  color[3] = 1.0; // Assume we don't support transparent lines for now
}

//-----------------------------------------------------------------------------
void pqCMBLine::setColor(double color[4], bool /*updateRep*/)
{
//  vtkSMPropertyHelper(this->LineWidget->getWidgetProxy(), "LineColor").Set(color, 3);
this->LineWidget->setLineColor(color);
}

//-----------------------------------------------------------------------------
void pqCMBLine::getBounds(double /*bounds*/[6]) const
{
  return;
  //this->LineWidget->getBounds(bounds);
}
//-----------------------------------------------------------------------------
void pqCMBLine::getDataBounds(double /*bounds*/[6]) const
{
  return;
  //this->getBounds(bounds);
}

//-----------------------------------------------------------------------------
pqCMBSceneObjectBase::enumObjectType pqCMBLine::getType() const
{
  return pqCMBSceneObjectBase::Line;
}
//-----------------------------------------------------------------------------

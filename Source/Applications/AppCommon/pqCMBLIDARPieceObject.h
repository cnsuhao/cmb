/*=========================================================================

  Program:   CMB
  Module:    pqCMBLIDARPieceObject.h

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
// .NAME pqCMBLIDARPieceObject - represents a 3D LIDAR preview piece object.
// .SECTION Description
// .SECTION Caveats

#ifndef __pqCMBLIDARPieceObject_h
#define __pqCMBLIDARPieceObject_h

#include "cmbAppCommonExport.h"
#include <QPointer>
#include "vtkBoundingBox.h"
#include <string>
#include "cmbSystemConfig.h"

class pqPipelineSource;
class pqDataRepresentation;
class pqRenderView;
class pqServer;
class vtkTransform;
class vtkSMProxy;
class vtkSMSourceProxy;

class  CMBAPPCOMMON_EXPORT pqCMBLIDARPieceObject
{
public:
  pqCMBLIDARPieceObject();
  virtual ~pqCMBLIDARPieceObject();

  void setSource(pqPipelineSource *source);
  pqPipelineSource * getSource() const;

  void setThresholdSource(pqPipelineSource *thresholdSource);
  pqPipelineSource * getThresholdSource() const;
  void setContourSource(pqPipelineSource *thresholdSource);
  pqPipelineSource * getContourSource() const;
  pqPipelineSource * getDiggerSource() const;
  pqPipelineSource * getElevationSource() const;

  void setRepresentation(pqDataRepresentation *rep);
  pqDataRepresentation * getRepresentation() const;

  void setFileName(const char *filename)
    {this->FileName = filename;}
  std::string getFileName() const
    {return this->FileName;}

  void setPieceName(const char *name)
    {this->PieceName = name;}
  std::string getPieceName() const
    {return this->PieceName;}

  //void setVisibility(bool mode);
  void updateRepresentation();

  void setElevationFilterLowPoint(double *lowPoint);
  void setElevationFilterHighPoint(double *lowPoint);
  void useElevationFilter(bool useElevation);

  void setPieceIndex(int pieceIdx);
  int getPieceIndex() { return this->PieceIndex;}
  int getDisplayOnRatio() {return this->DisplayOnRatio;}
  void setDisplayOnRatio(int ratio);
  int getSaveOnRatio() {return this->SaveOnRatio;}
  void setSaveOnRatio(int ratio);
  int getNumberOfPoints() { return this->TotalNumberOfPiecePoints;}
  void setNumberOfPoints(int numPts)
    { this->TotalNumberOfPiecePoints = numPts;}
  //int getFileOffset() {return this->FileOffset;}
  //void setFileOffset(int offset)
  //  { this->FileOffset = offset;}
  void setLODMode(int mode, bool updateRep);

  int getReadOnRatio() {return this->ReadOnRatio;}
  void setReadOnRatio(int ratio);

  int getNumberOfReadPoints()
    { return this->NumberOfReadPoints; }
  void setNumberOfReadPoints(int numPoints)
    { this->NumberOfReadPoints = numPoints; }

  //QTableWidgetItem* getWidget()
  //{ return this->Widget;}
  //void setWidget(QTableWidgetItem* widget)
  //{ this->Widget = Widget; }

  static pqCMBLIDARPieceObject *createObject(pqPipelineSource *source,
                                            double *bounds,
                                            pqServer *server, pqRenderView *view,
                                            bool updateRep = true);

  void zoomOnObject();
  void getBounds(double bounds[6]) const;
  int getVisibility()
    { return this->Visibility; }
  void setVisibility(int visible);
  int getHighlight()
    { return this->Highlight; }
  void setHighlight( int highlight);
  void setSelectionInput(vtkSMSourceProxy* selInput);

  void setClipBounds(double xMin, double xMax, double yMin, double yMax,
    double zMin, double zMax)
    { this->ClippingBounds.SetBounds(xMin, xMax, yMin, yMax, zMin, zMax); }
  void setClipBounds(double bounds[6])
    { this->ClippingBounds.SetBounds(bounds); }
  bool areClippingBoundsEqual(vtkBoundingBox &bbox)
    { return bbox == this->ClippingBounds; }
  bool isClipTransformationUnchanged();
  bool isThresholdTransformationUnchanged();

  bool getClipState()
    { return this->ClipState; }
  void setClipState(bool clipState)
    { this->ClipState = clipState; }

  bool isObjectUpToDate(bool clipEnabled, vtkBoundingBox &clipBBox);
  bool isClipUpToDate(bool clipEnabled, vtkBoundingBox &clipBBox);

  void setNumberOfDisplayPointsEstimate(int numPoints)
    { this->NumberOfDisplayPointsEstimate = numPoints; }
  int getNumberOfDisplayPointsEstimate()
    { return this->NumberOfDisplayPointsEstimate; }

  void setNumberOfSavePointsEstimate(int numPoints)
    { this->NumberOfSavePointsEstimate = numPoints; }
  int getNumberOfSavePointsEstimate()
    { return this->NumberOfSavePointsEstimate; }

  void getPosition(double pos[3]) const;
  void setPosition(double pos[3], bool updateRep = true);
  void getOrientation(double ori[3]) const;
  void setOrientation(double ori[3], bool updateRep = true);
  void getScale(double scale[3]) const;
  void setScale(double scale[3], bool updateRep = true);
  void getOrigin(double origin[3]) const;
  void setOrigin(double origin[3], bool updateRep = true);

  void saveClipPosition();
  void saveClipOrientation();
  void saveClipScale();
  void saveClipOrigin();

  void saveThresholdPosition();
  void saveThresholdOrientation();
  void saveThresholdScale();
  void saveThresholdOrigin();

  bool isPieceTransformed();
  void getTransform(vtkTransform *transform) const;

  void updateThresholdUseFilter(int idx, int useFilter);
  void updatePolygonUseFilter(int idx, int useFilter);
  void updatePolygonInvert(int idx, int invert);
  void updatePolygonROI(int idx, int roi);

  void updateGroupInvert(int groupIdx, int invert);

  void resetWithNoThresholds(bool update=true);
  void resetWithNoContours(bool update=true);

  void addThreshold();
  void addContour(vtkSMProxy* implicitLoop);
  void removeThreshold();
  void removeContour(vtkSMProxy* implicitLoop);

  void clearThresholds();
  void clearContours();
  bool hasActiveFilters();
  void setActiveContourGroup(int);

protected:

  void init();
  void updateProxyProperty(pqPipelineSource*, const char* name,
    int idx, int val);

  QPointer<pqPipelineSource> Source;
  QPointer<pqPipelineSource> ContourSource;
  QPointer<pqPipelineSource> DiggerSource;
  QPointer<pqPipelineSource> ThresholdSource;
  QPointer<pqPipelineSource> ElevationSource;
  QPointer<pqDataRepresentation> Representation;
  std::string FileName;
  std::string PieceName;
  int PieceIndex;
  int DisplayOnRatio;
  int SaveOnRatio;
  int ReadOnRatio;
  int TotalNumberOfPiecePoints;
  int NumberOfReadPoints;
  int NumberOfDisplayPointsEstimate;
  int NumberOfSavePointsEstimate;
  //int FileOffset;
  int Visibility;
  int Highlight;
  //QTableWidgetItem* Widget;
  double HighlightColor[3];
  double OriginalColor[3];
  bool ClipState;
  bool UseElevationFilter;
  vtkBoundingBox ClippingBounds;

  double ClipOrigin[3];
  double ClipPosition[3];
  double ClipScale[3];
  double ClipOrientation[3];

  double ThresholdOrigin[3];
  double ThresholdPosition[3];
  double ThresholdScale[3];
  double ThresholdOrientation[3];
};

#endif /* __pqCMBLIDARPieceObject_h */

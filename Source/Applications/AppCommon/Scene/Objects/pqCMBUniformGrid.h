//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqCMBUniformGrid - represents a 3D Scene object that is an uniform grid object.
// .SECTION Description
// .SECTION Caveats

#ifndef __pqCMBUniformGrid_h
#define __pqCMBUniformGrid_h

#include "cmbSystemConfig.h"
#include "pqCMBSceneObjectBase.h"

class CMBAPPCOMMON_EXPORT pqCMBUniformGrid : public pqCMBSceneObjectBase
{
public:
  pqCMBUniformGrid();
  pqCMBUniformGrid(pqPipelineSource* source, pqRenderView* view, pqServer* server,
    const char* filename, bool updateRep = true);
  pqCMBUniformGrid(
    const char* filename, pqServer* server, pqRenderView* view, bool updateRep = true);
  pqCMBUniformGrid(
    pqPipelineSource* source, pqServer* server, pqRenderView* view, bool updateRep = true);

  ~pqCMBUniformGrid() override;

  pqCMBSceneObjectBase* duplicate(
    pqServer* server, pqRenderView* view, bool updateRep = true) override;

  void setFileName(const char* type) { this->FileName = type; }
  enumObjectType getType() const override;
  std::string getFileName() const { return this->FileName; }

  void getDimensions(vtkIdType dims[2]);

  static bool isRawDEM(const QString& filename);
  bool isRawDEM() const;
  void setReadGroupOfFiles(bool mode);
  bool getReadGroupOfFiles() const;
  void setOnRatio(int r);
  int getOnRatio() const;
  bool hasInvalidValue();
  double invalidValue();
  void setExtents(vtkIdType rowExtents[2], vtkIdType columnExtents[2]);
  void getExtents(vtkIdType rowExtents[2], vtkIdType columnExtents[2]) const;

  void getAreaStats(double* areaStats) override;
  void getGeometryBounds(double* geoBounds) const override;
  void getPolySideStats(double* polySide) override;
  double getSurfaceArea() override;
  vtkIdType getNumberOfPoints() override;
  vtkIdType getNumberOfPolygons() override;
  void updatePolyDataStats() override;

  pqPipelineSource* getImageSource() { return ImageSource; }

protected:
  void prepGridObject(pqServer* server, pqRenderView* view, bool updateRep, bool transferColor);

  pqPipelineSource* ImageSource;

  std::string FileName;
  bool HasInvalidValue;
};

#endif /* __pqCMBUniformGrid_h */

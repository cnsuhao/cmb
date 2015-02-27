/*=========================================================================

  Program:   CMB
  Module:    pqCMBUniformGrid.h

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
// .NAME pqCMBUniformGrid - represents a 3D Scene object that is an uniform grid object.
// .SECTION Description
// .SECTION Caveats


#ifndef __pqCMBUniformGrid_h
#define __pqCMBUniformGrid_h

#include "pqCMBSceneObjectBase.h"
#include "cmbSystemConfig.h"

class  CMBAPPCOMMON_EXPORT pqCMBUniformGrid : public pqCMBSceneObjectBase
{
public:

  pqCMBUniformGrid();
  pqCMBUniformGrid(pqPipelineSource *source,
                      pqRenderView *view,
                      pqServer *server,
                      const char *filename,
                      bool updateRep=true);
  pqCMBUniformGrid(const char *filename,
                 pqServer *server, pqRenderView *view,
                 bool updateRep = true);
  pqCMBUniformGrid(pqPipelineSource *source,
                 pqServer *server, pqRenderView *view,
                 bool updateRep = true);

  virtual ~pqCMBUniformGrid();


  virtual pqCMBSceneObjectBase *duplicate(pqServer *server, pqRenderView *view,
                                    bool updateRep = true);

  void setFileName(const char *type)
    {this->FileName = type;}
  virtual enumObjectType getType() const;
  std::string getFileName() const
    {return this->FileName;}

  void getDimensions(vtkIdType dims[2]);

  static bool isRawDEM(const QString &filename);
  bool isRawDEM() const;
  void setReadGroupOfFiles(bool mode);
  bool getReadGroupOfFiles() const;
  void setOnRatio(int r);
  int getOnRatio() const;
  bool hasInvalidValue();
  double invalidValue();
  void setExtents(vtkIdType rowExtents[2], vtkIdType columnExtents[2]);
  void getExtents(vtkIdType rowExtents[2], vtkIdType columnExtents[2]) const;

  virtual void getAreaStats(double* areaStats);
  virtual void getGeometryBounds(double* geoBounds) const;
  virtual void getPolySideStats(double* polySide);
  virtual double getSurfaceArea();
  virtual vtkIdType getNumberOfPoints();
  virtual vtkIdType getNumberOfPolygons();
  virtual void updatePolyDataStats();

  pqPipelineSource* getImageSource()
  { return ImageSource; }

protected:
  void prepGridObject(pqServer *server, pqRenderView *view, bool updateRep);

  pqPipelineSource* ImageSource;

  std::string FileName;
  bool HasInvalidValue;
};

#endif /* __pqCMBUniformGrid_h */

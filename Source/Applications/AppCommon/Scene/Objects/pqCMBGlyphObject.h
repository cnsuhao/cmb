/*=========================================================================

  Program:   CMB
  Module:    pqCMBGlyphObject.h

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
// .NAME pqCMBGlyphObject - represents a 3D Scene object that is a facted object.
// .SECTION Description
// .SECTION Caveats


#ifndef __pqCMBGlyphObject_h
#define __pqCMBGlyphObject_h

#include "pqCMBSceneObjectBase.h"
#include "cmbSystemConfig.h"
class vtkSMCMBGlyphPointSourceProxy;

class  CMBAPPCOMMON_EXPORT pqCMBGlyphObject : public pqCMBSceneObjectBase
{
  typedef pqCMBSceneObjectBase Superclass;
public:

  pqCMBGlyphObject();
  pqCMBGlyphObject(pqPipelineSource *glyphSource,
                      pqRenderView *view,
                      pqServer *server,
                      const char *glyphFilename,
                      const char *pointsFileName,
                      bool updateRep = true);

  pqCMBGlyphObject(pqPipelineSource *glyphSource,
                      pqRenderView *view,
                      pqServer *server,
                      const char *glyphFilename,
                      bool updateRep = true);

  pqCMBGlyphObject(const char *glyphFilename,
                      pqServer *server, pqRenderView *view,
                      bool updateRep = true);

  pqCMBGlyphObject(const char *glyphFilename,
                      const char *pointsFilename,
                      pqServer *server, pqRenderView *view,
                      bool updateRep = true);

  virtual ~pqCMBGlyphObject();


  virtual pqCMBSceneObjectBase *duplicate(pqServer *server, pqRenderView *view,
                                    bool updateRep = true);

  virtual enumObjectType getType() const;

  std::string getGlyphFileName() const
    {return this->GlyphFileName;}

  std::string getPointsFileName() const
    {return this->PointsFileName;}
  void setPointsFileName(const char *fname)
    {this->PointsFileName = fname;}

  void setGlyphSource(pqPipelineSource *source);
  pqPipelineSource * getGlyphSource() const;
  virtual void clearSelectedPointsColor();
  void insertNextPoint(double *p);
  vtkIdType getNumberOfPoints() const;
  void getAveragePoint(double *pa) const;
  void getPoint(vtkIdType i, double *p) const;
  void setPoint(vtkIdType i, double *p);

  using pqCMBSceneObjectBase::getScale;
  using pqCMBSceneObjectBase::setScale;
  void getScale(vtkIdType i, double *s) const;
  void setScale(vtkIdType i, double *s);

  void getOrientation(vtkIdType i, double *o) const;
  void setOrientation(vtkIdType i, double *o);

  // Write the points to the name specified in PointsFileName
  void writePointsFile() const;

  void setSurfaceType(enumSurfaceType objtype)
    {this->SurfaceType = objtype;}
  enumSurfaceType getSurfaceType() const
  {return this->SurfaceType;}
  std::string getSurfaceTypeAsString() const;
  // Overwrite the databounds
  virtual void getDataBounds(double bounds[6]) const
    {this->getBounds(bounds);}

  virtual void getColor(double color[4]) const;
  virtual void setColor(double color[4], bool updateRep = true);
  virtual void applyTransform(double scaleDelta[3],
                              double orientationDelta[3],
                              double translationDelta[3]);
  void copyAttributes(pqCMBSceneObjectBase*);

  // Duplicate the pipeline source of the object being glyphed
  pqPipelineSource *duplicateGlyphPipelineSource(pqServer *server);


protected:
  QPointer<pqPipelineSource> GlyphSource;
  void initialize(pqRenderView *view, pqServer *server, bool updateRep = true);
  std::string GlyphFileName;
  std::string PointsFileName;
  vtkSMCMBGlyphPointSourceProxy *SourceProxy;
  enumSurfaceType SurfaceType;
};

#endif /* __pqCMBGlyphObject_h */

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
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

  ~pqCMBGlyphObject() override;


  pqCMBSceneObjectBase *duplicate(pqServer *server, pqRenderView *view,
                                    bool updateRep = true) override;

  enumObjectType getType() const override;

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
  vtkIdType getNumberOfPoints() override;
  void getAveragePoint(double *pa);
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
  void getDataBounds(double bounds[6]) const override
    {this->getBounds(bounds);}

  void getColor(double color[4]) const override;
  void setColor(double color[4], bool updateRep = true) override;
  void applyTransform(double scaleDelta[3],
                              double orientationDelta[3],
                              double translationDelta[3]) override;
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

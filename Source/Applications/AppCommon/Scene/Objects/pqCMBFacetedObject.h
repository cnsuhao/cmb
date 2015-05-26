//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqCMBFacetedObject - represents a 3D Scene object that is a facted object.
// .SECTION Description
// .SECTION Caveats


#ifndef __pqCMBFacetedObject_h
#define __pqCMBFacetedObject_h

#include "pqCMBTexturedObject.h"
#include "cmbSystemConfig.h"

class  CMBAPPCOMMON_EXPORT pqCMBFacetedObject : public pqCMBTexturedObject
{
public:

  pqCMBFacetedObject();
  pqCMBFacetedObject(pqPipelineSource *source,
                         pqRenderView *view,
                         pqServer *server,
                         const char *filename);
  pqCMBFacetedObject(const char *filename,
                 pqServer *server, pqRenderView *view,
                 bool updateRep = true);
  pqCMBFacetedObject(pqPipelineSource *source,
                 pqServer *server, pqRenderView *view,
                 bool updateRep = true);

  virtual ~pqCMBFacetedObject();


  virtual pqCMBSceneObjectBase *duplicate(pqServer *server, pqRenderView *view,
                                    bool updateRep = true);

  pqPipelineSource * getTransformedSource(pqServer *server) const;

  void setFileName(const char *type)
    {this->FileName = type;}
  virtual enumObjectType getType() const;
  std::string getFileName() const
    {return this->FileName;}
  void setSurfaceType(enumSurfaceType objtype)
    {this->SurfaceType = objtype;}
  enumSurfaceType getSurfaceType() const
  {return this->SurfaceType;}
  std::string getSurfaceTypeAsString() const;
  // Duplicate the pipeline source of the object
  pqPipelineSource *duplicatePipelineSource(pqServer *server);

protected:
  void prepFacetedObject(pqServer *server, pqRenderView *view);

  enumSurfaceType SurfaceType;
  std::string FileName;
};

#endif /* __pqCMBFacetedObject_h */

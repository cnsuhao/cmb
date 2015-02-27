/*=========================================================================

  Program:   CMB
  Module:    pqCMBFacetedObject.h

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

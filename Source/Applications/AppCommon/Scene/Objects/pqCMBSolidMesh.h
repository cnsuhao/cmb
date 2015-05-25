//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME pqCMBSolidMesh - represents a 3D Scene object that is a volume mesh.
// .SECTION Description
// .SECTION Caveats


#ifndef __pqCMBSolidMesh_h
#define __pqCMBSolidMesh_h

#include "pqCMBSceneObjectBase.h"
#include "cmbSystemConfig.h"

class  CMBAPPCOMMON_EXPORT pqCMBSolidMesh : public pqCMBSceneObjectBase
{
public:

  pqCMBSolidMesh();
  pqCMBSolidMesh(pqPipelineSource *source,
                         pqRenderView *view,
                         pqServer *server,
                         const char *filename);
  pqCMBSolidMesh(const char *filename,
                 pqServer *server, pqRenderView *view,
                 bool updateRep = true);
  pqCMBSolidMesh(pqPipelineSource *source,
                 pqServer *server, pqRenderView *view,
                 bool updateRep = true);

  virtual ~pqCMBSolidMesh();


  virtual pqCMBSceneObjectBase *duplicate(pqServer *server, pqRenderView *view,
                                    bool updateRep = true);

  pqPipelineSource * getTransformedSource(pqServer *server) const;

  void setFileName(const char *type)
    {this->FileName = type;}
  virtual enumObjectType getType() const;
  std::string getFileName() const
    {return this->FileName;}

protected:
  void prepSolidMesh(pqServer *server,
    pqRenderView *view,bool updateRep);

  std::string FileName;
};

#endif /* __pqCMBSolidMesh_h */

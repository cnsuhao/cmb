/*=========================================================================

Copyright (c) 2013 Kitware Inc. 28 Corporate Drive,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced, distributed,
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
// .NAME vtkCMBStandaloneModelFaceMesh.h - Helper class for running test outside of CMB apps
// .SECTION Description
//     Functions as a subclass of vtkCMBModelEntityMesh, implemented as a subclass of
//     vtkCMBModelFaceMeshServer so that I don't have to implement a bunch of pure
//     virtual methods.
// .SECTION See Also

#ifndef __vtkCMBStandaloneModelFaceMesh_h
#define __vtkCMBStandaloneModelFaceMesh_h

#include "vtkCMBModelFaceMeshServer.h"
#include "cmbSystemConfig.h"

class vtkCMBStandaloneModelFaceMesh : public vtkCMBModelFaceMeshServer
{
 public:
  static vtkCMBStandaloneModelFaceMesh* New();
  vtkTypeMacro(vtkCMBStandaloneModelFaceMesh,vtkCMBModelFaceMeshServer);

  virtual void Initialize(vtkCMBMesh* mesh, vtkModelFace* face);
 protected:
  vtkCMBStandaloneModelFaceMesh();
  virtual ~vtkCMBStandaloneModelFaceMesh();
 private:
  vtkCMBStandaloneModelFaceMesh(const vtkCMBStandaloneModelFaceMesh&);  // Not implemented.
  void operator=(const vtkCMBStandaloneModelFaceMesh&);  // Not implemented.
};

#endif

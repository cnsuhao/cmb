/*=========================================================================

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
// .NAME vtkCMBModelVertexMesh - Mesh representation for a vtkModelVertex
// .SECTION Description

#ifndef __vtkCMBModelVertexMesh_h
#define __vtkCMBModelVertexMesh_h

#include "vtkCMBModelEntityMesh.h"
#include "cmbSystemConfig.h"

class vtkModelVertex;

class VTK_EXPORT vtkCMBModelVertexMesh : public vtkCMBModelEntityMesh
{
public:
  static vtkCMBModelVertexMesh* New();
  vtkTypeMacro(vtkCMBModelVertexMesh,vtkCMBModelEntityMesh);
  void PrintSelf(ostream& os, vtkIndent indent);
  // Description:
  // Get the actual length the model edge will be meshed with.
  // 0 indicates no length has been set.
  virtual double GetActualLength()
  {
    return this->GetLength();
  }

  // Description:
  // Set the local mesh length on the entity.
  virtual bool SetLocalLength(double len)
  {
    this->SetLength(len);
    return true;
  }

//BTX
  virtual vtkModelGeometricEntity* GetModelGeometricEntity();

  void Initialize(vtkCMBMesh* mesh, vtkModelVertex* vertex);
//ETX
  // Description:
  // BuildModelEntityMesh will generate a mesh for the associated
  // model entity.  If meshHigherDimensionalEntities is set to true
  // it will also mesh any higher dimensional entities which need
  // to be meshed because of this object getting meshed.
  bool BuildModelEntityMesh(bool meshHigherDimensionalEntities);

  // Description:
  // Return true if the model entity should have a mesh
  // and false otherwise.
  virtual bool IsModelEntityMeshed()
  {
    return false;
  }

protected:
  vtkCMBModelVertexMesh();
  virtual ~vtkCMBModelVertexMesh();

private:
  vtkCMBModelVertexMesh(const vtkCMBModelVertexMesh&);  // Not implemented.
  void operator=(const vtkCMBModelVertexMesh&);  // Not implemented.

  vtkModelVertex* ModelVertex;
};

#endif


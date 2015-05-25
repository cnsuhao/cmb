//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkModelVertexUse - Abstract generic model entity class.
// .SECTION Description

#ifndef __vtkModelVertexUse_h
#define __vtkModelVertexUse_h

#include "vtkDiscreteModelModule.h" // For export macro
#include "vtkModelEntity.h"
#include "cmbSystemConfig.h"

class vtkModelEdgeUse;
class vtkModelVertex;

class VTKDISCRETEMODEL_EXPORT vtkModelVertexUse : public vtkModelEntity
{
public:
  static vtkModelVertexUse* New();
  vtkTypeMacro(vtkModelVertexUse,vtkModelEntity);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual int GetType();

  vtkModelVertex* GetModelVertex();

  int GetNumberOfModelEdgeUses();
  vtkModelItemIterator* NewModelEdgeUseIterator();

  using Superclass::Initialize;
  void Initialize(vtkModelVertex* vertex);

  // Description:
  // Reads the state of an instance from an archive OR
  // writes the state of an instance to an archive. See
  // the documentation for this class for details.
  virtual void Serialize(vtkSerializer* ser);

protected:
  vtkModelVertexUse();
  virtual ~vtkModelVertexUse();


  void AddModelEdgeUse(vtkModelEdgeUse* edgeUse);
  void RemoveModelEdgeUse(vtkModelEdgeUse* edgeUse);
//BTX
  friend class vtkModelEdge;
  friend class vtkModelEdgeUse;
//ETX

  virtual bool Destroy();
  // for destroying
//BTX
  friend class vtkModelFace;
  friend class vtkModelFaceUse;
  friend class vtkModelVertex;
//ETX

private:
  vtkModelVertexUse(const vtkModelVertexUse&);  // Not implemented.
  void operator=(const vtkModelVertexUse&);  // Not implemented.
};

#endif


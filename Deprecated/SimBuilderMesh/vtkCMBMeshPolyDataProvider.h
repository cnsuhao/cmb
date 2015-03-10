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
// .NAME vtkCMBMeshPolyDataProvider - Algorithm for getting a polydata from CmbMesh
// .SECTION Description
// Given the vtkCMBMeshServer, a model entity Id and optionally an entity type,
// creates a filter output of the polydata used to represent the
// analysis mesh of the model entity.

#ifndef __vtkCMBMeshPolyDataProvider_h
#define __vtkCMBMeshPolyDataProvider_h

#include "vtkPolyDataAlgorithm.h"
#include "cmbSystemConfig.h"

class vtkCMBMeshWrapper;
class vtkDataSet;
class vtkPolyData;

class VTK_EXPORT vtkCMBMeshPolyDataProvider : public vtkPolyDataAlgorithm
{
public:
  static vtkCMBMeshPolyDataProvider *New();
  vtkTypeMacro(vtkCMBMeshPolyDataProvider,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  void SetMeshWrapper(vtkCMBMeshWrapper*);
  vtkGetMacro(MeshWrapper, vtkCMBMeshWrapper*);

  // Description:
  // Set/get functions to set the model item type.  This does not need
  // to be set in order to get the desired vtkPolyData for a model entity
  // but it makes the search faster.  If ItemType is set to
  // vtkModelType (0), then the master vtkPolyData is returned
  // in the output of the filter.
  void SetItemType(int itemType);
  vtkGetMacro(ItemType, int);

  // Description:
  // Set/get functions for the model entity id.  This does not need to be
  // set and is not used if ItemType is set to vtkModelType.
  void SetEntityId(vtkIdType Id);
  vtkGetMacro(EntityId, vtkIdType);

  // Description:
  // Setting controls whether or not to create vertex for model edges
  vtkBooleanMacro(CreateEdgePointVerts, bool);
  vtkSetMacro(CreateEdgePointVerts, bool);
  vtkGetMacro(CreateEdgePointVerts, bool);

//BTX
protected:
  vtkCMBMeshPolyDataProvider();
  ~vtkCMBMeshPolyDataProvider();

  // Description:
  // This is called by the superclass.
  // This is the method you should override.
  virtual int RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector);

  // Description:
  // This is called by the superclass.
  // This is the method you should override.
  virtual int RequestUpdateExtent(vtkInformation*,
                                  vtkInformationVector**,
                                  vtkInformationVector*);

  // see algorithm for more info
  virtual int FillOutputPortInformation(int port, vtkInformation* info);

private:
  vtkCMBMeshPolyDataProvider(const vtkCMBMeshPolyDataProvider&);  // Not implemented.
  void operator=(const vtkCMBMeshPolyDataProvider&);  // Not implemented.

  vtkCMBMeshWrapper* MeshWrapper;
  int ItemType;
  bool ItemTypeIsSet;
  vtkIdType EntityId;
  bool EntityIdIsSet;
  bool CreateEdgePointVerts;
//ETX
};

#endif

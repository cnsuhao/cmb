//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __vtkPVSMTKModelSource_h
#define __vtkPVSMTKModelSource_h

#include "ModelBridgeClientModule.h"

#include "smtk/extension/vtk/vtkMeshMultiBlockSource.h"
#include "smtk/PublicPointerDefs.h"

#include <map>
#include <string>

class vtkModelManagerWrapper;

/**\brief An VTK source for passing SMTK Model Manager to smtk mesh source.
  *
  * This filter generates a single block per UUID, for every UUID
  * in model manager with a tessellation entry.
  */
class MODELBRIDGECLIENT_EXPORT vtkPVSMTKMeshSource : public vtkMeshMultiBlockSource
{
public:
  static vtkPVSMTKMeshSource* New();
  virtual void PrintSelf(ostream& os, vtkIndent indent);
  vtkTypeMacro(vtkPVSMTKMeshSource,vtkMeshMultiBlockSource);

  // Description:
  // Set model manager wrapper
  void SetModelManagerWrapper(vtkModelManagerWrapper *modelManager);
  vtkGetObjectMacro(ModelManagerWrapper, vtkModelManagerWrapper);

  // Description:
  // Model entity ID that this source will be built upon.
  // Forwarded to vtkMeshMultiBlockSource
  virtual void SetModelEntityID(const char*);
  virtual char* GetModelEntityID();

  // Description:
  // Forwarded to vtkMeshMultiBlockSource
  // Mesh collection ID that this source will be built upon.
  // Forwarded to vtkMeshMultiBlockSource
  virtual void SetMeshCollectionID(const char*);
  virtual char* GetMeshCollectionID();

  void MarkDirty() {this->Dirty();}
protected:
  vtkPVSMTKMeshSource();
  virtual ~vtkPVSMTKMeshSource();

  virtual int RequestData(
    vtkInformation* request,
    vtkInformationVector** inInfo,
    vtkInformationVector* outInfo);

  // Reference model Manager wrapper:
  vtkModelManagerWrapper* ModelManagerWrapper;

private:

  vtkPVSMTKMeshSource(const vtkPVSMTKMeshSource&); // Not implemented.
  void operator = (const vtkPVSMTKMeshSource&); // Not implemented.
};

#endif // __vtkPVSMTKModelSource_h

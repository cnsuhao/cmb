//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkPVSMTKMeshInformation - Light object for holding information
// about a smtk meshset.
// .SECTION Description
// .SECTION Caveats

#ifndef __vtkPVSMTKMeshInformation_h
#define __vtkPVSMTKMeshInformation_h

#include "ModelBridgeClientModule.h"
#include "smtk/mesh/MeshSet.h"
#include "vtkPVInformation.h"
#include <map>
#include <string>

class MODELBRIDGECLIENT_EXPORT vtkPVSMTKMeshInformation : public vtkPVInformation
{
public:
  static vtkPVSMTKMeshInformation* New();
  vtkTypeMacro(vtkPVSMTKMeshInformation, vtkPVInformation);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Transfer information about a single object into this object.
  void CopyFromObject(vtkObject*) override;

  // Description:
  // Merge another information object. Calls AddInformation(info, 0).
  void AddInformation(vtkPVInformation* info) override;

  void CopyToStream(vtkClientServerStream*) override { ; }
  void CopyFromStream(const vtkClientServerStream*) override { ; }

  // Description:
  // return the blockid given a meshet.
  // Caution: This will be slow if there are many blocks in the mesh
  virtual bool GetBlockId(const smtk::mesh::MeshSet& mesh, unsigned int& bid);
  // return the meshset given a blockid.
  // Caution: There is no valid check for this for performance reason
  virtual const smtk::mesh::MeshSet& GetMeshSet(unsigned int bid);

  virtual const smtk::common::UUID& GetModelUUID();
  virtual const smtk::common::UUID& GetMeshCollectionID();

  // Description:
  // return Mesh to BlockId map for all blocks
  const std::map<smtk::mesh::MeshSet, vtkIdType>& GetMesh2BlockIdMap() const
  {
    return this->Mesh2BlockIdMap;
  }

protected:
  vtkPVSMTKMeshInformation();
  ~vtkPVSMTKMeshInformation() override;

  std::map<smtk::mesh::MeshSet, vtkIdType> Mesh2BlockIdMap;
  std::map<unsigned int, smtk::mesh::MeshSet> BlockId2MeshMap;
  smtk::common::UUID m_ModelUUID;
  smtk::common::UUID m_MeshCollectionId;

private:
  vtkPVSMTKMeshInformation(const vtkPVSMTKMeshInformation&); // Not implemented
  void operator=(const vtkPVSMTKMeshInformation&);           // Not implemented
};

#endif

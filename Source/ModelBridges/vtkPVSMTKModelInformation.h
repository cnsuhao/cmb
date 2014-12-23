/*=========================================================================

  Program:   ParaView
  Module:    vtkPVSMTKModelInformation.h

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkPVSMTKModelInformation - Light object for holding information
// about a smtk model object.
// .SECTION Description
// .SECTION Caveats

#ifndef __vtkPVSMTKModelInformation_h
#define __vtkPVSMTKModelInformation_h

#include "ModelBridgeClientModule.h"
#include "vtkPVInformation.h"
#include <string>
#include <map>
#include "smtk/common/UUID.h"

class MODELBRIDGECLIENT_EXPORT vtkPVSMTKModelInformation : public vtkPVInformation
{
public:
  static vtkPVSMTKModelInformation* New();
  vtkTypeMacro(vtkPVSMTKModelInformation, vtkPVInformation);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Transfer information about a single object into this object.
  virtual void CopyFromObject(vtkObject*);

  // Description:
  // Merge another information object. Calls AddInformation(info, 0).
  virtual void AddInformation(vtkPVInformation* info);

  virtual void CopyToStream(vtkClientServerStream*){;}
  virtual void CopyFromStream(const vtkClientServerStream*){;}

  // Description:
  // return the blockid given a entity UUID.
  virtual bool GetBlockId(const smtk::common::UUID& uuid, unsigned int& bid);
  virtual const smtk::common::UUID&  GetModelEntityId(unsigned int bid);

  // Description:
  // return UUIDs for all blocks
  virtual smtk::common::UUIDs GetBlockUUIDs() const;

  //BTX
protected:
  vtkPVSMTKModelInformation();
  ~vtkPVSMTKModelInformation();

  std::map<smtk::common::UUID, unsigned int> UUID2BlockIdMap;
  std::map<unsigned int, smtk::common::UUID> BlockId2UUIDMap;

private:

  vtkPVSMTKModelInformation(const vtkPVSMTKModelInformation&); // Not implemented
  void operator=(const vtkPVSMTKModelInformation&); // Not implemented
  
  smtk::common::UUID m_dummyID;
  //ETX
};

#endif

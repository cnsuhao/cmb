//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBArcEndNode
// .SECTION Description
// Light weight class that holds all the info
// needed for an arc end node

#ifndef __vtkCMBArcEndNode_h
#define __vtkCMBArcEndNode_h

#include "vtkCMBGeneralModule.h" // For export macro
#include "vtkType.h"
#include "vtkABI.h"
#include "cmbSystemConfig.h"

class VTKCMBGENERAL_EXPORT vtkCMBArcEndNode
{
  friend class vtkCMBArcManager;
public:
  vtkCMBArcEndNode(double position[3]);
  ~vtkCMBArcEndNode();

  //comparison operator needed for storage
  bool operator<(const vtkCMBArcEndNode &p) const;

  //Description:
  //Get the Id of this arc
  vtkIdType GetId() const;

  //Description:
  //Get the position of this end node
  void GetPosition(double pos[3]) const;

  //Description:
  //Get the position of this end node
  const double* GetPosition( ) const;

protected:
  void SetPosition(double* position);

  double Position[3];
private:
  const vtkIdType Id;
  static vtkIdType NextId;
};

#endif

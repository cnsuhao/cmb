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
// .NAME vtkCMBArcUpdateOperator - Update an Arc with the new polydata info
// .SECTION Description
// Operator to update an arcs shape after editing

#ifndef __vtkCMBArcUpdateOperator_h
#define __vtkCMBArcUpdateOperator_h

#include "vtkCMBFilteringModule.h" // For export macro
#include "vtkObject.h"
#include "vtkABI.h"
#include "cmbSystemConfig.h"

class vtkCMBArc;
class vtkPolyData;

class VTKCMBFILTERING_EXPORT vtkCMBArcUpdateOperator : public vtkObject
{
public:
  static vtkCMBArcUpdateOperator * New();
  vtkTypeMacro(vtkCMBArcUpdateOperator,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  //BTX
  enum EndNodeStatus
    {
    EndNodeDefault = -1,
    EndNodeMoved = 0,
    EndNodeRecreated = 1
    };
  //ETX

  //Description:
  //the arc to update with the polydata
  void SetArcId(vtkIdType arcId);
  vtkGetMacro(ArcId,vtkIdType);

  //Description:
  //Sets the end nodes flag to be moved
  //This means the end node itself will be moved and all other arcs that it connects too will be moved
  bool SetEndNodeToMove(int endNode);

  //Description:
  //Sets the end nodes flag to be recreated
  //This means the end node itself will be removed, and recreated at the new position
  //Any other arcs connected to this arc will not be connected after this happens
  bool SetEndNodeToRecreate(int endNode);

  //Description:
  //Change the default behavior of the operator for an end node
  //This is used when somebody hasn't called SetEndNodeToMove or SetEndNodeToRecreate
  //on an end node.
  //Note: Default behavior is move.
  vtkSetMacro(RecreateArcBehavior,int);

  //Description:
  // Sets/Gets the range with Start/End point Ids that the input
  // polydata will update.
  // Default values are -1, and whole arc will be updated.
  vtkSetMacro(StartPointId, vtkIdType);
  vtkGetMacro(StartPointId,vtkIdType);
  vtkSetMacro(EndPointId, vtkIdType);
  vtkGetMacro(EndPointId,vtkIdType);

  //Description:
  //Convert the passed in the polydata into an arc
  bool Operate(vtkPolyData *source);

protected:
  vtkCMBArcUpdateOperator();
  virtual ~vtkCMBArcUpdateOperator();

  // clears the operator back to default values
  void Reset();
  // Update the specified sub-arc with source polydata.
  virtual bool UpdateSubArc(vtkPolyData *source, vtkCMBArc* updatedArc);

  void UpdateEndNode(vtkCMBArc *arc, const int &endNodePos, double pos[3]);
  vtkIdType ArcId;
  int RecreateArcBehavior;
  int EndNodeFlags[2];
  vtkIdType StartPointId;
  vtkIdType EndPointId;

private:
  vtkCMBArcUpdateOperator(const vtkCMBArcUpdateOperator&);  // Not implemented.
  void operator=(const vtkCMBArcUpdateOperator&);  // Not implemented.
};

#endif

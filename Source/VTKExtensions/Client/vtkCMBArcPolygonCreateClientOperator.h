//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBArcPolygonCreateClientOperator - Create a vtkCMBArcPolygonProvider on the server
// .SECTION Description
// Create a vtkCMBArcPolygonProvider on the server by using the vtkCMBPolygonFromArcsOperator
// on the arc inputed to this Operator.

#ifndef __vtkCMBArcPolygonCreateClientOperator_h
#define __vtkCMBArcPolygonCreateClientOperator_h

#include "cmbSystemConfig.h"
#include "vtkCMBClientModule.h" // For export macro
#include "vtkSMSourceProxy.h"
#include <list>

class vtkIdTypeArray;

class VTKCMBCLIENT_EXPORT vtkCMBArcPolygonCreateClientOperator : public vtkSMSourceProxy
{
public:
  static vtkCMBArcPolygonCreateClientOperator* New();
  vtkTypeMacro(vtkCMBArcPolygonCreateClientOperator, vtkSMSourceProxy);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  //Description:
  // Returns the arc ids of the newly created polygon
  vtkGetObjectMacro(ArcIds, vtkIdTypeArray);

  //Description:
  // Add an arc to the collection of arcs to make the polygon from
  void AddArc(vtkIdType arcId);

  // Description:
  // Copies data from a widget proxy to a vtkCMBArc
  virtual bool Create(double minAngle, double edgeLegth, vtkSMProxy* providerProxy);

protected:
  vtkCMBArcPolygonCreateClientOperator();
  ~vtkCMBArcPolygonCreateClientOperator() override;
  vtkIdTypeArray* ArcIds;
  std::list<vtkIdType> InputArcIds;

private:
  vtkCMBArcPolygonCreateClientOperator(
    const vtkCMBArcPolygonCreateClientOperator&);              // Not implemented
  void operator=(const vtkCMBArcPolygonCreateClientOperator&); // Not implemented
};

#endif

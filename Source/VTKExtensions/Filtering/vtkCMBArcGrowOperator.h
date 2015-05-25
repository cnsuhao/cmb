//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBArcGrowOperator - Split an Arc
// .SECTION Description
// Operator to grow the current 'selection' of arcs

#ifndef __vtkCMBArcGrowOperator_h
#define __vtkCMBArcGrowOperator_h

#include "vtkCMBFilteringModule.h" // For export macro
#include "vtkObject.h"
#include "vtkABI.h"
#include "cmbSystemConfig.h"
#include <set>

class vtkIdTypeArray;
class VTKCMBFILTERING_EXPORT vtkCMBArcGrowOperator : public vtkObject
{
public:
  static vtkCMBArcGrowOperator * New();
  vtkTypeMacro(vtkCMBArcGrowOperator,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  //Description:
  //Add an Id of an arc to the set of arcs to grow from
  void AddArc(vtkIdType arcId);

  //Description:
  //Clear the input arcid collection
  void ClearInputArcs();

  //Description:
  //Calling this will clear the input arcs
  bool Operate();

  //Description:
  //If the grow works this these are the arc ids of the grow
  vtkGetObjectMacro(GrownArcSetIds,vtkIdTypeArray);



protected:
  vtkCMBArcGrowOperator();
  virtual ~vtkCMBArcGrowOperator();

  typedef std::set<vtkIdType> ArcSet;
  ArcSet InputArcs;
  vtkIdTypeArray* GrownArcSetIds;

private:
  vtkCMBArcGrowOperator(const vtkCMBArcGrowOperator&);  // Not implemented.
  void operator=(const vtkCMBArcGrowOperator&);  // Not implemented.
//ETX
};

#endif

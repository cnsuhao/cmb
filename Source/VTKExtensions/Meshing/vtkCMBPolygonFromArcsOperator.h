//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBPolygonFromArcsOperator
// .SECTION Description

#ifndef __vtkCMBPolygonFromArcsOperator_h
#define __vtkCMBPolygonFromArcsOperator_h

#include "cmbSystemConfig.h"
#include "vtkABI.h"
#include "vtkCMBMeshingModule.h" // For export macro
#include "vtkObject.h"
#include <set>

class vtkCMBArc;
class vtkIdTypeArray;
class vtkPolyData;
class vtkCMBArcManager;

class VTKCMBMESHING_EXPORT vtkCMBPolygonFromArcsOperator : public vtkObject
{
public:
  static vtkCMBPolygonFromArcsOperator* New();
  vtkTypeMacro(vtkCMBPolygonFromArcsOperator, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  //Description:
  //Add this arc id to the list of ids to use to create the polygon
  void AddArcId(vtkIdType arcId);

  //Description:
  //Determine which arc ids form the outer loop and the inner loops
  //Note: This operator does not support finding multiple outer loops, it only
  //determines if an arc is part of the outer loop or which inner loop it is part of.
  bool Operate();

  //Decription:
  //Returns the array of outer loop arc ids
  vtkIdTypeArray* GetOuterLoop();

  //Decription:
  //Returns the number of inner loops
  vtkIdType GetNumberOfInnerLoops();

  //Decription:
  //Returns the array of inner loop arc ids for the given index
  //Return NULL if the index doesn't exist
  vtkIdTypeArray* GetInnerLoop(const vtkIdType& index);

protected:
  vtkCMBPolygonFromArcsOperator();
  ~vtkCMBPolygonFromArcsOperator() override;

  void BuildArcsToUse();
  void BuildLoops();

  typedef std::set<vtkCMBArc*> ArcSet;
  ArcSet ArcsToUse;

  typedef std::set<vtkIdType> ArcIdSet;
  ArcIdSet ArcIdsToUse;

  class InternalLoops;
  InternalLoops* Loops;

private:
  vtkCMBPolygonFromArcsOperator(const vtkCMBPolygonFromArcsOperator&); // Not implemented.
  void operator=(const vtkCMBPolygonFromArcsOperator&);                // Not implemented.
};

#endif

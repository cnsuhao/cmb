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
// .NAME vtkCMBPolygonFromArcsOperator
// .SECTION Description

#ifndef __vtkCMBPolygonFromArcsOperator_h
#define __vtkCMBPolygonFromArcsOperator_h

#include "vtkCMBMeshingModule.h" // For export macro
#include "vtkObject.h"
#include "vtkABI.h"
#include "cmbSystemConfig.h"
#include <set>

class vtkCMBArc;
class vtkIdTypeArray;
class vtkPolyData;
class vtkCMBArcManager;

class VTKCMBMESHING_EXPORT vtkCMBPolygonFromArcsOperator : public vtkObject
{
public:
  static vtkCMBPolygonFromArcsOperator *New();
  vtkTypeMacro(vtkCMBPolygonFromArcsOperator,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

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
  ~vtkCMBPolygonFromArcsOperator();

  void BuildArcsToUse();
  void BuildLoops();

  typedef std::set<vtkCMBArc*> ArcSet;
  ArcSet ArcsToUse;

  typedef std::set<vtkIdType> ArcIdSet;
  ArcIdSet ArcIdsToUse;

  class InternalLoops;
  InternalLoops* Loops;

private:

  vtkCMBPolygonFromArcsOperator(const vtkCMBPolygonFromArcsOperator&);  // Not implemented.
  void operator=(const vtkCMBPolygonFromArcsOperator&);  // Not implemented.
};

#endif

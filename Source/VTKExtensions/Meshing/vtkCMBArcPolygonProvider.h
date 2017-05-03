//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBArcPolygonProvider
// .SECTION Description

#ifndef __vtkCMBArcPolygonProvider_h
#define __vtkCMBArcPolygonProvider_h

#include "cmbSystemConfig.h"
#include "vtkCMBMeshingModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"

class vtkIdTypeArray;
class vtkCMBArcManager;
class vtkPolyData;

class VTKCMBMESHING_EXPORT vtkCMBArcPolygonProvider : public vtkPolyDataAlgorithm
{
public:
  static vtkCMBArcPolygonProvider* New();
  vtkTypeMacro(vtkCMBArcPolygonProvider, vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  //Description:
  //Set the ids of the outer loop
  void SetOuterLoopArcIds(vtkIdTypeArray* ids);

  //Description:
  //Add an inner loop to the mesh using the passed in ids
  void AddInnerLoopArcIds(vtkIdTypeArray* ids);

  //Description:
  //Clear all inner loops
  void ClearInnerLoops();

  //Description:
  //Set the min angle to pass down to the mesher
  //Note an min angle of zero means disabled
  vtkSetMacro(MinAngle, double);

  //Description:
  //Set the edge length.
  //Note an edge length of zero means disabled
  vtkSetMacro(EdgeLength, double);

protected:
  vtkCMBArcPolygonProvider();
  ~vtkCMBArcPolygonProvider() override;

  // Description:
  // This is called by the superclass.
  // This is the method you should override.
  int RequestData(vtkInformation* request, vtkInformationVector** inputVector,
    vtkInformationVector* outputVector) override;

  //Description:
  //Generate the polyData representation for the associated arc
  vtkPolyData* CreatePolyDataRepresentation();

  vtkCMBArcManager* ArcManager;

  class InternalStorage;
  InternalStorage* Loops;

  double MinAngle;
  double EdgeLength;

private:
  vtkCMBArcPolygonProvider(const vtkCMBArcPolygonProvider&); // Not implemented.
  void operator=(const vtkCMBArcPolygonProvider&);           // Not implemented.
};

#endif

/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkGMSMeshSource.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkGMSMeshSource - "Dummy" source so we can treat data as a source
// .SECTION Description
// The input Source data is shallow copied to the output

#ifndef __vtkGMSMeshSource_h
#define __vtkGMSMeshSource_h

#include "vtkCMBGeneralModule.h" // For export macro
#include "vtkUnstructuredGridAlgorithm.h"
#include "cmbSystemConfig.h"
class vtkUnstructuredGrid;
class vtkPolyData;
class vtkAbstractTransform;
class vtkPoints;
class vtkIdList;
class vtkIdTypeArray;


class VTKCMBGENERAL_EXPORT vtkGMSMeshSource : public vtkUnstructuredGridAlgorithm
{
public:
  static vtkGMSMeshSource *New();
  vtkTypeMacro(vtkGMSMeshSource,vtkUnstructuredGridAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  virtual void CopyData(vtkUnstructuredGrid *source);
  vtkGetObjectMacro(Source, vtkUnstructuredGrid);

  virtual bool MoveTransformPoints(vtkPolyData *movedPoly, vtkAbstractTransform* transform);
  virtual bool MovePoints(vtkPolyData *movedPoly);

protected:
  vtkGMSMeshSource();
  ~vtkGMSMeshSource();

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  bool MoveVolumePoints(vtkIdTypeArray* meshCellArray,
    vtkIdList* meshIdList, vtkPoints* transformedPts);
  bool MoveSurfacePoints(vtkIdTypeArray* meshCellArray,
    vtkIdList* meshIdList, vtkPoints* transformedPts);
  bool MoveMeshPoints(
    vtkIdList* meshIdList, vtkPoints* transformedPts);

  vtkUnstructuredGrid* Source;
private:
  vtkGMSMeshSource(const vtkGMSMeshSource&);  // Not implemented.
  void operator=(const vtkGMSMeshSource&);  // Not implemented.
};

#endif

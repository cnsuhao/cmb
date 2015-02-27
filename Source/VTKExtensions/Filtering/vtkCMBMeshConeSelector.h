/*=========================================================================

Program:   Visualization Toolkit
Module:    vtkCMBMeshConeSelector.h

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
All rights reserved.
See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkCMBMeshConeSelector - create selection by cone on mesh
// .SECTION Description
// vtkCMBMeshConeSelector is a filter that will create a selection
// based on the cone selection type.

// .SECTION See Also
// vtkSelectionAlgorithm,

#ifndef __CmbMeshConeSelector_h
#define __CmbMeshConeSelector_h

#include "vtkCMBFilteringModule.h" // For export macro
#include "vtkSelectionAlgorithm.h"
#include "cmbSystemConfig.h"

class vtkCMBConeSource;
class vtkTransform;
class vtkUnstructuredGrid;

class VTKCMBFILTERING_EXPORT vtkCMBMeshConeSelector : public vtkSelectionAlgorithm
  {
public:
  vtkTypeMacro(vtkCMBMeshConeSelector,vtkSelectionAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);
  static vtkCMBMeshConeSelector *New();

  // Description:
  // Control whether to invert the output selection.
  vtkSetMacro(InsideOut,int);
  vtkGetMacro(InsideOut,int);
  vtkBooleanMacro(InsideOut,int);

  //BTX
  enum enumSelectCellThroughType {
    ALL_IN = 0,
    PARTIAL_OR_ALL_IN,
    INTERSECT_ONLY };
    //ETX

    // Description:
    // Set/Get the select cell through type
  // default: 0
  vtkSetClampMacro(SelectConeType, int , 0 , 2);
  vtkGetMacro(SelectConeType,int);

  //BTX
  enum enumSelectionFieldType {
    CELL = 0,
    POINT };
    //ETX

  // Description:
  // Set/Get the output selection field type
  // default: 0
  vtkSetClampMacro(SelectionFieldType, int , 0 , 1);
  vtkGetMacro(SelectionFieldType,int);

  // Description:
  // Transform to apply to the pts being read in for determining whether the
  // data is in/out of the ReadBounds (if LimitReadToBounds is true), or for
  // transforming data for the output (or both);  Note, the transform is
  // ignored if neither LimitReadToBounds nor TransformOutputData is true.
  void SetTransform(vtkTransform *transform);
  vtkGetObjectMacro(Transform, vtkTransform);
  void SetTransform(double elements[16]);
  void ClearTransform()
    {this->SetTransform(static_cast<vtkTransform*>(0));}

  // Description:
  // Set contour function
  void SetConeSource(vtkCMBConeSource *coneSource);
  vtkGetObjectMacro(ConeSource, vtkCMBConeSource);

  // Description:
  // Return the mtime also considering the cone source.
  unsigned long GetMTime();

  // Description:
  // Flag to indicate whether the output selection is empty, which
  // will be true, if all cells are selected, but InsideOut is set;
  // or no cells are selection and InsideOut is not set.
  vtkGetMacro(IsSelectionEmpty,int);

    //BTX
protected:
  vtkCMBMeshConeSelector();
  ~vtkCMBMeshConeSelector();

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  virtual int FillInputPortInformation(int port, vtkInformation* info);
  virtual int FillOutputPortInformation(int port, vtkInformation* info);

  virtual bool DoCellConeCheck(
    vtkIdType npts, vtkIdType* pts, vtkUnstructuredGrid* input,
    double BaseCenter[3],double AxisUnitDir[3], double Height,
    double BaseRadius, double TopRadius);

  int SelectConeType;
  int SelectionFieldType;
  int InsideOut;
  int IsSelectionEmpty;

  vtkTransform *Transform;
  vtkCMBConeSource* ConeSource;

private:
  vtkCMBMeshConeSelector(const vtkCMBMeshConeSelector&);  // Not implemented.
  void operator=(const vtkCMBMeshConeSelector&);  // Not implemented.

  bool IsProcessing;
  //ETX
};

#endif

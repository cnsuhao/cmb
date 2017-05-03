//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBMeshSelectionConverter - convert mesh selection.
// .SECTION Description
// vtkCMBMeshSelectionConverter tConvert a mesh subset selection to
// an Indices Selection on original mesh
//
// .SECTION See Also
// vtkSelection

#ifndef __vtkCMBMeshSelectionConverter_h
#define __vtkCMBMeshSelectionConverter_h

#include "cmbSystemConfig.h"
#include "vtkCMBFilteringModule.h" // For export macro
#include "vtkSelectionAlgorithm.h"

class vtkIntArray;
class vtkIdTypeArray;
class vtkSelectionNode;
class vtkUnstructuredGrid;

class VTKCMBFILTERING_EXPORT vtkCMBMeshSelectionConverter : public vtkSelectionAlgorithm
{
public:
  static vtkCMBMeshSelectionConverter* New();
  vtkTypeMacro(vtkCMBMeshSelectionConverter, vtkSelectionAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Specify the vtkSelection object used for selecting the
  // mesh facets.
  void SetSelectionConnection(vtkAlgorithmOutput* algOutput)
  {
    this->SetInputConnection(1, algOutput);
  }
  // Description:
  // Removes all inputs from input port 1.
  void RemoveAllSelectionsInputs() { this->SetInputConnection(1, 0); }
  // Description:
  // Specify the mesh object used for change regions.
  void SetMeshConnection(vtkAlgorithmOutput* algOutput) { this->SetInputConnection(2, algOutput); }
  // Description:
  // Removes all inputs from input port 2.
  void RemoveAllMeshsInputs() { this->SetInputConnection(2, 0); }

protected:
  vtkCMBMeshSelectionConverter();
  ~vtkCMBMeshSelectionConverter() override;

  int FillInputPortInformation(int port, vtkInformation* info) override;

  // runs the algorithm and fills the output with results
  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

  int CreateSelectionLists(vtkSelectionNode* selNode, vtkIdTypeArray* outSelectionList,
    int fieldType, vtkUnstructuredGrid* meshInput);

private:
  vtkCMBMeshSelectionConverter(const vtkCMBMeshSelectionConverter&); // Not implemented.
  void operator=(const vtkCMBMeshSelectionConverter&);               // Not implemented.
};

#endif

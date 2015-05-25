//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkGMSMeshSelectionRegionFilter - assign a region id for selected cells.
// .SECTION Description
// vtkGMSMeshSelectionRegionFilter takes an unstructure-grid and a selection
// as input, and change the "Region" array value of selected cells to a new reion id
//
// .SECTION See Also
// vtkSelection

#ifndef __vtkGMSMeshSelectionRegionFilter_h
#define __vtkGMSMeshSelectionRegionFilter_h

#include "vtkCMBFilteringModule.h" // For export macro
#include "vtkUnstructuredGridAlgorithm.h"
#include "cmbSystemConfig.h"

class vtkIntArray;
class vtkIdTypeArray;
class vtkSelectionNode;

class VTKCMBFILTERING_EXPORT vtkGMSMeshSelectionRegionFilter : public vtkUnstructuredGridAlgorithm
{
public:
  static vtkGMSMeshSelectionRegionFilter *New();
  vtkTypeMacro(vtkGMSMeshSelectionRegionFilter, vtkUnstructuredGridAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Specify the vtkSelection object used for selecting the
  // mesh facets.
  void SetSelectionConnection(vtkAlgorithmOutput* algOutput)
    { this->SetInputConnection(1, algOutput); }
  // Description:
  // Removes all inputs from input port 1.
  void RemoveAllSelectionsInputs()
    { this->SetInputConnection(1, 0); }
  // Description:
  // Specify the mesh object used for change regions.
  void SetMeshConnection(vtkAlgorithmOutput* algOutput)
  { this->SetInputConnection(2, algOutput); }
  // Description:
  // Removes all inputs from input port 2.
  void RemoveAllMeshsInputs()
  { this->SetInputConnection(2, 0); }

  // Description:
  // Get/Set the RegionId, which will set to the selected cells.
  void SetSelectionRegionId(int);
  vtkGetMacro(SelectionRegionId,int);

protected:
  vtkGMSMeshSelectionRegionFilter();
  ~vtkGMSMeshSelectionRegionFilter();

  virtual int FillInputPortInformation(int port, vtkInformation* info);

  // runs the algorithm and fills the output with results
  virtual int RequestData(vtkInformation *,
                  vtkInformationVector **,
                  vtkInformationVector *);


  int ModifySelectedCellRegions(
    vtkSelectionNode* selNode, vtkIntArray* outArray, vtkIdTypeArray* cellIDArray);

  int SelectionRegionId;
  int IsNewRegionIdSet;

private:
  vtkGMSMeshSelectionRegionFilter(const vtkGMSMeshSelectionRegionFilter&);  // Not implemented.
  void operator=(const vtkGMSMeshSelectionRegionFilter&);  // Not implemented.
};

#endif

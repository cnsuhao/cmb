//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBMeshContourSelector - create selection by contour on mesh
// .SECTION Description
// vtkCMBMeshContourSelector is a filter that will create a polydata and
// a point Id selection that is inside the contour (selection loop).
// The output polydata contains vertex cells for those selection points.
// Based on the selection type (surface or select through), a surface
// filter may be used or not before doing selection. The input to this
// filter is a surface or volume mesh.

// .SECTION See Also
// vtkSelectionAlgorithm, vtkImplicitSelectionLoop

#ifndef __vtkCMBMeshContourSelector_h
#define __vtkCMBMeshContourSelector_h

#include "vtkCMBFilteringModule.h" // For export macro
#include "vtkSelectionAlgorithm.h"
#include "cmbSystemConfig.h"

class vtkImplicitSelectionLoop;
class vtkPolyData;
class vtkPoints;
class vtkIdTypeArray;
class vtkCellArray;
class vtkUnstructuredGrid;
class vtkIdList;

#include <map>

class VTKCMBFILTERING_EXPORT vtkCMBMeshContourSelector : public vtkSelectionAlgorithm
{
public:
  vtkTypeMacro(vtkCMBMeshContourSelector,vtkSelectionAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  static vtkCMBMeshContourSelector *New();

  // Description:
  // Specify the vtkSelection object used for selecting the
  // mesh points.
  void SetSelectionConnection(vtkAlgorithmOutput* algOutput)
  { this->SetInputConnection(1, algOutput); }

  // Description:
  // Removes all inputs from input port 1.
  void RemoveAllSelectionsInputs()
  { this->SetInputConnection(1, 0); }

  // Description:
  // Specify the vktPolyData object as mesh surface
  void SetSurfaceConnection(vtkAlgorithmOutput* algOutput)
  { this->SetInputConnection(2, algOutput); }

  // Description:
  // Removes all inputs from input port 2.
  void RemoveAllSurfaceInputs()
  { this->SetInputConnection(2, 0); }

  // Description:
  // Return the Polydata generated with contour selection
  vtkPolyData* GetSelectionPolyData();

  // Description:
  // Return the average normals of the output SelectionPolyData,
  // calculated from averaging cell normals.
  vtkGetVector3Macro(OrientationOfSelectedNodes, double);

  // Description:
  // Control whether to select through or just on surface
  // If this flag is set, the GenerateSelectedOutput is ignored,
  // so this filter will only generate a selection with cell IDs.
  vtkSetMacro(SelectCellThrough,int);
  vtkGetMacro(SelectCellThrough,int);
  vtkBooleanMacro(SelectCellThrough,int);

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
  vtkSetClampMacro(SelectContourType, int , 0 , 2);
  vtkGetMacro(SelectContourType,int);

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
  // Control whether a second output is generated. The second output
  // contains the polygonal data (only points) that's been selected.
  // This flag is just to create a polydata for the selected nodes,
  // so that it can be displayed inside the contour widget and
  // transformed with a box widget
  vtkSetMacro(GenerateSelectedOutput,int);
  vtkGetMacro(GenerateSelectedOutput,int);
  vtkBooleanMacro(GenerateSelectedOutput,int);

  // Description:
  // Flag to indicate whether the output selection is empty, which
  // will be true, if all cells are selected, but InsideOut is set;
  // or no cells are selection and InsideOut is not set.
  vtkGetMacro(IsSelectionEmpty,int);

  // Description:
  // Return the mtime also considering the contour function.
  vtkMTimeType GetMTime() override;

  // Description:
  // Set contour function
  void SetContour(vtkImplicitSelectionLoop *contour);
  vtkGetObjectMacro(Contour, vtkImplicitSelectionLoop);

//BTX
protected:
  vtkCMBMeshContourSelector();
  ~vtkCMBMeshContourSelector() override;

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *) override;
  int FillInputPortInformation(int port, vtkInformation* info) override;
  int FillOutputPortInformation(int port, vtkInformation* info) override;

  virtual void DoSurfaceSelectionCheck(int selType, vtkIdList* tmpIds,
    bool bVolume, vtkIdType selId, vtkUnstructuredGrid* input,
    vtkIdTypeArray* meshCellIdArray, vtkIdTypeArray* meshNodeIdArray,
    vtkPoints* newPoints, vtkCellArray* outVerts, vtkIdList* outMeshCellIds,
    vtkIdList* outNodeIdList, vtkIdList* surfaceNodeList,
    vtkIdTypeArray* outSelectionList, double* totNormal, int& totNumNormals);
  virtual bool DoCellContourCheck(
    vtkIdType npts, vtkIdType* pts, vtkUnstructuredGrid* input);

  int SelectCellThrough;
  int SelectContourType;
  int SelectionFieldType;
  int InsideOut;
  int IsSelectionEmpty;

  vtkImplicitSelectionLoop* Contour;
  int GenerateSelectedOutput;
  vtkPolyData* SelectionPolyData;
  double OrientationOfSelectedNodes[3];

private:
  vtkCMBMeshContourSelector(const vtkCMBMeshContourSelector&);  // Not implemented.
  void operator=(const vtkCMBMeshContourSelector&);  // Not implemented.

  bool IsProcessing;
//ETX
};

#endif

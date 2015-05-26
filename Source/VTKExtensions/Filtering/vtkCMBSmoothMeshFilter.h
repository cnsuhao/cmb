//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBSmoothMeshFilter - create smoothed polydata by contour on mesh
// .SECTION Description
// vtkCMBSmoothMeshFilter is a filter that will create a smoothed polydata
// given the selection and input mesh. The output only contains vertexes that
// has cell arrays mapping back to original mesh.

// .SECTION See Also
// vtkPolyDataAlgorithm

#ifndef __vtkCMBSmoothMeshFilter_h
#define __vtkCMBSmoothMeshFilter_h

#include "vtkCMBFilteringModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"
#include "cmbSystemConfig.h"

class vtkPolyData;
class vtkUnstructuredGrid;
class vtkSmoothPolyDataFilter;

class VTKCMBFILTERING_EXPORT vtkCMBSmoothMeshFilter : public vtkPolyDataAlgorithm
{
public:
  vtkTypeMacro(vtkCMBSmoothMeshFilter,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);
  static vtkCMBSmoothMeshFilter *New();

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
  // Specify a convergence criterion for the iteration
  // process. Smaller numbers result in more smoothing iterations.
  vtkSetClampMacro(Convergence,double,0.0,1.0);
  vtkGetMacro(Convergence,double);

  // Description:
  // Specify the number of iterations for Laplacian smoothing,
  vtkSetClampMacro(NumberOfIterations,int,0,VTK_INT_MAX);
  vtkGetMacro(NumberOfIterations,int);

  // Description:
  // Specify the relaxation factor for Laplacian smoothing. As in all
  // iterative methods, the stability of the process is sensitive to
  // this parameter. In general, small relaxation factors and large
  // numbers of iterations are more stable than larger relaxation
  // factors and smaller numbers of iterations.
  vtkSetMacro(RelaxationFactor,double);
  vtkGetMacro(RelaxationFactor,double);

  // Description:
  // Turn on/off smoothing along sharp interior edges.
  vtkSetMacro(FeatureEdgeSmoothing,int);
  vtkGetMacro(FeatureEdgeSmoothing,int);
  vtkBooleanMacro(FeatureEdgeSmoothing,int);

  // Description:
  // Specify the feature angle for sharp edge identification.
  vtkSetClampMacro(FeatureAngle,double,0.0,180.0);
  vtkGetMacro(FeatureAngle,double);

  // Description:
  // Specify the edge angle to control smoothing along edges (either interior
  // or boundary).
  vtkSetClampMacro(EdgeAngle,double,0.0,180.0);
  vtkGetMacro(EdgeAngle,double);

  // Description:
  // Turn on/off the smoothing of vertices on the boundary of the mesh.
  vtkSetMacro(BoundarySmoothing,int);
  vtkGetMacro(BoundarySmoothing,int);
  vtkBooleanMacro(BoundarySmoothing,int);

//BTX
protected:
  vtkCMBSmoothMeshFilter();
  ~vtkCMBSmoothMeshFilter();

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  virtual int FillInputPortInformation(int port, vtkInformation* info);
  void ExtractSurfaceCells(bool bVolume,
    vtkIdList* tmpIds, vtkIdList* nxtPts, vtkIdType cellId, vtkUnstructuredGrid* input,
    vtkIdTypeArray* meshCellIdArray, vtkIdTypeArray* meshNodeIdArray,
    vtkPoints* newPoints, vtkCellArray* smoothPolys, vtkIdList* outMeshCellIds,
    vtkIdList* outNodeIdList, vtkIdList* surfaceNodeList);

  double Convergence;
  int NumberOfIterations;
  double RelaxationFactor;
  int FeatureEdgeSmoothing;
  double FeatureAngle;
  double EdgeAngle;
  int BoundarySmoothing;
  vtkSmoothPolyDataFilter* SmoothPolyFilter;

private:
  vtkCMBSmoothMeshFilter(const vtkCMBSmoothMeshFilter&);  // Not implemented.
  void operator=(const vtkCMBSmoothMeshFilter&);  // Not implemented.

//ETX
};

#endif

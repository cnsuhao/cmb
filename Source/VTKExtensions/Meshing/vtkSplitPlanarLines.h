#ifndef __vtkSplitPlanarLines_h
#define __vtkSplitPlanarLines_h

// .NAME vtkSplitPlanarLines - Split polyline data at all intersection points.
// .SECTION Description
// This filter uses the Bentley-Ottman line sweep algorithm as modified by
// deBerg, Overmars, van Kreveld, and Schwarzkopf to find all line segment
// intersections. New points are inserted at the end of the existing point
// coordinates (preserving the original point IDs).
// There is no guarantee that edge order is preserved, but pedigree IDs are
// generated to indicate the correspondence between input and output edges.

#include "vtkCMBMeshingModule.h" // for export macro
#include "vtkPolyDataAlgorithm.h"
#include "cmbSystemConfig.h"

class VTKCMBMESHING_EXPORT vtkSplitPlanarLines : public vtkPolyDataAlgorithm
{
public:
  static vtkSplitPlanarLines* New();
  vtkTypeMacro(vtkSplitPlanarLines,vtkPolyDataAlgorithm);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/get the geometric tolerance for point merging and intersection testing.
  vtkGetMacro(Tolerance,double);
  vtkSetMacro(Tolerance,double);

protected:
  vtkSplitPlanarLines();
  virtual ~vtkSplitPlanarLines();

  virtual int RequestData(
    vtkInformation* req,
    vtkInformationVector** inVec,
    vtkInformationVector* outVec);

  double Tolerance;
private:
  vtkSplitPlanarLines(const vtkSplitPlanarLines&); // Not implemented.
  void operator = (const vtkSplitPlanarLines&); // Not implemented.
};

#endif // __vtkSplitPlanarLines_h

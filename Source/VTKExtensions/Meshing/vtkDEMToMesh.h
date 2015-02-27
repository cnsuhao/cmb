#ifndef __vtkDEMToMesh_h
#define __vtkDEMToMesh_h

#include "vtkCMBMeshingModule.h" // for EXPORT macro
#include "vtkPolyDataAlgorithm.h"
#include "cmbSystemConfig.h"


class VTKCMBMESHING_EXPORT vtkDEMToMesh : public vtkPolyDataAlgorithm
{
public:
  static vtkDEMToMesh* New();
  vtkTypeMacro(vtkDEMToMesh,vtkPolyDataAlgorithm);

  void SetUseScalerForZ(int v);

protected:
  vtkDEMToMesh();
  virtual ~vtkDEMToMesh();

  virtual int FillInputPortInformation(int port, vtkInformation* info);

  virtual int RequestData(vtkInformation* req,
                          vtkInformationVector** inInfo,
                          vtkInformationVector* outInfo);

  int UseScalerForZ;
  int SubSampleStepSize;

private:
  vtkDEMToMesh(const vtkDEMToMesh&); // Not implemented.
  void operator = (const vtkDEMToMesh&); // Not implemented.
};

#endif // __vtkPolylineTriangulator_h

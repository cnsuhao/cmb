//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBMeshTerrainWithArcs
// .SECTION Description

#ifndef __vtkCMBMeshTerrainWithArcs_h
#define __vtkCMBMeshTerrainWithArcs_h

#include "vtkCMBMeshingModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"
#include "cmbSystemConfig.h"

class vtkPoints;
class vtkCellArray;
class vtkCMBTriangleMesher;

class VTKCMBMESHING_EXPORT vtkCMBMeshTerrainWithArcs : public vtkPolyDataAlgorithm
{
public:
  static vtkCMBMeshTerrainWithArcs *New();
  vtkTypeMacro(vtkCMBMeshTerrainWithArcs,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  //Description:
  //Set/Get the bounds of the VOI.
  vtkGetVector6Macro(VOIBounds,double);
  vtkSetVector6Macro(VOIBounds,double);

  //Description:
  //Set/Get the raidus of the cone to use for elevation smoothing
  vtkSetMacro(ElevationRadius,double);
  vtkGetMacro(ElevationRadius,double);

  //Description:
  //Set the point cloud we are going to be using as the terrain
  void RemoveInputConnections();

  //Description:
  //add an arc id that will be used
  void RemoveSourceConnections();

protected:
  vtkCMBMeshTerrainWithArcs();
  ~vtkCMBMeshTerrainWithArcs() override;

  int FillInputPortInformation(int port, vtkInformation *info) override;
  int RequestData(vtkInformation*,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector) override;

  void DetermineNumberOfProgressSteps(const int &numInputs);
  void NextProgressStep();

  //methods for preprocess of mesh
  bool PrepForMeshing(vtkInformationVector* input, vtkPolyData *mesh);
  bool AssignPolygonIds(vtkPolyData *mesh, const vtkIdType &size) const;
  bool FlattenMesh(vtkPoints*) const;
  void InsertGroundPlane(vtkPoints*,vtkCellArray*) const;

  //methods for ground mesh reconstruction
  bool GenerateGroundMesh(vtkPolyData *mesh);
  bool ExtrudeMeshPoints(vtkPoints *points) const;

  //methods for building arc sets into shells
  bool GenerateExtrudedArcSets(vtkPolyData* input, vtkPolyData* output);
  void CreateArcSetForMeshing(vtkPolyData* input, vtkPolyData* output, const int &index) const;
  bool ExtrudeArcMesh(vtkPolyData *mesh, const int &index) const;
  bool UnFlattenMesh(vtkPoints*, const double& height) const;

  int NumberOfProgressSteps;
  int CurrentProgressStep;
  double StepIncrement;

  double VOIBounds[6];
  double ElevationRadius;

  //BTX
  class vtkCmbPolygonInfo;
  vtkCmbPolygonInfo *PolygonInfo;
  //ETX

  //BTX
  class vtkCmbInternalTerrainInfo;
  vtkCmbInternalTerrainInfo *TerrainInfo;
  //ETX

  vtkCMBTriangleMesher *Mesher;
  double MesherMaxArea;

private:
  vtkCMBMeshTerrainWithArcs(const vtkCMBMeshTerrainWithArcs&);  // Not implemented.
  void operator=(const vtkCMBMeshTerrainWithArcs&);  // Not implemented.
};

#endif

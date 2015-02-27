/*=========================================================================

Copyright (c) 1998-2012 Kitware Inc. 28 Corporate Drive,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/
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
  void PrintSelf(ostream& os, vtkIndent indent);

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
  ~vtkCMBMeshTerrainWithArcs();

  virtual int FillInputPortInformation(int port, vtkInformation *info);
  virtual int RequestData(vtkInformation*,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector);

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

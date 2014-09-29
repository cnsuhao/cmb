/*=========================================================================

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
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
// .NAME vtkSeedGrowSelectionFilter
// .SECTION Description
#ifndef __vtkSeedGrowSelectionFilter_h
#define __vtkSeedGrowSelectionFilter_h

#include "vtkCmbDiscreteModelModule.h" // For export macro
#include "vtkSelectionAlgorithm.h"
#include "cmbSystemConfig.h"

class DiscreteMesh;
class vtkDiscreteModelWrapper;
class vtkIdList;
class vtkIdTypeArray;
class vtkIntArray;
class vtkPolyData;

class VTKCMBDISCRETEMODEL_EXPORT vtkSeedGrowSelectionFilter :
public vtkSelectionAlgorithm
{
public:
  static vtkSeedGrowSelectionFilter* New();
  vtkTypeMacro(vtkSeedGrowSelectionFilter, vtkSelectionAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Specify the vtkSelection object used for selecting the
  // mesh facets.
  void SetSelectionConnection(vtkAlgorithmOutput* algOutput);

  // Description:
  // Removes all inputs from input port 0.
  void RemoveAllSelectionsInputs()
    {
      this->SetInputConnection(0, 0);
    }

  // Description:
  // Specify the angle that defines a sharp edge. If the difference in
  // angle across neighboring polygons is greater than this value, the
  // shared edge is considered "sharp".
  vtkSetClampMacro(FeatureAngle,double,0.0,180.0);
  vtkGetMacro(FeatureAngle,double);

  // Description:
  // Macro to set/get CellId and FaceId.  The CellId is the id of the cell on
  // the model face to start the grow operation from.
  void SetFaceCellId(vtkIdType faceId, vtkIdType cellId);
  vtkGetMacro(CellId, vtkIdType);
  vtkGetMacro(ModelFaceCompositeIdx, vtkIdType);

  // Description:
  // Macro to set/get CellId and FaceId
  // 0, Normal (ignore selection input);
  // 1, Plus (merge result with selection input);
  // 2, Minus (remove result from selection input);
  vtkSetClampMacro(GrowMode, int, 0, 2);
  vtkGetMacro(GrowMode, int);

  // Description:
  // Set/clear the model face Ids array that will be grown upon.
  void SetGrowFaceIds(vtkIdType* );
  void RemoveAllGrowFaceIds();

  // Description:
  // Set/get macros for the vtkDiscreteModelWrapper.
  void SetModelWrapper(vtkDiscreteModelWrapper*);
  vtkGetObjectMacro(ModelWrapper, vtkDiscreteModelWrapper);

//BTX
protected:
  vtkSeedGrowSelectionFilter();
  ~vtkSeedGrowSelectionFilter();

  double FeatureAngle;
  // Cosine of the feature angle.
  double FeatureAngleCosine;

  // Description:
  // The starting CellId and FaceId of the grow operation
  vtkIdType CellId;
  vtkIdType ModelFaceCompositeIdx;

  // 0, Normal (ignore selection input);
  // 1, Plus (merge result with selection input);
  // 2, Minus (remove result from selection input);
  int GrowMode;

  // Description:
  // A list of cell Ids on the master poly data that are created from the
  // grow operation.
  vtkIdList* GrowCellIds;

  // Description:
  // The vtkDiscreteModelWrapper for the algorithm to extract the model
  // information from.
  vtkDiscreteModelWrapper* ModelWrapper;

  // Description:
  // Iterative algorithm to grow from a passed in cell id in a vtkPolyData.
  // The marked array is used to store whether or not the cell is in the
  // list of "grown" cells.
  void GrowFromCell(const DiscreteMesh* mesh, vtkIntArray* marked, vtkIdType cellId,
                    vtkIdTypeArray* selectionList);

  void GrowAndRemoveFromSelection(
    const DiscreteMesh* mesh, vtkIntArray* marked, vtkIdType cellId,
    vtkIdTypeArray* outSelectionList, vtkSelection* inSelection);

  // Description:
  // Computes the normal of a triangle (or the first 3 points of polygon)
  // with points in ptids and returns the values in normal. We have assumed
  // that all polygons are planar but possibly not convex as per
  // discussions with CMB
  void ComputeNormal(vtkIdList* ptids, const DiscreteMesh* mesh, double normal[3]);

  // Description:
  // We cannot assume that the normals are consistent between
  // model faces.  Because of this we check to see the canonical
  // ordering of the cells' points. PointId1 is the first point
  // along the edge of the first triangle and PointId2 is the second
  // point (canonicall ordered).  NeighborCellIds are the point ids
  // of the neighboring cell (also canonically ordered).  Returns
  // true if they are consistent.
  bool AreCellNormalsConsistent(vtkIdType PointId1, vtkIdType PointId2,
                                vtkIdList * NeighborCellIds);

  // Description:
  // Recursive algorithm to merge which cells have been selected.
  void MergeGrowSelection(vtkSelection*, vtkIntArray*,
                          vtkIdTypeArray* outSelectionList);

  // Description:
  // This is called within ProcessRequest when a request asks the algorithm
  // to do its work. This is the method you should override to do whatever the
  // algorithm is designed to do. This happens during the fourth pass in the
  // pipeline execution process.
  virtual int RequestData(vtkInformation*,
                          vtkInformationVector**,
                          vtkInformationVector*);
  virtual int FillInputPortInformation(int port, vtkInformation *info);

private:
  vtkSeedGrowSelectionFilter(const vtkSeedGrowSelectionFilter&); // Not implemented.
  void operator=(const vtkSeedGrowSelectionFilter&); // Not implemented.

  class vtkInternal;
  vtkInternal* Internal;

//ETX
};

#endif


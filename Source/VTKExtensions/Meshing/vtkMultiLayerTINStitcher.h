/*=========================================================================

Copyright (c) 1998-2012 Kitware Inc. 28 Corporate Drive,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed, or modified, in any form or by any means, without
permission in writing from Kitware Inc.

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
// .NAME vtkMultiLayerTINStitcher - stitch two or more TINs/meshes together
// .SECTION Description
// This filter is intended as a multi-layer version of the vtkTINStitcher, a
// version that can take more than two layers as input (vtkTINStitcher is
// limited to two layers).  This filter sorts the layers by the maximum z value
// in the layer (thus assumes layers don't cross, or touch at maximum z), and
// sends each consecutive pair of layers to an internal vtkTINStitcher.  Each
// stitched pair is a block in a multi-block dataset.
// .SECTION See Also
// vtkTINStitcher

#ifndef __vtkMultiLayerTINStitcher_h
#define __vtkMultiLayerTINStitcher_h

#include "vtkCMBMeshingModule.h" // For export macro
#include "vtkMultiBlockDataSetAlgorithm.h"
#include "cmbSystemConfig.h"

class vtkPolyData;
class vtkUnstructuredGrid;

class VTKCMBMESHING_EXPORT vtkMultiLayerTINStitcher : public vtkMultiBlockDataSetAlgorithm
{
public:
  vtkTypeMacro(vtkMultiLayerTINStitcher,vtkMultiBlockDataSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);
  static vtkMultiLayerTINStitcher *New();

  // Description:
  // Set 2nd input input to the filter (required)
  void AddInputData(vtkPolyData *input);
  void AddInputData(vtkUnstructuredGrid *input);

  // Description:
  // Indicate the minimum target angle to pass to Triangle if Type 2 TIN
  // (or Type I using Triangle to stitch).
  vtkSetClampMacro(MinimumAngle, double, 0, 34);
  vtkGetMacro(MinimumAngle, double);
  vtkBooleanMacro(MinimumAngle, double);

  // Description:
  // Indicate whether the resulting mesh should be quads if possible.
  // This only used if the loops contain the same number of points (Type I TIN).
  vtkSetMacro(UseQuads, bool);
  vtkGetMacro(UseQuads, bool);
  vtkBooleanMacro(UseQuads, bool);

  // Description:
  // Specifies whether Triangle is allowd to do point insertion on the interior
  // in order to meet minimum angle criteria.
  // Note, if a Type I TIN and UseQuads is TRUE, this is ignored.
  vtkSetMacro(AllowInteriorPointInsertion, bool);
  vtkGetMacro(AllowInteriorPointInsertion, bool);
  vtkBooleanMacro(AllowInteriorPointInsertion, bool);

  // Description:
  // Set/Get tolerance value, which is multiplied by the short side of the
  // input bounding box to compute a MaxDistance used when determining
  // TIN "sides" as well as Type I versus Type II.  Defaults to 1e-6.
  vtkSetClampMacro(Tolerance, double, 0, 0.01);
  vtkGetMacro(Tolerance, double);

  // Description:
  // Set/Get user specified TIN type.  If 0, auto-detect the TIN type and
  // stitch accordingly (only type I or type II right now); if 1, will stitch
  // as type I (if possible).  In auto-detect mode, determining TIN type is
  // 1st attempted using the member Tolerance as the tolerance factor.  If
  // not classified at type I or II at that tolerance level it is successively
  // increased by an order of magnitude, and auto-detection repeated, up to
  // maximum value of 0.05
  vtkSetClampMacro(UserSpecifiedTINType, int, 0, 1);
  vtkGetMacro(UserSpecifiedTINType, int);

protected:
  vtkMultiLayerTINStitcher();
  ~vtkMultiLayerTINStitcher();

  // Usual data generation method
  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  virtual int FillInputPortInformation(int, vtkInformation *);

  bool   UseQuads;
  double MinimumAngle;
  bool AllowInteriorPointInsertion;
  double Tolerance;
  int UserSpecifiedTINType;

private:
  vtkMultiLayerTINStitcher(const vtkMultiLayerTINStitcher&);  // Not implemented.
  void operator=(const vtkMultiLayerTINStitcher&);  // Not implemented.
};

#endif

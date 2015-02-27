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
// .NAME vtkCMBGeometryReader - "reader" for various SceneGen geometry formats
// .SECTION Description
// Not actually a reader in the sense that it internally creates the appropriate
// reader based on the filename's extension.

#ifndef __CMBGeometryReader_h
#define __CMBGeometryReader_h

#include "vtkCMBIOModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"
#include "cmbSystemConfig.h"

class VTKCMBIO_EXPORT vtkCMBGeometryReader : public vtkPolyDataAlgorithm
{
public:
  static vtkCMBGeometryReader *New();
  vtkTypeMacro(vtkCMBGeometryReader,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Name of the file to be read.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  // Description:
  // If the file read contains a 2D mesh, that has boundary edges,
  // PrepNonClosedSurfaceForModelCreation determines
  // whether to prep the data for model creation; whether or not vertices /
  // edges / loops are detected.  Right now only used for 3dm and vtk files
  vtkBooleanMacro(PrepNonClosedSurfaceForModelCreation, bool);
  vtkSetMacro(PrepNonClosedSurfaceForModelCreation, bool);
  vtkGetMacro(PrepNonClosedSurfaceForModelCreation, bool);

  // Description:
  // If enabled, the data read by the reader if further post-processed after
  // reading.  For now, this only has an affect for vtk, and 2dm/3dm files...
  // other file types are post processed (or not) as previously done before
  // adding this variable.
  vtkBooleanMacro(EnablePostProcessMesh, bool);
  vtkSetMacro(EnablePostProcessMesh, bool);
  vtkGetMacro(EnablePostProcessMesh, bool);

  // Description:
  // Get whether the mesh has boundary edges (only for 3dm and vtk files that do
  // NOT contain volume elments).  Note the value will always return false for
  // all other reader types.
  vtkGetMacro(HasBoundaryEdges, bool);

  // Description:
  // Get whether the Cmb3dm reader modified the regions identifiers
  vtkGetMacro(RegionIdentifiersModified, bool);

protected:
  vtkCMBGeometryReader();
  ~vtkCMBGeometryReader();

  int RequestInformation(vtkInformation *,
                         vtkInformationVector **,
                         vtkInformationVector *);
  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

  void PostProcessMesh(vtkDataSet *dataset, bool is3DVolumeMesh,
    bool passThroughPointIds, const char *regionArrayName, vtkPolyData *output);

  char *FileName;

  vtkSetMacro(HasBoundaryEdges, bool);
  vtkSetMacro(RegionIdentifiersModified, bool);

private:
  vtkCMBGeometryReader(const vtkCMBGeometryReader&);  // Not implemented.
  void operator=(const vtkCMBGeometryReader&);  // Not implemented.

  bool PrepNonClosedSurfaceForModelCreation;
  bool HasBoundaryEdges;
  bool RegionIdentifiersModified;
  bool EnablePostProcessMesh;
};

#endif

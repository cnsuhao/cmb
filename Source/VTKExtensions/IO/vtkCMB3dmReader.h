//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMB3dmReader - Read in a 3dm file into CMB.
// .SECTION Description
// Read in a 3dm file into CMB.  The 3dm mesh is an unstructured,
// volumetric mesh.

#ifndef __vtkCMB3dmReader_h
#define __vtkCMB3dmReader_h

#include "vtkCMBIOModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"
#include "cmbSystemConfig.h"

class VTKCMBIO_EXPORT vtkCMB3dmReader : public vtkPolyDataAlgorithm
{
public:
  static vtkCMB3dmReader * New();
  vtkTypeMacro(vtkCMB3dmReader,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get/Set the name of the input file.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  // Description:
  // If the 3dm file contains a 2D mesh, Prep2DMeshForModelCreation determines
  // whether to prep the  data for model creation; whether or not vertices /
  // edges / loops are detected
  vtkBooleanMacro(Prep2DMeshForModelCreation, bool);
  vtkSetMacro(Prep2DMeshForModelCreation, bool);
  vtkGetMacro(Prep2DMeshForModelCreation, bool);

  // Description:
  // Get whether this is a 2D mesh
  vtkGetMacro(Is2DMesh, bool);

  // Description:
  // Get whether the region identifiers were modified within the
  // extract edges filter.  If so, should write new 3dm file.
  vtkGetMacro(RegionIdentifiersModified, bool);

protected:
  vtkCMB3dmReader();
  virtual ~vtkCMB3dmReader();

  int RequestInformation(vtkInformation *,
                         vtkInformationVector **,
                         vtkInformationVector *);
  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

private:
  vtkCMB3dmReader(const vtkCMB3dmReader&);  // Not implemented.
  void operator=(const vtkCMB3dmReader&);  // Not implemented.

  // Description:
  // The name of the file to be read in.
  char* FileName;
  bool Prep2DMeshForModelCreation;
  bool Is2DMesh;
  bool RegionIdentifiersModified;
};

#endif

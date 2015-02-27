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

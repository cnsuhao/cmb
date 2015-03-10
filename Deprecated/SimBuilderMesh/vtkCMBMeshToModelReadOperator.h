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
// .NAME vtkCMBMeshToModelReadOperator -
// .SECTION Description
// Front end for the readers.  Reads in a "m2m" file and load the meshing
// info to the model with vtkCMBMeshToModelReader

#ifndef __vtkCMBMeshToModelReadOperator_h
#define __vtkCMBMeshToModelReadOperator_h

#include "vtkObject.h"
#include "cmbSystemConfig.h"

class vtkCMBParserBase;
class vtkDiscreteModelWrapper;
class vtkPolyData;
class vtkDiscreteModel;

class VTK_EXPORT vtkCMBMeshToModelReadOperator : public vtkObject
{
public:
  static vtkCMBMeshToModelReadOperator * New();
  vtkTypeMacro(vtkCMBMeshToModelReadOperator,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Load the file into Model.
  void Operate(vtkDiscreteModelWrapper* ModelWrapper);

  // Description:
  // Get/Set the name of the input file.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  // Description:
  // Returns success (1) or failue (0) for Operation.
  vtkGetMacro(OperateSucceeded, int);

protected:
  vtkCMBMeshToModelReadOperator();
  virtual ~vtkCMBMeshToModelReadOperator();

private:
  // Description:
  // The name of the file to be read in.
  char* FileName;

  vtkCMBMeshToModelReadOperator(const vtkCMBMeshToModelReadOperator&);  // Not implemented.
  void operator=(const vtkCMBMeshToModelReadOperator&);  // Not implemented.

  // Description:
  // Flag to indicate that the operation on the model succeeded (1) or not (0).
  int OperateSucceeded;
};

#endif

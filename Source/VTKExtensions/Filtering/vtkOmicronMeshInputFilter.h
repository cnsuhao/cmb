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
// .NAME vtkOmicronMeshInputFilter - final prepraration for Omicron mesh input.
// .SECTION Description
// This filter find material IDs to associate with each object and also
// adds soil and point in soil to output.

#ifndef __vtkOmicronMeshInputFilter_h
#define __vtkOmicronMeshInputFilter_h

#include "vtkCMBFilteringModule.h" // For export macro
#include "vtkMultiBlockDataSetAlgorithm.h"
#include "cmbSystemConfig.h"

class vtkMultiBlockDataSet;

class VTKCMBFILTERING_EXPORT vtkOmicronMeshInputFilter : public vtkMultiBlockDataSetAlgorithm
{
public:
  static vtkOmicronMeshInputFilter* New();
  vtkTypeMacro(vtkOmicronMeshInputFilter, vtkMultiBlockDataSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set the input to this writer.
  void SetInputData(vtkMultiBlockDataSet* dataSet);

//BTX
protected:
  vtkOmicronMeshInputFilter();
  ~vtkOmicronMeshInputFilter();

  // Description:
  // This is called within ProcessRequest when a request asks the algorithm
  // to do its work. This is the method you should override to do whatever the
  // algorithm is designed to do. This happens during the fourth pass in the
  // pipeline execution process.
  virtual int RequestData(vtkInformation*,
                          vtkInformationVector**,
                          vtkInformationVector*);

private:
  vtkOmicronMeshInputFilter(const vtkOmicronMeshInputFilter&); // Not implemented.
  void operator=(const vtkOmicronMeshInputFilter&); // Not implemented.

//ETX
};

#endif

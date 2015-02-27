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
// .NAME vtkCMBWriter - creates a vtkMultiBlockDataSet
// .SECTION Description
// Filter to output a vtkPolyData to a file with necessary associated
//  BCS, model face, region, and material data.

#ifndef __vtkCMBWriter_h
#define __vtkCMBWriter_h

#include "vtkCMBIOModule.h" // For export macro
#include "vtkMultiBlockDataSetAlgorithm.h"
#include "cmbSystemConfig.h"

class vtkIdList;

class VTKCMBIO_EXPORT vtkCMBWriter : public vtkMultiBlockDataSetAlgorithm
{
public:
  static vtkCMBWriter *New();
  vtkTypeMacro(vtkCMBWriter,vtkMultiBlockDataSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  vtkSetMacro(BinaryOutput, bool);
  vtkGetMacro(BinaryOutput, bool);

  // Description:
  // Writes out the vtkPolyData information by just calling Update()
  // on this filter which will call vtkXMLPolyDataWriter::Write().
  void Write();

protected:
  vtkCMBWriter();
  ~vtkCMBWriter();

  char* FileName;
  bool BinaryOutput;
  virtual int RequestData(vtkInformation *, vtkInformationVector **,
                          vtkInformationVector *);

private:
  vtkCMBWriter(const vtkCMBWriter&);  // Not implemented.
  void operator=(const vtkCMBWriter&);  // Not implemented.
};

#endif

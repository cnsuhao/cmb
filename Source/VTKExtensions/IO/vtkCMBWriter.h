//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
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

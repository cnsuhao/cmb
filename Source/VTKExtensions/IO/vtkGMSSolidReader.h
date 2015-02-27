/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkGMSSolidReader.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkGMSSolidReader - Reader for GMS solid files
// .SECTION Description
// Reads GMS solid files (ASCII only). It is assumed that the
// vertex indices start at 0.

#ifndef __vtkGMSSolidReader_h
#define __vtkGMSSolidReader_h

#include "vtkCMBIOModule.h" // For export macro
#include "vtkMultiBlockDataSetAlgorithm.h"
#include "cmbSystemConfig.h"

//BTX
struct vtkGMSSolidReaderInternals;
//ETX

class vtkCellArray;
class vtkDoubleArray;
class vtkMultiBlockDataSet;
class vtkUnsignedCharArray;

class VTKCMBIO_EXPORT vtkGMSSolidReader : public vtkMultiBlockDataSetAlgorithm
{
public:
  static vtkGMSSolidReader *New();
  vtkTypeMacro(vtkGMSSolidReader,vtkMultiBlockDataSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Name of the file to be read.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

protected:
  vtkGMSSolidReader();
  ~vtkGMSSolidReader();

  int RequestInformation(vtkInformation *,
                         vtkInformationVector **,
                         vtkInformationVector *);
  int RequestData(vtkInformation *,
                  vtkInformationVector **,
                  vtkInformationVector *);

  int ReadSolid(unsigned int block, vtkMultiBlockDataSet* output);
  void ReadTriangles(vtkCellArray*, vtkUnsignedCharArray*);
  void ReadVerts(vtkDoubleArray*);

  char * FileName;

private:
  vtkGMSSolidReader(const vtkGMSSolidReader&);  // Not implemented.
  void operator=(const vtkGMSSolidReader&);  // Not implemented.

  vtkGMSSolidReaderInternals* Internals;
};
#endif

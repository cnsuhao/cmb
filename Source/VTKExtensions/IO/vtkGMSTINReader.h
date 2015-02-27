/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkGMSTINReader.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkGMSTINReader - Reader for GMS TIN files
// .SECTION Description
// Reads GMS TIN files (ASCII only). It is assumed that the
// vertex indices start at 0.

#ifndef __vtkGMSTINReader_h
#define __vtkGMSTINReader_h

#include "vtkCMBIOModule.h" // For export macro
#include "vtkMultiBlockDataSetAlgorithm.h"
#include "cmbSystemConfig.h"

//BTX
struct vtkGMSTINReaderInternals;
//ETX

class vtkCellArray;
class vtkFloatArray;
class vtkMultiBlockDataSet;
class vtkUnsignedCharArray;
class vtkPolyData;

class VTKCMBIO_EXPORT vtkGMSTINReader : public vtkMultiBlockDataSetAlgorithm
{
public:
  static vtkGMSTINReader *New();
  vtkTypeMacro(vtkGMSTINReader,vtkMultiBlockDataSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Name of the file to be read.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

protected:
  vtkGMSTINReader();
  ~vtkGMSTINReader();

  int RequestInformation(vtkInformation *,
                         vtkInformationVector **,
                         vtkInformationVector *);
  int RequestData(vtkInformation *,
                  vtkInformationVector **,
                  vtkInformationVector *);

  int ReadTIN(unsigned int block, vtkMultiBlockDataSet* output);
  void ReadTriangles(vtkCellArray*);
  void ReadVerts(vtkPolyData*);

  char * FileName;

private:
  vtkGMSTINReader(const vtkGMSTINReader&);  // Not implemented.
  void operator=(const vtkGMSTINReader&);  // Not implemented.

  vtkGMSTINReaderInternals* Internals;
};
#endif

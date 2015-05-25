//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkOmicronMeshInputWriter - Writer for Omicron mesh input files.
// .SECTION Description
// vtkOmicronMeshInputWriter writes format appropraite for input into
// Omicron "mesh" program.

#ifndef __vtkOmicronMeshInputWriter_h
#define __vtkOmicronMeshInputWriter_h

#include "vtkCMBIOModule.h" // For export macro
#include "vtkWriter.h"
#include "cmbSystemConfig.h"
class vtkMultiBlockDataSet;

class VTKCMBIO_EXPORT vtkOmicronMeshInputWriter : public vtkWriter
{
public:
  static vtkOmicronMeshInputWriter* New();
  vtkTypeMacro(vtkOmicronMeshInputWriter, vtkWriter);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set the input to this writer.
  void SetInputData(vtkMultiBlockDataSet* dataSet);

  // Description:
  // Get/Set the filename.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);


  // Description:
  // Get/Set the filename of the geometry file associated with this file.
  vtkSetStringMacro(GeometryFileName);
  vtkGetStringMacro(GeometryFileName);

  vtkSetMacro(VolumeConstraint, double);
  vtkGetMacro(VolumeConstraint, double);

  //BTX

protected:
  vtkOmicronMeshInputWriter();
  ~vtkOmicronMeshInputWriter();

  // Actual writing.
  virtual void WriteData();
  char* FileName;
  char* GeometryFileName;
  double VolumeConstraint;

  virtual int FillInputPortInformation(int port, vtkInformation *info);
private:
  vtkOmicronMeshInputWriter(const vtkOmicronMeshInputWriter&); // Not implemented.
  void operator=(const vtkOmicronMeshInputWriter&); // Not implemented.
//ETX
};

#endif



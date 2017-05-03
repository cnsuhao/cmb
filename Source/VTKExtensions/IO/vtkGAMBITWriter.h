//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkGAMBITWriter - creates a GAMBIT Format file for polydata and unstructured grids.
// .SECTION Description
// vtkGAMBITWriter writes creates a GAMBIT Format file for polydata and unstructured grids in ASCII.
// Supported elements are: Triangle, Quad, Tet, Wedge, Pyramid, and Hex (Linear Elements).
// It raises an error if input data has cells that cannot be represented in any
// of these structures.  In the case of polydata it assumes that there are only triangles
// and quads (no triangle strips)
// This can take a vtkMultiGroupDataSet as input, however in that case it write
// the first leaf DataSet out.

#ifndef __vtkGAMBITWriter_h
#define __vtkGAMBITWriter_h

#include "cmbSystemConfig.h"
#include "vtkCMBIOModule.h" // For export macro
#include "vtkWriter.h"
class vtkUnstructuredGrid;
class vtkPolyData;
class vtkDataSet;
class VTKCMBIO_EXPORT vtkGAMBITWriter : public vtkWriter
{
public:
  static vtkGAMBITWriter* New();
  vtkTypeMacro(vtkGAMBITWriter, vtkWriter);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Set the input to this writer.
  void SetInputData(vtkDataObject* ug);

  // Description:
  // Get/Set the filename.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);
  //BTX
protected:
  enum GAMBITCellType
  {
    EDGE = 1,
    QUAD = 2,
    TRI = 3,
    BRICK = 4,
    PRISM = 5,
    TETRA = 6,
    PYRAMID = 7
  };

  vtkGAMBITWriter();
  ~vtkGAMBITWriter() override;

  ostream* OpenFile();
  void CloseFile(ostream* fp);
  bool WriteHeader(ostream& fp);
  bool WriteCells(ostream& fp);
  bool WritePoints(ostream& fp);
  bool WriteGroups(ostream& fp);

  // Actual writing.
  void WriteData() override;
  char* FileName;

  vtkUnstructuredGrid* InputGrid;
  vtkPolyData* InputPoly;

  int FillInputPortInformation(int port, vtkInformation* info) override;

private:
  vtkGAMBITWriter(const vtkGAMBITWriter&); // Not implemented.
  void operator=(const vtkGAMBITWriter&);  // Not implemented.
  //ETX
};

#endif

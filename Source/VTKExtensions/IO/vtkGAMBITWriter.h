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

#include "vtkCMBIOModule.h" // For export macro
#include "vtkWriter.h"
#include "cmbSystemConfig.h"
class vtkUnstructuredGrid;
class vtkPolyData;
class vtkDataSet;
class VTKCMBIO_EXPORT vtkGAMBITWriter : public vtkWriter
{
public:
  static vtkGAMBITWriter* New();
  vtkTypeMacro(vtkGAMBITWriter, vtkWriter);
  void PrintSelf(ostream& os, vtkIndent indent);

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
    EDGE    = 1,
    QUAD    = 2,
    TRI     = 3,
    BRICK   = 4,
    PRISM   = 5,
    TETRA   = 6,
    PYRAMID = 7
  };

  vtkGAMBITWriter();
  ~vtkGAMBITWriter();

  ostream* OpenFile();
  void CloseFile(ostream* fp);
  bool WriteHeader(ostream& fp);
  bool WriteCells(ostream& fp);
  bool WritePoints(ostream& fp);
  bool WriteGroups(ostream& fp);

  // Actual writing.
  virtual void WriteData();
  char* FileName;

  vtkUnstructuredGrid* InputGrid;
  vtkPolyData* InputPoly;

  virtual int FillInputPortInformation(int port, vtkInformation *info);
private:
  vtkGAMBITWriter(const vtkGAMBITWriter&); // Not implemented.
  void operator=(const vtkGAMBITWriter&); // Not implemented.
//ETX
};

#endif

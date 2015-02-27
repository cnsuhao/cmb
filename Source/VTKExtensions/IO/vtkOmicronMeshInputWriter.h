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



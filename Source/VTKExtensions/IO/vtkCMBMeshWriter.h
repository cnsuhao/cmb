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
// .NAME vtkCMBMeshWriter - Writer for CMB ADH, WASH123D, PT123, and XMS
// mesh files
// .SECTION Description
// vtkCMBMeshWriter writes 1D, 2D, 3D meshes in ASCII.
// It raises an error if the input data has cells that cannot be
// represented by the file format.
// This can take a vtkMultiGroupDataSet as input, however in that case it
// writes the first leaf vtkPointSet out.

#ifndef __vtkCMBMeshWriter_h
#define __vtkCMBMeshWriter_h

#include "vtkCMBIOModule.h" // For export macro
#include "vtkWriter.h"
#include "string"
#include "cmbSystemConfig.h"

//BTX
struct vtkCMBMeshWriterInternals;
//ETX

//class vtkDataSet;
class VTKCMBIO_EXPORT vtkCMBMeshWriter : public vtkWriter
{
public:
  //BTX
  enum vtkCMBMeshDimension { MESH1D = 1, MESH2D = 2, MESH3D = 3 };
  // Make sure to update vtkCMBMeshFormatStrings in vtkCMBMeshWriter.cxx
  enum vtkCMBMeshFormat { ADH = 0, PT123 = 1, WASH123D = 2, XMS = 3,
    NUMBER_OF_FORMATS = 4 };
  //ETX

  static vtkCMBMeshWriter* New();
  vtkTypeMacro(vtkCMBMeshWriter, vtkWriter);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set the input to this writer.
  void SetInputData(vtkDataObject* ug);

  // Description:
  // Get/Set the filename.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  // Description:
  // Get/Set the mesh dimension (MESH1D, MESH2D, MESH3D). Valid range (1,3).
  vtkSetClampMacro(MeshDimension, int, 1, 3);
  vtkGetMacro(MeshDimension, int);

  // Description:
  // Turn on/off the validation of elements of differing dimension.
  vtkBooleanMacro(ValidateDimension, bool);
  vtkSetMacro(ValidateDimension, bool);
  vtkGetMacro(ValidateDimension, bool);

  // Description:
  // Get/Set the file format (ADH, PT123, WASH123D, XMS). Valid range set by vtkCMBMeshFormat enumeration.
  vtkSetClampMacro(FileFormat, int, 0, NUMBER_OF_FORMATS - 1);
  vtkGetMacro(FileFormat, int);

  // Description:
  // Turn on/off the inclusion of geometry meta information.
  vtkBooleanMacro(WriteMetaInfo, bool);
  vtkSetMacro(WriteMetaInfo, bool);
  vtkGetMacro(WriteMetaInfo, bool);

  // Description:
  // Turn on/off the use of scientific notation for writing floating point values.
  vtkBooleanMacro(UseScientificNotation, bool);
  vtkSetMacro(UseScientificNotation, bool);
  vtkGetMacro(UseScientificNotation, bool);

  // Description:
  // Get/Set the precision for writing floating point values.
  vtkSetMacro(FloatPrecision, int);
  vtkGetMacro(FloatPrecision, int);

//BTX
protected:
  vtkCMBMeshWriter();
  ~vtkCMBMeshWriter();

  ostream* OpenFile();
  void CloseFile(ostream* fp);
  bool WriteHeader(ostream& fp);
  bool WriteCells(ostream& fp);
  bool WritePoints(ostream& fp);
  bool WriteFooter(ostream& fp);
  bool ValidateFileFormat();

  // Actual writing.
  virtual void WriteData();
  char* FileName;
  int FileFormat;

  bool ValidateDimension;
  bool WriteMetaInfo;
  bool UseScientificNotation;
  int FloatPrecision;
  int MeshDimension;

  virtual int FillInputPortInformation(int port, vtkInformation *info);
private:
  vtkCMBMeshWriter(const vtkCMBMeshWriter&); // Not implemented.
  void operator=(const vtkCMBMeshWriter&); // Not implemented.

  vtkCMBMeshWriterInternals* Internals;
//ETX
};

#endif

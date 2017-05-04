//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBPtsWriter - Writer to produce a Points File (.pts)
// .SECTION Description
// vtkCMBPtsWriter writes Points and classification in ASCII.
// This can take a vtkMultiGroupDataSet as input, however in that case it uses
// the first leaf vtkDataSet.

#ifndef __vtkCMBPtsWriter_h
#define __vtkCMBPtsWriter_h

#include "cmbSystemConfig.h"
#include "vtkCMBIOModule.h" // For export macro
#include "vtkWriter.h"
#include <string>
class vtkIdTypeArray;
class vtkDataSet;

class VTKCMBIO_EXPORT vtkCMBPtsWriter : public vtkWriter
{
public:
  static vtkCMBPtsWriter* New();
  vtkTypeMacro(vtkCMBPtsWriter, vtkWriter);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Set the input to this writer.
  void SetInputData(vtkDataObject* ug);

  // Description:
  // Get/Set the filename.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  // Description:
  // Get/Set an optional header.
  vtkSetStringMacro(Header);
  vtkGetStringMacro(Header);

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
  vtkCMBPtsWriter();
  ~vtkCMBPtsWriter() override;

  ostream* OpenFile();
  void CloseFile(ostream* fp);
  bool WriteHeader(ostream& fp);
  bool WritePoints(ostream& fp);
  bool WriteFooter(ostream& fp);

  // Actual writing.
  void WriteData() override;
  char* FileName;
  char* Header;
  bool UseScientificNotation;
  int FloatPrecision;
  vtkDataSet* MyGeom;
  vtkIdTypeArray* MyData;

  int FillInputPortInformation(int port, vtkInformation* info) override;

private:
  vtkCMBPtsWriter(const vtkCMBPtsWriter&); // Not implemented.
  void operator=(const vtkCMBPtsWriter&);  // Not implemented.
  //ETX
};

#endif

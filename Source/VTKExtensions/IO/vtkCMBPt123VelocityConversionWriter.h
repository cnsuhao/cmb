//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBPt123VelocityConversionWriter - Writer to produce a PT123 Velocity VN
// based on a 3 component Data Array on a Data Set's Points
// .SECTION Description
// vtkCMBPt123VelocityConversionWriter writes a Vector Field in ASCII.
// This can take a vtkMultiGroupDataSet as input, however in that case it uses
// the first leaf vtkDataSet.

#ifndef __vtkCMBPt123VelocityConversionWriter_h
#define __vtkCMBPt123VelocityConversionWriter_h

#include "cmbSystemConfig.h"
#include "string"
#include "vtkCMBIOModule.h" // For export macro
#include "vtkWriter.h"
class vtkDataArray;
class VTKCMBIO_EXPORT vtkCMBPt123VelocityConversionWriter : public vtkWriter
{
public:
  static vtkCMBPt123VelocityConversionWriter* New();
  vtkTypeMacro(vtkCMBPt123VelocityConversionWriter, vtkWriter);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Set the input to this writer.
  void SetInputData(vtkDataObject* ug);

  // Description:
  // Get/Set the filename.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

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
  vtkCMBPt123VelocityConversionWriter();
  ~vtkCMBPt123VelocityConversionWriter() override;

  ostream* OpenFile();
  void CloseFile(ostream* fp);
  bool WriteHeader(ostream& fp);
  bool WriteTimeStep(ostream& fp, double t);
  bool WriteFooter(ostream& fp);

  // Actual writing.
  void WriteData() override;
  char* FileName;
  bool UseScientificNotation;
  int FloatPrecision;
  vtkDataArray* MyData;

  int FillInputPortInformation(int port, vtkInformation* info) override;

private:
  vtkCMBPt123VelocityConversionWriter(
    const vtkCMBPt123VelocityConversionWriter&);              // Not implemented.
  void operator=(const vtkCMBPt123VelocityConversionWriter&); // Not implemented.
  //ETX
};

#endif

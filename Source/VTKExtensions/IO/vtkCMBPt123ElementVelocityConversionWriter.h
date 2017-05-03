//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBPt123ElementVelocityConversionWriter - Writer to produce a PT123 element-based
// velocity conversion factor based on a Data Set's Cell Data
// .SECTION Description
// vtkCMBPt123VelocityConversionWriter writes a Scalar Field in ASCII.
// This can take a vtkMultiGroupDataSet as input, however in that case it uses
// the first leaf vtkDataSet.

#ifndef __vtkCMBPt123ElementVelocityConversionWriter_h
#define __vtkCMBPt123ElementVelocityConversionWriter_h

#include "cmbSystemConfig.h"
#include "string"
#include "vtkCMBIOModule.h" // For export macro
#include "vtkWriter.h"
class vtkDataArray;

class VTKCMBIO_EXPORT vtkCMBPt123ElementVelocityConversionWriter : public vtkWriter
{
public:
  static vtkCMBPt123ElementVelocityConversionWriter* New();
  vtkTypeMacro(vtkCMBPt123ElementVelocityConversionWriter, vtkWriter);
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
  vtkCMBPt123ElementVelocityConversionWriter();
  ~vtkCMBPt123ElementVelocityConversionWriter() override;

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
  vtkCMBPt123ElementVelocityConversionWriter(
    const vtkCMBPt123ElementVelocityConversionWriter&);              // Not implemented.
  void operator=(const vtkCMBPt123ElementVelocityConversionWriter&); // Not implemented.
  //ETX
};

#endif

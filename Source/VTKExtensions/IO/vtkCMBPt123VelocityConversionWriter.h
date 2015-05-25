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

#include "vtkCMBIOModule.h" // For export macro
#include "vtkWriter.h"
#include "string"
#include "cmbSystemConfig.h"
class vtkDataArray;
class VTKCMBIO_EXPORT vtkCMBPt123VelocityConversionWriter : public vtkWriter
{
public:

  static vtkCMBPt123VelocityConversionWriter* New();
  vtkTypeMacro(vtkCMBPt123VelocityConversionWriter, vtkWriter);
  void PrintSelf(ostream& os, vtkIndent indent);

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
  ~vtkCMBPt123VelocityConversionWriter();

  ostream* OpenFile();
  void CloseFile(ostream* fp);
  bool WriteHeader(ostream& fp);
  bool WriteTimeStep(ostream& fp, double t);
  bool WriteFooter(ostream& fp);

  // Actual writing.
  virtual void WriteData();
  char* FileName;
  bool UseScientificNotation;
  int FloatPrecision;
  vtkDataArray *MyData;

  virtual int FillInputPortInformation(int port, vtkInformation *info);
private:
  vtkCMBPt123VelocityConversionWriter(const vtkCMBPt123VelocityConversionWriter&); // Not implemented.
  void operator=(const vtkCMBPt123VelocityConversionWriter&); // Not implemented.
//ETX
};

#endif



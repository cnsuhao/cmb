//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBPt123VelocityWriter - Writer to produce a PT123 Velocity VN
// based on a 3 component Data Array on a Data Set's Points
// .SECTION Description
// vtkCMBPt123VelocityWriter writes a Vector Field in ASCII.
// This can take a vtkMultiGroupDataSet as input, however in that case it uses
// the first leaf vtkDataSet.

#ifndef __vtkCMBPt123VelocityWriter_h
#define __vtkCMBPt123VelocityWriter_h

#include "cmbSystemConfig.h"
#include "string"
#include "vtkCMBIOModule.h" // For export macro
#include "vtkWriter.h"
class vtkDataArray;
class vtkDataSet;
class VTKCMBIO_EXPORT vtkCMBPt123VelocityWriter : public vtkWriter
{
public:
  static vtkCMBPt123VelocityWriter* New();
  vtkTypeMacro(vtkCMBPt123VelocityWriter, vtkWriter);
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

  // Description:
  // Get/Set the type of velocity field to write (element based or nodal)
  vtkSetMacro(WriteCellBased, bool);
  vtkGetMacro(WriteCellBased, bool);

  // Description:
  // Get/Set the actual spatial dimension of the velocity field
  vtkSetMacro(SpatialDimension, int);
  vtkGetMacro(SpatialDimension, int);
  //BTX
protected:
  vtkCMBPt123VelocityWriter();
  ~vtkCMBPt123VelocityWriter() override;

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
  bool WriteCellBased;
  int SpatialDimension;
  vtkDataSet* MyDataSet;

  int FillInputPortInformation(int port, vtkInformation* info) override;

private:
  vtkCMBPt123VelocityWriter(const vtkCMBPt123VelocityWriter&); // Not implemented.
  void operator=(const vtkCMBPt123VelocityWriter&);            // Not implemented.
  //ETX
};

#endif

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBPt123PointsWriter - Writer to produce a PT123 Points (.pt)
// based on a scalar classification Data Array on a Data Set's Points
// .SECTION Description
// vtkCMBPt123PointsWriter writes Points and classification in ASCII.
// This can take a vtkMultiGroupDataSet as input, however in that case it uses
// the first leaf vtkDataSet.

#ifndef __vtkCMBPt123PointsWriter_h
#define __vtkCMBPt123PointsWriter_h

#include "vtkCMBIOModule.h" // For export macro
#include "vtkWriter.h"
#include "cmbSystemConfig.h"
#include "string"
class vtkIdTypeArray;
class vtkDataSet;

class VTKCMBIO_EXPORT vtkCMBPt123PointsWriter : public vtkWriter
{
public:

  static vtkCMBPt123PointsWriter* New();
  vtkTypeMacro(vtkCMBPt123PointsWriter, vtkWriter);
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
  vtkCMBPt123PointsWriter();
  ~vtkCMBPt123PointsWriter() override;

  ostream* OpenFile();
  void CloseFile(ostream* fp);
  bool WriteHeader(ostream& fp);
  bool WritePoints(ostream& fp);
  bool WriteFooter(ostream& fp);

  // Actual writing.
  void WriteData() override;
  char* FileName;
  char * Header;
  bool UseScientificNotation;
  int FloatPrecision;
  vtkDataSet *MyGeom;
  vtkIdTypeArray *MyData;

  int FillInputPortInformation(int port, vtkInformation *info) override;
private:
  vtkCMBPt123PointsWriter(const vtkCMBPt123PointsWriter&); // Not implemented.
  void operator=(const vtkCMBPt123PointsWriter&); // Not implemented.
//ETX
};

#endif



//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBICMReader
// .SECTION Description
// Reads ICM .dat files and coresponding time data
// By default it assumes a Long/Lat dataset with positive
// latitude values going east
// into a vtkUnstructuredGrid.

#ifndef __vtkCMBICMReader_h
#define __vtkCMBICMReader_h

#include "cmbSystemConfig.h"
#include "vtkCMBIOModule.h" // For export macro
#include "vtkFloatArray.h"
#include "vtkHexahedron.h"
#include "vtkPointData.h"
#include "vtkSmartPointer.h"
#include "vtkUnstructuredGridAlgorithm.h"

class VTKCMBIO_EXPORT vtkCMBICMReader : public vtkUnstructuredGridAlgorithm
{
public:
  static vtkCMBICMReader* New();
  vtkTypeMacro(vtkCMBICMReader, vtkUnstructuredGridAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Name of the file to be read.
  vtkSetStringMacro(DataFileName);
  vtkGetStringMacro(DataFileName);
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  vtkGetMacro(NumberOfTimeSteps, int);
  vtkGetVector2Macro(TimeStepRange, double);
  vtkSetVector2Macro(TimeStepRange, double);

  vtkSetMacro(DataIsLatLong, bool);
  vtkGetMacro(DataIsLatLong, bool);

  vtkSetMacro(DataIsPositiveEast, bool);
  vtkGetMacro(DataIsPositiveEast, bool);

protected:
  vtkCMBICMReader();
  ~vtkCMBICMReader() override;

  int RequestInformation(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;
  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

  int ReadGeometryData(vtkInformationVector* outputVector);
  int ReadTemporalData(vtkInformationVector* outputVector);

  char* FileName;
  char* DataFileName;

  bool DataIsLatLong;
  bool DataIsPositiveEast;

  //Values to check to see if
  //we need to reread geometry
  //or temporal data
  char* OldDataFileName;
  char* OldFileName;
  vtkSetStringMacro(OldDataFileName);
  vtkSetStringMacro(OldFileName);
  bool OldDataIsLatLong;
  bool OldDataIsPositiveEast;

  int TimeStep;
  double TimeValue;
  double* TimeSteps;

  int NumberOfTimeSteps;
  int NumberOfPoints;
  int NumberOfCells;

  // Descriptions:
  // Store the range of time steps
  double TimeStepRange[2];

  vtkPoints* points;
  vtkFloatArray* vectors;

  //BTX
  //The Cells
  vtkCellArray* HexCells;
  //The Color of the Cells
  std::vector<vtkSmartPointer<vtkFloatArray> > CellData;
  //ETX
private:
  vtkCMBICMReader(const vtkCMBICMReader&); // Not implemented.
  void operator=(const vtkCMBICMReader&);  // Not implemented.
};

#endif

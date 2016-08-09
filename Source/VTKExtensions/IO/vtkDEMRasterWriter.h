//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

// .NAME vtkDEMRasterWriter - Writer for DEM Raster Files
// .SECTION Description

#ifndef __DEMRasterWriter_h
#define __DEMRasterWriter_h

#include "vtkCMBIOModule.h" // For export macro
#include "vtkWriter.h"
#include "cmbSystemConfig.h"
#include <map>

class vtkPolyData;

#define VTK_ASCII 1
#define VTK_BINARY 2

class VTKCMBIO_EXPORT vtkDEMRasterWriter : public vtkWriter
{
public:
  static vtkDEMRasterWriter *New();
  vtkTypeMacro(vtkDEMRasterWriter,vtkWriter);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get/Set the filename.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  // Description:
  // Add an input to this writer
  void AddInputData(vtkDataObject *input) {this->AddInputData(0, input);}
  void AddInputData(int, vtkDataObject*);

  // Description:
  // Set/Get whether or not to write multiple pieces as a single piece
  vtkBooleanMacro(WriteAsSinglePiece, bool);
  vtkSetMacro(WriteAsSinglePiece, bool);
  vtkGetMacro(WriteAsSinglePiece, bool);

  vtkSetMacro(Zone, int);

  vtkBooleanMacro(IsNorth, bool);
  vtkSetMacro(IsNorth, bool);

  vtkSetVector2Macro(RasterSize, int);
  vtkSetMacro(RadiusX, double);
  vtkSetMacro(RadiusY, double);

  vtkSetMacro(Scale, double);

  //BTX
  // Description:
  // Unlike vtkWriter which assumes data per port - this Writer can have multiple connections
  // on Port 0
  vtkDataObject *GetInputFromPort0(int connection);
  vtkDataObject *GetInputFromPort0() { return this->GetInputFromPort0( 0 ); };
  //ETX

  //BTX

protected:
  vtkDEMRasterWriter();
  ~vtkDEMRasterWriter();

  int Zone;
  bool IsNorth;

  int RasterSize[2];
  double RadiusX, RadiusY;

  double Scale;

  // Actual writing.
  virtual void WriteData();

  char* FileName;
  bool WriteAsSinglePiece;

  virtual int FillInputPortInformation(int port, vtkInformation *info);

  void createOutputFile(std::string fname, vtkPolyData *inputPoly);


private:
  vtkDEMRasterWriter(const vtkDEMRasterWriter&);  // Not implemented.
  void operator=(const vtkDEMRasterWriter&);  // Not implemented.

  //ETX
};

#endif

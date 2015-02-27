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
// .NAME vtkLIDARPtsWriter - Writer for LIDAR point files
// .SECTION Description

#ifndef __LIDARPtsWriter_h
#define __LIDARPtsWriter_h

#include "vtkCMBIOModule.h" // For export macro
#include "vtkWriter.h"
#include "cmbSystemConfig.h"
#include <map>

class vtkPolyData;

#define VTK_ASCII 1
#define VTK_BINARY 2

class VTKCMBIO_EXPORT vtkLIDARPtsWriter : public vtkWriter
{
public:
  static vtkLIDARPtsWriter *New();
  vtkTypeMacro(vtkLIDARPtsWriter,vtkWriter);
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

//BTX
  // Description:
  // Unlike vtkWriter which assumes data per port - this Writer can have multiple connections
  // on Port 0
  vtkDataObject *GetInputFromPort0(int connection);
  vtkDataObject *GetInputFromPort0() { return this->GetInputFromPort0( 0 ); };
//ETX

  //BTX

protected:
  vtkLIDARPtsWriter();
  ~vtkLIDARPtsWriter();

  // Actual writing.
  virtual void WriteData();
  // return write_status: OK, Abort, or Error
  int WriteFile(ofstream& ofp);
  int ComputeRequiredAxisPrecision(double min, double max);
  int WritePoints(ofstream& ofp, vtkPolyData *inputPoly);

  ofstream* OpenOutputFile();
  bool IsBinaryType(const char* filename);

  void CloseFile(ios* fp);

  char* FileName;
  int OutputIsBinary;
  bool WriteAsSinglePiece;

  virtual int FillInputPortInformation(int port, vtkInformation *info);


private:
  vtkLIDARPtsWriter(const vtkLIDARPtsWriter&);  // Not implemented.
  void operator=(const vtkLIDARPtsWriter&);  // Not implemented.

  //ETX
};

#endif

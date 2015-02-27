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
// .NAME vtkCMBPtsWriter - Writer to produce a Points File (.pts)
// .SECTION Description
// vtkCMBPtsWriter writes Points and classification in ASCII.
// This can take a vtkMultiGroupDataSet as input, however in that case it uses
// the first leaf vtkDataSet.

#ifndef __vtkCMBPtsWriter_h
#define __vtkCMBPtsWriter_h

#include "vtkCMBIOModule.h" // For export macro
#include "vtkWriter.h"
#include <string>
#include "cmbSystemConfig.h"
class vtkIdTypeArray;
class vtkDataSet;

class VTKCMBIO_EXPORT vtkCMBPtsWriter : public vtkWriter
{
public:

  static vtkCMBPtsWriter* New();
  vtkTypeMacro(vtkCMBPtsWriter, vtkWriter);
  void PrintSelf(ostream& os, vtkIndent indent);

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
  ~vtkCMBPtsWriter();

  ostream* OpenFile();
  void CloseFile(ostream* fp);
  bool WriteHeader(ostream& fp);
  bool WritePoints(ostream& fp);
  bool WriteFooter(ostream& fp);

  // Actual writing.
  virtual void WriteData();
  char* FileName;
  char * Header;
  bool UseScientificNotation;
  int FloatPrecision;
  vtkDataSet *MyGeom;
  vtkIdTypeArray *MyData;

  virtual int FillInputPortInformation(int port, vtkInformation *info);
private:
  vtkCMBPtsWriter(const vtkCMBPtsWriter&); // Not implemented.
  void operator=(const vtkCMBPtsWriter&); // Not implemented.
//ETX
};

#endif

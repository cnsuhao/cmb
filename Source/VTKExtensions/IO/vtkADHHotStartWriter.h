//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkADHHotStartWriter - Write .hot file given one or more scalar
// and/or vector arrays
//
// .SECTION Description
//

#ifndef __vtkADHHotStartWriter_h
#define __vtkADHHotStartWriter_h

#include "vtkCMBIOModule.h" // For export macro
#include "vtkWriter.h"
#include "string"
#include "cmbSystemConfig.h"

//BTX
struct vtkADHHotStartWriterInternal;
//ETX

class VTKCMBIO_EXPORT vtkADHHotStartWriter : public vtkWriter
{
public:

  static vtkADHHotStartWriter* New();
  vtkTypeMacro(vtkADHHotStartWriter, vtkWriter);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get/Set the filename.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  // Description:
  // Add to list of arrays which are used for writing .hot file.
  void AddInputPointArrayToProcess(const char* name);
  void ClearInputPointArrayToProcess();

//BTX
protected:
  vtkADHHotStartWriter();
  ~vtkADHHotStartWriter();

  ostream* OpenFile();
  void CloseFile(ostream* fp);

  // Actual writing.
  virtual void WriteData();
  //bool WriteHeader(ostream& fp);
  bool WriteArrays(ostream& fp);
  bool WriteArrayHeader(ostream& fp, vtkDataArray* darray);
  bool WriteArray(ostream& fp, vtkDataArray* darray);
  bool WriteArrayFooter(ostream& fp);

  virtual int FillInputPortInformation(int port, vtkInformation *info);


  char* FileName;

private:
  vtkADHHotStartWriter(const vtkADHHotStartWriter&);  // Not implemented.
  void operator=(const vtkADHHotStartWriter&);    // Not implemented.

  vtkADHHotStartWriterInternal* Implementation;
//ETX
};

#endif // __vtkADHHotStartWriter_h

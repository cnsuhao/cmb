/*=========================================================================

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
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

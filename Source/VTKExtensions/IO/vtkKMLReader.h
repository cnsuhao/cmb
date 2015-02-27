/*=========================================================================

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive,
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
// .NAME vtkKMLReader - Read in OGC KML file into VTK data structure.
// .SECTION Description
// vtkKML required libkml and support reading kml tags specific to polygon
// dataset. Currently it supports minimal features to support the requirements.


#ifndef __vtkKMLReader_h
#define __vtkKMLReader_h

#include "vtkCMBIOModule.h" // For export macro
#include <vtkPolyDataAlgorithm.h>
#include "cmbSystemConfig.h"

// Forward declarations.
class vtkKMLReaderInternal;

class VTKCMBIO_EXPORT vtkKMLReader : public vtkPolyDataAlgorithm
{
public:

  // Usual VTK functions.
  vtkTypeMacro(vtkKMLReader, vtkPolyDataAlgorithm);
  static vtkKMLReader* New();
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Get / Set the KML file with the path to be read.
  vtkGetStringMacro(FileName);
  vtkSetStringMacro(FileName);


protected:
  // Constructor / Destructor.
  vtkKMLReader();
  virtual ~vtkKMLReader();

  // Overridden function. This is where the conversion from KML
  // to VTK data structure happens.
  virtual int RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector);

private:
  vtkKMLReader   (const vtkKMLReader&);    // Not implemented.
  void operator= (const vtkKMLReader&);    // Not implemented.

  char* FileName;

  vtkKMLReaderInternal* Implementation;
};

#endif // _vtkKMLReader_h

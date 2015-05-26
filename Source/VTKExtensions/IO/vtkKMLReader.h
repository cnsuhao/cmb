//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
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

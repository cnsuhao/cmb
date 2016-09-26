//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBPolyReader - Reader for *.poly files
// .SECTION Description
// Reader for *.poly files.  The format is simple, conatining only points and
// then indices for each facet, which is made up of polygons.  Currently
// the reader doesn't handle holes in facets (ignores them), but does add
// field data for each facet (the facet index) as well as the boundary marker
// value (if present).

#ifndef __vtkCMBPolyReader_h
#define __vtkCMBPolyReader_h

#include "vtkCMBIOModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"
#include "cmbSystemConfig.h"

class VTKCMBIO_EXPORT vtkCMBPolyReader : public vtkPolyDataAlgorithm
{
public:
  static vtkCMBPolyReader *New();
  vtkTypeMacro(vtkCMBPolyReader,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Name of the file to be read.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

protected:
  vtkCMBPolyReader();
  ~vtkCMBPolyReader() override;

  int RequestInformation(vtkInformation *,
                         vtkInformationVector **,
                         vtkInformationVector *) override;
  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *) override;
  char *FileName;

  //BTX
  // Description:
  // Get next line of data (and put in lineStream); skips over comments or blank lines
  int GetNextLineOfData(ifstream &fin, std::stringstream &lineStream);
  // For reading 2 Dimemnsional (line only) files
  int Read2DFile(ifstream &fin, int numberOfPoints, int numberOfAttributes, int hasBoundaryMarkers,
                 vtkPolyData *output);
  // For reading 3 Dimemnsional (line only) files
  int Read3DFile(ifstream &fin,int numberOfPoints, int numberOfAttributes, int hasBoundaryMarkers,
                 vtkPolyData *output);
  //ETX


private:
  vtkCMBPolyReader(const vtkCMBPolyReader&);  // Not implemented.
  void operator=(const vtkCMBPolyReader&);  // Not implemented.
};

#endif

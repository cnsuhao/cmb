//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkOmicronModelInputReader - Reader for Omicron "model" input files
// .SECTION Description
// Reads files of type expexted by Omicron "model" program (ASCII only).
// This is the file output by SceneGen (and also very similar to the
// SceneGen input?).  The "important" components of the file which are read
// are the "volume_constraint" (put in field data), each model name and the
// corresponding translation component (which is the "point inside object")
// that the CMB will write out for input into the "mesh" program.  All of
// the above is saved as field data, but the point inside coordinates are
// also saved as points to simplify visualization.

#ifndef __OmicronModelInputReader_h
#define __OmicronModelInputReader_h

#include "vtkCMBIOModule.h" // For export macro
#include "vtkMultiBlockDataSetAlgorithm.h"
#include "cmbSystemConfig.h"

class vtkMultiBlockDataSet;
class vtkPolyData;

class VTKCMBIO_EXPORT vtkOmicronModelInputReader : public vtkMultiBlockDataSetAlgorithm
{
public:
  static vtkOmicronModelInputReader *New();
  vtkTypeMacro(vtkOmicronModelInputReader,vtkMultiBlockDataSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Name of the file to be read.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  // Description:
  // Flag indicating whether or not to load geometry
  vtkBooleanMacro(LoadGeometry, bool);
  vtkSetMacro(LoadGeometry, bool);
  vtkGetMacro(LoadGeometry, bool);

  //BTX
protected:
  vtkOmicronModelInputReader();
  ~vtkOmicronModelInputReader();

  char *FileName;
  bool LoadGeometry;

  vtkPolyData* AddBlock(vtkMultiBlockDataSet *output, const char *fileName,
    double translation[3], double rotation[3], double scale, double color[3],
    const char *additionalIdentifier=0);
  int AddROIBlock(vtkMultiBlockDataSet *output, double (*boundaryCoordinates)[2],
    vtkPolyData *surface, double translation[3], double bottom);

  int RequestInformation(vtkInformation *,
                         vtkInformationVector **,
                         vtkInformationVector *);
  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

private:
  vtkOmicronModelInputReader(const vtkOmicronModelInputReader&);  // Not implemented.
  void operator=(const vtkOmicronModelInputReader&);  // Not implemented.
  //ETX
};

#endif

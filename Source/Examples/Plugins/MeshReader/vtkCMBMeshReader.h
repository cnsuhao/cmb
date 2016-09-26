//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBMeshReader - Reader for cmbmesh (GAMBIT neutral) files
// .SECTION Description

#ifndef __CMBMeshReader_h
#define __CMBMeshReader_h

#include "vtkUnstructuredGridAlgorithm.h"

class vtkDoubleArray;
class vtkCMBMeshReader : public vtkUnstructuredGridAlgorithm
{
public:
  static vtkCMBMeshReader *New();
  vtkTypeMacro(vtkCMBMeshReader,vtkUnstructuredGridAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Specify the file name of the GAMBIT data file to read.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  // Description:
  // Get the total number of cells. The number of cells is only valid after a
  // successful read of the data file is performed.
  vtkGetMacro(NumberOfCells,int);

  // Description:
  // Get the total number of nodes. The number of nodes is only valid after a
  // successful read of the data file is performed.
  vtkGetMacro(NumberOfNodes,int);

  // Description:
  // Get the number of data components at the nodes and cells.
  vtkGetMacro(NumberOfNodeFields,int);
  vtkGetMacro(NumberOfCellFields,int);

protected:
  vtkCMBMeshReader();
  ~vtkCMBMeshReader() override;
  int RequestInformation(vtkInformation *, vtkInformationVector **, vtkInformationVector *) override;
  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *) override;

  char *FileName;

  int NumberOfNodes;
  int NumberOfCells;
  int NumberOfNodeFields;
  int NumberOfCellFields;
  int NumberOfElementGroups;
  int NumberOfBoundaryConditionSets;
  int NumberOfCoordinateDirections;
  int NumberOfVelocityComponents;
  ifstream *FileStream;

  //BTX
  enum GAMBITCellType
  {
    EDGE    = 1,
    QUAD    = 2,
    TRI     = 3,
    BRICK   = 4,
    PRISM   = 5,
    TETRA   = 6,
    PYRAMID = 7
  };
  //ETX

private:
  void ReadFile(vtkUnstructuredGrid *output);
  void ReadGeometry(vtkUnstructuredGrid *output);
  void ReadNodeData(vtkUnstructuredGrid *output);
  void ReadCellData(vtkUnstructuredGrid *output);

  void ReadXYZCoords(vtkDoubleArray *coords);

  void ReadCellConnectivity(vtkUnstructuredGrid *output);
  void ReadMaterialTypes(vtkUnstructuredGrid *output);
  void ReadBoundaryConditionSets(vtkUnstructuredGrid *output);

  vtkCMBMeshReader(const vtkCMBMeshReader&);  // Not implemented.
  void operator=(const vtkCMBMeshReader&);  // Not implemented.
};

#endif

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkSceneGenVegetationReader - reader for SceneGen vegetation file
// .SECTION Description
// Reader for SceneGen vegetation file.

#ifndef __SceneGenVegetationReader_h
#define __SceneGenVegetationReader_h

#include "cmbSystemConfig.h"
#include "vtkCMBIOModule.h" // For export macro
#include "vtkMultiBlockDataSetAlgorithm.h"
#include <map>
#include <string>
#include <vector>

class vtkMultiBlockDataSet;
class vtkPolyData;

class VTKCMBIO_EXPORT vtkSceneGenVegetationReader : public vtkMultiBlockDataSetAlgorithm
{
public:
  static vtkSceneGenVegetationReader* New();
  vtkTypeMacro(vtkSceneGenVegetationReader, vtkMultiBlockDataSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Name of the file to be read.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  //BTX

protected:
  vtkSceneGenVegetationReader();
  ~vtkSceneGenVegetationReader() override;

  char* FileName;
  std::string NodeFile;
  std::string EnsightNodeFile;
  std::string MetFile;
  long StartSimTime;
  long EndSimTime;
  double MetWindHeight;
  std::string OutputMesh;
  std::string EnsightOutputMesh;
  long InputFluxFile;
  std::vector<std::string> EnsightStomatal;

  struct VegetationModel
  {
    VegetationModel()
    {
      this->LeafSize = -1;
      this->Material = -1;
      this->Dataset = 0;
    }
    std::string FileName;
    double LeafSize;
    long Material;
    vtkPolyData* Dataset;
  };
  std::map<std::string, VegetationModel> Models;

  void ClearModel();

  int AddBlock(vtkMultiBlockDataSet* output, VegetationModel& model, double scale, double zRotation,
    double translation[3], double color[3]);

  int RequestInformation(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;
  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

private:
  vtkSceneGenVegetationReader(const vtkSceneGenVegetationReader&); // Not implemented.
  void operator=(const vtkSceneGenVegetationReader&);              // Not implemented.

  //ETX
};

#endif

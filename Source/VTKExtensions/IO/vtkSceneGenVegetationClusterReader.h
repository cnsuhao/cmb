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
// .NAME vtkSceneGenVegetationClusterReader - reader for SceneGen vegetation file
// .SECTION Description
// Reader for SceneGen vegetation file.

#ifndef __SceneGenVegetationClusterReader_h
#define __SceneGenVegetationClusterReader_h

#include "vtkCMBIOModule.h" // For export macro
#include "vtkMultiBlockDataSetAlgorithm.h"
#include "cmbSystemConfig.h"
#include <string>
#include <vector>
#include <map>

class vtkMultiBlockDataSet;
class vtkPolyData;

class VTKCMBIO_EXPORT vtkSceneGenVegetationClusterReader : public vtkMultiBlockDataSetAlgorithm
{
public:
  static vtkSceneGenVegetationClusterReader *New();
  vtkTypeMacro(vtkSceneGenVegetationClusterReader,vtkMultiBlockDataSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Name of the file to be read.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  //BTX

protected:
  vtkSceneGenVegetationClusterReader();
  ~vtkSceneGenVegetationClusterReader();

  char *FileName;
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
      this->Plants = 0;
      this->Scale = 0.0;
      }
    std::string FileName;
    double LeafSize;
    long Material;
    vtkPolyData *Dataset;
    vtkPolyData *Plants;
    double Scale;
    };
  std::map<std::string, VegetationModel> Models;

  void ClearModel();

  int AddBlock(vtkMultiBlockDataSet *output, VegetationModel &model, double color[3]);

  int RequestInformation(vtkInformation *,
                         vtkInformationVector **,
                         vtkInformationVector *);
  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

private:
  vtkSceneGenVegetationClusterReader(const vtkSceneGenVegetationClusterReader&);  // Not implemented.
  void operator=(const vtkSceneGenVegetationClusterReader&);  // Not implemented.

  //ETX
};

#endif

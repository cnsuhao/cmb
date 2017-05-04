//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBBorFileReader - "reader" for the BorFile formats
// .SECTION Description - This reader reads in CMB/GMS .bor file and
//   outputs a multiblock dataset contains all the boreholes and
//   cross-sections in the .bor file. The first block(0) is a multiblock
//   contains all the boreholes as its children, and each borehole is
//   a polydata generated using vtkTubeFilter given the borehole line.
//   The second block(1) is also a multiblock contains all the cross sections
//   as its children, and each cross-section is a polydata output from
//   vtkTriangleFilter taking the polygon input from bor file. Lines and
//   vertices are also added to the cross-sections.

#ifndef __vtkCMBBorFileReader__
#define __vtkCMBBorFileReader__

#include "cmbSystemConfig.h"
#include "vtkCMBIOModule.h" // For export macro
#include <map>
#include <vector>
#include <vtkMultiBlockDataSetAlgorithm.h>

class vtkMultiBlockDataSet;
class BorHoleInfo;
class BorCrossSection;

class VTKCMBIO_EXPORT vtkCMBBorFileReader : public vtkMultiBlockDataSetAlgorithm
{
public:
  static vtkCMBBorFileReader* New();
  void PrintSelf(ostream& os, vtkIndent indent) override;
  vtkTypeMacro(vtkCMBBorFileReader, vtkMultiBlockDataSetAlgorithm);

  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);
  vtkGetMacro(NumberOfBoreholes, int);
  vtkGetMacro(NumberOfCrossSections, int);

  //BTX
protected:
  vtkCMBBorFileReader();
  ~vtkCMBBorFileReader() override;
  //Reader functions for various file formats found in the output of BorFile
  int ReadBorFile(const char* filename);
  int ProcessBorFileInfo(vtkMultiBlockDataSet* output);

  int RequestInformation(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;
  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

  char* FileName;
  double BHDisplayWidth;
  // Borehole sample data will not be supported
  // (not sure if GMS even exports datasets to this file anymore)
  // so if the number of datasets is non-zero then a message
  // stating that sample data will be ignored should be provided.
  int BHNumSampleDatasets;

  int NumberOfBoreholes;
  int NumberOfCrossSections;

  std::map<std::string, BorHoleInfo> BoreHoles;
  std::vector<BorCrossSection> CrossSections;
  //ETX
};

#endif //__vtkCMBBorFileReader__

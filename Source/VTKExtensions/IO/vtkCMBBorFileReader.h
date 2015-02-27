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

#include "vtkCMBIOModule.h" // For export macro
#include "cmbSystemConfig.h"
#include <vtkMultiBlockDataSetAlgorithm.h>
#include <map>
#include <vector>

class vtkMultiBlockDataSet;
class BorHoleInfo;
class BorCrossSection;

class VTKCMBIO_EXPORT vtkCMBBorFileReader : public vtkMultiBlockDataSetAlgorithm
{
  public:
    static vtkCMBBorFileReader* New();
    virtual void PrintSelf( ostream&os, vtkIndent indent );
    vtkTypeMacro(vtkCMBBorFileReader, vtkMultiBlockDataSetAlgorithm);

    vtkSetStringMacro(FileName);
    vtkGetStringMacro(FileName);
    vtkGetMacro(NumberOfBoreholes, int);
    vtkGetMacro(NumberOfCrossSections, int);

//BTX
  protected:
    vtkCMBBorFileReader();
    virtual ~vtkCMBBorFileReader();
    //Reader functions for various file formats found in the output of BorFile
    int ReadBorFile(const char* filename);
    int ProcessBorFileInfo(vtkMultiBlockDataSet* output);

    int RequestInformation( vtkInformation*, vtkInformationVector**, vtkInformationVector* );
    int RequestData( vtkInformation*, vtkInformationVector**, vtkInformationVector* );

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

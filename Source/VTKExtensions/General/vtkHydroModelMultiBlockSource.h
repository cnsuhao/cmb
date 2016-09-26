//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkHydroModelMultiBlockSource - "Dummy" source so we can treat data as a source
// .SECTION Description
// The input Source data is shallow copied to the output

#ifndef __vtkHydroModelMultiBlockSource_h
#define __vtkHydroModelMultiBlockSource_h

#include "vtkCMBGeneralModule.h" // For export macro
#include "vtkMultiBlockDataSetAlgorithm.h"
#include "cmbSystemConfig.h"

class VTKCMBGENERAL_EXPORT vtkHydroModelMultiBlockSource : public vtkMultiBlockDataSetAlgorithm
{
public:
  static vtkHydroModelMultiBlockSource *New();
  vtkTypeMacro(vtkHydroModelMultiBlockSource,vtkMultiBlockDataSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  void CopyData(vtkMultiBlockDataSet *source);
  vtkGetObjectMacro(Source, vtkMultiBlockDataSet);

protected:
  vtkHydroModelMultiBlockSource();
  ~vtkHydroModelMultiBlockSource() override;

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *) override;

  vtkMultiBlockDataSet *Source;

private:
  vtkHydroModelMultiBlockSource(const vtkHydroModelMultiBlockSource&);  // Not implemented.
  void operator=(const vtkHydroModelMultiBlockSource&);  // Not implemented.
};

#endif

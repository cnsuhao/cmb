//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __vtkCMBSpacingFlip_h
#define __vtkCMBSpacingFlip_h

#include "vtkCMBFilteringModule.h" // For export macro

#include "vtkImageAlgorithm.h"

class VTKCMBFILTERING_EXPORT vtkCMBSpacingFlip : public vtkImageAlgorithm
{
public:
  static vtkCMBSpacingFlip *New();
  vtkTypeMacro(vtkCMBSpacingFlip,vtkImageAlgorithm);

protected:

  vtkCMBSpacingFlip();

  int RequestData(vtkInformation *, vtkInformationVector **,
                  vtkInformationVector *);

private:
  vtkCMBSpacingFlip(const vtkCMBSpacingFlip&);  // Not implemented.
  void operator=(const vtkCMBSpacingFlip&);  // Not implemented.
};

#endif
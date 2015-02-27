/*=========================================================================

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
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
// .NAME vtkCMBExtractContours
// .SECTION Description

#ifndef __vtkCMBExtractContours_h
#define __vtkCMBExtractContours_h

#include "vtkCMBFilteringModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"
#include "cmbSystemConfig.h"
class vtkPolyData;

class VTKCMBFILTERING_EXPORT vtkCMBExtractContours : public vtkPolyDataAlgorithm
{
public:
  static vtkCMBExtractContours *New();
  vtkTypeMacro(vtkCMBExtractContours,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  vtkSetMacro(ContourIndex,vtkIdType);
  vtkGetMacro(ContourIndex,vtkIdType);

  vtkSetMacro(NumberOfContours,vtkIdType);
  vtkGetMacro(NumberOfContours,vtkIdType);

protected:
  vtkCMBExtractContours();
  ~vtkCMBExtractContours();

  virtual int RequestData(vtkInformation* request,
                                 vtkInformationVector** inputVector,
                                 vtkInformationVector* outputVector);


  void BuildSelectedIds(vtkPolyData* input);
  void BuildGlobalPointCollection(vtkPolyData* input);

  //this is the index of the contour we are going to read.
  vtkIdType ContourIndex;
  vtkIdType NumberOfContours;

  //Stores all the id's of selected points across all the contours
  class vtkInternalSet;
  vtkInternalSet *SelectedIds;
private:
  bool BuildGlobalPointCollectionBefore;
  vtkCMBExtractContours(const vtkCMBExtractContours&);  // Not implemented.
  void operator=(const vtkCMBExtractContours&);  // Not implemented.
};

#endif

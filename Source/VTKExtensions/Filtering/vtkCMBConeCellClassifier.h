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
// .NAME vtkCMBConeCellClassifier
// .SECTION Description

#ifndef __vtkCMBConeCellClassifier_h
#define __vtkCMBConeCellClassifier_h

#include "vtkCMBFilteringModule.h" // For export macro
#include "vtkUnstructuredGridAlgorithm.h"
#include "cmbSystemConfig.h"

class vtkIntArray;
class vtkDoubleArray;
class VTKCMBFILTERING_EXPORT vtkCMBConeCellClassifier : public vtkUnstructuredGridAlgorithm
{
public:
  static vtkCMBConeCellClassifier *New();
  vtkTypeMacro(vtkCMBConeCellClassifier,vtkUnstructuredGridAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  //Description:
  //Set/Get the Base Center of the cone.  The default is (0, 0, 0).
  vtkSetVector3Macro(BaseCenter, double);
  vtkGetVector3Macro(BaseCenter, double);

  //Description:
  //Set/Get the Axis direction of the cone.
  // Note that this does not have to normalized.  The default is (0, 0, 1).
  vtkSetVector3Macro(AxisDirection, double);
  vtkGetVector3Macro(AxisDirection, double);

  //Description:
  //Set/Get the Base Radius of the cone. The default is 0.5.
  vtkSetMacro(BaseRadius, double);
  vtkGetMacro(BaseRadius, double);

  //Description:
  //Set/Get the Top Radius of the cone. The default is 0.
  vtkSetMacro(TopRadius, double);
  vtkGetMacro(TopRadius, double);

  //Description:
  //Set/Get the Height of the cone. The default is 1.
  vtkSetMacro(Height, double);
  vtkGetMacro(Height, double);

  //Description:
  //Set/Get the Translation of the cone.  The default is (0, 0, 0).
  vtkSetVector3Macro(Translation, double);
  vtkGetVector3Macro(Translation, double);

  //Description:
  //Set/Get the Orientation of the cone.  The default is (0, 0, 0).
  vtkSetVector3Macro(Orientation, double);
  vtkGetVector3Macro(Orientation, double);

  //Description:
  //Set/Get the Scaling of the cone.  The default is (1, 1, 1).
  vtkSetVector3Macro(Scaling, double);
  vtkGetVector3Macro(Scaling, double);

  //Description:
  //Set/Get the Original value in the cell data array to be changed.
  // The default is 0.
  vtkSetMacro(OriginalCellValue, int);
  vtkGetMacro(OriginalCellValue, int);

  //Description:
  //Set/Get the New value in the cell data array that will replace OriginalCellValue.
  // The defaukt is 1.
  vtkSetMacro(NewCellValue, int);
  vtkGetMacro(NewCellValue, int);

  //Description:
  //Set/Get the classification mode.  If mode=0 then cells will be
  // changed if at least one of its points are inside a cone.  If mode is 1
  // then all of the cells points must be inside a cone. If mode is 2 then
  // only those cells that the cone passes through will be changed.  Default is 0.
  vtkSetMacro(ClassificationMode,int);
  vtkGetMacro(ClassificationMode,int);

  // Returns true if the point p is inside a truncated cone
  bool IsInside(const double p[3]);

protected:
  vtkCMBConeCellClassifier();
  ~vtkCMBConeCellClassifier();

  virtual int RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector);


  double BaseCenter[3];
  double AxisDirection[3];
  double Height;
  double BaseRadius;
  double TopRadius;
  double Translation[3];
  double Orientation[3];
  double Scaling[3];

  int OriginalCellValue;
  int NewCellValue;
  int ClassificationMode;
  double AxisUnitDir[3];
private:
  vtkCMBConeCellClassifier(const vtkCMBConeCellClassifier&);  // Not implemented.
  void operator=(const vtkCMBConeCellClassifier&);  // Not implemented.
};

#endif

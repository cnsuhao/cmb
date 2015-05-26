//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBScalarLineSource - Source to representing a line with point data
// .SECTION Description

#ifndef __CmbScalarLineSource_h
#define __CmbScalarLineSource_h

#include "vtkCMBGeneralModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"
#include "cmbSystemConfig.h"

class VTKCMBGENERAL_EXPORT vtkCMBScalarLineSource : public vtkPolyDataAlgorithm
{
public:
  static vtkCMBScalarLineSource *New();
  vtkTypeMacro(vtkCMBScalarLineSource,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/Get the first point of the line
  vtkSetVector3Macro(Point1, double);
  vtkGetVector3Macro(Point1, double);


  // Set/Get the scalar associated with the first point
  vtkSetMacro(Scalar1, double);
  vtkGetMacro(Scalar1, double);

  // Description:
  // Set/Get the second point of the line
  vtkSetVector3Macro(Point2, double);
  vtkGetVector3Macro(Point2, double);


  // Set/Get the scalar associated with the second point
  vtkSetMacro(Scalar2, double);
  vtkGetMacro(Scalar2, double);

protected:
  vtkCMBScalarLineSource();
  ~vtkCMBScalarLineSource();

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

  double Point1[3];
  double Point2[3];
  double Scalar1;
  double Scalar2;

private:
  vtkCMBScalarLineSource(const vtkCMBScalarLineSource&);  // Not implemented.
  void operator=(const vtkCMBScalarLineSource&);  // Not implemented.
};

#endif

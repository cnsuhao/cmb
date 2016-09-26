//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkModelLineSource - create a line defined by two end points
// .SECTION Description
// vtkModelLineSource is a source object that creates a polyline defined by
// two endpoints. The number of segments composing the polyline is
// controlled by setting the object resolution.

#ifndef __vtkModelLineSource_h
#define __vtkModelLineSource_h

#include "vtkCMBGeneralModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"
#include "cmbSystemConfig.h"

class VTKCMBGENERAL_EXPORT vtkModelLineSource : public vtkPolyDataAlgorithm
{
public:
  static vtkModelLineSource *New();
  vtkTypeMacro(vtkModelLineSource,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Set position of first end point.
  vtkSetVector3Macro(Point1,double);
  vtkGetVectorMacro(Point1,double,3);

  // Description:
  // Set position of other end point.
  vtkSetVector3Macro(Point2,double);
  vtkGetVectorMacro(Point2,double,3);

  // Description:
  // Divide line into resolution number of pieces.
  vtkSetClampMacro(Resolution,int,1,VTK_INT_MAX);
  vtkGetMacro(Resolution,int);

  // Description:
  // Whether or not to build verts to the output.
  vtkSetMacro(BuildVertex,int);
  vtkGetMacro(BuildVertex,int);
  vtkBooleanMacro(BuildVertex,int);

protected:
  vtkModelLineSource(int res=1);
  ~vtkModelLineSource() override {};

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *) override;
  int RequestInformation(vtkInformation *, vtkInformationVector **, vtkInformationVector *) override;
  double Point1[3];
  double Point2[3];
  int Resolution;
  int BuildVertex;
private:
  vtkModelLineSource(const vtkModelLineSource&);  // Not implemented.
  void operator=(const vtkModelLineSource&);  // Not implemented.
};

#endif

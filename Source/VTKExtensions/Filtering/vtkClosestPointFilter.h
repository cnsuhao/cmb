//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkClosestPointFilter - Calculates the closest point to PolyData
// .SECTION Description
// Reader for SceneGen vegetation file.

#ifndef __ClosestPointFilter_h
#define __ClosestPointFilter_h

#include "vtkCMBFilteringModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"
#include "cmbSystemConfig.h"

class vtkCellLocator;
class vtkTransform;
class vtkAbstractTransform;

class VTKCMBFILTERING_EXPORT vtkClosestPointFilter : public vtkPolyDataAlgorithm
{
public:
  static vtkClosestPointFilter *New();
  vtkTypeMacro(vtkClosestPointFilter,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get/Set the Test Point - note that this will not
  // modify the filter
  void SetTestPoint(double x, double y, double z)
  {
    vtkDebugMacro(<< this->GetClassName() << " (" << this
                  << "): setting TestPoint to (" << x << "," << y << ","
                  << z << ")");
    if ((this->TestPoint[0] != x)||(this->TestPoint[1] != y)||
        (this->TestPoint[2] != z) || (!this->PointMode))
      {
      this->TestPoint[0] = x;
      this->TestPoint[1] = y;
      this->TestPoint[2] = z;
      this->Modified();
      }
    this->PointMode = true;
  };
  void SetTestPoint (double _arg[3])
  {
  this->SetTestPoint (_arg[0], _arg[1], _arg[2]);
  }

  vtkGetVector3Macro(TestPoint, double);

  // Description:
  // Get/Set the Test Line
  void SetTestLine(double x1, double y1, double z1, double x2, double y2, double z2)
  {
    vtkDebugMacro(<< this->GetClassName() << " (" << this
                  << "): setting TestLine to (("
                  << x1 << "," << y1 << "," << z1 << ") ("
                  << x2 << "," << y2 << "," << z2 << "))");
    if ((this->TestLine[0] != x1)||(this->TestLine[1] != y1)||
        (this->TestLine[2] != z1) ||
        (this->TestLine[3] != x2)||(this->TestLine[4] != y2)||
        (this->TestLine[5] != z2)  || this->PointMode)
      {
      this->TestLine[0] = x1;
      this->TestLine[1] = y1;
      this->TestLine[2] = z1;
      this->TestLine[3] = x2;
      this->TestLine[4] = y2;
      this->TestLine[5] = z2;
      this->Modified();
      }
    this->PointMode = false;
  };

  void SetTestLine (double _arg[6])
  {
    this->SetTestLine (_arg[0], _arg[1], _arg[2], _arg[3], _arg[4], _arg[5]);
  }

  vtkGetVector6Macro(TestLine, double);

  // Description:
  // Get/Set the components of the Transform that maps data space to
  // world space
  vtkSetVector3Macro(Translation, double);
  vtkGetVector3Macro(Translation, double);
  vtkSetVector3Macro(Orientation, double);
  vtkGetVector3Macro(Orientation, double);
  vtkSetVector3Macro(Scale, double);
  vtkGetVector3Macro(Scale, double);

  // Description:
  // Calculate the closest point
  vtkGetVector3Macro(ClosestPoint, double);

  // Description:
  // Return the time of the last transform build.
  vtkGetMacro(BuildTime, unsigned long);

  //BTX

protected:
  vtkClosestPointFilter();
  ~vtkClosestPointFilter();

  double TestPoint[3];
  double TestLine[6];
  double Translation[3];
  double Orientation[3];
  double ClosestPoint[3];
  double Scale[3];
  bool PointMode;
  vtkTransform *Transform;
  vtkAbstractTransform *TransformInverse;
  vtkCellLocator *Locator;
  int RequestData(vtkInformation *, vtkInformationVector **,
                  vtkInformationVector *);
  vtkTimeStamp BuildTime;  // time at which the transform was built

private:
  vtkClosestPointFilter(const vtkClosestPointFilter&);  // Not implemented.
  void operator=(const vtkClosestPointFilter&);  // Not implemented.

  //ETX
};

#endif

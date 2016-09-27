//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkTexturePointIntensityFilter -
// .SECTION Description


#ifndef __TexturePointIntensityFilter_h
#define __TexturePointIntensityFilter_h

#include "vtkCMBFilteringModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"
#include "cmbSystemConfig.h"

class vtkCellLocator;
class vtkImageData;
class vtkTransform;
class vtkAbstractTransform;

class VTKCMBFILTERING_EXPORT vtkTexturePointIntensityFilter : public vtkPolyDataAlgorithm
{
public:
  static vtkTexturePointIntensityFilter *New();
  vtkTypeMacro(vtkTexturePointIntensityFilter,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Set the image being used as the texture
  void SetTextureData(vtkImageData *pd);
  vtkImageData *GetTextureData();
  void SetTextureDataConnection(vtkAlgorithmOutput* algOutput);

  // Description:
  // Get/Set the Test Point - note that this will not
  // modify the filter
  void SetTestPoint(double x, double y, double z)
    {
    vtkDebugMacro(<< this->GetClassName() << " (" << this
                  << "): setting TestPoint to (" << x << "," << y << ","
                  << z << ")");
    if ((this->TestPoint[0] != x)||(this->TestPoint[1] != y)||
        (this->TestPoint[2] != z))
      {
      this->TestPoint[0] = x;
      this->TestPoint[1] = y;
      this->TestPoint[2] = z;
      this->Modified();
      }
    }
  void SetTestPoint (double _arg[3])
    {
    this->SetTestPoint (_arg[0], _arg[1], _arg[2]);
    }

  vtkGetVector3Macro(TestPoint, double);

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
  vtkGetMacro(Intensity, double);

  // Description:
  // Return the time of the last transform build.
  vtkGetMacro(BuildTime, unsigned long);

  //BTX

protected:
  vtkTexturePointIntensityFilter();
  ~vtkTexturePointIntensityFilter() override;

  double TestPoint[3];
  double Translation[3];
  double Orientation[3];
  double Scale[3];
  double Intensity;
  vtkTransform *Transform;
  vtkAbstractTransform *TransformInverse;
  vtkCellLocator *Locator;
  vtkTimeStamp BuildTime;  // time at which the transform was built

  int RequestData(vtkInformation *, vtkInformationVector **,
                  vtkInformationVector *) override;
  int FillInputPortInformation(int, vtkInformation *) override;

private:
  vtkTexturePointIntensityFilter(const vtkTexturePointIntensityFilter&);  // Not implemented.
  void operator=(const vtkTexturePointIntensityFilter&);  // Not implemented.

  //ETX
};

#endif

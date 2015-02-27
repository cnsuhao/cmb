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
  void PrintSelf(ostream& os, vtkIndent indent);

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
  ~vtkTexturePointIntensityFilter();

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
                  vtkInformationVector *);
  virtual int FillInputPortInformation(int, vtkInformation *);

private:
  vtkTexturePointIntensityFilter(const vtkTexturePointIntensityFilter&);  // Not implemented.
  void operator=(const vtkTexturePointIntensityFilter&);  // Not implemented.

  //ETX
};

#endif

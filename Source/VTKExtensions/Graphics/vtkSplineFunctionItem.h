//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef __vtkSplineFunctionItem_h
#define __vtkSplineFunctionItem_h

#include "vtkCMBGraphicsModule.h" // For export macro
#include "vtkScalarsToColorsItem.h"
#include "vtkSmartPointer.h"

class vtkPiecewiseFunction;
class vtkKochanekSpline;

/// vtkSplineFunctionItem internall uses vtkPlot::Color, white by default
class VTKCMBGRAPHICS_EXPORT vtkSplineFunctionItem : public vtkScalarsToColorsItem
{
public:
  static vtkSplineFunctionItem* New();
  vtkTypeMacro(vtkSplineFunctionItem, vtkScalarsToColorsItem);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  void SetSplineFunction(vtkPiecewiseFunction* t);
  vtkGetObjectMacro(PiecewiseFunction, vtkPiecewiseFunction);

  void SetControls(double tension, double continuity, double bias);

  void SetDrawAsSpline(bool b)
  {
    DrawAsSpline = b;
    this->Modified();
  }

  void RefreshTexture() { ComputeTexture(); }

protected:
  vtkSplineFunctionItem();
  ~vtkSplineFunctionItem() override;

  // Description:
  // Reimplemented to return the range of the piecewise function
  void ComputeBounds(double bounds[4]) override;

  // Description
  // Compute the texture from the PiecewiseFunction
  void ComputeTexture() override;

  vtkSmartPointer<vtkKochanekSpline> SplineFunction;
  vtkPiecewiseFunction* PiecewiseFunction;
  double Controls[3];
  bool DrawAsSpline;

private:
  vtkSplineFunctionItem(const vtkSplineFunctionItem&); // Not implemented.
  void operator=(const vtkSplineFunctionItem&);        // Not implemented.
};

#endif

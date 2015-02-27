/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkSplineFunctionItem.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __vtkSplineFunctionItem_h
#define __vtkSplineFunctionItem_h

#include "vtkCMBGraphicsModule.h" // For export macro
#include "vtkScalarsToColorsItem.h"
#include "vtkSmartPointer.h"

class vtkPiecewiseFunction;
class vtkKochanekSpline;

/// vtkSplineFunctionItem internall uses vtkPlot::Color, white by default
class VTKCMBGRAPHICS_EXPORT vtkSplineFunctionItem: public vtkScalarsToColorsItem
{
public:
  static vtkSplineFunctionItem* New();
  vtkTypeMacro(vtkSplineFunctionItem, vtkScalarsToColorsItem);
  virtual void PrintSelf(ostream &os, vtkIndent indent);

  void SetSplineFunction(vtkPiecewiseFunction* t);
  vtkGetObjectMacro(PiecewiseFunction, vtkPiecewiseFunction);

  void SetControls(double tension, double continuity, double bias);

  void SetDrawAsSpline(bool b)
  {
    DrawAsSpline = b;
    this->Modified();
  }

  void RefreshTexture()
  {
    ComputeTexture();
  }

protected:
  vtkSplineFunctionItem();
  virtual ~vtkSplineFunctionItem();

  // Description:
  // Reimplemented to return the range of the piecewise function
  virtual void ComputeBounds(double bounds[4]);

  // Description
  // Compute the texture from the PiecewiseFunction
  virtual void ComputeTexture();

  vtkSmartPointer<vtkKochanekSpline> SplineFunction;
  vtkPiecewiseFunction *PiecewiseFunction;
  double Controls[3];
  bool DrawAsSpline;

private:
  vtkSplineFunctionItem(const vtkSplineFunctionItem &); // Not implemented.
  void operator=(const vtkSplineFunctionItem &); // Not implemented.
};

#endif

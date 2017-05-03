//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkSpline.h"
#include "vtkBrush.h"
#include "vtkCallbackCommand.h"
#include "vtkContext2D.h"
#include "vtkImageData.h"
#include "vtkKochanekSpline.h"
#include "vtkObjectFactory.h"
#include "vtkPen.h"
#include "vtkPiecewiseFunction.h"
#include "vtkPointData.h"
#include "vtkPoints2D.h"
#include "vtkSplineFunctionItem.h"

#include <cassert>

vtkStandardNewMacro(vtkSplineFunctionItem);

vtkSplineFunctionItem::vtkSplineFunctionItem()
{
  this->PolyLinePen->SetLineType(vtkPen::SOLID_LINE);
  this->PolyLinePen->SetColor(1.0, 0, 0);
  this->SplineFunction = 0;
  this->PiecewiseFunction = NULL;
  this->SetColor(1., 1., 1.);
  Controls[0] = 0;
  Controls[1] = 0;
  Controls[2] = 0;
  DrawAsSpline = true;
}

vtkSplineFunctionItem::~vtkSplineFunctionItem()
{
  if (this->SplineFunction)
  {
    this->SplineFunction = 0;
  }
  if (this->PiecewiseFunction)
  {
    this->PiecewiseFunction->RemoveObserver(this->Callback);
    this->PiecewiseFunction = 0;
  }
}

void vtkSplineFunctionItem::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "SplineFunction: ";
  if (this->SplineFunction)
  {
    os << endl;
    this->SplineFunction->PrintSelf(os, indent.GetNextIndent());
  }
  else
  {
    os << "(none)" << endl;
  }
}

void vtkSplineFunctionItem::ComputeBounds(double* bounds)
{
  this->Superclass::ComputeBounds(bounds);
  if (this->PiecewiseFunction)
  {
    this->PiecewiseFunction->GetRange(bounds);
  }
}

void vtkSplineFunctionItem::SetSplineFunction(vtkPiecewiseFunction* t)
{
  if (t == this->PiecewiseFunction)
  {
    return;
  }
  if (this->PiecewiseFunction)
  {
    this->PiecewiseFunction->RemoveObserver(this->Callback);
  }
  vtkSetObjectBodyMacro(PiecewiseFunction, vtkPiecewiseFunction, t);
  if (t)
  {
    t->AddObserver(vtkCommand::ModifiedEvent, this->Callback);
  }
  this->ScalarsToColorsModified(this->PiecewiseFunction, vtkCommand::ModifiedEvent, 0);
}

void vtkSplineFunctionItem::ComputeTexture()
{
  double bounds[4];
  this->GetBounds(bounds);
  if (!PiecewiseFunction)
  {
    return;
  }
  if (DrawAsSpline)
  {
    SplineFunction = vtkKochanekSpline::New();
    /*For Now ignoring the extra  control*/
    //SplineFunction->SetDefaultTension (Controls[0]);
    //SplineFunction->SetDefaultContinuity (Controls[1]);
    //SplineFunction->SetDefaultBias (Controls[2]);
    double v[4];
    double previous;
    for (int i = 0; i < PiecewiseFunction->GetSize(); ++i)
    {
      PiecewiseFunction->GetNodeValue(i, v);
      if (i != 0 && v[0] == previous)
      {
        v[0] += 0.000001;
      }
      SplineFunction->AddPoint(v[0], v[1]);
      previous = v[0];
    }
    SplineFunction->Compute();
    if (!this->SplineFunction)
      return;
  }

  if (bounds[0] == bounds[1])
  {
    return;
  }
  if (this->Texture == 0)
  {
    this->Texture = vtkImageData::New();
  }

  const int dimension = this->GetTextureWidth();
  double* values = new double[dimension];
  // should depends on the true size on screen
  this->Texture->SetExtent(0, dimension - 1, 0, 0, 0, 0);
  this->Texture->AllocateScalars(VTK_UNSIGNED_CHAR, 4);

  double x = 0;
  double min = 0;
  double max = 1;
  if (DrawAsSpline)
  {
    for (int i = 0; i < dimension; ++i)
    {
      x = bounds[0] + (double(i) / double(dimension - 1)) * (bounds[1] - bounds[0]);
      values[i] = this->SplineFunction->Evaluate(x);
      if (values[i] < min)
        min = values[i];
      if (values[i] > max)
        max = values[i];
    }
  }
  else
  {
    this->PiecewiseFunction->GetTable(bounds[0], bounds[1], dimension, values);
  }

  if (min != 0 || max != 1)
  {
    this->PolyLinePen->SetColor(255, 0, 0);
    this->Pen->SetColor(255, 0, 0);
    SetColor(255, 0, 0);
  }
  else
  {
    this->PolyLinePen->SetColor(1.0, 1.0, 1.0);
    this->Pen->SetColor(1.0, 0, 0);
    SetColor(1.0, 0, 0);
  }

  unsigned char* ptr = reinterpret_cast<unsigned char*>(this->Texture->GetScalarPointer(0, 0, 0));
  this->Shape->SetNumberOfPoints(dimension);
  double step = (bounds[1] - bounds[0]) / dimension;
  for (int i = 0; i < dimension; ++i)
  {
    this->Pen->GetColor(ptr);
    ptr[3] = static_cast<unsigned char>(0);
    this->Shape->SetPoint(i, bounds[0] + step * i, values[i]);
    ptr += 4;
  }
  this->Shape->Modified();
  delete[] values;
  return;
}

void vtkSplineFunctionItem::SetControls(double tension, double continuity, double bias)
{
  Controls[0] = tension;
  Controls[1] = continuity;
  Controls[2] = bias;
}

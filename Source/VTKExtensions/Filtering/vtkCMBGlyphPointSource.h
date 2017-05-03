//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBGlyphPointSource - Represents a set of points that will be used for Glyphing
// .SECTION Description
// The input Source data is shallow copied to the output

#ifndef __vtkCMBGlyphPointSource_h
#define __vtkCMBGlyphPointSource_h

#include "cmbSystemConfig.h"
#include "vtkCMBFilteringModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"
#include "vtkSmartPointer.h"

class vtkPoints;
class vtkUnsignedCharArray;
class vtkDoubleArray;
class vtkBitArray;
class vtkCellArray;
class vtkTransform;

class VTKCMBFILTERING_EXPORT vtkCMBGlyphPointSource : public vtkPolyDataAlgorithm
{
public:
  static vtkCMBGlyphPointSource* New();
  vtkTypeMacro(vtkCMBGlyphPointSource, vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Insert the next point into the object
  vtkIdType InsertNextPoint(double x, double y, double z);
  vtkIdType InsertNextPoint(double p[3]) { return this->InsertNextPoint(p[0], p[1], p[2]); }

  // Description:
  // Insert the next point and its properties into the object
  vtkIdType InsertNextPoint(double x, double y, double z, double r, double g, double b, double a,
    double sx, double sy, double sz, double ox, double oy, double oz, int vis);

  void SetScale(vtkIdType index, double sx, double sy, double sz);
  void SetOrientation(vtkIdType index, double ox, double oy, double oz);
  void ApplyTransform(double* orinetationDelta, double* positionDelta, double* scaleDelta);
  void ApplyTransform(
    vtkIdType index, double* orinetationDelta, double* positionDelta, double* scaleDelta);
  double* GetBounds(vtkIdType index);
  void SetVisibility(vtkIdType index, int flag);
  void SetColor(vtkIdType index, double r, double g, double b, double a);
  void UnsetColor(vtkIdType index);
  void ResetColorsToDefault();
  vtkIdType GetNumberOfPoints() { return this->Source->GetNumberOfPoints(); }
  void SetPoint(vtkIdType index, double x, double y, double z);
  void GetPoint(vtkIdType index, double* p);
  double* GetPoint(vtkIdType index)
  {
    this->GetPoint(index, this->TempData);
    return this->TempData;
  }
  void GetScale(vtkIdType index, double* s);
  double* GetScale(vtkIdType index)
  {
    this->GetScale(index, this->TempData);
    return this->TempData;
  }

  void GetOrientation(vtkIdType index, double* o);
  double* GetOrientation(vtkIdType index)
  {
    this->GetOrientation(index, this->TempData);
    return this->TempData;
  }
  int GetVisibility(vtkIdType index);

  void GetColor(vtkIdType index, double* color);
  double* GetColor(vtkIdType index)
  {
    this->GetColor(index, this->TempData);
    return this->TempData;
  }

  void SetDefaultColor(double r, double g, double b, double a);
  const double* GetDefaultColor() const { return this->DefaultColor; }
  void SetGlyphSourceBounds(double bounds[6])
  {
    for (int i = 0; i < 6; i++)
      this->GlyphSourceBounds[i] = bounds[i];
  }
  const double* GetGlyphSourceBounds() const { return this->GlyphSourceBounds; }

  // Load the point information from a file
  void ReadFromFile(const char*);

  // Write the point information to a file
  void WriteToFile(const char*);

protected:
  vtkCMBGlyphPointSource();
  ~vtkCMBGlyphPointSource() override;

  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

  vtkSmartPointer<vtkPolyData> Source;
  vtkSmartPointer<vtkPoints> Points;
  vtkSmartPointer<vtkUnsignedCharArray> Color;
  vtkSmartPointer<vtkBitArray> Visibility;
  vtkSmartPointer<vtkBitArray> SelectionMask;
  vtkSmartPointer<vtkDoubleArray> Scaling;
  vtkSmartPointer<vtkDoubleArray> Orientation;
  vtkSmartPointer<vtkCellArray> CellIds;
  vtkSmartPointer<vtkTransform> Transform;
  double TempData[6];
  double DefaultColor[4];
  double GlyphSourceBounds[6];

private:
  vtkCMBGlyphPointSource(const vtkCMBGlyphPointSource&); // Not implemented.
  void operator=(const vtkCMBGlyphPointSource&);         // Not implemented.
};

#endif

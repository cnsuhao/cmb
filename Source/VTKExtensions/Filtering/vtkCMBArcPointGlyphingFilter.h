//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __vtkCMBArcPointGlyphingFilter_h
#define __vtkCMBArcPointGlyphingFilter_h

#include "cmbSystemConfig.h"
#include "vtkCMBFilteringModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"

#include <set>

class VTKCMBFILTERING_EXPORT vtkCMBArcPointGlyphingFilter : public vtkPolyDataAlgorithm
{
public:
  vtkTypeMacro(vtkCMBArcPointGlyphingFilter, vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  static vtkCMBArcPointGlyphingFilter* New();

  void clearVisible();
  void setVisible(int);
  void setScale(double);

protected:
  vtkCMBArcPointGlyphingFilter();
  ~vtkCMBArcPointGlyphingFilter() override;

  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

  std::set<int> visible;
  double scale;

private:
  vtkCMBArcPointGlyphingFilter(const vtkCMBArcPointGlyphingFilter&)
    : vtkPolyDataAlgorithm()
  {
  }                                                      // Not implemented.
  void operator=(const vtkCMBArcPointGlyphingFilter&) {} // Not implemented.
};

#endif

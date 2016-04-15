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

#include "vtkCMBFilteringModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"
#include "cmbSystemConfig.h"

#include <set>

class VTKCMBFILTERING_EXPORT vtkCMBArcPointGlyphingFilter : public vtkPolyDataAlgorithm
{
public:
  vtkTypeMacro(vtkCMBArcPointGlyphingFilter,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  static vtkCMBArcPointGlyphingFilter *New();

  void clearVisible();
  void setVisible(int);

protected:
  vtkCMBArcPointGlyphingFilter();
  ~vtkCMBArcPointGlyphingFilter();

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

  std::set<int> visible;

private:
  vtkCMBArcPointGlyphingFilter(const vtkCMBArcPointGlyphingFilter&):vtkPolyDataAlgorithm()
  {}  // Not implemented.
  void operator=(const vtkCMBArcPointGlyphingFilter&){}  // Not implemented.
};

#endif

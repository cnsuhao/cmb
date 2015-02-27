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
// .NAME vtkCMBGeometry2DReader - "reader" for various SceneGen geometry formats
// .SECTION Description
// Not actually a reader in the sense that it internally creates the appropriate
// reader based on the filename's extension.

#ifndef __vtkCMBGeometry2DReader_h
#define __vtkCMBGeometry2DReader_h

#include "vtkCMBIOModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"
#include "cmbSystemConfig.h"

class VTKCMBIO_EXPORT vtkCMBGeometry2DReader : public vtkPolyDataAlgorithm
{
public:
  static vtkCMBGeometry2DReader *New();
  vtkTypeMacro(vtkCMBGeometry2DReader,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Name of the file to be read.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  /// BoundaryStyle enumerants
  enum BoundaryStyleValue
    {
    NONE,
    RELATIVE_MARGIN,
    ABSOLUTE_MARGIN,
    ABSOLUTE_BOUNDS,
    IMPORTED_POLYGON
    };

  // Description:
  // Set/get whether or how a clip boundary should be added to the model.
  //
  // The default is NONE.
  // \sa BoundaryStyleValue
  vtkSetClampMacro(BoundaryStyle,int,NONE,IMPORTED_POLYGON);
  vtkGetMacro(BoundaryStyle,int);
  void SetBoundaryStyleToNone()            { this->SetBoundaryStyle(NONE); }
  void SetBoundaryStyleToAbsoluteBounds()  { this->SetBoundaryStyle(ABSOLUTE_BOUNDS); }
  void SetBoundaryStyleToAbsoluteMargin()  { this->SetBoundaryStyle(ABSOLUTE_MARGIN); }
  void SetBoundaryStyleToRelativeMargin()  { this->SetBoundaryStyle(RELATIVE_MARGIN); }
  void SetBoundaryStyleToImportedPolygon() { this->SetBoundaryStyle(IMPORTED_POLYGON); }

  // Description:
  // Set/get the relative margin to use when BoundaryStyle is RELATIVE_MARGIN.
  //
  // The default is 5; the units are percent of the length of the bounding-box diagonal.
  //
  // When specified as a string, either 1, 2, or 4 comma-separated values may be passed.
  // Each is a percentage. When one number is passed, it is applied uniformly to all
  // margins. When two are passed, the first is applied to the horizontal margins and
  // the second to the vertical. When all 4 are passed, each margin is explicitly specified.
  vtkSetVector4Macro(RelativeMargin,double);
  vtkGetVector4Macro(RelativeMargin,double);
  virtual void SetRelativeMarginString(const char* text);

  // Description:
  // Set/get the absolute margin to use when BoundaryStyle is ABSOLUTE_MARGIN.
  //
  // The default is 1.0 and the units are world coordinate units.
  //
  // When specified as a string, either 1, 2, or 4 comma-separated values may be passed.
  // When one number is passed, it is applied uniformly to all
  // margins. When two are passed, the first is applied to the horizontal margins and
  // the second to the vertical. When all 4 are passed, each margin is explicitly specified.
  vtkSetVector4Macro(AbsoluteMargin,double);
  vtkGetVector4Macro(AbsoluteMargin,double);
  virtual void SetAbsoluteMarginString(const char* text);

  // Description:
  // Set/get the absolute coordinates to use when BoundaryStyle is ABSOLUTE_BOUNDS.
  //
  // The default is the invalid tuple (+1, -1, +1, -1) and the units are world coordinate units.
  // If fewer or more than 4 values are specified, the bounds are set to
  // the invalid tuple (+1, -1, +1, -1).
  vtkSetVector4Macro(AbsoluteBounds,double);
  vtkGetVector4Macro(AbsoluteBounds,double);
  virtual void SetAbsoluteBoundsString(const char* text);

  // Description:
  // Set/get the name of a second shapefile to use as a boundary.
  //
  // The default is NULL.
  // This value is only used when BoundaryStyle is set to IMPORTED_POLYGON.
  vtkSetStringMacro(BoundaryFile);
  vtkGetStringMacro(BoundaryFile);

protected:
  vtkCMBGeometry2DReader();
  ~vtkCMBGeometry2DReader();

  int GetMarginFromString(const char* text, double margin[4]);

  int RequestInformation(
    vtkInformation* request,
    vtkInformationVector** inInfo,
    vtkInformationVector* outInfo);
  int RequestData(
    vtkInformation* request,
    vtkInformationVector** inInfo,
    vtkInformationVector* outInfo);

  char* FileName;
  int BoundaryStyle;
  double RelativeMargin[4];
  double AbsoluteMargin[4];
  double AbsoluteBounds[4];
  char* BoundaryFile;

private:
  vtkCMBGeometry2DReader(const vtkCMBGeometry2DReader&);  // Not implemented.
  void operator=(const vtkCMBGeometry2DReader&);  // Not implemented.
};

#endif // __vtkCMBGeometry2DReader_h

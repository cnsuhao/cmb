/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkOrientedGlyphContourRepresentation2.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkOrientedGlyphContourRepresentation2 - Default representation for the contour widget
// .SECTION Description
// This class provides the default concrete representation for the
// vtkContourWidget. It works in conjunction with the
// vtkContourLineInterpolator and vtkPointPlacer. See vtkContourWidget
// for details.
// .SECTION See Also
// vtkOrientedGlyphContourRepresentation vtkContourRepresentation vtkContourWidget vtkPointPlacer
// vtkContourLineInterpolator

#ifndef __vtkOrientedGlyphContourRepresentation2_h
#define __vtkOrientedGlyphContourRepresentation2_h

#include "vtkCMBGraphicsModule.h" // For export macro
#include "vtkOrientedGlyphContourRepresentation.h"
#include "cmbSystemConfig.h"

class vtkProperty;
class vtkActor;
class vtkPolyDataMapper;
class vtkPolyData;
class vtkGlyph3D;
class vtkPoints;

class VTKCMBGRAPHICS_EXPORT vtkOrientedGlyphContourRepresentation2 : public vtkOrientedGlyphContourRepresentation
{
public:
  // Description:
  // Instantiate this class.
  static vtkOrientedGlyphContourRepresentation2 *New();

  // Description:
  // Standard methods for instances of this class.
  vtkTypeMacro(vtkOrientedGlyphContourRepresentation2,vtkOrientedGlyphContourRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Controls whether the contour widget should
  // generate a point array that represents if the point
  // was modifed. If used remember to turn off during construction
  // of the initial points
  // Default is to set it to false.
  vtkSetMacro( LoggingEnabled, int );
  vtkGetMacro( LoggingEnabled, int );
  vtkBooleanMacro( LoggingEnabled, int );

  //overloaded for logging purposes
  virtual int DeleteNthNode(int n);
  virtual int SetNthNodeSelected(int);

  virtual int AddNodeOnContour(int X, int Y);

  // Description:
  // Get the points in this contour as a vtkPolyData.
  virtual vtkPolyData * GetContourRepresentationAsPolyData();

  //Description:
  // Get the flags for a given point
  // the flags represent if the point has been moved, inserted or deleted
  // A point can have all three flags or none of them
  int GetNodeModifiedFlags(int n);

  // Description:
  // Methods to make this class behave as a vtkProp. Using openGL
  // to do AlwaysOnTop.
  virtual int RenderOpaqueGeometry(vtkViewport *viewport);

protected:
  vtkOrientedGlyphContourRepresentation2();
  ~vtkOrientedGlyphContourRepresentation2();

  virtual void UpdateLines(int index);

  // Description:
  // Build a contour representation from externally supplied PolyData. This
  // is very useful when you use an external program to compute a set of
  // contour nodes, let's say based on image features. Subsequently, you want
  // to build and display a contour that runs through those points.
  // This method is protected and accessible only from
  // vtkContourWidget::Initialize( vtkPolyData * )

  //Note: While this method will only render the first line cell in the polydata
  //it will compute if the contour is closed based on this first cell number of points
  //versus the number of points in the polydata. So don't have any extra points
  virtual void Initialize( vtkPolyData * );

  //support logging of point changes
  int LoggingEnabled;

  class vtkInternalMap;
  vtkInternalMap *ModifiedPointMap;


  void UpdatePropertyMap(int index, int flags);


private:
  vtkOrientedGlyphContourRepresentation2(const vtkOrientedGlyphContourRepresentation2&);  //Not implemented
  void operator=(const vtkOrientedGlyphContourRepresentation2&);  //Not implemented
};

#endif

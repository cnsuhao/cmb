/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkPolyDataStatsFilter.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkPolyDataStatsFilter - Calculates statistics of PolyData
// .SECTION Description
// Reader for SceneGen vegetation file.

#ifndef __PolyDataStatsFilter_h
#define __PolyDataStatsFilter_h

#include "vtkCMBFilteringModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"
#include "cmbSystemConfig.h"

class vtkCellLocator;
class vtkTransform;
class vtkAbstractTransform;

class VTKCMBFILTERING_EXPORT vtkPolyDataStatsFilter : public vtkPolyDataAlgorithm
{
public:
  static vtkPolyDataStatsFilter *New();
  vtkTypeMacro(vtkPolyDataStatsFilter,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);


  vtkGetVector3Macro(AreaStats, double);
  vtkGetMacro(TotalSurfaceArea, double);
  vtkGetMacro(NumberOfPoints, vtkIdType);
  vtkGetMacro(NumberOfLines, vtkIdType);
  vtkGetMacro(NumberOfPolygons, vtkIdType);
  vtkGetVector3Macro(PolygonalSideStats, double);
  vtkGetVector6Macro(GeometryBounds, double);

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
  // Return the time of the last transform build.
  vtkGetMacro(BuildTime, unsigned long);

  //BTX

protected:
  vtkPolyDataStatsFilter();
  ~vtkPolyDataStatsFilter();

  double AreaStats[3];
  double GeometryBounds[6];
  double PolygonalSideStats[3];
  double TotalSurfaceArea;
  vtkIdType NumberOfPolygons;
  vtkIdType NumberOfLines;
  vtkIdType NumberOfPoints;
  double Translation[3];
  double Orientation[3];
  double Scale[3];
  vtkTransform *Transform;
  int RequestData(vtkInformation *, vtkInformationVector **,
                  vtkInformationVector *);
  vtkTimeStamp BuildTime;  // time at which the transform was built

private:
  vtkPolyDataStatsFilter(const vtkPolyDataStatsFilter&);  // Not implemented.
  void operator=(const vtkPolyDataStatsFilter&);  // Not implemented.

  //ETX
};

#endif

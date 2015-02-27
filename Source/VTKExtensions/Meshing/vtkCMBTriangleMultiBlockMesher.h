/*=========================================================================

Program:   Visualization Toolkit
Module:    vtkCMBTriangleMesher.h

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
All rights reserved.
See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkCMBTriangleMesher
// .SECTION Description
// This class will mesh a given 2 dimensional polygon and return each
// loop as a unique vtkPolyData
//
// Input: vtkPolyData
// Output: vtkMultiBlockDataSet
//
//   The input vtkPolyData must have certain properties.
//   It is assumed that all lines form closed polygons.
//   Any polygon that you want to be meshed must be stored
//   as a series of VTK_LINE's with the following properties
//
//   The input vtkPolyData needs to be processed using
//   the vtkCMBPrepareForTriangleMesher class that helps you describe
//   how the poly data will be meshed
//
//   See vtkCMBPrepareForTriangleMesher for how to format the input poly data

#ifndef __vtkCMBTriangleMultiBlockMesher_h
#define __vtkCMBTriangleMultiBlockMesher_h

#include "vtkCMBMeshingModule.h" // For export macro
#include "vtkMultiBlockDataSetAlgorithm.h"
#include "cmbSystemConfig.h"
#include <map>
#include <limits>

class VTKCMBMESHING_EXPORT vtkCMBTriangleMultiBlockMesher : public vtkMultiBlockDataSetAlgorithm
{

  public:
    static vtkCMBTriangleMultiBlockMesher *New();
    vtkTypeMacro(vtkCMBTriangleMultiBlockMesher,vtkMultiBlockDataSetAlgorithm );
    void PrintSelf(ostream& os, vtkIndent indent);


    //For info on properties see vtkCMBTriangleMesher.h
    vtkSetMacro(UseUniqueAreas,bool);
    vtkGetMacro(UseUniqueAreas,bool);
    vtkSetMacro(PreserveBoundaries,bool); //allows triangle to insert points on boundaries
    vtkGetMacro(PreserveBoundaries,bool);
    vtkSetMacro(PreserveEdges,bool); //Adds VTK_LINES with arcIds preserved
    vtkGetMacro(PreserveEdges,bool);
    vtkSetMacro(MaxArea, double);
    vtkGetMacro(MaxArea, double);
    vtkGetMacro(ComputedMaxArea,double);
    enum MaxAreaModeOptions
      {
      NoMaxArea,
      AbsoluteArea,
      RelativeToBounds,
      RelativeToBoundsAndSegments
      };
    vtkSetClampMacro(MaxAreaMode, int , 0 , 3);
    vtkGetMacro(MaxAreaMode,int);
    vtkSetMacro(VerboseOutput,bool);
    vtkGetMacro(VerboseOutput,bool);
    vtkBooleanMacro(VerboseOutput,bool);
    vtkSetMacro(UseMinAngle,bool);
    vtkGetMacro(UseMinAngle,bool);
    vtkSetClampMacro(MinAngle,double,0,VTK_DOUBLE_MAX);
    vtkGetMacro(MinAngle,double);


  protected:
    vtkCMBTriangleMultiBlockMesher();
    ~vtkCMBTriangleMultiBlockMesher();

    bool UseOldMesher;
    double MinAngle;
    bool UseMinAngle;
    bool PreserveBoundaries;
    bool PreserveEdges;
    double MaxArea;
    double ComputedMaxArea;
    int MaxAreaMode;
    bool UseUniqueAreas;

    //Used to configure triangle's 'V' flag
    bool VerboseOutput;

    int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
    int FillInputPortInformation(int port, vtkInformation* info);
  private:
    vtkCMBTriangleMultiBlockMesher(const vtkCMBTriangleMultiBlockMesher&);  // Not implemented.
    void operator=(const vtkCMBTriangleMultiBlockMesher&);  // Not implemented.
};

#endif

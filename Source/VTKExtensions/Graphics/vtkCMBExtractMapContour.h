/*=========================================================================

Program:   Visualization Toolkit
Module:    vtkCMBExtractMapContour.h

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
All rights reserved.
See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkCMBExtractMapContour - Extract contours out of the map reader
// .SECTION Description
// Takes output from vtkCMBMapReader

#ifndef __vtkCMBExtractMapContour_h
#define __vtkCMBExtractMapContour_h

#include "vtkCMBGraphicsModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"
#include "cmbSystemConfig.h"
#include <vector>

class VTKCMBGRAPHICS_EXPORT vtkCMBExtractMapContour : public vtkPolyDataAlgorithm
{

  public:
    static vtkCMBExtractMapContour *New();
    vtkTypeMacro(vtkCMBExtractMapContour,vtkPolyDataAlgorithm);
    void PrintSelf(ostream& os, vtkIndent indent);

    void AddContourToExtract(int index);
    void ExtractSingleContour(int index);


  protected:
    vtkCMBExtractMapContour();
    ~vtkCMBExtractMapContour();

    int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

    std::vector<int> contoursToExtract;
  private:

    //Helper function used to make reading lines easier
    vtkCMBExtractMapContour(const vtkCMBExtractMapContour&);  // Not implemented.
    void operator=(const vtkCMBExtractMapContour&);  // Not implemented.

};

#endif

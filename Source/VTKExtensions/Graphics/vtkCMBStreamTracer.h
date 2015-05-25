//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBStreamTracer -
// .SECTION Description
//
// .SECTION See Also
// vtkStreamTracer

#ifndef __vtkCMBStreamTracer_h
#define __vtkCMBStreamTracer_h

#include "vtkCMBGraphicsModule.h" // For export macro
#include "vtkStreamTracer.h"
#include "cmbSystemConfig.h"
class vtkIdList;
class vtkIntArray;
class vtkAbstractInterpolatedVelocityField;

class VTKCMBGRAPHICS_EXPORT vtkCMBStreamTracer : public vtkStreamTracer
{
public:

    // Description:
    // Construct object using vtkCMBInitialValueProblemSolver
    static vtkCMBStreamTracer *New();
    vtkTypeMacro(vtkCMBStreamTracer, vtkStreamTracer);
    void PrintSelf(ostream& os, vtkIndent indent);

    // Description:
    // Set/Get the number of test locations for each agent
    // Default is 0.
    virtual void SetNumberOfTestLocations(int NumTests);
    virtual int GetNumberOfTestLocations();

    // Description:
    // Get/Set the relative offset of test locations for all agents.
    virtual void GetRelativeOffsetOfTestLocation(
      int index, double offset[3]);
    virtual void SetRelativeOffsetOfTestLocation(
      int index, double offset[3]);
    virtual void SetRelativeOffsetOfTestLocation(
      int index, double x, double y, double z)
      {
      double offset[3]={x, y, z};
      this->SetRelativeOffsetOfTestLocation(index, offset);
      }

    // Description:
    // Get/Set the default relative offset of all test locations.
    virtual double GetSensorDefaultRelativeOffset();
    virtual void SetSensorDefaultRelativeOffset(double offset);

  protected:

     vtkCMBStreamTracer();
    ~vtkCMBStreamTracer();

    //
    // Generate output
    //
    virtual int RequestData(vtkInformation* request,
                            vtkInformationVector** inputVector,
                            vtkInformationVector* outputVector);

    virtual void Integrate(vtkDataSet *input,
      vtkPolyData* output,
      vtkDataArray* seedSource,
      vtkIdList* seedIds,
      vtkIntArray* integrationDirections,
      double lastPoint[3],
      vtkAbstractInterpolatedVelocityField* func,
      int maxCellSize,
      const char *vecFieldName,
      double& propagation,
      vtkIdType& numSteps);

private:
  vtkCMBStreamTracer(const vtkCMBStreamTracer&);  // Not implemented.
  void operator=(const vtkCMBStreamTracer&);  // Not implemented.
};

#endif

/*=========================================================================

Copyright (c) 1998-2010 Kitware Inc. 28 Corporate Drive, Suite 204,
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

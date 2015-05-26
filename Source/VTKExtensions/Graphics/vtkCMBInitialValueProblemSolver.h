//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBInitialValueProblemSolver -

// .SECTION Description
// This is a concrete sub-class of vtkInitialValueProblemSolver.

// .SECTION See Also
// vtkInitialValueProblemSolver vtkFunctionSet

#ifndef __vtkCMBInitialValueProblemSolver_h
#define __vtkCMBInitialValueProblemSolver_h

#include "vtkCMBGraphicsModule.h" // For export macro
#include "vtkInitialValueProblemSolver.h"
#include "cmbSystemConfig.h"
#include <vector>

//BTX
namespace vtkCMBTracerNamespace
{
  class TestLocation{
    public:
      TestLocation()
        {
        this->cellId = -1;
        this->pos[0]=this->pos[1]=this->pos[2]=0.0;
        this->offset[0]=this->offset[1]=this->offset[2]=0.0;
        this->velocity[0]=this->velocity[1]=this->velocity[2]=0.0;
        }
      ~TestLocation()
        {
        }
      double pos[3];
      double offset[3];
      double seed[3];
      double velocity[3];
      vtkIdType cellId;
  };

  typedef std::vector<TestLocation*>  TestLocationVector;
  typedef TestLocationVector::iterator  TestLocationIterator;
}
//ETX

class VTKCMBGRAPHICS_EXPORT vtkCMBInitialValueProblemSolver : public vtkInitialValueProblemSolver
{
public:

  // Description:
  // Construct a vtkCMBInitialValueProblemSolver with no initial FunctionSet.
  static vtkCMBInitialValueProblemSolver *New();
  vtkTypeMacro(vtkCMBInitialValueProblemSolver, vtkInitialValueProblemSolver);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Given initial values, xprev , initial time, t and a requested time
  // interval, delT calculate values of x at t+delT (xnext).
  // delTActual is always equal to delT.
  // Since this class can not provide an estimate for the error error
  // is set to 0.
  // maxStep, minStep and maxError are unused.
  // This method returns an error code representing the nature of
  // the failure:
  // OutOfDomain = 1,
  // NotInitialized = 2,
  // UnexpectedValue = 3
  virtual int ComputeNextStep(double* xprev, double* xnext, double t,
    double& delT, double maxError,
    double& error)
  {
    double minStep = delT;
    double maxStep = delT;
    double delTActual;
    return this->ComputeNextStep(xprev, 0, xnext, t, delT, delTActual,
      minStep, maxStep, maxError, error);
  }
  virtual int ComputeNextStep(double* xprev, double* dxprev, double* xnext,
    double t, double& delT, double maxError,
    double& error)
  {
    double minStep = delT;
    double maxStep = delT;
    double delTActual;
    return this->ComputeNextStep(xprev, dxprev, xnext, t, delT, delTActual,
      minStep, maxStep, maxError, error);
  }
  virtual int ComputeNextStep(double* xprev, double* xnext,
    double t, double& delT, double& delTActual,
    double minStep, double maxStep,
    double maxError, double& error)
  {
    return this->ComputeNextStep(xprev, 0, xnext, t, delT, delTActual,
      minStep, maxStep, maxError, error);
  }
  virtual int ComputeNextStep(double* xprev, double* dxprev, double* xnext,
                              double t, double& delT, double& delTActual,
                              double minStep, double maxStep,
                              double maxError, double& error);

//BTX
  // Description:
  // Compute the next step for the given input point (xprev), and output
  // the next point (xnext) based on the test locations of the input points
  virtual int ComputeNextStepWithTestLocations(
    double* xprev, double* xnext,
    double t, double& delT, double& delTActual,
    double, double, double, double& error);

  // Description:
  // Initialize the test locations for all seeds
  // return true on success; false on failure
  virtual bool InitializeTestLocations(double *xprev);

//ETX
  // Description:
  // Set/Get the number of test locations for each agent
  // Default is 1.
  virtual void SetNumberOfTestLocations(int val);
  vtkGetMacro(NumberOfTestLocations, int);

  // Description:
  // Get/Set the default relative offset of all test locations.
  vtkSetMacro(DefaultRelativeOffset, double);
  vtkGetMacro(DefaultRelativeOffset, double);

  // Description:
  // Get/Set the relative offset of test location given its index.
  virtual void GetRelativeOffsetOfTestLocation( int index,
    double* OffsetOfTestLocation );
  virtual void SetRelativeOffsetOfTestLocation(
    int index, double* OffsetOfTestLocation);

  // Description:
  // Get the xyz-position of the test location given its index and relative offset
  virtual void GetTestLocationPosition(int testLocactionIndex,
    double* seedpos, double* relativeoffset, double* testlocation);
//BTX
  // Description:
  // Get the cell-Id and interpolated weights of the test location given its position
  // return true if the position is valid (a cell is found); false, on failure (out of bounds)
  virtual bool GetTestLocationCellInfo(
    double testlocation[3], double* intepolatedweights,
    vtkIdType& cellId);

protected:
  vtkCMBInitialValueProblemSolver();
  ~vtkCMBInitialValueProblemSolver();

  virtual void ClearTestLocations();
  int NumberOfTestLocations;
  double DefaultRelativeOffset;


private:
  vtkCMBInitialValueProblemSolver(const vtkCMBInitialValueProblemSolver&);  // Not implemented.
  void operator=(const vtkCMBInitialValueProblemSolver&);  // Not implemented.

  vtkCMBTracerNamespace::TestLocationVector TestLocations;
  std::vector<double*> TestLocationOffsets;
//ETX

};

#endif

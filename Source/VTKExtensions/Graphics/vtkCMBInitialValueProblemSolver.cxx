//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkCMBInitialValueProblemSolver.h"

#include "vtkAbstractInterpolatedVelocityField.h"
#include "vtkDataSet.h"
#include "vtkFunctionSet.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"

vtkStandardNewMacro(vtkCMBInitialValueProblemSolver);

using namespace vtkCMBTracerNamespace;

//---------------------------------------------------------------------------
vtkCMBInitialValueProblemSolver::vtkCMBInitialValueProblemSolver()
{
  this->NumberOfTestLocations = 1;
  this->DefaultRelativeOffset=0.0;
  double* offset = new double[3];
  offset[0]=offset[1]=offset[2]=this->DefaultRelativeOffset;
  this->TestLocationOffsets.push_back(offset);
}

//---------------------------------------------------------------------------
vtkCMBInitialValueProblemSolver::~vtkCMBInitialValueProblemSolver()
{
  this->ClearTestLocations();
}

//---------------------------------------------------------------------------
void vtkCMBInitialValueProblemSolver::ClearTestLocations()
{
  for(TestLocationIterator iter =this->TestLocations.begin();
    iter != this->TestLocations.end(); ++iter)
    {
    delete *iter;
    }
  this->TestLocations.clear();
  for(std::vector<double*>::iterator iter=this->TestLocationOffsets.begin();
    iter != this->TestLocationOffsets.end(); ++iter)
    {
    delete[] *iter;
    }
  this->TestLocationOffsets.clear();
  this->Modified();
}
//---------------------------------------------------------------------------
int vtkCMBInitialValueProblemSolver::ComputeNextStep(
  double* xprev, double* dxprev, double* xnext,
  double t, double& delT, double& delTActual,
  double minStep, double maxStep,
  double maxError, double& error)
{
  int i, numDerivs, numVals;

  delTActual = delT;
  error = 0.0;

  if (!this->FunctionSet)
    {
    vtkErrorMacro("No derivative functions are provided!");
    return NOT_INITIALIZED;
    }

  if (!this->Initialized)
    {
    vtkErrorMacro("Integrator not initialized!");
    return NOT_INITIALIZED;
    }

  numDerivs = this->FunctionSet->GetNumberOfFunctions();
  numVals = numDerivs + 1;
  for(i=0; i<numVals-1; i++)
    {
    this->Vals[i] = xprev[i];
    }
  this->Vals[numVals-1] = t;

  // Obtain the derivatives dx_i at x_i
  if (dxprev)
    {
    for(i=0; i<numDerivs; i++)
      {
      this->Derivs[i] = dxprev[i];
      }
    }
  else if ( !this->FunctionSet->FunctionValues(this->Vals, this->Derivs) )
    {
    memcpy(xnext, this->Vals, (numVals-1)*sizeof(double));
    return OUT_OF_DOMAIN;
    }
  else if(this->NumberOfTestLocations > 0)
    {
    // This is where we generate and use the test locations for computing
    // next point position.
    if(!this->InitializeTestLocations(xprev))
      {
      vtkErrorMacro("Failed to initialize the test locations!");
      return UNEXPECTED_VALUE;
      }
    return this->ComputeNextStepWithTestLocations(
      xprev, xnext,
      t, delT, delTActual,
      minStep, maxStep,
      maxError, error);
    }

// **** Otherwise using the vtkRungeKutta2

  // Half-step
  for(i=0; i<numVals-1; i++)
    {
    this->Vals[i] = xprev[i] + delT/2.0*this->Derivs[i];
    }
  this->Vals[numVals-1] = t + delT/2.0;

  // Obtain the derivatives at x_i + dt/2 * dx_i
  if (!this->FunctionSet->FunctionValues(this->Vals, this->Derivs))
    {
    memcpy(xnext, this->Vals, (numVals-1)*sizeof(double));
    return OUT_OF_DOMAIN;
    }

  // Calculate x_i using improved values of derivatives
  for(i=0; i<numDerivs; i++)
    {
    xnext[i] = xprev[i] + delT*this->Derivs[i];
    }

  return 0;
}

//---------------------------------------------------------------------------
int vtkCMBInitialValueProblemSolver::ComputeNextStepWithTestLocations(
    double* xprev, double* xnext,
    double t, double& delT, double& /*delTActual*/,
    double, double, double, double& /*error*/)
{
  int numDerivs = this->FunctionSet->GetNumberOfFunctions();
  int numVals = numDerivs + 1;
  double* avgf = new double[numDerivs];
  for(int i=0; i<numDerivs; i++)
    {
    avgf[i]=0.0;
    }

  // do an average on velocity of all the test locations
  double *velocity;
  for(TestLocationIterator iter =this->TestLocations.begin();
    iter != this->TestLocations.end(); ++iter)
    {
    velocity = (*iter)->velocity;
    for(int j=0; j<numDerivs; j++)
      {
      avgf[j] += velocity[j];
      }
    }

  // Half-step
  for(int i=0; i<numDerivs; i++)
    {
    avgf[i] = avgf[i]/this->NumberOfTestLocations;
    this->Vals[i] = xprev[i] + delT/2.0*avgf[i];
//    this->Vals[i] = xprev[i] + delT/2.0*this->Derivs[i];
    }
  this->Vals[numVals-1] = t + delT/2.0;

  // Obtain the derivatives at x_i + dt/2 * dx_i
  if (!this->FunctionSet->FunctionValues(this->Vals, this->Derivs))
    {
    delete[] avgf;
    memcpy(xnext, this->Vals, (numVals-1)*sizeof(double));
    return OUT_OF_DOMAIN;
    }
//  memcpy(xnext, this->Vals, (numVals-1)*sizeof(double));

  // Calculate x_i using improved values of derivatives
  for(int i=0; i<numDerivs; i++)
    {
    xnext[i] = xprev[i] + delT*this->Derivs[i];
    }

  delete[] avgf;
  return 0;
}

//---------------------------------------------------------------------------
bool vtkCMBInitialValueProblemSolver::InitializeTestLocations(double* xprev)
{
  this->TestLocations.clear();
  for(int i=0; i<this->NumberOfTestLocations; i++)
    {
    TestLocation* testLoc = new TestLocation();
    for(int j=0; j<3; j++)
      {
      testLoc->seed[j] = xprev[j];
      }

    this->GetRelativeOffsetOfTestLocation(i, testLoc->offset);
    this->GetTestLocationPosition(i, xprev, testLoc->offset, testLoc->pos);
    if(!this->GetTestLocationCellInfo(
      testLoc->pos, testLoc->velocity, testLoc->cellId))
      {
      // should be use some algorithm to find another test location ???
      }
    this->TestLocations.push_back(testLoc);
    }
  return true;
}
//---------------------------------------------------------------------------
void vtkCMBInitialValueProblemSolver::SetNumberOfTestLocations(int val)
{
  if (this->NumberOfTestLocations != val)
    {
    this->NumberOfTestLocations = val;
    this->ClearTestLocations();
    for(int i=0; i<this->NumberOfTestLocations; i++)
      {
      double* offset = new double[3];
      offset[0]=offset[1]=offset[2]=this->DefaultRelativeOffset;
      this->TestLocationOffsets.push_back(offset);
      }
    this->Modified();
    }
}

//---------------------------------------------------------------------------
void vtkCMBInitialValueProblemSolver::SetRelativeOffsetOfTestLocation(
  int testLocactionIndex, double* OffsetOfTestLocation )
{
  if(testLocactionIndex >= this->NumberOfTestLocations ||
    this->TestLocationOffsets.size() != static_cast<size_t>(this->NumberOfTestLocations))
    {
    return;
    }
  for(int i=0; i<3; i++)
    {
    this->TestLocationOffsets.at(testLocactionIndex)[i] =
      OffsetOfTestLocation[i];
    }
  this->Modified();
}
//---------------------------------------------------------------------------
void vtkCMBInitialValueProblemSolver::GetRelativeOffsetOfTestLocation(
  int testLocactionIndex, double* OffsetOfTestLocation )
{
  if(testLocactionIndex >= this->NumberOfTestLocations ||
    this->TestLocationOffsets.size() != static_cast<size_t>(this->NumberOfTestLocations))
    {
    return;
    }
  for(int i=0; i<3; i++)
    {
    OffsetOfTestLocation[i]=
      this->TestLocationOffsets.at(testLocactionIndex)[i];
    }
}

//---------------------------------------------------------------------------
void vtkCMBInitialValueProblemSolver::GetTestLocationPosition(
  int /*testLocactionIndex*/,  double* seedpos,
  double* relativeoffset, double* testlocation)
{
  int numVals = this->FunctionSet->GetNumberOfFunctions();
  for(int j=0; j<numVals && j<3; j++)
    {
    testlocation[j] = seedpos[j]+relativeoffset[j];
    }
}
//---------------------------------------------------------------------------
bool vtkCMBInitialValueProblemSolver::GetTestLocationCellInfo(
  double testlocation[3], double* velocity, vtkIdType& cellId)
{
  if ( this->FunctionSet->FunctionValues(testlocation, velocity) )
    {
    vtkAbstractInterpolatedVelocityField* velField =
      vtkAbstractInterpolatedVelocityField::SafeDownCast(this->FunctionSet);
    cellId = velField->GetLastCellId();
    return true;
    }
  return false;
}

//---------------------------------------------------------------------------
void vtkCMBInitialValueProblemSolver::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "NumberOfTestLocations: " << this->NumberOfTestLocations << endl;
  os << indent << "DefaultRelativeOffset: " << this->DefaultRelativeOffset << endl;
}

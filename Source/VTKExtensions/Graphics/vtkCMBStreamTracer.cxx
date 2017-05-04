//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkCMBStreamTracer.h"

#include "vtkAbstractInterpolatedVelocityField.h"
#include "vtkCMBInitialValueProblemSolver.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkCharArray.h"
#include "vtkCompositeDataIterator.h"
#include "vtkCompositeDataPipeline.h"
#include "vtkDataSetAttributes.h"
#include "vtkDoubleArray.h"
#include "vtkDoubleArray.h"
#include "vtkExecutive.h"
#include "vtkFloatArray.h"
#include "vtkGenericCell.h"
#include "vtkIdList.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkIntArray.h"
#include "vtkMath.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkOutputWindow.h"
#include "vtkPointData.h"
#include "vtkPointSet.h"
#include "vtkPolyData.h"
#include "vtkPolyLine.h"
#include "vtkSmartPointer.h"

#include "vtkCMBInitialValueProblemSolver.h"

#include <algorithm>
#include <functional>

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkCMBStreamTracer);

//---------------------------------------------------------------------------
vtkCMBStreamTracer::vtkCMBStreamTracer()
{
  this->SetInterpolatorTypeToCellLocator();
  this->SetComputeVorticity(false);

  vtkCMBInitialValueProblemSolver* solver = vtkCMBInitialValueProblemSolver::New();
  this->SetIntegrator(solver);
  solver->Delete();

  //this->SetIntegratorTypeToRungeKutta45();
}

//---------------------------------------------------------------------------
vtkCMBStreamTracer::~vtkCMBStreamTracer()
{
}

//---------------------------------------------------------------------------
int vtkCMBStreamTracer::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  if (!this->SetupOutput(inInfo, outInfo))
  {
    return 0;
  }

  vtkInformation* sourceInfo = inputVector[1]->GetInformationObject(0);
  vtkDataSet* source = 0;
  if (sourceInfo)
  {
    source = vtkDataSet::SafeDownCast(sourceInfo->Get(vtkDataObject::DATA_OBJECT()));
  }
  vtkPolyData* output = vtkPolyData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkDataArray* seeds = 0;
  vtkIdList* seedIds = 0;
  vtkIntArray* integrationDirections = 0;
  this->InitializeSeeds(seeds, seedIds, integrationDirections, source);

  if (seeds)
  {
    vtkAbstractInterpolatedVelocityField* func;
    int maxCellSize = 0;
    if (this->CheckInputs(func, &maxCellSize) != VTK_OK)
    {
      vtkDebugMacro("No appropriate inputs have been found. Can not execute.");
      func->Delete();
      seeds->Delete();
      integrationDirections->Delete();
      seedIds->Delete();
      this->InputData->UnRegister(this);
      return 1;
    }

    // initialize test locations

    vtkCompositeDataIterator* iter = this->InputData->NewIterator();
    vtkSmartPointer<vtkCompositeDataIterator> iterP(iter);
    iter->Delete();

    iterP->GoToFirstItem();
    vtkDataSet* input0 = 0;
    if (!iterP->IsDoneWithTraversal())
    {
      input0 = vtkDataSet::SafeDownCast(iterP->GetCurrentDataObject());
    }
    vtkDataArray* vectors = this->GetInputArrayToProcess(0, input0);
    if (vectors)
    {
      const char* vecName = vectors->GetName();
      double propagation = 0;
      vtkIdType numSteps = 0;
      double lastPoint[3];
      this->Integrate(input0, output, seeds, seedIds, integrationDirections, lastPoint, func,
        maxCellSize, vecName, propagation, numSteps);
    }
    func->Delete();
    seeds->Delete();
  }

  integrationDirections->Delete();
  seedIds->Delete();

  this->InputData->UnRegister(this);
  return 1;
}

//---------------------------------------------------------------------------
void vtkCMBStreamTracer::Integrate(vtkDataSet* input0, vtkPolyData* output,
  vtkDataArray* seedSource, vtkIdList* seedIds, vtkIntArray* integrationDirections,
  double lastPoint[3], vtkAbstractInterpolatedVelocityField* func, int maxCellSize,
  const char* vecName, double& inPropagation, vtkIdType& inNumSteps)
{
  int i;
  vtkIdType numLines = seedIds->GetNumberOfIds();
  double propagation = inPropagation;
  vtkIdType numSteps = inNumSteps;

  // Useful pointers
  vtkDataSetAttributes* outputPD = output->GetPointData();
  vtkDataSetAttributes* outputCD = output->GetCellData();
  vtkPointData* inputPD;
  vtkDataSet* input;
  vtkDataArray* inVectors;

  int direction = 1;

  double* weights = 0;
  if (maxCellSize > 0)
  {
    weights = new double[maxCellSize];
  }

  if (this->GetIntegrator() == 0)
  {
    vtkErrorMacro("No integrator is specified.");
    return;
  }

  // Used in GetCell()
  vtkGenericCell* cell = vtkGenericCell::New();

  // Create a new integrator, the type is the same as Integrator
  vtkInitialValueProblemSolver* integrator =
    vtkInitialValueProblemSolver::SafeDownCast(this->GetIntegrator())->NewInstance();
  integrator->SetFunctionSet(func);

  // Since we do not know what the total number of points
  // will be, we do not allocate any. This is important for
  // cases where a lot of streamers are used at once. If we
  // were to allocate any points here, potentially, we can
  // waste a lot of memory if a lot of streamers are used.
  // Always insert the first point
  vtkPoints* outputPoints = vtkPoints::New();
  vtkCellArray* outputLines = vtkCellArray::New();

  // We will keep track of integration time in this array
  vtkDoubleArray* time = vtkDoubleArray::New();
  time->SetName("IntegrationTime");

  // This array explains why the integration stopped
  vtkIntArray* retVals = vtkIntArray::New();
  retVals->SetName("ReasonForTermination");

  // We will interpolate all point attributes of the input on each point of
  // the output (unless they are turned off). Note that we are using only
  // the first input, if there are more than one, the attributes have to match.
  //
  // Note: We have to use a specific value (safe to employ the maximum number
  //       of steps) as the size of the initial memory allocation here. The
  //       use of the default argument might incur a crash problem (due to
  //       "insufficient memory") in the parallel mode. This is the case when
  //       a streamline intensely shuttles between two processes in an exactly
  //       interleaving fashion --- only one point is produced on each process
  //       (and actually two points, after point duplication, are saved to a
  //       vtkPolyData in vtkDistributedStreamTracer::NoBlockProcessTask) and
  //       as a consequence a large number of such small vtkPolyData objects
  //       are needed to represent a streamline, consuming up the memory before
  //       the intermediate memory is timely released.
  outputPD->InterpolateAllocate(input0->GetPointData(), this->MaximumNumberOfSteps);

  vtkIdType numPtsTotal = 0;
  double velocity[3];

  int shouldAbort = 0;

  for (int currentLine = 0; currentLine < numLines; currentLine++)
  {
    double progress = static_cast<double>(currentLine) / numLines;
    this->UpdateProgress(progress);

    switch (integrationDirections->GetValue(currentLine))
    {
      case FORWARD:
        direction = 1;
        break;
      case BACKWARD:
        direction = -1;
        break;
    }

    // temporary variables used in the integration
    double point1[3], point2[3];
    vtkIdType numPts = 0;

    // Clear the last cell to avoid starting a search from
    // the last point in the streamline
    func->ClearLastCellId();

    // Initial point
    seedSource->GetTuple(seedIds->GetId(currentLine), point1);
    memcpy(point2, point1, 3 * sizeof(double));
    if (!func->FunctionValues(point1, velocity))
    {
      continue;
    }

    if (propagation >= this->MaximumPropagation || numSteps > this->MaximumNumberOfSteps)
    {
      continue;
    }

    numPts++;
    numPtsTotal++;
    vtkIdType nextPoint = outputPoints->InsertNextPoint(point1);
    time->InsertNextValue(0.0);

    // We will always pass an arc-length step size to the integrator.
    // If the user specifies a step size in cell length unit, we will
    // have to convert it to arc length.
    IntervalInformation stepSize; // either positive or negative
    stepSize.Unit = LENGTH_UNIT;
    stepSize.Interval = 0;
    IntervalInformation aStep; // always positive
    aStep.Unit = LENGTH_UNIT;
    double step, minStep = 0, maxStep = 0;
    double stepTaken, accumTime = 0;
    double speed;
    double cellLength;
    int retVal = OUT_OF_LENGTH, tmp;

    // Make sure we use the dataset found by the vtkAbstractInterpolatedVelocityField
    input = func->GetLastDataSet();
    inputPD = input->GetPointData();
    inVectors = inputPD->GetVectors(vecName);

    // Convert intervals to arc-length unit
    input->GetCell(func->GetLastCellId(), cell);
    cellLength = sqrt(static_cast<double>(cell->GetLength2()));
    speed = vtkMath::Norm(velocity);
    // Never call conversion methods if speed == 0
    if (speed != 0.0)
    {
      this->ConvertIntervals(stepSize.Interval, minStep, maxStep, direction, cellLength);
    }

    // Interpolate all point attributes on first point
    func->GetLastWeights(weights);
    outputPD->InterpolatePoint(inputPD, nextPoint, cell->PointIds, weights);

    double error = 0;
    // Integrate until the maximum propagation length is reached,
    // maximum number of steps is reached or until a boundary is encountered.
    // Begin Integration
    while (propagation < this->MaximumPropagation)
    {

      if (numSteps > this->MaximumNumberOfSteps)
      {
        retVal = OUT_OF_STEPS;
        break;
      }

      if (numSteps++ % 1000 == 1)
      {
        progress = (currentLine + propagation / this->MaximumPropagation) / numLines;
        this->UpdateProgress(progress);

        if (this->GetAbortExecute())
        {
          shouldAbort = 1;
          break;
        }
      }

      // Never call conversion methods if speed == 0
      if ((speed == 0) || (speed <= this->TerminalSpeed))
      {
        retVal = STAGNATION;
        break;
      }

      // If, with the next step, propagation will be larger than
      // max, reduce it so that it is (approximately) equal to max.
      aStep.Interval = fabs(stepSize.Interval);

      if ((propagation + aStep.Interval) > this->MaximumPropagation)
      {
        aStep.Interval = this->MaximumPropagation - propagation;
        if (stepSize.Interval >= 0)
        {
          stepSize.Interval = this->ConvertToLength(aStep, cellLength);
        }
        else
        {
          stepSize.Interval = this->ConvertToLength(aStep, cellLength) * (-1.0);
        }
        maxStep = stepSize.Interval;
      }
      this->LastUsedStepSize = stepSize.Interval;

      // Calculate the next step using the integrator provided
      // Break if the next point is out of bounds.
      func->SetNormalizeVector(true);
      tmp = integrator->ComputeNextStep(point1, point2, 0, stepSize.Interval, stepTaken, minStep,
        maxStep, this->MaximumError, error);
      func->SetNormalizeVector(false);
      if (tmp != 0)
      {
        retVal = tmp;
        memcpy(lastPoint, point2, 3 * sizeof(double));
        break;
      }

      // It is not enough to use the starting point for stagnation calculation
      // Use delX/stepSize to calculate speed and check if it is below
      // stagnation threshold
      double disp[3];
      for (i = 0; i < 3; i++)
      {
        disp[i] = point2[i] - point1[i];
      }
      if ((stepSize.Interval == 0) ||
        (vtkMath::Norm(disp) / fabs(stepSize.Interval) <= this->TerminalSpeed))
      {
        retVal = STAGNATION;
        break;
      }

      accumTime += stepTaken / speed;
      // Calculate propagation (using the same units as MaximumPropagation
      propagation += fabs(stepSize.Interval);

      // This is the next starting point
      for (i = 0; i < 3; i++)
      {
        point1[i] = point2[i];
      }

      // Interpolate the velocity at the next point
      if (!func->FunctionValues(point2, velocity))
      {
        retVal = OUT_OF_DOMAIN;
        memcpy(lastPoint, point2, 3 * sizeof(double));
        break;
      }
      // Make sure we use the dataset found by the vtkAbstractInterpolatedVelocityField
      input = func->GetLastDataSet();
      inputPD = input->GetPointData();
      inVectors = inputPD->GetVectors(vecName);

      // Point is valid. Insert it.
      numPts++;
      numPtsTotal++;
      nextPoint = outputPoints->InsertNextPoint(point1);
      time->InsertNextValue(accumTime);

      // Calculate cell length and speed to be used in unit conversions
      input->GetCell(func->GetLastCellId(), cell);
      cellLength = sqrt(static_cast<double>(cell->GetLength2()));
      speed = vtkMath::Norm(velocity);
      // Interpolate all point attributes on current point
      func->GetLastWeights(weights);
      outputPD->InterpolatePoint(inputPD, nextPoint, cell->PointIds, weights);

      // Never call conversion methods if speed == 0
      if ((speed == 0) || (speed <= this->TerminalSpeed))
      {
        retVal = STAGNATION;
        break;
      }

      // Convert all intervals to arc length
      this->ConvertIntervals(step, minStep, maxStep, direction, cellLength);

      // If the solver is adaptive and the next step size (stepSize.Interval)
      // that the solver wants to use is smaller than minStep or larger
      // than maxStep, re-adjust it. This has to be done every step
      // because minStep and maxStep can change depending on the cell
      // size (unless it is specified in arc-length unit)
      if (integrator->IsAdaptive())
      {
        if (fabs(stepSize.Interval) < fabs(minStep))
        {
          stepSize.Interval = fabs(minStep) * stepSize.Interval / fabs(stepSize.Interval);
        }
        else if (fabs(stepSize.Interval) > fabs(maxStep))
        {
          stepSize.Interval = fabs(maxStep) * stepSize.Interval / fabs(stepSize.Interval);
        }
      }
      else
      {
        stepSize.Interval = step;
      }

      // End Integration
    }

    if (shouldAbort)
    {
      break;
    }

    if (numPts > 1)
    {
      outputLines->InsertNextCell(numPts);
      for (i = numPtsTotal - numPts; i < numPtsTotal; i++)
      {
        outputLines->InsertCellPoint(i);
      }
      retVals->InsertNextValue(retVal);
    }

    // Initialize these to 0 before starting the next line.
    // The values passed in the function call are only used
    // for the first line.
    inPropagation = propagation;
    inNumSteps = numSteps;

    propagation = 0;
    numSteps = 0;
  }

  if (!shouldAbort)
  {
    // Create the output polyline
    output->SetPoints(outputPoints);
    outputPD->AddArray(time);

    vtkIdType numPts = outputPoints->GetNumberOfPoints();
    if (numPts > 1)
    {
      // Assign geometry and attributes
      output->SetLines(outputLines);
      if (this->GenerateNormalsInIntegrate)
      {
        this->GenerateNormals(output, 0, vecName);
      }

      outputCD->AddArray(retVals);
    }
  }

  retVals->Delete();

  outputPoints->Delete();
  outputLines->Delete();

  time->Delete();

  integrator->Delete();
  cell->Delete();

  delete[] weights;

  output->Squeeze();
  return;
}

//---------------------------------------------------------------------------
void vtkCMBStreamTracer::SetNumberOfTestLocations(int NumTests)
{
  vtkCMBInitialValueProblemSolver* solver =
    vtkCMBInitialValueProblemSolver::SafeDownCast(this->GetIntegrator());
  if (solver)
  {
    solver->SetNumberOfTestLocations(NumTests);
  }
}
//---------------------------------------------------------------------------
int vtkCMBStreamTracer::GetNumberOfTestLocations()
{
  vtkCMBInitialValueProblemSolver* solver =
    vtkCMBInitialValueProblemSolver::SafeDownCast(this->GetIntegrator());
  if (solver)
  {
    return solver->GetNumberOfTestLocations();
  }
  return 0;
}
//---------------------------------------------------------------------------
double vtkCMBStreamTracer::GetSensorDefaultRelativeOffset()
{
  vtkCMBInitialValueProblemSolver* solver =
    vtkCMBInitialValueProblemSolver::SafeDownCast(this->GetIntegrator());
  if (solver)
  {
    return solver->GetDefaultRelativeOffset();
  }
  return 0.0;
}
//---------------------------------------------------------------------------
void vtkCMBStreamTracer::SetSensorDefaultRelativeOffset(double offset)
{
  vtkCMBInitialValueProblemSolver* solver =
    vtkCMBInitialValueProblemSolver::SafeDownCast(this->GetIntegrator());
  if (solver)
  {
    solver->SetDefaultRelativeOffset(offset);
  }
}

//---------------------------------------------------------------------------
void vtkCMBStreamTracer::SetRelativeOffsetOfTestLocation(int index, double offset[3])
{
  vtkCMBInitialValueProblemSolver* solver =
    vtkCMBInitialValueProblemSolver::SafeDownCast(this->GetIntegrator());
  if (solver)
  {
    solver->SetRelativeOffsetOfTestLocation(index, offset);
  }
}
//---------------------------------------------------------------------------
void vtkCMBStreamTracer::GetRelativeOffsetOfTestLocation(int index, double offset[3])
{
  vtkCMBInitialValueProblemSolver* solver =
    vtkCMBInitialValueProblemSolver::SafeDownCast(this->GetIntegrator());
  if (solver)
  {
    solver->GetRelativeOffsetOfTestLocation(index, offset);
  }
}

//---------------------------------------------------------------------------
void vtkCMBStreamTracer::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

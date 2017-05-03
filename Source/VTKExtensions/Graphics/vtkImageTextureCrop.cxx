//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkImageTextureCrop.h"

#include "vtkCamera.h"
#include "vtkCellData.h"
#include "vtkCompositeDataIterator.h"
#include "vtkCompositeDataSet.h"
#include "vtkDoubleArray.h"
#include "vtkExtentTranslator.h"
#include "vtkExtractSelectedFrustum.h"
#include "vtkFloatArray.h"
#include "vtkImageData.h"
#include "vtkImageResample.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMatrix4x4.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPlanes.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkRenderer.h"
#include "vtkSignedCharArray.h"
#include "vtkSmartPointer.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkTransform.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkViewport.h"

#include <algorithm>

vtkStandardNewMacro(vtkImageTextureCrop);
vtkCxxSetObjectMacro(vtkImageTextureCrop, Renderer, vtkRenderer);
vtkCxxSetObjectMacro(vtkImageTextureCrop, TransformationMatrix, vtkMatrix4x4);

vtkImageTextureCrop::vtkImageTextureCrop()
{
  this->Renderer = 0;
  this->SetNumberOfInputPorts(2);
  this->SetNumberOfOutputPorts(2);

  // setup the frustrum selector objects
  vtkPlanes* planes = vtkPlanes::New();
  this->ExtractFrustum = vtkExtractSelectedFrustum::New();
  this->ExtractFrustum->SetFrustum(planes);
  planes->FastDelete();

  vtkPoints* points = vtkPoints::New();
  points->SetNumberOfPoints(6);
  vtkDoubleArray* normals = vtkDoubleArray::New();
  normals->SetNumberOfComponents(3);
  normals->SetNumberOfTuples(6);
  planes->SetPoints(points);
  points->FastDelete();
  planes->SetNormals(normals);
  normals->FastDelete();

  this->MaxOutputImageDimensions[0] = 1024;
  this->MaxOutputImageDimensions[1] = 1024;

  this->TransformationMatrix = 0;
}

vtkImageTextureCrop::~vtkImageTextureCrop()
{
  this->SetRenderer(0);
  this->ExtractFrustum->Delete();
  this->SetTransformationMatrix(0);
}

void vtkImageTextureCrop::SetImageData(vtkDataSet* input)
{
  this->SetInputData(1, input);
}

vtkDataSet* vtkImageTextureCrop::GetImageData()
{
  if (this->GetNumberOfInputConnections(2) != 2)
  {
    return NULL;
  }

  return vtkDataSet::SafeDownCast(this->GetExecutive()->GetInputData(1, 0));
}

void vtkImageTextureCrop::SetImageDataConnection(vtkAlgorithmOutput* algOutput)
{
  this->SetInputConnection(1, algOutput);
}

void vtkImageTextureCrop::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

// Change the WholeExtent
int vtkImageTextureCrop::RequestInformation(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  int /*wholeMin, wholeMax, axis, */ ext[6];
  double spacing[3] /*, factor*/;

  vtkInformation* inInfo = inputVector[1]->GetInformationObject(0);
  vtkInformation* outInfo = outputVector->GetInformationObject(1);

  inInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), ext);
  inInfo->Get(vtkDataObject::SPACING(), spacing);

  // use a max image size
  if (ext[1] - ext[0] + 1 > this->MaxOutputImageDimensions[0])
  {
    ext[1] = this->MaxOutputImageDimensions[0] + ext[0] - 1;
  }
  if (ext[3] - ext[2] + 1 > this->MaxOutputImageDimensions[1])
  {
    ext[3] = this->MaxOutputImageDimensions[1] + ext[2] - 1;
  }

  outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), ext, 6);
  outInfo->Set(vtkDataObject::SPACING(), spacing, 3);

  return 1;
}

// This method simply copies by reference the input data to the output.
int vtkImageTextureCrop::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  vtkInformation* inInfo = inputVector[1]->GetInformationObject(0);
  vtkInformation* outInfo = outputVector->GetInformationObject(1);

  vtkImageData* outputImage =
    vtkImageData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkImageData* inputImage = vtkImageData::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkInformation* outPDInfo = outputVector->GetInformationObject(0);
  vtkPolyData* outputPD = vtkPolyData::SafeDownCast(outPDInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData* inputPD = vtkPolyData::SafeDownCast(
    (inputVector[0]->GetInformationObject(0)->Get(vtkDataObject::DATA_OBJECT())));

  outputPD->ShallowCopy(inputPD);

  // if inputExtent == outputExtent, nothing to do
  int inputExtent[6], *outputExtent;
  inputImage->GetExtent(inputExtent);
  outputExtent = outInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT());
  if (inputExtent[0] == outputExtent[0] && inputExtent[1] == outputExtent[1] &&
    inputExtent[2] == outputExtent[2] && inputExtent[3] == outputExtent[3] &&
    inputExtent[4] == outputExtent[4] && inputExtent[5] == outputExtent[5])
  {
    outputImage->ShallowCopy(inputImage);
    return VTK_OK;
  }

  vtkFloatArray* tCoords =
    vtkFloatArray::SafeDownCast(vtkDataSet::SafeDownCast(inputPD)->GetPointData()->GetTCoords());
  if (!tCoords)
  {
    vtkErrorMacro("Must have Texture coordinates on Input!  Inputs just passed through!");
    return VTK_ERROR;
  }

  // Float fornow, but need to be more careful (doesn't have to be float, I think!)
  double inputSRange[2], inputTRange[2], *sRange, *tRange;
  double computedSRange[2], computedTRange[2];
  tCoords->GetRange(inputSRange, 0);
  tCoords->GetRange(inputTRange, 1);

  int cropExtent[6];
  double requiredExtent[6];
  vtkSignedCharArray* insidedness = 0;
  if (!this->Renderer)
  {
    sRange = inputSRange;
    tRange = inputTRange;
  }
  else
  {
    this->ComputeSAndTRangeBasedOnRenderer(
      inputPD, tCoords, insidedness, computedSRange, computedTRange);
    if (!insidedness) // not in the view at all, but want to output something
    {
      sRange = inputSRange;
      tRange = inputTRange;
    }
    else
    {
      sRange = computedSRange;
      tRange = computedTRange;
    }
  }

  requiredExtent[0] = inputExtent[0] + (inputExtent[1] - inputExtent[0]) * sRange[0];
  requiredExtent[1] = inputExtent[0] + (inputExtent[1] - inputExtent[0]) * sRange[1];
  requiredExtent[2] = inputExtent[2] + (inputExtent[3] - inputExtent[2]) * tRange[0];
  requiredExtent[3] = inputExtent[2] + (inputExtent[3] - inputExtent[2]) * tRange[1];
  cropExtent[4] = requiredExtent[4] = inputExtent[4];
  cropExtent[5] = requiredExtent[5] = inputExtent[5];

  // now, compute crop extents
  for (int i = 0; i < 4; i += 2)
  {
    cropExtent[i] = static_cast<int>(floor(requiredExtent[i]));
    cropExtent[i + 1] = static_cast<int>(ceil(requiredExtent[i + 1]));
  }

  // 1st Crop, if necessary (though we always do for now)
  vtkImageData* cropData = vtkImageData::New();
  cropData->SetExtent(inputExtent);
  cropData->GetPointData()->PassData(inputImage->GetPointData());
  cropData->GetCellData()->PassData(inputImage->GetCellData());
  cropData->Crop(cropExtent);

  cropExtent[1] -= cropExtent[0];
  cropExtent[1] = std::min(cropExtent[1], inputExtent[1]);
  cropExtent[0] = 0;
  cropExtent[3] -= cropExtent[2];
  cropExtent[3] = std::min(cropExtent[3], inputExtent[3]);
  cropExtent[2] = 0;
  cropData->SetExtent(cropExtent);

  this->DoImageResample(cropExtent, outputExtent, cropData, outputImage);
  cropData->Delete();

  this->ComputeTCoords(tCoords, insidedness, outputPD, sRange, tRange);

  return VTK_OK;
}

int vtkImageTextureCrop::ComputeSAndTRangeBasedOnRenderer(vtkPolyData* inputPD,
  vtkFloatArray* tCoords, vtkSignedCharArray*& insidedness, double computedSRange[2],
  double computedTRange[2])
{
  if (!this->Renderer)
  {
    return VTK_ERROR;
  }
  vtkCamera* camera = this->Renderer->GetActiveCamera();

  double frustumPlanes[24];
  camera->GetFrustumPlanes(this->Renderer->GetAspect()[0], frustumPlanes);

  vtkPoints* points = this->ExtractFrustum->GetFrustum()->GetPoints();
  vtkDoubleArray* normals =
    vtkDoubleArray::SafeDownCast(this->ExtractFrustum->GetFrustum()->GetNormals());
  int maxIndex = 0;
  for (int i = 0; i < 6; i++)
  {
    maxIndex = fabs(frustumPlanes[i * 4]) > fabs(frustumPlanes[i * 4 + 1]) ? 0 : 1;
    maxIndex =
      fabs(frustumPlanes[i * 4 + maxIndex]) > fabs(frustumPlanes[i * 4 + 2]) ? maxIndex : 2;

    double origin[3] = { 0, 0, 0 };
    origin[maxIndex] = -frustumPlanes[i * 4 + 3] / frustumPlanes[i * 4 + maxIndex];

    points->SetPoint(i, origin);
    normals->SetTuple3(
      i, -frustumPlanes[i * 4], -frustumPlanes[i * 4 + 1], -frustumPlanes[i * 4 + 2]);
  }

  this->ExtractFrustum->Modified();
  if (this->TransformationMatrix && (this->TransformationMatrix->GetElement(0, 0) != 1.0 ||
                                      this->TransformationMatrix->GetElement(0, 1) != 0.0 ||
                                      this->TransformationMatrix->GetElement(0, 2) != 0.0 ||
                                      this->TransformationMatrix->GetElement(0, 3) != 0.0 ||
                                      this->TransformationMatrix->GetElement(1, 0) != 0.0 ||
                                      this->TransformationMatrix->GetElement(1, 1) != 1.0 ||
                                      this->TransformationMatrix->GetElement(1, 2) != 0.0 ||
                                      this->TransformationMatrix->GetElement(1, 3) != 0.0 ||
                                      this->TransformationMatrix->GetElement(2, 0) != 0.0 ||
                                      this->TransformationMatrix->GetElement(2, 1) != 0.0 ||
                                      this->TransformationMatrix->GetElement(2, 2) != 1.0 ||
                                      this->TransformationMatrix->GetElement(2, 3) != 0.0 ||
                                      this->TransformationMatrix->GetElement(3, 0) != 0.0 ||
                                      this->TransformationMatrix->GetElement(3, 1) != 0.0 ||
                                      this->TransformationMatrix->GetElement(3, 2) != 0.0 ||
                                      this->TransformationMatrix->GetElement(3, 3) != 1.0))
  {
    vtkNew<vtkTransformPolyDataFilter> transformPD;
    vtkNew<vtkTransform> transform;
    transform->SetMatrix(this->TransformationMatrix);
    transformPD->SetTransform(transform.GetPointer());
    transformPD->SetInputData(inputPD);
    this->ExtractFrustum->SetInputConnection(transformPD->GetOutputPort());
  }
  else
  {
    this->ExtractFrustum->SetInputData(inputPD);
  }
  this->ExtractFrustum->PreserveTopologyOn();
  this->ExtractFrustum->Update();

  insidedness =
    vtkSignedCharArray::SafeDownCast(vtkDataSet::SafeDownCast(this->ExtractFrustum->GetOutput())
                                       ->GetPointData()
                                       ->GetArray("vtkInsidedness"));
  if (!insidedness) // then not in view at all
  {
    return VTK_OK;
  }

  computedSRange[0] = 1;
  computedSRange[1] = 0;
  computedTRange[0] = 1;
  computedTRange[1] = 0;
  for (int i = 0; i < insidedness->GetNumberOfTuples(); i++)
  {
    signed char value = insidedness->GetValue(i);
    if (value > 0)
    {
      float* tuple = tCoords->GetPointer(i * 2);
      if (tuple[0] < computedSRange[0])
      {
        computedSRange[0] = tuple[0];
      }
      if (tuple[0] > computedSRange[1])
      {
        computedSRange[1] = tuple[0];
      }
      if (tuple[1] < computedTRange[0])
      {
        computedTRange[0] = tuple[1];
      }
      if (tuple[1] > computedTRange[1])
      {
        computedTRange[1] = tuple[1];
      }
    }
  }

  return VTK_OK;
}

void vtkImageTextureCrop::DoImageResample(
  int inputExtents[6], int outputExtents[6], vtkImageData* cropImage, vtkImageData* outputImage)
{
  double magnificationFactor[2];

  magnificationFactor[0] =
    1e-5 + static_cast<double>(outputExtents[1]) / static_cast<double>(inputExtents[1]);
  magnificationFactor[1] =
    1e-5 + static_cast<double>(outputExtents[3]) / static_cast<double>(inputExtents[3]);

  if (magnificationFactor[0] != 1 || magnificationFactor[1] != 1)
  {
    vtkSmartPointer<vtkImageResample> resample = vtkSmartPointer<vtkImageResample>::New();
    resample->SetAxisMagnificationFactor(0, magnificationFactor[0]);
    resample->SetAxisMagnificationFactor(1, magnificationFactor[1]);
    resample->SetInputData(cropImage);
    resample->Update();
    outputImage->ShallowCopy(resample->GetOutput());
  }
  else
  {
    outputImage->ShallowCopy(cropImage);
  }
}

void vtkImageTextureCrop::ComputeTCoords(vtkFloatArray* inputTCoords,
  vtkSignedCharArray* insidedness, vtkPolyData* outputPD, double* sRange, double* tRange)
{
  vtkFloatArray* tCoords = vtkFloatArray::New();
  tCoords->SetNumberOfComponents(inputTCoords->GetNumberOfComponents());
  tCoords->SetNumberOfTuples(inputTCoords->GetNumberOfTuples());
  tCoords->SetName(inputTCoords->GetName());
  outputPD->GetPointData()->SetTCoords(tCoords);
  tCoords->FastDelete();

  for (int i = 0; i < tCoords->GetNumberOfTuples(); i++)
  {
    if (!insidedness || insidedness->GetValue(i) > 0)
    {
      float* origTuple = inputTCoords->GetPointer(i * 2);
      tCoords->SetTuple2(i, (origTuple[0] - sRange[0]) / (sRange[1] - sRange[0]),
        (origTuple[1] - tRange[0]) / (tRange[1] - tRange[0]));
    }
  }
}

int vtkImageTextureCrop::FillInputPortInformation(int port, vtkInformation* info)
{
  if (port == 0)
  {
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPolyData");
    return 1;
  }
  else if (port == 1)
  {
    info->Set(vtkAlgorithm::INPUT_IS_REPEATABLE(), 0);
    info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 0);
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkImageData");
    return 1;
  }
  return 0;
}

int vtkImageTextureCrop::FillOutputPortInformation(int port, vtkInformation* info)
{
  if (port == 0)
  {
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkPolyData");
    return 1;
  }
  else if (port == 1)
  {
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkImageData");
    return 1;
  }
  return 0;
}

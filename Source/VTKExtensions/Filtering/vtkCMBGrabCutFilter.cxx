//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBGrabCutFilter.h"
#include "vtkCMBOpenCVHelper.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkImageData.h"
#include "vtkSmartPointer.h"
#include "vtkImageData.h"
#include "vtkPolyData.h"
#include "vtkImageImport.h"

#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

#include <iostream>

vtkStandardNewMacro(vtkCMBGrabCutFilter);

vtkCMBGrabCutFilter::vtkCMBGrabCutFilter()
: NumberOfIterations(12),
  PotentialForegroundValue(25),
  PotentialBackgroundValue(255),
  ForegroundValue(0),
  BackgroundValue(125)
{
  this->SetNumberOfInputPorts(2);
  this->SetNumberOfOutputPorts(3);
}

int vtkCMBGrabCutFilter::RequestData(vtkInformation *vtkNotUsed(request),
                                     vtkInformationVector **inputVector,
                                     vtkInformationVector *outputVector)
{
  // Get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *maskInfo = inputVector[1]->GetInformationObject(0);
  vtkInformation *outLabelInfo = outputVector->GetInformationObject(0);
  vtkInformation *outNextIterInfo = outputVector->GetInformationObject(1);
  vtkInformation *outPolyDataInfo = outputVector->GetInformationObject(2);

  // Get the input and ouptut
  vtkImageData *inputVTK = vtkImageData::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkImageData *maskVTK = vtkImageData::SafeDownCast(maskInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkImageData *outputLable =
                      vtkImageData::SafeDownCast(outLabelInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkImageData *outputNext =
                    vtkImageData::SafeDownCast(outNextIterInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData * outputPoly =
                    vtkPolyData::SafeDownCast(outPolyDataInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkSmartPointer<vtkImageData> image = vtkSmartPointer<vtkImageData>::New();
  vtkSmartPointer<vtkImageData> mask = vtkSmartPointer<vtkImageData>::New();
  mask->DeepCopy(maskVTK);
  image->DeepCopy(inputVTK);

  cv::Mat imageCV, maskCV;
  vtkCMBOpenCVHelper::VTKToOpenCV(image, imageCV);
  vtkCMBOpenCVHelper::VTKToOpenCV(mask, maskCV, /*to_gray*/ true);

  //cv::namedWindow( "Display window", cv::WINDOW_AUTOSIZE );// Create a window for display.
  //cv::imshow( "Display window", maskCV );                   // Show our image inside it.

  //cv::waitKey(0);

  for(int i = 0; i < maskCV.rows; i++)
  {
    uchar* Mi = maskCV.ptr<uchar>(i);
    for(int j = 0; j < maskCV.cols; j++)
    {
      if(Mi[j] != PotentialBackgroundValue &&
         Mi[j] != PotentialForegroundValue &&
         Mi[j] != ForegroundValue &&
         Mi[j] != BackgroundValue )
      {
        if(std::abs(Mi[j]-PotentialForegroundValue) < std::abs(Mi[j]-PotentialBackgroundValue))
        {
          Mi[j] = PotentialForegroundValue;
        }
        else
        {
          Mi[j] = PotentialBackgroundValue;
        }
      }
    }
  }

  {
    cv::Mat pBG = maskCV == PotentialBackgroundValue;
    cv::Mat pFG = maskCV == PotentialForegroundValue;
    cv::Mat BG = maskCV == BackgroundValue;
    cv::Mat FG = maskCV == ForegroundValue;
    maskCV.setTo(cv::GC_PR_BGD, pBG);
    maskCV.setTo(cv::GC_PR_FGD, pFG);
    maskCV.setTo(cv::GC_FGD, FG);
    maskCV.setTo(cv::GC_BGD, BG);
  }

  cv::Mat bgdModel, fgdModel;
  cv::Rect rect;

  cv::grabCut(imageCV, maskCV, rect,bgdModel,fgdModel,NumberOfIterations,cv::GC_INIT_WITH_MASK);

  cv::Mat outputLabledImageCV = maskCV.clone();

  {
    cv::Mat pBG = maskCV == cv::GC_PR_BGD;
    cv::Mat pFG = maskCV == cv::GC_PR_FGD;
    cv::Mat BG = maskCV == cv::GC_BGD;
    cv::Mat FG = maskCV == cv::GC_FGD;

    maskCV.setTo(PotentialBackgroundValue, pBG);
    maskCV.setTo(ForegroundValue, FG);
    maskCV.setTo(PotentialForegroundValue, pFG);
    maskCV.setTo(BackgroundValue, BG);

    outputLabledImageCV.setTo(BackgroundValue, pBG);
    outputLabledImageCV.setTo(ForegroundValue, FG);
    outputLabledImageCV.setTo(ForegroundValue, pFG);
    outputLabledImageCV.setTo(BackgroundValue, BG);
  }

  vtkSmartPointer<vtkPolyData> poly = vtkSmartPointer<vtkPolyData>::New();
  vtkCMBOpenCVHelper::ExtractContours(outputLabledImageCV, ForegroundValue, poly);

  cv::flip(outputLabledImageCV, outputLabledImageCV, 0);
  vtkCMBOpenCVHelper::OpenCVToVTK(outputLabledImageCV, outputLable);
  cv::flip(maskCV, maskCV, 0);
  vtkCMBOpenCVHelper::OpenCVToVTK(maskCV, outputNext);

  outputPoly->DeepCopy(poly);
  
  return 1;
}

int vtkCMBGrabCutFilter::FillOutputPortInformation(int port, vtkInformation *info)
{
  if (port == 0)
  {
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkImageData");
    return 1;
  }
  else if (port == 1)
  {
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkImageData");
    return 1;
  }
  else if (port == 2)
  {
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkPolyData");
    return 1;
  }
  return 0;
}

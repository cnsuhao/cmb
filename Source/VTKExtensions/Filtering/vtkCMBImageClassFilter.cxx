//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBImageClassFilter.h"
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

vtkStandardNewMacro(vtkCMBImageClassFilter);

vtkCMBImageClassFilter::vtkCMBImageClassFilter()
: ForegroundValue(0),
  BackgroundValue(125),
  MinFGSize(0),
  MinBGSize(0)
{
}

vtkCMBImageClassFilter::~vtkCMBImageClassFilter()
{}


int vtkCMBImageClassFilter::RequestData(vtkInformation *vtkNotUsed(request),
                                        vtkInformationVector **inputVector,
                                        vtkInformationVector *outputVector)
{
  // Get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outLabelInfo = outputVector->GetInformationObject(0);

  // Get the input and ouptut
  vtkImageData *inputVTK = vtkImageData::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkImageData *outputLable = vtkImageData::SafeDownCast(outLabelInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkSmartPointer<vtkImageData> image = vtkSmartPointer<vtkImageData>::New();
  image->DeepCopy(inputVTK);

  cv::Mat imageCV;
  vtkCMBOpenCVHelper::VTKToOpenCV(image, imageCV);

  double spacing[2] = {std::abs(image->GetSpacing()[0]),
                       std::abs(image->GetSpacing()[1])};

  cv::Mat BG  = imageCV == BackgroundValue;
  cv::Mat FG  = imageCV == ForegroundValue;

  if(MinFGSize != 0)
  {
    cv::Mat l, stats, centroids;
    int num = cv::connectedComponentsWithStats(imageCV, l, stats, centroids);

    for (int i = 0; i < num; ++i)
    {
      double area = stats.at<int>(i,cv::CC_STAT_AREA) * spacing[0] * spacing[1];
      if(area >= MinFGSize)
      {
        continue;
      }
      cv::Mat areaM = l == i;
      imageCV.setTo(BackgroundValue, areaM);
    }
  }

  if(MinBGSize != 0)
  {
    cv::Mat l, stats, centroids;
    cv::Mat iInv = imageCV.clone();
    iInv.setTo(BackgroundValue, FG);
    iInv.setTo(ForegroundValue, BG);

    int num = cv::connectedComponentsWithStats(iInv, l, stats, centroids);

    for (int i = 0; i < num; ++i)
    {
      double area = stats.at<int>(i,cv::CC_STAT_AREA) * spacing[0] * spacing[1];
      if(area >= MinBGSize)
      {
        continue;
      }
      cv::Mat areaM = l == i;
      imageCV.setTo(ForegroundValue, areaM);
    }
  }

  vtkCMBOpenCVHelper::OpenCVToVTK(imageCV, image->GetOrigin(),
                                  image->GetSpacing(), outputLable);


  return 1;
}

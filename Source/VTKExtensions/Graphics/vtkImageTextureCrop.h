//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkImageTextureCrop - Crop and sample an image used for a texture
// .SECTION Description
#ifndef __vtkImageTextureCrop_h
#define __vtkImageTextureCrop_h

#include "vtkCMBGraphicsModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"
#include "cmbSystemConfig.h"

class vtkDataSet;
class vtkExtractSelectedFrustum;
class vtkFloatArray;
class vtkImageData;
class vtkMatrix4x4;
class vtkPolyData;
class vtkRenderer;
class vtkSignedCharArray;

class VTKCMBGRAPHICS_EXPORT vtkImageTextureCrop : public vtkPolyDataAlgorithm
{
public:
  static vtkImageTextureCrop *New();
  vtkTypeMacro(vtkImageTextureCrop,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Set the vtkImageData which is the actual texture input
  void SetImageData(vtkDataSet *pd);
  vtkDataSet *GetImageData();
  void SetImageDataConnection(vtkAlgorithmOutput* algOutput);

  // Description:
  // Set/Get the renderer in which the texture is being rendered.  If this is
  // NOT set, then the filter just coprs and samples input image based
  // on usage by the first (vtkPolyData) input
  void SetRenderer(vtkRenderer *renderer);
  vtkGetObjectMacro(Renderer, vtkRenderer);

  // Description:
  // Set/Get the trasnformation matrix to apply to the texture canvas pts
  // before calculating what is in the view frustrum.
  void SetTransformationMatrix(vtkMatrix4x4*);
  vtkGetObjectMacro(TransformationMatrix, vtkMatrix4x4);

  // Description:
  // Set/Get the maximum output image dimensions.  If an axis of the input
  // image had dimension greater than the max output dimension, it is clamped
  // at the max dimension.  Defaults to 1024 x 1024.
  vtkSetVector2Macro(MaxOutputImageDimensions, int);
  vtkGetVector2Macro(MaxOutputImageDimensions, int);

protected:
  vtkImageTextureCrop();
  ~vtkImageTextureCrop() override;

  vtkRenderer *Renderer;
  vtkExtractSelectedFrustum *ExtractFrustum;

  float ComputeSRange[2];
  float ComputeTRange[2];
  int OutputExtents[6];
  double MagnificationFactor[2];

  int RequestInformation (vtkInformation *,
                                  vtkInformationVector **,
                                  vtkInformationVector *) override;
  vtkImageData *AllocateOutputData(vtkDataObject *output);

  int RequestData(vtkInformation *,
                          vtkInformationVector **,
                          vtkInformationVector *) override;
  int FillInputPortInformation(int, vtkInformation *) override;
  int FillOutputPortInformation(int, vtkInformation *) override;

  int ComputeSAndTRangeBasedOnRenderer(vtkPolyData *inputPD,
    vtkFloatArray *tCoords, vtkSignedCharArray *&insidedness,
    double computedSRange[2], double computedTRange[2]);
  void DoImageResample(int inputExtents[6], int outputExtents[6],
    vtkImageData *cropImage, vtkImageData *outputImage);

  void ComputeTCoords(vtkFloatArray *inputTCoords,
    vtkSignedCharArray *insidedness, vtkPolyData *outputPD,
    double *sRange, double *tRange);

  int MaxOutputImageDimensions[2];

  vtkMatrix4x4 *TransformationMatrix;

private:
  vtkImageTextureCrop(const vtkImageTextureCrop&);  // Not implemented.
  void operator=(const vtkImageTextureCrop&);  // Not implemented.
};



#endif




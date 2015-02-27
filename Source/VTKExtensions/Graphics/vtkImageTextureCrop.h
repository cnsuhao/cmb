/*=========================================================================

Copyright (c) 1998-2012 Kitware Inc. 28 Corporate Drive,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed, or modified, in any form or by any means, without
permission in writing from Kitware Inc.

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
  void PrintSelf(ostream& os, vtkIndent indent);

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
  ~vtkImageTextureCrop();

  vtkRenderer *Renderer;
  vtkExtractSelectedFrustum *ExtractFrustum;

  float ComputeSRange[2];
  float ComputeTRange[2];
  int OutputExtents[6];
  double MagnificationFactor[2];

  virtual int RequestInformation (vtkInformation *,
                                  vtkInformationVector **,
                                  vtkInformationVector *);
  vtkImageData *AllocateOutputData(vtkDataObject *output);

  virtual int RequestData(vtkInformation *,
                          vtkInformationVector **,
                          vtkInformationVector *);
  virtual int FillInputPortInformation(int, vtkInformation *);
  virtual int FillOutputPortInformation(int, vtkInformation *);

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




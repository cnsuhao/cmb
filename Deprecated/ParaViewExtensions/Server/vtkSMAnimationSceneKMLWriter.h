//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkSMAnimationSceneKMLWriter - Writes data as KML with animation
// enabled.
//
// .SECTION Description
// This class enables  the use to convert VTK data into KML data. Each
// timestep is written as single KML file and another KML (super kml)
// references each of these generated KML files along with time span
// data.
//
// .SECTION Notes
// Requires libkml.
//

#ifndef __vtkSMAnimationSceneKMLWriter_h
#define __vtkSMAnimationSceneKMLWriter_h

#include "vtkSMAnimationSceneWriter.h"
#include "cmbSystemConfig.h"

#include "vtkKMLExporter.h"

class vtkImageData;
class vtkSMViewProxy;

class VTK_EXPORT vtkSMAnimationSceneKMLWriter : public vtkSMAnimationSceneWriter
{
public:
  static vtkSMAnimationSceneKMLWriter* New();
  vtkTypeMacro(vtkSMAnimationSceneKMLWriter, vtkSMAnimationSceneWriter);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set the magnification factor to use for the saved animation.
  vtkSetClampMacro(Magnification, int, 1, VTK_INT_MAX);
  vtkGetMacro(Magnification, int);

  // Description:
  // Get the error code which is set if there's an error while writing
  // the images.
  vtkGetMacro(ErrorCode, int);

  // Description:
  // Get/Set the RGB background color to use to fill empty spaces in the image.
  // RGB components are in the range [0,1].
  vtkSetVector3Macro(BackgroundColor, double);
  vtkGetVector3Macro(BackgroundColor, double);


  // Description:
  // Get/Set KML exporter to use.
  vtkGetMacro(KMLExporter, vtkKMLExporter*);
  vtkSetMacro(KMLExporter, vtkKMLExporter*);

protected:
  vtkSMAnimationSceneKMLWriter();
  ~vtkSMAnimationSceneKMLWriter();

  // Description:
  // Called to initialize saving.
  virtual bool SaveInitialize(int fileCount);

  // Description:
  // Called to save a particular frame.
  virtual bool SaveFrame(double time);

  // Description:
  // Called to finalize saving.
  virtual bool SaveFinalize();

  // Creates the writer based on file type.
  bool CreateWriter();

  // Updates the ActualSize which is the
  // resolution of the generated animation frame.
  void UpdateImageSize();

  // Description:
  // Captures the view from the given module and
  // returns a new Image data object. May return NULL.
  // Default implementation can only handle vtkSMViewProxy subclasses.
  // Subclassess must override to handle other types of view modules.
  void CaptureViewImage(vtkSMViewProxy*, int magnification);

  vtkSetVector2Macro(ActualSize, int);

  int ActualSize[2];
  int Magnification;
  int FileCount;
  int ErrorCode;

  char* Prefix;
  char* Suffix;
  vtkSetStringMacro(Prefix);
  vtkSetStringMacro(Suffix);

  double BackgroundColor[3];

  vtkKMLExporter* KMLExporter;

private:
  vtkSMAnimationSceneKMLWriter(const vtkSMAnimationSceneKMLWriter&); // Not implemented.
  void operator=(const vtkSMAnimationSceneKMLWriter&); // Not implemented.
};


#endif


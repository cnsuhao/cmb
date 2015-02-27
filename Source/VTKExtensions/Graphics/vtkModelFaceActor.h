/*=========================================================================

  Program:   CMB
  Module:    vtkModelFaceActor.h

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkModelFaceActor - an actor that supports CMB Model Faces
// .SECTION Description
// vtkModelFaceActor  is a very simple version of vtkPVLODActor.
// It supports the ability to determine its color based on the value
// of ModelFaceColorMode:
// 0 - Mapper: use whatever the mapper is currently set to
// 1 - ModelFaceID: use the model face id along with the lookup table set to the mapper
// 2 - ShellID: use the shell id along with the lookup table set to the mapper
// 3 - MaterialID: use the material id along with the lookup table set to the mapper
// 4 - Boundary Condition Set ID - yet to be determined
// .SECTION see also
// vtkActor vtkPVLODActor vtkRenderer vtkLODProp3D vtkLODActor

#ifndef __vtkModelFaceActor_h
#define __vtkModelFaceActor_h

#include "vtkCMBGraphicsModule.h" // For export macro
#include "vtkPVLODActor.h"
#include "cmbSystemConfig.h"

class vtkMapper;

class VTKCMBGRAPHICS_EXPORT vtkModelFaceActor : public vtkPVLODActor
{
public:
  vtkTypeMacro(vtkModelFaceActor,vtkPVLODActor);
  void PrintSelf(ostream& os, vtkIndent indent);

  static vtkModelFaceActor *New();

  // Description:
  // This causes the actor to be rendered. It, in turn, will render the actor's
  // property and then mapper.
  virtual void Render(vtkRenderer *, vtkMapper *);

  // Description:
  // Shallow copy of an LOD actor. Overloads the virtual vtkProp method.
  void ShallowCopy(vtkProp *prop);

  // Description:
  // When set, LODMapper, if present it used, otherwise the regular mapper is
  // used.
  vtkSetMacro(ModelFaceColorMode, int);
  vtkGetMacro(ModelFaceColorMode, int);

  // Description:
  // Set the color to be used for the backface (if UseBackface is on)
  void SetBackfaceColor(double *color);
  void SetBackfaceColor(double c0, double c1, double c2)
    {
    double color[3] = {c0, c1, c2};
    this->SetBackfaceColor(color);
    }
  vtkGetVector3Macro(BackfaceColor, double);

  // Description:
  // Turn on/off wheter or not to use the BackfaceColor
  void SetUseBackface(bool useBackface);
  vtkGetMacro(UseBackface, bool);

protected:
  vtkModelFaceActor();
  ~vtkModelFaceActor();
  int ModelFaceColorMode;
  double BackfaceColor[3];
  bool UseBackface;

private:
  vtkModelFaceActor(const vtkModelFaceActor&); // Not implemented.
  void operator=(const vtkModelFaceActor&); // Not implemented.
};

#endif



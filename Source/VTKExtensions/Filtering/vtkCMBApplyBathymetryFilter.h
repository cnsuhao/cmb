/*=========================================================================

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

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
// .NAME vtkCMBApplyBathymetryFilter
// .SECTION Description

#ifndef __vtkCMBApplyBathymetryFilter_h
#define __vtkCMBApplyBathymetryFilter_h

#include "vtkCMBFilteringModule.h" // For export macro
#include "vtkDataSetAlgorithm.h"
#include "cmbSystemConfig.h"

class vtkPoints;

class VTKCMBFILTERING_EXPORT vtkCMBApplyBathymetryFilter : public vtkDataSetAlgorithm
{
public:
  static vtkCMBApplyBathymetryFilter *New();
  vtkTypeMacro(vtkCMBApplyBathymetryFilter,vtkDataSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  //Description:
  //Set/Get the radius of the cone to use for elevation smoothing
  vtkSetMacro(ElevationRadius,double);
  vtkGetMacro(ElevationRadius,double);

  //Description:
  //Set/Get the highest z value which the input will be set to if
  // UseHighestZValue is set to true. Default is 0.0
  vtkSetMacro(HighestZValue,double);
  vtkGetMacro(HighestZValue,double);

  // Description:
  // If "on", the input points' highest z values will be set to HighestZValue
  // Default is OFF
  vtkBooleanMacro(UseHighestZValue, bool);
  vtkSetMacro(UseHighestZValue, bool);
  vtkGetMacro(UseHighestZValue, bool);

  //Description:
  //Set/Get the lowest z value which the input will be set to if
  // UseLowestZValue is set to true. Default is 0.0
  vtkSetMacro(LowestZValue,double);
  vtkGetMacro(LowestZValue,double);

  // Description:
  // If "on", the input points' lowest z values will be set to LowestZValue
  // Default is OFF;
  vtkBooleanMacro(UseLowestZValue, bool);
  vtkSetMacro(UseLowestZValue, bool);
  vtkGetMacro(UseLowestZValue, bool);

  //Description:
  //Set/Get the z value which the input will be set to if
  // FlattenZValues is set to true. Default is 0.0
  vtkSetMacro(FlatZValue,double);
  vtkGetMacro(FlatZValue,double);

  // Description:
  // If "on", the input points' z values will be set to FlatZValue
  vtkBooleanMacro(FlattenZValues, bool);
  vtkSetMacro(FlattenZValues, bool);
  vtkGetMacro(FlattenZValues, bool);

  // Description:
  // If "on", the output is simply a shallowcopy of input
  vtkBooleanMacro(NoOP, bool);
  vtkSetMacro(NoOP, bool);
  vtkGetMacro(NoOP, bool);

  //Description:
  //Remove all connections on port 0, dataset that will be altered
  //with bathymetry
  void RemoveInputConnections();

  //Description:
  //Remove all connection on port 1, point sources
  void RemoveSourceConnections();

protected:
  vtkCMBApplyBathymetryFilter();
  ~vtkCMBApplyBathymetryFilter();

  virtual int FillInputPortInformation(int port, vtkInformation *info);
  virtual int RequestData(vtkInformation* request,
                                 vtkInformationVector** inputVector,
                                 vtkInformationVector* outputVector);


  //methods for flattening mesh
  bool FlattenMesh(vtkPoints*);

  //methods for apply bathymetry
  bool ApplyBathymetry(vtkPoints *points);

  double ElevationRadius;
  double HighestZValue;
  bool UseHighestZValue;
  double LowestZValue;
  bool UseLowestZValue;
  double FlatZValue;
  bool FlattenZValues;
  bool NoOP;

  //BTX
  class vtkCmbInternalTerrainInfo;
  vtkCmbInternalTerrainInfo *TerrainInfo;
  //ETX

private:
  vtkCMBApplyBathymetryFilter(const vtkCMBApplyBathymetryFilter&);  // Not implemented.
  void operator=(const vtkCMBApplyBathymetryFilter&);  // Not implemented.
};

#endif

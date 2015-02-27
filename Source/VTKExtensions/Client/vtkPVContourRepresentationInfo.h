/*=========================================================================

  Program:   ParaView
  Module:    vtkPVContourRepresentationInfo.h

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkPVContourRepresentationInfo -
// .SECTION Description
// .SECTION Caveats

#ifndef __vtkPVContourRepresentationInfo_h
#define __vtkPVContourRepresentationInfo_h

#include "vtkCMBClientModule.h" // For export macro
#include "vtkPVInformation.h"
#include "cmbSystemConfig.h"
#include <string>

class vtkStringArray;
class vtkFloatArray;
class vtkDoubleArray;
class vtkIdTypeArray;
class vtkIdList;

class vtkContourRepresentation;
class vtkSceneContourSource;

class VTKCMBCLIENT_EXPORT vtkPVContourRepresentationInfo : public vtkPVInformation
{
public:
  static vtkPVContourRepresentationInfo* New();
  vtkTypeMacro(vtkPVContourRepresentationInfo, vtkPVInformation);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Transfer information about a single object into this object.
  virtual void CopyFromObject(vtkObject*);

  // Description:
  // Manage a serialized version of the information.
  virtual void CopyToStream(vtkClientServerStream*);
  virtual void CopyFromStream(const vtkClientServerStream*);

  // Description:
  // Convenient method to get the world locations for all the nodes,
  vtkGetObjectMacro( AllNodesWorldPositions, vtkDoubleArray );

  // Description:
  // Convenient method to get selected node indices,
  vtkGetObjectMacro( SelectedNodes, vtkIdTypeArray );

  // Description:
  // Get the number of nodes.
  virtual int GetNumberOfAllNodes();
  virtual int GetNumberOfSelectedNodes();

  // Description:
  // Get if the loop is closed
  vtkGetMacro(ClosedLoop,int);

  // Description:
  // Get the bounds of the box (defined by vtk style)
  void GetBounds(double bounds[6]) const
  {
    this->GetBounds(bounds[0], bounds[1], bounds[2],
      bounds[3], bounds[4], bounds[5]);
  }
  void GetBounds(double &xMin, double &xMax,
    double &yMin, double &yMax,
    double &zMin, double &zMax) const;

  //BTX
protected:
  vtkPVContourRepresentationInfo();
  ~vtkPVContourRepresentationInfo();

  void CopyFromContourPolySource( vtkSceneContourSource* source );
  void CopyFromContourRepresentation(vtkContourRepresentation* contourRep);


  vtkDoubleArray* AllNodesWorldPositions;
  vtkIdTypeArray* SelectedNodes;
  int ClosedLoop;
  double Bounds[6];

private:

  vtkPVContourRepresentationInfo(const vtkPVContourRepresentationInfo&); // Not implemented
  void operator=(const vtkPVContourRepresentationInfo&); // Not implemented
  //ETX
};

#endif

/*=========================================================================

  Program:   ParaView
  Module:    vtkPVArcInfo.h

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkPVArcInfo -
// .SECTION Description
// .SECTION Caveats This class only works in built-in mode

#ifndef __vtkPVArcInfo_h
#define __vtkPVArcInfo_h

#include "vtkCMBClientModule.h" // For export macro
#include "vtkPVInformation.h"
#include "cmbSystemConfig.h"
class vtkDoubleArray;
class vtkIdTypeArray;

class VTKCMBCLIENT_EXPORT vtkPVArcInfo : public vtkPVInformation
{
public:
  static vtkPVArcInfo* New();
  vtkTypeMacro(vtkPVArcInfo, vtkPVInformation);
  void PrintSelf(ostream& os, vtkIndent indent);

  //Description:
  //Tell the gather to only fill if the arc is a closed loop
  //Note: This is the default
  void SetGatherLoopInfoOnly(){GatherLoopInfoOnly=true;}

  //Description:
  //Gather all information for the arc
  void SetGatherAllInfo(){GatherLoopInfoOnly=false;}

  // Description:
  // Transfer information about a single object into this object.
  virtual void CopyFromObject(vtkObject*);

  // Description:
  // Manage a serialized version of the information.
  virtual void CopyToStream(vtkClientServerStream*);
  virtual void CopyFromStream(const vtkClientServerStream*);

  //Description:
  //Returns if the this arc is a closed loop
  bool IsClosedLoop(){return ClosedLoop;}

  //Description:
  //Returns the total number of points which is end nodes + internal points
  vtkIdType GetNumberOfPoints(){return NumberOfPoints;};

  //Description:
  //Returns the position of the end node at a given position
  bool GetEndNodePos(vtkIdType index, double pos[3]);

  //Description:
  //Returns the position of the point position, given the point index;
  bool GetPointLocation(vtkIdType index, double pos[3]);

  //Description:
  //Returns the location of all the points in the arc
  vtkGetObjectMacro(PointLocations,vtkDoubleArray);

  //Description
  //Returns the ids of the end nodes
  vtkGetObjectMacro(EndNodeIds,vtkIdTypeArray)

  //BTX
protected:
  vtkPVArcInfo();
  ~vtkPVArcInfo();

  void GatherLoopInfo();
  void GatherDetailedInfo();

  bool ClosedLoop;
  bool GatherLoopInfoOnly;
  vtkIdType ArcId;

  vtkIdType NumberOfPoints;
  double *EndNodePos;
  vtkDoubleArray* PointLocations;

  vtkIdTypeArray* EndNodeIds;

private:

  vtkPVArcInfo(const vtkPVArcInfo&); // Not implemented
  void operator=(const vtkPVArcInfo&); // Not implemented
  //ETX
};

#endif

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBArc - vtkDataObject that represent a single arc
// .SECTION Description
// An arc is represented by a line with 1 or 2 end nodes
// and a collection of internal points.
// Each arc has a unique Id

#ifndef __vtkCMBArcProvider_h
#define __vtkCMBArcProvider_h


#include "vtkCMBGeneralModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"
#include "cmbSystemConfig.h"
#include <list>

class vtkCMBArc;
class vtkCMBArcManager;
class vtkPolyData;

class VTKCMBGENERAL_EXPORT vtkCMBArcProvider : public vtkPolyDataAlgorithm
{
public:
  static vtkCMBArcProvider *New();
  vtkTypeMacro(vtkCMBArcProvider,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);


  void SetArcId(vtkIdType arcId);
  vtkGetMacro(ArcId,vtkIdType);
  void SetStartPointId(vtkIdType startPointId);
  vtkGetMacro(StartPointId,vtkIdType);
  void SetEndPointId(vtkIdType endPointId);
  vtkGetMacro(EndPointId,vtkIdType);

  // Description:
  // Return the mtime also considering the arc.
  unsigned long GetMTime();

protected:
  vtkCMBArcProvider();
  ~vtkCMBArcProvider();

  // Description:
  // This is called by the superclass.
  // This is the method you should override.
  virtual int RequestData(vtkInformation* request,
                          vtkInformationVector** inputVector,
                          vtkInformationVector* outputVector);

  //Description:
  //Generate the polyData representation for the associated arc
  vtkPolyData* CreatePolyDataRepresentation();

  //Description:
  //Generate the sub-arc polyData representation specified by
  // StartPointId and EndPointId, from the associated arc
  vtkPolyData* CreateSubArcPolyDataRepresentation();

  vtkCMBArcManager *ArcManager;
  vtkCMBArc *Arc;
  vtkIdType ArcId;
  vtkIdType StartPointId;
  vtkIdType EndPointId;

private:
  vtkCMBArcProvider(const vtkCMBArcProvider&);  // Not implemented.
  void operator=(const vtkCMBArcProvider&);  // Not implemented.
};

#endif

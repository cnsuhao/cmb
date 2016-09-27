//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkSceneContourSource - "Dummy" source so we can treat data as a source
// .SECTION Description
// The input Source data is shallow copied to the output

#ifndef __vtkSceneContourSource_h
#define __vtkSceneContourSource_h

#include "vtkCMBGeneralModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"
#include "cmbSystemConfig.h"

class vtkIdTypeArray;
class vtkContourPointCollection;

class VTKCMBGENERAL_EXPORT vtkSceneContourSource : public vtkPolyDataAlgorithm
{
public:
  static vtkSceneContourSource *New();
  vtkTypeMacro(vtkSceneContourSource,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  void CopyData(vtkPolyData *source);
  vtkGetObjectMacro(Source, vtkPolyData);

  //Description:
  // Returns a poly data that can be used in the contour widget.
  //This means it doesn't use the contourPointCollection point set.
  //NOTE: Who ever calls this method needs to delete the returned data.
  vtkPolyData* GetWidgetOutput();

  //Description:
  //returns the current end nodes for this object
  //for the most up to date end nodes, call
  //RegenerateEndNodes first
  vtkGetObjectMacro(EndNodes,vtkIdTypeArray);

  //Description:
  //Syncs this objects end nodes with
  //the global end node collection
  void RegenerateEndNodes();

  vtkSetMacro(ClosedLoop,int);
  vtkGetMacro(ClosedLoop,int);

  vtkIdType GetInstanceId() const {return Id;};

  //Description:
  //Since the node is going to be removed ( added to the undo stack )
  //We need to remove the ends from the global end node collection
  void MarkedForDeletion();

  //Description:
  //Since the node is not being removed ( removed from the undo stack )
  //We need to add the ends from the global end node collection
  void UnMarkedForDeletion();

protected:
  vtkSceneContourSource();
  ~vtkSceneContourSource() override;

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *) override;

  void InitSourceData(vtkPolyData *source);
  void EditSourceData(vtkPolyData *source);

  void UpdateSelectedNodes(vtkPolyData *source);

  vtkIdTypeArray *EndNodes;
  vtkPolyData *Source;
  vtkContourPointCollection *Collection;

  int ClosedLoop;
private:
  vtkSceneContourSource(const vtkSceneContourSource&);  // Not implemented.
  void operator=(const vtkSceneContourSource&);  // Not implemented.
  const vtkIdType Id;
  static vtkIdType NextId;
};

#endif

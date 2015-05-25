//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkMergeFacesFilter
// .SECTION Description
// This filter can be used to merge faces with different ids.

#ifndef __vtkMergeFacesFiter_h
#define __vtkMergeFacesFiter_h

#include "vtkCMBFilteringModule.h" // For export macro
#include "vtkMultiBlockDataSetAlgorithm.h"
#include "cmbSystemConfig.h"

class vtkIdList;

class VTKCMBFILTERING_EXPORT vtkMergeFacesFilter : public vtkMultiBlockDataSetAlgorithm
{
public:
  static vtkMergeFacesFilter* New();
  vtkTypeMacro(vtkMergeFacesFilter, vtkMultiBlockDataSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Merge face with id \c id1 with face with id \c id2.
  void Merge(vtkIdType id1, vtkIdType id2);
  void RemoveAllMergedFaces();
  void RemoveAllMergedFacesInternal();

  // Description:
  // Returns a list of ids. The index is the original id,
  // while the value is the new id. This is valid only after the filter has
  // executed and will change everytime the filter is executed.
  //vtkGetObjectMacro(NewIds, vtkIdList);

  // API to iterate over merged groups.
  void Begin();

  bool IsDoneGroup();
  vtkIdType GetElement();
  void NextElement();
  void NextGroup();

  bool IsDone();

  //vtkIdType GetNewFaceId(vtkIdType oldId);

//BTX
protected:
  vtkMergeFacesFilter();
  ~vtkMergeFacesFilter();

  //vtkIdList* NewIds;

  // Description:
  // This is called within ProcessRequest when a request asks the algorithm
  // to do its work. This is the method you should override to do whatever the
  // algorithm is designed to do. This happens during the fourth pass in the
  // pipeline execution process.
  virtual int RequestData(vtkInformation*,
                          vtkInformationVector**,
                          vtkInformationVector*);
  virtual int FillInputPortInformation(int port, vtkInformation *info);

private:
  vtkMergeFacesFilter(const vtkMergeFacesFilter&); // Not implemented.
  void operator=(const vtkMergeFacesFilter&); // Not implemented.


  class vtkInternal;
  vtkInternal* Internal;
//ETX
};

#endif



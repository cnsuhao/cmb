//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkFacesConnectivityFilter
// .SECTION Description
// This filter can be used to merge faces with different ids.

#ifndef __vtkFacesConnectivityFilter_h
#define __vtkFacesConnectivityFilter_h

#include "vtkCMBFilteringModule.h" // For export macro
#include "vtkMultiBlockDataSetAlgorithm.h"
#include "cmbSystemConfig.h"
#include <map>

class vtkIntArray;
class vtkIdTypeArray;
class vtkIdList;

class VTKCMBFILTERING_EXPORT vtkFacesConnectivityFilter : public vtkMultiBlockDataSetAlgorithm
{
public:
  static vtkFacesConnectivityFilter* New();
  vtkTypeMacro(vtkFacesConnectivityFilter, vtkMultiBlockDataSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Returns a list of new block indices,
  //vtkGetObjectMacro(NewBlockIndices, vtkIdList);

  // Description:
  // Specify the angle that defines a sharp edge. If the difference in
  // angle across neighboring polygons is greater than this value, the
  // shared edge is considered "sharp".
  vtkSetClampMacro(FeatureAngle,double,0.0,180.0);
  vtkGetMacro(FeatureAngle,double);

  // Description:
  // Specify the face Id that will be splitted.
  vtkSetMacro(FaceID,int);
  vtkGetMacro(FaceID,int);

  // Description:
  // Convenience method to specify the selection connection (2nd input
  // port)
  //void SetSelectionConnection(vtkAlgorithmOutput* algOutput)
  //{
  //  this->SetInputConnection(1, algOutput);
  //};

  // Description:
  // Obtain the number of connected faces.
  // int GetNumberOfExtractedFaces();

//BTX
protected:
  vtkFacesConnectivityFilter();
  ~vtkFacesConnectivityFilter();

  // Description:
  // This is called within ProcessRequest when a request asks the algorithm
  // to do its work. This is the method you should override to do whatever the
  // algorithm is designed to do. This happens during the fourth pass in the
  // pipeline execution process.
  //virtual int RequestData(vtkInformation*,
  //                        vtkInformationVector**,
  //                        vtkInformationVector*);

  // Usual data generation method
  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  virtual int FillInputPortInformation(int port, vtkInformation *info);

private:
  vtkFacesConnectivityFilter(const vtkFacesConnectivityFilter&); // Not implemented.
  void operator=(const vtkFacesConnectivityFilter&); // Not implemented.

  void UpdateFaceIDArray(
    int maxFaceId, vtkIdTypeArray* newRegionArray,
    vtkIntArray* selCellIndices,
    std::map<vtkIdType, vtkIdList*> & faceList);

  double FeatureAngle;
  int FaceID;
  //vtkIdList* NewBlockIndices;

  class vtkInternal;
  vtkInternal* Internal;
//ETX
};

#endif



//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkClipPolygons - clip polygonal data with user-specified implicit function or input scalar data
// .SECTION Description
// vtkClipPolygons is a filter that clips polygonal data using either
// any subclass of vtkImplicitFunction, or the input scalar
// data. Clipping means that it actually "cuts" through the cells of
// the dataset, returning everything inside of the specified implicit
// function (or greater than the scalar value) including "pieces" of
// a cell. (Compare this with vtkExtractGeometry, which pulls out
// entire, uncut cells.) The output of this filter is polygonal data.
//
// To use this filter, you must decide if you will be clipping with an
// implicit function, or whether you will be using the input scalar
// data.  If you want to clip with an implicit function, you must:
// 1) define an implicit function
// 2) set it with the SetClipFunction method
// 3) apply the GenerateClipScalarsOn method
// If a ClipFunction is not specified, or GenerateClipScalars is off
// (the default), then the input's scalar data will be used to clip
// the polydata.
//
// You can also specify a scalar value, which is used to
// decide what is inside and outside of the implicit function. You can
// also reverse the sense of what inside/outside is by setting the
// InsideOut instance variable. (The cutting algorithm proceeds by
// computing an implicit function value or using the input scalar data
// for each point in the dataset. This is compared to the scalar value
// to determine inside/outside.)
//
// This filter can be configured to compute a second output. The
// second output is the polygonal data that is clipped away. Set the
// GenerateClippedData boolean on if you wish to access this output data.

// .SECTION Caveats
// In order to cut all types of cells in polygonal data, vtkClipPolygons
// triangulates some cells, and then cuts the resulting simplices
// (i.e., points, lines, and triangles). This means that the resulting
// output may consist of different cell types than the input data.

// .SECTION See Also
// vtkClipPolyData vtkImplicitFunction vtkCutter vtkClipVolume

#ifndef __vtkClipPolygons_h
#define __vtkClipPolygons_h

#include "vtkCMBFilteringModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"
#include "cmbSystemConfig.h"

class vtkImplicitFunction;
class vtkIncrementalPointLocator;
class vtkTransform;
#include <map>

class VTKCMBFILTERING_EXPORT vtkClipPolygons : public vtkPolyDataAlgorithm
{
public:
  vtkTypeMacro(vtkClipPolygons,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Construct with user-specified implicit function; InsideOut turned off;
  // value set to 0.0; and generate clip scalars turned off.
  static vtkClipPolygons *New();

  // Description:
  // Set the active group to modify
  void SetActiveGroupIndex(int i)
    {
    if (i != this->ActiveGroupIdx)
      {
      ActiveGroupIdx = i;
      //Do not want to trigger the filter
      // this->Modified();
      }
    }

  void SetGroupInvert(int val);

  // Description:
  // Check if the valid group is valid
  int IsActiveGroupValid();

  // Description:
  // Control whether a second output is generated. The second output
  // contains the polygonal data that's been clipped away.
  vtkSetMacro(GenerateClippedOutput,int);
  vtkGetMacro(GenerateClippedOutput,int);
  vtkBooleanMacro(GenerateClippedOutput,int);

  // Description:
  // Return the Clipped output.
  vtkPolyData *GetClippedOutput();

  // Description:
  // Return the output port (a vtkAlgorithmOutput) of the clipped output.
  vtkAlgorithmOutput* GetClippedOutputPort();

  // Description:
  // Set the clipping value of the implicit function (if clipping with
  // implicit function) or scalar value (if clipping with
  // scalars). The default value is 0.0.
  vtkSetMacro(Value,double);
  vtkGetMacro(Value,double);

  // Description:
  // Specify a spatial locator for merging points. By default, an
  // instance of vtkMergePoints is used.
  void SetLocator(vtkIncrementalPointLocator *locator);
  vtkGetObjectMacro(Locator,vtkIncrementalPointLocator);

  // Description:
  // Create default locator. Used to create one when none is specified. The
  // locator is used to merge coincident points.
  void CreateDefaultLocator();

  // Description:
  // Return the mtime also considering the locator and clip function.
  vtkMTimeType GetMTime() override;

  // Description:
  // Transform to apply to the pts being read in for determining whether the
  // data is in/out of the ReadBounds (if LimitReadToBounds is true), or for
  // transforming data for the output (or both);  Note, the transform is
  // ignored if neither LimitReadToBounds nor TransformOutputData is true.
  void SetTransform(vtkTransform *transform);
  vtkGetObjectMacro(Transform, vtkTransform);
  void SetTransform(double elements[16]);
  void ClearTransform()
    {
    this->SetTransform(static_cast<vtkTransform*>(0));
    }

  // Description:
  // Add/Remove clip functions
  void AddClipPolygon(vtkImplicitFunction*);
  void RemoveClipPolygon(vtkImplicitFunction*);
  void RemoveAllClipPolygons();

  // Description:
  // Change the Value/InsideOut/AsROI of a clip function given its index
  void SetClipApplyPolygon(int idx, int ApplyPolygon);
  void SetClipInsideOut(int idx, int vaule);
  void SetClipAsROI(int idx, int vaule);

  // Description:
  // Get number of active polygons, whose ApplyPolygon is 1;
  int GetNumberOfActivePolygons();

//BTX
protected:
  vtkClipPolygons();
  ~vtkClipPolygons() override;

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *) override;

  vtkIncrementalPointLocator *Locator;
  int GenerateClippedOutput;
  double Value;

  vtkTransform *Transform;
  bool TransformOutputData;
  int ActiveGroupIdx;

  struct PolygonInfo
    {
    PolygonInfo()
      {
      this->InsideOut = 1;
      this->Polygon = NULL;
      this->ApplyPolygon = 0;
      this->AsROI = 0;
      }
    int InsideOut;
    int ApplyPolygon;
    int AsROI;
    vtkImplicitFunction* Polygon;
    };

  std::map<int, std::vector<PolygonInfo*> >Polygons;
  std::map<int, int> GroupInvert;

private:
  vtkClipPolygons(const vtkClipPolygons&);  // Not implemented.
  void operator=(const vtkClipPolygons&);  // Not implemented.

  bool IsProcessing;
//ETX
};

#endif

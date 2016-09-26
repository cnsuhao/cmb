//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBBandedPolyDataContourFilter - generate filled contours for vtkPolyData
// .SECTION Description
// vtkCMBBandedPolyDataContourFilter is a filter that takes as input vtkPolyData
// and produces as output filled contours (also represented as vtkPolyData).
// Filled contours are bands of cells that all have the same cell scalar
// value, and can therefore be colored the same. The method is also referred
// to as filled contour generation.
//
// To use this filter you must specify one or more contour values.  You can
// either use the method SetValue() to specify each contour value, or use
// GenerateValues() to generate a series of evenly spaced contours.  Each
// contour value divides (or clips) the data into two pieces, values below
// the contour value, and values above it. The scalar values of each
// band correspond to the specified contour value.  Note that if the first and
// last contour values are not the minimum/maximum contour range, then two
// extra contour values are added corresponding to the minimum and maximum
// range values. These extra contour bands can be prevented from being output
// by turning clipping on.
//
// .SECTION See Also
// vtkClipDataSet vtkClipPolyData vtkClipVolume vtkContourFilter
//
#ifndef __vtkCMBBandedPolyDataContourFilter_h
#define __vtkCMBBandedPolyDataContourFilter_h

#include "vtkPolyDataAlgorithm.h"
#include "cmbSystemConfig.h"

#include "vtkContourValues.h" // Needed for inline methods

class vtkPoints;
class vtkCellArray;
class vtkPointData;
class vtkDataArray;
class vtkFloatArray;
class vtkDoubleArray;

#define VTK_SCALAR_MODE_INDEX 0
#define VTK_SCALAR_MODE_VALUE 1

class VTK_EXPORT vtkCMBBandedPolyDataContourFilter : public vtkPolyDataAlgorithm
{
public:
  vtkTypeMacro(vtkCMBBandedPolyDataContourFilter,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Values that OutputEdges may take on; these determine whether isocontours and mesh boundary edges are output.
  enum {
    NO_EDGES = 0,
    BAND_EDGES,
    BAND_AND_BOUNDARY_EDGES
  };

  // Description:
  // Construct object with no contours defined.
  static vtkCMBBandedPolyDataContourFilter *New();

  // Description:
  // Methods to set / get contour values. A single value at a time can be
  // set with SetValue(). Multiple contour values can be set with
  // GenerateValues(). Note that GenerateValues() generates n values
  // inclusive of the start and end range values.
  void SetValue(int i, double value);
  double GetValue(int i);
  double *GetValues();
  void GetValues(double *contourValues);
  void SetNumberOfContours(int number);
  int GetNumberOfContours();
  void GenerateValues(int numContours, double range[2]);
  void GenerateValues(int numContours, double rangeStart, double rangeEnd);

  // Description:
  // Indicate whether to clip outside the range specified by the user.
  // (The range is contour value[0] to contour value[numContours-1].)
  // Clipping means all cells outside of the range specified are not
  // sent to the output.
  vtkSetMacro(Clipping,int);
  vtkGetMacro(Clipping,int);
  vtkBooleanMacro(Clipping,int);

  // Description:
  // Control whether the cell scalars are output as an integer index or
  // a scalar value. If an index, the index refers to the bands produced
  // by the clipping range. If a value, then a scalar value which is a
  // value between clip values is used.
  vtkSetClampMacro(ScalarMode,int,VTK_SCALAR_MODE_INDEX,VTK_SCALAR_MODE_VALUE);
  vtkGetMacro(ScalarMode,int);
  void SetScalarModeToIndex()
    {this->SetScalarMode(VTK_SCALAR_MODE_INDEX);}
  void SetScalarModeToValue()
    {this->SetScalarMode(VTK_SCALAR_MODE_VALUE);}

  // Description:
  // Get/set the name of the output scalar array.
  // The default is a NULL pointer, which forces the output scalar array to the same name as the input array.
  vtkGetStringMacro(ScalarName);
  vtkSetStringMacro(ScalarName);

  // Description:
  // Turn on/off a flag to control which edges are generated in addition to polygons.
  // Contour edges are the edges between bands. If enabled, they are
  // generated from polygons/triangle strips and placed into the second
  // output (the ContourEdgesOutput). Boundary edges are edges of cells that are on the
  // boundary of the output polygonal bands.
  vtkSetClampMacro(OutputEdges,int,NO_EDGES,BAND_AND_BOUNDARY_EDGES);
  vtkGetMacro(OutputEdges,int);
  void SetOutputEdgesToNoEdges()
    { this->SetOutputEdges( NO_EDGES ); }
  void SetOutputEdgesToBandEdges()
    { this->SetOutputEdges( BAND_EDGES ); }
  void SetOutputEdgesToBandAndBoundaryEdges()
    { this->SetOutputEdges( BAND_AND_BOUNDARY_EDGES ); }

  // Description:
  // Get the second output which contains the edges dividing the contour
  // bands. This output is empty unless OutputEdges is enabled.
  vtkPolyData *GetContourEdgesOutput();

  // Description:
  // Overload GetMTime because we delegate to vtkContourValues so its
  // modified time must be taken into account.
  vtkMTimeType GetMTime() override;

protected:
  vtkCMBBandedPolyDataContourFilter();
  ~vtkCMBBandedPolyDataContourFilter() override;

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *) override;

  int ComputeScalarIndex(double);
  int IsContourValue(double val);
  int ClipEdge(int v1, int v2, vtkPoints *pts, vtkDataArray *inScalars,
               vtkDoubleArray *outScalars,
               vtkPointData *inPD, vtkPointData *outPD);
  int InsertCell(vtkCellArray *cells, int npts, vtkIdType *pts,
                 int cellId, double s, vtkFloatArray *newS);

  // data members
  vtkContourValues *ContourValues;

  int Clipping;
  int ScalarMode;

  // sorted and cleaned contour values
  double *ClipValues;
  int   NumberOfClipValues;
  int ClipIndex[2]; //indices outside of this range (inclusive) are clipped
  double ClipTolerance; //used to clean up numerical problems
  char* ScalarName; // if not empty, specifies the name to give the output scalar array.

  //the second output
  int OutputEdges;

private:
  vtkCMBBandedPolyDataContourFilter(const vtkCMBBandedPolyDataContourFilter&);  // Not implemented.
  void operator=(const vtkCMBBandedPolyDataContourFilter&);  // Not implemented.
};

// Description:
// Set a particular contour value at contour number i. The index i ranges
// between 0<=i<NumberOfContours.
inline void vtkCMBBandedPolyDataContourFilter::SetValue(int i, double value)
  {this->ContourValues->SetValue(i,value);}

// Description:
// Get the ith contour value.
inline double vtkCMBBandedPolyDataContourFilter::GetValue(int i)
  {return this->ContourValues->GetValue(i);}

// Description:
// Get a pointer to an array of contour values. There will be
// GetNumberOfContours() values in the list.
inline double *vtkCMBBandedPolyDataContourFilter::GetValues()
  {return this->ContourValues->GetValues();}

// Description:
// Fill a supplied list with contour values. There will be
// GetNumberOfContours() values in the list. Make sure you allocate
// enough memory to hold the list.
inline void vtkCMBBandedPolyDataContourFilter::GetValues(double *contourValues)
  {this->ContourValues->GetValues(contourValues);}

// Description:
// Set the number of contours to place into the list. You only really
// need to use this method to reduce list size. The method SetValue()
// will automatically increase list size as needed.
inline void vtkCMBBandedPolyDataContourFilter::SetNumberOfContours(int number)
  {this->ContourValues->SetNumberOfContours(number);}

// Description:
// Get the number of contours in the list of contour values.
inline int vtkCMBBandedPolyDataContourFilter::GetNumberOfContours()
  {return this->ContourValues->GetNumberOfContours();}

// Description:
// Generate numContours equally spaced contour values between specified
// range. Contour values will include min/max range values.
inline void vtkCMBBandedPolyDataContourFilter::GenerateValues(int numContours,
                                                           double range[2])
  {this->ContourValues->GenerateValues(numContours, range);}

// Description:
// Generate numContours equally spaced contour values between specified
// range. Contour values will include min/max range values.
inline void vtkCMBBandedPolyDataContourFilter::GenerateValues(int numContours,
                                                           double rangeStart,
                                                           double rangeEnd)
  {this->ContourValues->GenerateValues(numContours, rangeStart, rangeEnd);}


#endif

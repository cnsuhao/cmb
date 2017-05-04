//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkRawDEMReader - Reader for Raw DEM files
// .SECTION Description

#ifndef __RawDEMReader_h
#define __RawDEMReader_h

#include "cmbSystemConfig.h"
#include "vtkCMBIOModule.h" // For export macro
#include "vtkDataSetAlgorithm.h"
#include "vtkSmartPointer.h"

#include "vtkBoundingBox.h"
#include <string>

class vtkCellArray;
class vtkFloatArray;
class vtkPoints;
class vtkTransform;
class vtkGeoSphereTransform;
class vtkRawDEMReaderInternals;

#define VTK_ASCII 1
#define VTK_BINARY 2

//BTX
struct RawDEMReaderFileInfo
{
  std::string FileName;
  vtkIdType NumberOfColumns;
  vtkIdType NumberOfRows;
  double LongitudeOrigin;
  double LatitudeOrigin;
  double PointSpacing;
  float NoDataValue;
  vtkIdType Offset[2]; // column and row, from LL of all datasets
};
//ETX

class VTKCMBIO_EXPORT vtkRawDEMReader : public vtkDataSetAlgorithm
{
public:
  static vtkRawDEMReader* New();
  vtkTypeMacro(vtkRawDEMReader, vtkDataSetAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Name of the file to be read.
  void SetFileName(const char* filename);
  vtkGetStringMacro(FileName);

  // Description:
  // Get the total number of points in the file or -1 if the file hasn't
  // already been read (ReadFileInfo())
  vtkIdType GetTotalNumberOfPoints();

  // Description:
  // Get the total number of points in the file or -1 if the file hasn't
  // already been read (ReadFileInfo())
  vtkIdType GetRealNumberOfOutputPoints();

  // Description:
  // Support getting the (image) dimensions of the data given the current settings
  void GatherDimensions();
  vtkGetVector2Macro(Dimensions, vtkIdType);

  // Description:
  // The OnRatio for all pieces. Initialized to 1 (full resolution),
  // but will be updated with an estimated number based on the estimated
  // total number of points in the file, so that the first preview render
  // window is reasonably interactive, which may have 1 Million points [?].
  // This will ONLY be used if this->RequestedReadPieces is empty.
  vtkSetClampMacro(OnRatio, int, 1, VTK_INT_MAX);
  vtkGetMacro(OnRatio, int);

  // Description:
  // Boolean value indicates whether or not to limit points read to a specified
  // (ReadBounds) region.
  vtkBooleanMacro(LimitReadToBounds, bool);
  vtkSetMacro(LimitReadToBounds, bool);
  vtkGetMacro(LimitReadToBounds, bool);

  // Description:
  // Bounds to use if LimitReadToBounds is On
  vtkSetVector6Macro(ReadBounds, double);
  vtkGetVector6Macro(ReadBounds, double);

  // Description:
  // Row and Columnn Read Extents (disregarding OnRatio); if value is -1, then
  // corresponding min/max value is used.
  vtkSetVector2Macro(RowReadExtents, vtkIdType);
  vtkGetVector2Macro(RowReadExtents, vtkIdType);
  vtkSetVector2Macro(ColumnReadExtents, vtkIdType);
  vtkGetVector2Macro(ColumnReadExtents, vtkIdType);

  // Description:
  vtkSetVector2Macro(TransformForZOrigin, double);
  vtkGetVector2Macro(TransformForZOrigin, double);

  // Description
  // Retrieve bounds for the data in the file.  More specifically, gets
  // the bounds of data/pieces that have been read.
  vtkGetVector6Macro(DataBounds, double);

  // Description:
  // Transform to apply to the pts being read in for determining whether the
  // data is in/out of the ReadBounds (if LimitReadToBounds is true), or for
  // transforming data for the output (or both);  Note, the transform is
  // ignored if neither LimitReadToBounds nor TransformOutputData is true.
  void SetTransform(vtkTransform* transform);
  vtkGetObjectMacro(Transform, vtkTransform);
  void SetTransform(double elements[16]);
  void ClearTransform() { this->SetTransform(static_cast<vtkTransform*>(0)); }

  // Description:
  // Whether or not to transform the data by this->Transform for the output
  vtkBooleanMacro(TransformOutputData, bool);
  vtkSetMacro(TransformOutputData, bool);
  vtkGetMacro(TransformOutputData, bool);

  // Description:
  // Boolean value indicates whether or not to limit number of points read
  // based on MaxNumbeOfPoints.
  vtkBooleanMacro(LimitToMaxNumberOfPoints, bool);
  vtkSetMacro(LimitToMaxNumberOfPoints, bool);
  vtkGetMacro(LimitToMaxNumberOfPoints, bool);

  // Description:
  // The maximum number of points to load if LimitToMaxNumberOfPoints is on/true.
  // Sets a temporary onRatio.
  vtkSetClampMacro(MaxNumberOfPoints, vtkIdType, 1, VTK_INT_MAX);
  vtkGetMacro(MaxNumberOfPoints, vtkIdType);

  // Description:
  // Setting controls whether or not to convert from Lat/Long to x,y,z coordinates
  vtkBooleanMacro(ConvertFromLatLongToXYZ, bool);
  vtkSetMacro(ConvertFromLatLongToXYZ, bool);
  vtkGetMacro(ConvertFromLatLongToXYZ, bool);

  vtkSetVector3Macro(Origin, double);

  // Description:
  vtkBooleanMacro(TransformForZUp, bool);
  vtkSetMacro(TransformForZUp, bool);
  vtkGetMacro(TransformForZUp, bool);

  // Description:
  vtkBooleanMacro(RemoveCurvature, bool);
  vtkSetMacro(RemoveCurvature, bool);
  vtkGetMacro(RemoveCurvature, bool);

  vtkBooleanMacro(FileInfoMode, bool);
  vtkSetMacro(FileInfoMode, bool);
  vtkGetMacro(FileInfoMode, bool);

  vtkBooleanMacro(OutputImageData, bool);
  vtkSetMacro(OutputImageData, bool);
  vtkGetMacro(OutputImageData, bool);

  vtkBooleanMacro(ReadSetOfFiles, bool);
  vtkSetMacro(ReadSetOfFiles, bool);
  vtkGetMacro(ReadSetOfFiles, bool);

  // Description:
  // The output type defaults to float, but can instead be double.
  vtkBooleanMacro(OutputDataTypeIsDouble, bool);
  vtkSetMacro(OutputDataTypeIsDouble, bool);
  vtkGetMacro(OutputDataTypeIsDouble, bool);

  // Description:
  // The ZRotationAngle for aligning x-y to long-lat. Default is 0;
  vtkSetMacro(ZRotationAngle, double);
  vtkGetMacro(ZRotationAngle, double);

  //BTX

protected:
  vtkRawDEMReader();
  ~vtkRawDEMReader() override;

  int RequestInformation(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;
  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;
  int RequestDataObject(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;
  vtkImageData* AllocateOutputData(vtkDataObject* out);

  void ReadPolyDataOutput(vtkPolyData* output);

  void ReadImageDataOutput(vtkImageData* output);

  void ReadData(
    RawDEMReaderFileInfo& fileInfo, vtkFloatArray* scalars, vtkPoints* pts, vtkCellArray* verts);

  void SetupReadExtents();

  char* FileName;

  bool FileInfoMode;
  int OnRatio;

  bool LimitReadToBounds;
  double ReadBounds[6];
  double DataBounds[6];
  vtkBoundingBox ReadBBox;

  vtkIdType MaxNumberOfPoints;
  bool LimitToMaxNumberOfPoints;

  vtkIdType RealNumberOfOutputPoints;

  vtkTransform* Transform;

  bool ConvertFromLatLongToXYZ;
  bool TransformForZUp;
  bool RemoveCurvature;
  double TransformForZOrigin[2];
  double ZRotationAngle;

  vtkSmartPointer<vtkGeoSphereTransform> LatLongTransform1;
  vtkSmartPointer<vtkTransform> LatLongTransform2;

  bool TransformOutputData;
  bool OutputDataTypeIsDouble;

  bool OutputImageData;
  bool ReadSetOfFiles;

  vtkIdType RowReadExtents[2];
  vtkIdType ColumnReadExtents[2];
  vtkIdType Dimensions[2];

  vtkRawDEMReaderInternals* Internals;

  double Origin[3];

private:
  vtkRawDEMReader(const vtkRawDEMReader&); // Not implemented.
  void operator=(const vtkRawDEMReader&);  // Not implemented.
  //ETX
};

#endif

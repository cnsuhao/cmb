/*=========================================================================

Copyright (c) 1998-2012 Kitware Inc. 28 Corporate Drive,
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
// .NAME vtkLIDARMultiFilesReader - Reader for multiple LIDAR point files
// .SECTION Description
// Reader for binary and ascii LIDAR files.  If ascii format, the file MAY contain
// rgb information for each vertex.  The format, ascii or Binary, must be
// specified before reading the files.
//
// It is possible to only load every nth (OnRatio) point and also, individual pieces
// can be read and appended as a single dataset.

#ifndef __LIDARMultiFilesReader_h
#define __LIDARMultiFilesReader_h

#include "vtkCMBIOModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"
#include "vtkSmartPointer.h"
#include "cmbSystemConfig.h"

#include "vtkBoundingBox.h"
#include <vector>
#include <map>

class vtkTransform;
class vtkLIDARReader;

class VTKCMBIO_EXPORT vtkLIDARMultiFilesReader : public vtkPolyDataAlgorithm
{
public:
  static vtkLIDARMultiFilesReader *New();
  vtkTypeMacro(vtkLIDARMultiFilesReader,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Adds names of files to be read. The files are read in the order
  // they are added.
  virtual void AddFileName(const char* fname);

  // Description:
  // Remove all file names.
  virtual void RemoveAllFileNames();

  // Description:
  // Returns the number of file names added by AddFileName.
  virtual unsigned int GetNumberOfFileNames();

  // Description:
  // Returns the name of a file with index idx.
  virtual const char* GetFileName(unsigned int idx);

  // Description:
  // Name of the Current File being processed
  vtkSetStringMacro(CurrentFileName);
  vtkGetStringMacro(CurrentFileName);

  // Description:
  // Get the number of pieces in the dataset that we currently know about
  // (does NOT read the file)
  int GetKnownNumberOfPieces();
  int GetKnownNumberOfPieces(const char* filename);

  // Description:
  // Get the total number of points in the file or -1 if the file hasn't
  // already been read (ReadFilesInfo())
  vtkIdType GetTotalNumberOfPoints();
  vtkIdType GetTotalNumberOfPoints(const char* filename);

  vtkIdType GetRealNumberOfOutputPoints();
  vtkIdType GetRealNumberOfOutputPoints(const char* filename);

  // Description:
  // Set/Get the index of the desired piece of data
  //void SetPieceIndex(const char* filename, int pieceIndex);

  // Description:
  // Get the number of points in the requested piece.  Note, this returns ALL
  // the points in the piece AND the file info must already have been read.
  vtkIdType GetNumberOfPointsInPiece(const char* filename, int pieceIndex);

  // Description:
  // Add individual pieces and their OnRatio for reading. The pieces will
  // be read and appended together as a single dataset.
  // Index:   Add the index of the desired piece of data for reading
  // OnRatio: Perform vtkMaskPoints(like) operation as we read in the points.  By
  // default the OnRatio is 1, so we get every point, but can increase
  // such that points are skipped.
  void AddRequestedPieceForRead(const char* filename,
    int pieceIndex, int OnRatio);
  void RemoveAllRequestedReadPieces();

  // Description:
  // Read the number of pieces and points per piece.  Unfortunately, we
  // have to read though the whole file to get this information (it's not in a
  // header)
  int ReadFilesInfo();

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

  // Description
  // Retrieve bounds for the data in the file.  More specifically, gets
  // the bounds of data/pieces that have been read.
  vtkGetVector6Macro(DataBounds, double);

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
  vtkSetClampMacro(MaxNumberOfPoints,int,1,VTK_INT_MAX);
  vtkGetMacro(MaxNumberOfPoints,int);

  // Description:
  // The output type defaults to float, but can instead be double.
  vtkBooleanMacro(OutputDataTypeIsDouble, bool);
  vtkSetMacro(OutputDataTypeIsDouble, bool);
  vtkGetMacro(OutputDataTypeIsDouble, bool);

  // Description:
  // Setting controls whether or not to convert from Lat/Long to x,y,z coordinates
  void SetConvertFromLatLongToXYZ(const char* filename, bool mode);
  bool GetConvertFromLatLongToXYZ(const char* filename);
  void SetConvertFromLatLongToXYZToAll(bool mode);
  bool GetConvertFromLatLongToXYZ(); // Get the first one available

  // Description:
  // Transform to apply to the pts being read in for determining whether the
  // data is in/out of the ReadBounds (if LimitReadToBounds is true), or for
  // transforming data for the output (or both);  Note, the transform is
  // ignored if neither LimitReadToBounds nor TransformOutputData is true.
  void SetTransform(const char* filename,vtkTransform *transform);
  void SetTransform(const char* filename, double elements[16]);
  vtkTransform* GetTransform(const char* filename);
  void ClearTransform(const char* filename)
    { this->SetTransform(filename, static_cast<vtkTransform*>(0)); }

  // Description:
  // Set/get transform of all files being read.
  void SetTransformToAll(vtkTransform *transform);
  void SetTransformToAll(double elements[16]);
  vtkTransform* GetTransform(); // Get the first one available
  void ClearTransformToAll()
    {this->SetTransformToAll(static_cast<vtkTransform*>(0));}

//BTX
protected:
  vtkLIDARMultiFilesReader();
  ~vtkLIDARMultiFilesReader();
  void Initialize();
  void InitLIDARReader(vtkLIDARReader* lidarReader);

  int RequestInformation(vtkInformation *,
                         vtkInformationVector **,
                         vtkInformationVector *);
  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  vtkIdType GetEstimatedNumOfOutPoints();

  bool LimitReadToBounds;
  double ReadBounds[6];
  double DataBounds[6];

  int MaxNumberOfPoints;
  bool LimitToMaxNumberOfPoints;
  bool OutputDataTypeIsDouble;

  bool TransformOutputData;
  char* CurrentFileName;

private:
  vtkLIDARMultiFilesReader(const vtkLIDARMultiFilesReader&);  // Not implemented.
  void operator=(const vtkLIDARMultiFilesReader&);  // Not implemented.

  class InternalLIDARFileInfo;

  std::map<std::string, InternalLIDARFileInfo*> LIDARFiles;
  // <filename, pair<pieceIndex, onRatio> >
  std::map<std::string, std::vector< std::pair<int, int> > > RequestedReadFilePieces;
//ETX
};

#endif

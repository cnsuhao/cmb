//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBPt123Reader - "reader" for the pt123 formats
// .SECTION Description - pt123 outputs several files all listed in a
//   sup file. This reader reads the sup file and then reads the data
//   files listed in the sup file
#ifndef __vtkCMBPt123Reader__
#define __vtkCMBPt123Reader__

#include "vtkCMBIOModule.h" // For export macro
#include <vtkDoubleArray.h>
#include <vtkMultiBlockDataSetAlgorithm.h>
#include <vtkSmartPointer.h>
#include "cmbSystemConfig.h"

class Pt123TemporalData;
class vtkMultiBlockDataSet;
class vtkPolyData;

class VTKCMBIO_EXPORT vtkCMBPt123Reader : public vtkMultiBlockDataSetAlgorithm
{
  public:
    static vtkCMBPt123Reader* New();
    virtual void PrintSelf( ostream&os, vtkIndent indent );
    vtkTypeMacro(vtkCMBPt123Reader, vtkMultiBlockDataSetAlgorithm);

    vtkSetStringMacro(FileName);
    vtkGetStringMacro(FileName);

    vtkSetClampMacro(CacheSize,int,1,VTK_INT_MAX);

  protected:
    vtkCMBPt123Reader();
    virtual ~vtkCMBPt123Reader();

    void ResetAllData();

    //Reader functions for various file formats found in the output of pt123
    int ReadSUPFile(const char* filename);
    int ReadPts2File(vtkPolyData* polyData,const char* filename);
    int ScanTemporalData(Pt123TemporalData* dat, const char* filename,
        bool isAScalar);
    int UpdateTimeData(Pt123TemporalData* dat, double timeValue);
    int ReadBinaryStreams(vtkPolyData* polyData,const char* filename);
    vtkDoubleArray *GetDataAtTime(Pt123TemporalData *dat, double ts);

    int RequestInformation( vtkInformation*, vtkInformationVector**, vtkInformationVector* );
    int RequestData( vtkInformation*, vtkInformationVector**, vtkInformationVector* );

    char* FileName;
    char* OldFileName;
    vtkSetStringMacro(OldFileName);
    const char* FileNamePath;

    double *TimeSteps;
    double TimeStepRange[2];
    int NumberOfTimeSteps;
    int SpaceDimension;
    int TransientVelocity;
    int VelocityFormat;

    int CacheSize; //Tells how many ADHTemporalDatas can be stored at the same time

    //Geometry
    vtkMultiBlockDataSet* PrereadGeometry;
    //TimeData
    Pt123TemporalData* VelData;
    Pt123TemporalData* NemcData;

};

#endif //__vtkCMBPt123Reader__

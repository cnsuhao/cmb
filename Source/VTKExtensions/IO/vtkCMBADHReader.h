//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBADHReader
// .SECTION Description
// Reads in ADH data and applies it to a dataset

#ifndef _vtkCMBADHReader_h_
#define _vtkCMBADHReader_h_

#include "vtkCMBIOModule.h" // For export macro
#include "cmbSystemConfig.h"
#include <vtkFloatArray.h>
#include <vtkPointSetAlgorithm.h>
#include <vtkSmartPointer.h>

class ADHTemporalData;

class VTKCMBIO_EXPORT vtkCMBADHReader : public vtkPointSetAlgorithm
{
  public:
    static vtkCMBADHReader *New();
    vtkTypeMacro(vtkCMBADHReader,vtkPointSetAlgorithm );

    // Description:
    // Name of the file to be read.
    vtkSetStringMacro(FileName);
    vtkGetStringMacro(FileName);

    // Description:
    // Prefix string to be prepended to filename to name the data array.
    vtkSetStringMacro(Prefix);
    vtkGetStringMacro(Prefix);

    // Description:
    // Suffix string to be prepended to filename to name the data array.
    vtkSetStringMacro(Suffix);
    vtkGetStringMacro(Suffix);

    vtkSetClampMacro(CacheSize,int,1,VTK_INT_MAX);
    vtkSetMacro(PrimaryDataSet,int);
  protected:
    vtkCMBADHReader();
    ~vtkCMBADHReader() override;

    int ReadTemporalData();

    //This is here because of a bug in vtkPointSetAlgorithm
    //It can be removed whenever the ExecuteInformation function in
    //vtkPointSetAlgorithm is changed to RequestInformation
    int ExecuteInformation(vtkInformation*,
        vtkInformationVector**,
        vtkInformationVector*) override;
    int RequestInformation(vtkInformation *,
        vtkInformationVector **,
        vtkInformationVector *);
    int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *) override;

    char *FileName;
    char *OldFileName;
    char *Suffix;
    char *OldSuffix;
    char *Prefix;
    char *OldPrefix;
    vtkSetStringMacro(OldFileName);
    vtkSetStringMacro(OldSuffix);
    vtkSetStringMacro(OldPrefix);

    double TimeValue; //Which time value we are on
    int DataSet; //Which dataset currently in use by the program. Not set by user
    int PrimaryDataSet; //Which dataset we use for time values. Set by user
    int CacheSize; //Tells how many ADHTemporalDatas can be stored at the same time

    int GetNumberOfTimeSteps();
    double* GetTimeStepRange();

    //BTX
    std::vector<ADHTemporalData*> DataSets;
    //ETX

  private:
    int ScanFile();
    vtkCMBADHReader(const vtkCMBADHReader&);  // Not implemented.
    void operator=(const vtkCMBADHReader&);  // Not implemented.
};

#endif

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBSceneV2WriterHelper - writes the contents of a string into a file
// .SECTION Description

#ifndef __CmbSceneV2WriterHelper_h
#define __CmbSceneV2WriterHelper_h

#include "cmbSystemConfig.h"
#include "vtkCMBIOModule.h" // For export macro
#include "vtkPolyDataAlgorithm.h"
#include <map>
#include <string>
#include <vector>

class vtkMultiBlockDataSet;
class vtkPolyData;

class VTKCMBIO_EXPORT vtkCMBSceneV2WriterHelper : public vtkPolyDataAlgorithm
{
public:
  static vtkCMBSceneV2WriterHelper* New();
  vtkTypeMacro(vtkCMBSceneV2WriterHelper, vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  // Name of the file to write.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  // Description:
  // Set/Get the text to be written in the file.
  vtkSetStringMacro(Text);
  vtkGetStringMacro(Text);

  // Description:
  // Set/Get the mode to operate in
  vtkSetMacro(Mode, int);
  vtkGetMacro(Mode, int);
  vtkBooleanMacro(Mode, int);

  // Description:
  // Set/Get the mode to operate in
  vtkSetMacro(PackageScene, int);
  vtkGetMacro(PackageScene, int);
  vtkBooleanMacro(PackageScene, int);

  // Description:
  // There may be one mode file (usually for actual modes) or multiple mode
  // files (which usually actually represent time series).  These methods
  // set and clear the list of mode files (which can be a single mode file).
  virtual void AddObjectFileName(const char* fname);
  virtual void RemoveAllObjectFileNames();
  virtual unsigned int GetNumberOfObjectFileNames();
  virtual const char* GetObjectFileName(unsigned int idx);

protected:
  vtkCMBSceneV2WriterHelper();
  ~vtkCMBSceneV2WriterHelper() override;

  int RequestInformation(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;
  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

  void ProcessFileNames();

  char* FileName;
  char* Text;
  int Mode;
  int PackageScene;

  class vtkSceneGenInternal;
  vtkCMBSceneV2WriterHelper::vtkSceneGenInternal* Internal;

private:
  vtkCMBSceneV2WriterHelper(const vtkCMBSceneV2WriterHelper&); // Not implemented.
  void operator=(const vtkCMBSceneV2WriterHelper&);            // Not implemented.
};

#endif

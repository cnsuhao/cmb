//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkHydroModelSelectionSource - "Dummy" source so we can treat data as a source
// .SECTION Description
// The input Source data is shallow copied to the output

#ifndef __vtkHydroModelSelectionSource_h
#define __vtkHydroModelSelectionSource_h

#include "cmbSystemConfig.h"
#include "vtkCMBGeneralModule.h" // For export macro
#include "vtkSelectionAlgorithm.h"
class vtkSelectionSource;

class VTKCMBGENERAL_EXPORT vtkHydroModelSelectionSource : public vtkSelectionAlgorithm
{
public:
  static vtkHydroModelSelectionSource* New();
  vtkTypeMacro(vtkHydroModelSelectionSource, vtkSelectionAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // Description:
  void CopyData(vtkSelection* selection);
  void CopyData(vtkAlgorithm* algOut);
  vtkGetObjectMacro(Selection, vtkSelection);

  // Description:
  // Add a (piece, id) to the selection set. The source will generate
  // only the ids for which piece == UPDATE_PIECE_NUMBER.
  // If piece == -1, the id applies to all pieces.
  void AddID(vtkIdType piece, vtkIdType id);
  void RemoveAllIDs();
  void RemoveAllIDsInternal();

  // Description:
  // Get the output selection field type
  // default: 0
  int GetSelectionFieldType();

  // Description:
  // Invert the selection.
  void InvertSelection(int insideOut);

  //BTX
protected:
  vtkHydroModelSelectionSource();
  ~vtkHydroModelSelectionSource() override;

  int RequestData(vtkInformation*, vtkInformationVector**, vtkInformationVector*) override;

  vtkSelectionSource* Source;
  vtkSelection* Selection;
  int InsideOut;

private:
  vtkHydroModelSelectionSource(const vtkHydroModelSelectionSource&); // Not implemented.
  void operator=(const vtkHydroModelSelectionSource&);               // Not implemented.

  class vtkInternal;
  vtkInternal* Internal;
  //ETX
};

#endif

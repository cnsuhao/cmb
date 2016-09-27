//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkSGXMLBCSWriter - outputs a BCS file in XML format
// .SECTION Description
// Filter to output an XML file with the mesh edges, model edge id
// they belong to and the BCSs defined over the model edge. This
// filter takes in an array of points and an array that contains
// the ids of which points are model vertices.

#ifndef __vtkSGXMLBCSWriter_h
#define __vtkSGXMLBCSWriter_h

#include "vtkCMBIOModule.h" // For export macro
#include "vtkXMLWriter.h"
#include "cmbSystemConfig.h"

class vtkIdTypeArray;
class vtkDoubleArray;
class vtkXMLDataElement;

// cannot derive from vtkXMLUnstructuredDataWriter unless the
// input is a subclass of vtkPointSet
class VTKCMBIO_EXPORT vtkSGXMLBCSWriter : public vtkXMLWriter
{
public:
  static vtkSGXMLBCSWriter *New();
  vtkTypeMacro(vtkSGXMLBCSWriter,vtkXMLWriter);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  const char* GetDefaultFileExtension() override;

  // Description:
  // Methods to define the file's major and minor version numbers.
  int GetDataSetMajorVersion() override;
  int GetDataSetMinorVersion() override;

//BTX
  void SetCoords(vtkDoubleArray*);
  vtkGetMacro(Coords, vtkDoubleArray*);

  void SetModelVertexIds(vtkIdTypeArray*);
  vtkGetMacro(ModelVertexIds, vtkIdTypeArray*);
//ETX

protected:
  vtkSGXMLBCSWriter();
  ~vtkSGXMLBCSWriter() override;

  int WriteData() override;
  const char* GetDataSetName() override;

private:
  vtkSGXMLBCSWriter(const vtkSGXMLBCSWriter&);  // Not implemented.
  void operator=(const vtkSGXMLBCSWriter&);  // Not implemented.

  // Description:
  // The coordinates of the points from the contour widget.  They are
  // assumed to be ordered in a single loop such that point N is adjacent to
  // edge N-1 and edge N.
  vtkDoubleArray* Coords;

  // Description:
  // An array of point Ids that have are also model vertices.
  vtkIdTypeArray* ModelVertexIds;
};

#endif

/*=========================================================================

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
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
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual const char* GetDefaultFileExtension();

  // Description:
  // Methods to define the file's major and minor version numbers.
  virtual int GetDataSetMajorVersion();
  virtual int GetDataSetMinorVersion();

//BTX
  void SetCoords(vtkDoubleArray*);
  vtkGetMacro(Coords, vtkDoubleArray*);

  void SetModelVertexIds(vtkIdTypeArray*);
  vtkGetMacro(ModelVertexIds, vtkIdTypeArray*);
//ETX

protected:
  vtkSGXMLBCSWriter();
  ~vtkSGXMLBCSWriter();

  virtual int WriteData();
  const char* GetDataSetName();

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

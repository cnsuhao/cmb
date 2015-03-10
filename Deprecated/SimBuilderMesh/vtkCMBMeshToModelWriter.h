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
// .NAME vtkCMBMeshToModelWriter - outputs a m2m file in XML format
// .SECTION Description
// Filter to output an XML file with mapping from analysis mesh info to
// model topology.

#ifndef __vtkCMBMeshToModelWriter_h
#define __vtkCMBMeshToModelWriter_h

#include "vtkXMLWriter.h"
#include "cmbSystemConfig.h"

class vtkDiscreteModelWrapper;

class VTK_EXPORT vtkCMBMeshToModelWriter : public vtkXMLWriter
{
public:
  static vtkCMBMeshToModelWriter *New();
  vtkTypeMacro(vtkCMBMeshToModelWriter,vtkXMLWriter);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual const char* GetDefaultFileExtension();

  // Description:
  // Methods to define the file's major and minor version numbers.
  virtual int GetDataSetMajorVersion();
  virtual int GetDataSetMinorVersion();

  // Description:
  // Set/get functions for the ModelWrapper.
  vtkGetMacro(ModelWrapper, vtkDiscreteModelWrapper*);
  void SetModelWrapper(vtkDiscreteModelWrapper* Wrapper);

protected:
  vtkCMBMeshToModelWriter();
  ~vtkCMBMeshToModelWriter();

  virtual int WriteData();
  virtual int WriteHeader(vtkIndent* parentindent);
  virtual int Write3DModelMeshInfo(vtkIndent* indent);
  virtual int Write2DModelMeshInfo(vtkIndent* indent);
  virtual int WriteFooter(vtkIndent* parentindent);

  const char* GetDataSetName();

private:
  vtkCMBMeshToModelWriter(const vtkCMBMeshToModelWriter&);  // Not implemented.
  void operator=(const vtkCMBMeshToModelWriter&);  // Not implemented.

  // Description:
  // The vtkDiscreteModelWrapper for the algorithm to extract the model
  // information from.
  vtkDiscreteModelWrapper* ModelWrapper;

};

#endif

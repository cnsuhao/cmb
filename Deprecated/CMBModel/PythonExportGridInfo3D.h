/*=========================================================================

Copyright (c) 2013 Kitware Inc. 28 Corporate Drive,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced, distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
=========================================================================*/
// .NAME PythonExportGridInfo3D.h - 3D grid representation for smtk interface
// .SECTION Description
// .SECTION See Also

#ifndef __PythonExportGridInfo3D_h
#define __PythonExportGridInfo3D_h

#include "PythonExportGridInfo.h"

class vtkDiscreteModel;


class PythonExportGridInfo3D : public PythonExportGridInfo
{
  typedef smtk::model::GridInfo::ApiStatus ApiStatus;

 public:
  /// Constructor & destructor
  PythonExportGridInfo3D(vtkDiscreteModel *model);
  virtual ~PythonExportGridInfo3D();

  /// Returns analysis grid cells for specified model entity.
  virtual std::vector<int>
  analysisGridCells(int modelEntityId, ApiStatus& status);

  /// Returns "grid items" for the geometry on the boundary of a model entity.
  virtual std::vector<std::pair<int, int> >
  boundaryItemsOf(int modelEntityId, ApiStatus& status);

  /// Returns "grid items" for the geometry of a model entity that is on
  //  the boundary of a next-higher-dimension model entity.
  virtual std::vector<std::pair<int, int> >
  asBoundaryItems(int modelEntityId,
                  int boundedModelEntityId,
                  ApiStatus& status);

protected:
  virtual std::string getClassName() const;  // for status messages
};

#endif  /* __PythonExportGridInfo3D_h */
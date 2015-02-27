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
#include "vtkIdentifyNonManifoldPts.h"

#include "vtkCellArray.h"
#include "vtkCleanPolyData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkMath.h"
#include "vtkNew.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include "vtkStripper.h"

vtkStandardNewMacro(vtkIdentifyNonManifoldPts);

//----------------------------------------------------------------------------
int vtkIdentifyNonManifoldPts::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and output
  vtkPolyData *input = vtkPolyData::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkSmartPointer<vtkPoints> outPts = vtkSmartPointer<vtkPoints>::New();
  output->SetPoints(outPts);
  vtkSmartPointer<vtkCellArray> outVerts = vtkSmartPointer<vtkCellArray>::New();
  output->SetVerts(outVerts);

  // get rid of any duplicated points
  vtkNew<vtkCleanPolyData> cleaner;
  cleaner->SetInputData( input );
  cleaner->Update();

  vtkSmartPointer<vtkPolyData> tmpPD = vtkSmartPointer<vtkPolyData>::New();
  tmpPD->SetLines( cleaner->GetOutput()->GetLines() );
  tmpPD->SetPoints( cleaner->GetOutput()->GetPoints() );
  tmpPD->BuildLinks();
  vtkPoints *pts = tmpPD->GetPoints();
  unsigned short ncells;
  vtkIdType* cells;
  for (vtkIdType i = 0; i < pts->GetNumberOfPoints(); i++)
    {
    tmpPD->GetPointCells(i, ncells, cells);
    if (ncells > 2)
      {
      vtkIdType nextPt = outPts->InsertNextPoint(pts->GetPoint(i));
      outVerts->InsertNextCell(1, &nextPt);
      }
    }
  return 1;
}

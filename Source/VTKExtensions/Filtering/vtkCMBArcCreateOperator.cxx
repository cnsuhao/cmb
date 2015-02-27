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

#include "vtkCMBArcCreateOperator.h"

#include "vtkCMBArcManager.h"
#include "vtkCMBArcEndNode.h"
#include "vtkCMBArc.h"

#include "vtkCellArray.h"
#include "vtkIdTypeArray.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include "vtkObjectFactory.h"

vtkStandardNewMacro(vtkCMBArcCreateOperator);

//----------------------------------------------------------------------------
vtkCMBArcCreateOperator::vtkCMBArcCreateOperator()
{
  this->CreatedArcId = -1;
}

//----------------------------------------------------------------------------
vtkCMBArcCreateOperator::~vtkCMBArcCreateOperator()
{

}

//----------------------------------------------------------------------------
bool vtkCMBArcCreateOperator::Operate(vtkPolyData *source)
{
  if (source == NULL)
    {
    return false;
    }

  //we need at least two point to create a valid arc
  if(source->GetNumberOfPoints() < 2 ||
     source->GetNumberOfLines() == 0)
    {
    return false;
    }
  //create the new arc we are going to add all the info too
  vtkCMBArc* createdArc = vtkCMBArc::New();
  createdArc->ClearPoints();

  this->CreatedArcId = createdArc->GetId();

  //variables for polydata to arc conversion
  double pos[3];
  vtkCellArray* lines = source->GetLines();
  vtkSmartPointer<vtkIdList> ids = vtkSmartPointer<vtkIdList>::New();

  vtkIdType currentIndex = 0;
  vtkIdType numberOfCells = lines->GetNumberOfCells();
  vtkIdType numberOfInternalEndNodes = lines->GetNumberOfConnectivityEntries() - numberOfCells;

  //walk the polydata and create the arc
  lines->InitTraversal();
  while (lines->GetNextCell( ids ))
    {
    //for each cell we convert the ids from the old set, to the new id's
    for ( vtkIdType i=0; i < ids->GetNumberOfIds(); ++i)
      {
      source->GetPoint(ids->GetId(i),pos);
      if (currentIndex != 0 && currentIndex != numberOfInternalEndNodes-1)
        {
        createdArc->InsertNextPoint(pos);
        }
      else if (currentIndex == 0)
        {
        createdArc->SetEndNode(0,pos);
        }
      else
        {
        createdArc->SetEndNode(1,pos);
        }
      ++currentIndex;
      }
    //clear the id structures
    ids->Reset();
    }

  return true;
  }

//----------------------------------------------------------------------------
void vtkCMBArcCreateOperator::PrintSelf(ostream& os, vtkIndent indent)
  {
  this->Superclass::PrintSelf(os,indent);
}

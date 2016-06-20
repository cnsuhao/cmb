//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkCMBArcCreateOperator.h"

#include "vtkCMBArcManager.h"
#include "vtkCMBArcEndNode.h"
#include "vtkCMBArc.h"

#include "vtkCellArray.h"
#include "vtkIdTypeArray.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"

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
  vtkDataArray * vda = source->GetPointData()->GetScalars();

  //walk the polydata and create the arc
  lines->InitTraversal();
  while (lines->GetNextCell( ids ))
    {
    //for each cell we convert the ids from the old set, to the new id's
    for ( vtkIdType i=0; i < ids->GetNumberOfIds(); ++i)
      {
      source->GetPoint(ids->GetId(i),pos);
      unsigned int pointID = i;
      if(vda != NULL)
        {
        pointID = vda->GetTuple1(ids->GetId(i));
        }
      vtkCMBArc::Point tmpPt(pos, pointID);
      if (currentIndex != 0 && currentIndex != numberOfInternalEndNodes-1)
        {
        createdArc->InsertNextPoint(tmpPt);
        }
      else if (currentIndex == 0)
        {
        createdArc->SetEndNode(0, tmpPt);
        }
      else
        {
        createdArc->SetEndNode(1, tmpPt);
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

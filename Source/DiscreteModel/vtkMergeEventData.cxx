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

#include "vtkMergeEventData.h"

#include "vtkIdTypeArray.h"
#include "vtkObjectFactory.h"

vtkStandardNewMacro(vtkMergeEventData);
vtkCxxSetObjectMacro(vtkMergeEventData, LowerDimensionalIds, vtkIdTypeArray);

vtkMergeEventData::vtkMergeEventData()
{
  this->SourceEntity = NULL;
  this->TargetEntity = NULL;
  this->LowerDimensionalIds = NULL;
}

vtkMergeEventData::~vtkMergeEventData()
{
  // SourceEntity and TargetEntity don't strictly need to be set to NULL
  // now but that may change in the future
  this->SetSourceEntity(0);
  this->SetTargetEntity(0);
  this->SetLowerDimensionalIds(0);
}


void vtkMergeEventData::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "SourceEntity: ";
  if(this->SourceEntity)
    {
    os << this->SourceEntity << endl;
    }
  else
    {
    os << "(NULL)\n";
    }
  os << indent << "TargetEntity: ";
  if(this->TargetEntity)
    {
    os << this->TargetEntity << endl;
    }
  else
    {
    os << "(NULL)\n";
    }
  os << indent << "LowerDimensionalIds: ";
  if(this->LowerDimensionalIds)
    {
    os << this->LowerDimensionalIds << endl;
    }
  else
    {
    os << "(NULL)\n";
    }
}

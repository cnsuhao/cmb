//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkCMBArcSnapClientOperator.h"


#include "vtkObjectFactory.h"
#include "vtkCMBArcManager.h"

vtkStandardNewMacro(vtkCMBArcSnapClientOperator);

//---------------------------------------------------------------------------
vtkCMBArcSnapClientOperator::vtkCMBArcSnapClientOperator()
{
}

//---------------------------------------------------------------------------
vtkCMBArcSnapClientOperator::~vtkCMBArcSnapClientOperator()
{
}

//----------------------------------------------------------------------------
double vtkCMBArcSnapClientOperator::GetCurrentRadius( )
{
  vtkCMBArcManager *manager = vtkCMBArcManager::GetInstance();
  if (manager->GetUseSnapping())
    {
    return manager->GetSnapRadius();
    }
  return 0.0;
}

//----------------------------------------------------------------------------
bool vtkCMBArcSnapClientOperator::Operate(const double& radius)
{
  vtkCMBArcManager *manager = vtkCMBArcManager::GetInstance();
  double r = radius <= 0.0 ? 0.0 : radius;
  manager->SetSnapRadius(r);
  manager->SetUseSnapping(r != 0.0);
  return true;
}

//----------------------------------------------------------------------------
void vtkCMBArcSnapClientOperator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

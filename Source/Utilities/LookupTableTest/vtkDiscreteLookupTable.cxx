/*=========================================================================

  Program:   ParaView
  Module:    vtkDiscreteLookupTable.cxx

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkDiscreteLookupTable.h"

#include "vtkObjectFactory.h"
#include "vtkLookupTable.h"
#include "vtkMath.h"

#include <vector>

#define TABLE_CIRCLE_DEGREE 800

vtkStandardNewMacro(vtkDiscreteLookupTable);
//-----------------------------------------------------------------------------
vtkDiscreteLookupTable::vtkDiscreteLookupTable()
{
  this->Discretize = 1;
  this->NumberOfValues = 256;
  this->SetColorSpaceToHSV();

  this->HueDelta = 101.0; // degree in 360 degree circle.

  this->SaturationMax = 0.99;
  this->SaturationMin = 0.2;
  this->SaturationDelta = 0.2;

  this->ValueMax = 0.99;
  this->ValueMin = 0.2;
    this->ValueDelta = 0.2;
}

//-----------------------------------------------------------------------------
vtkDiscreteLookupTable::~vtkDiscreteLookupTable()
{
}

//-----------------------------------------------------------------------------
void vtkDiscreteLookupTable::Build()
{
  // skip the vtkPVLookupTable::Build()
  this->vtkColorTransferFunction::Build();

  this->LookupTable->SetVectorMode(this->VectorMode);
  this->LookupTable->SetVectorComponent(this->VectorComponent);

  if (this->Discretize && (this->GetMTime() > this->BuildTime ||
    this->GetMTime() > this->BuildTime))
    {
    unsigned char* lut_ptr = this->LookupTable->WritePointer(0,
      this->NumberOfValues);
    double* table = new double[this->NumberOfValues*3];

    int numColors = 0;
    double h=0.0, s = this->SaturationMax, v = this->ValueMax;
    double deltaH = this->HueDelta/TABLE_CIRCLE_DEGREE;
    while(numColors < this->NumberOfValues)
      {
      this->AddHSVPoint(numColors, h, s, v);
      h += deltaH;
      s = h>1.0 ? s-this->SaturationDelta : s;
      s = s<this->SaturationMin ? this->SaturationMax : s;

      v = h>1.0 ? v-this->ValueDelta : v;
      v = v<this->ValueMin ? this->ValueMax : s;

      h = h>1.0 ? h-1.0 : h;
      numColors++;
      }

    double range[2];
    this->GetRange(range);
    bool logRangeValid = true;
    if(this->UseLogScale)
      {
      logRangeValid = range[0] > 0.0 || range[1] < 0.0;
      if(!logRangeValid && this->LookupTable->GetScale() == VTK_SCALE_LOG10)
        {
        this->LookupTable->SetScaleToLinear();
        }
      }

    this->LookupTable->SetRange(range);
    if(this->UseLogScale && logRangeValid &&
        this->LookupTable->GetScale() == VTK_SCALE_LINEAR)
      {
      this->LookupTable->SetScaleToLog10();
      }

    //this->SetColorSpaceToHSV();
    //this->AddHSVPoint(range[0], 0.66, 1.0, 1.0);
    //this->AddHSVPoint((range[0] + range[1]) * 0.5, 0.0, 1.0, 1.0);
    //this->AddHSVPoint(range[1], 0.33, 1.0, 1.0);

    this->GetTable(range[0], range[1], this->NumberOfValues, table);
    // Now, convert double to unsigned chars and fill the LUT.
    for (int cc=0; cc < this->NumberOfValues; cc++)
      {
      lut_ptr[4*cc]   = static_cast<unsigned char>(255.0*table[3*cc] + 0.5);
      lut_ptr[4*cc+1] = static_cast<unsigned char>(255.0*table[3*cc+1] + 0.5);
      lut_ptr[4*cc+2] = static_cast<unsigned char>(255.0*table[3*cc+2] + 0.5);
      lut_ptr[4*cc+3] = 255;
      }
    delete [] table;

    this->BuildTime.Modified();
    }
}

//-----------------------------------------------------------------------------
void vtkDiscreteLookupTable::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "HueDelta: " << this->HueDelta << endl;
  os << indent << "SaturationDelta: " << this->SaturationDelta << endl;
  os << indent << "ValueDelta: " << this->ValueDelta << endl;
  os << indent << "SaturationMin: " << this->SaturationMin << endl;
  os << indent << "SaturationMax: " << this->SaturationMax << endl;
  os << indent << "ValueMin: " << this->ValueMin << endl;
  os << indent << "ValueMax: " << this->ValueMax << endl;
}

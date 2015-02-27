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

#include "vtkDoubleArray.h"
#include "vtkObjectFactory.h"
#include "vtkLookupTable.h"
#include "vtkMath.h"
#include "vtkUnsignedCharArray.h"

#include <vector>
#include <string>

#define TABLE_CIRCLE_DEGREE 800

vtkStandardNewMacro(vtkDiscreteLookupTable);
//-----------------------------------------------------------------------------
vtkDiscreteLookupTable::vtkDiscreteLookupTable()
{
  this->Discretize = 1;
  this->NumberOfValues = 256;

  this->ValueMax = 0.9;
  this->ValueMin = 0.2;

}

//-----------------------------------------------------------------------------
vtkDiscreteLookupTable::~vtkDiscreteLookupTable()
{
}

//-----------------------------------------------------------------------------
void vtkDiscreteLookupTable::Build()
{
  // skip the vtkDiscretizableColorTransferFunction::Build()
  //this->vtkColorTransferFunction::Build();

  this->LookupTable->SetVectorMode(this->VectorMode);
  this->LookupTable->SetVectorComponent(this->VectorComponent);

  if (this->Discretize && (this->GetMTime() > this->BuildTime ||
    this->GetMTime() > this->BuildTime))
    {
    this->LookupTable->SetNumberOfTableValues(this->NumberOfValues);
  //this->LookupTable->WritePointer(0,
  //      this->NumberOfValues);
    this->LookupTable->SetRange(0, this->NumberOfValues-1);
    this->LookupTable->Build();
    this->CreateLookupTable(
      this->LookupTable, this->ValueMin, this->ValueMax);

    this->BuildTime.Modified();
    }
}

//----------------------------------------------------------------------------
void vtkDiscreteLookupTable::CalcRGB(
  double llimit, double ulimit, double &r, double &g, double &b)
{
  double l;
  while(1)
    {
    r = vtkMath::Random(0.0, 1.0);
    g = vtkMath::Random(0.0, 1.0);
    b = vtkMath::Random(0.0, 1.0);

    l = (0.11 *b) + (0.3 *r) + (0.59*g);
    if ((l >= llimit) && ( l <= ulimit))
      {
      return;
      }
    }
}

//----------------------------------------------------------------------------
vtkIdType vtkDiscreteLookupTable::GetNextIndex(
  vtkIdType i, vtkUnsignedCharArray *avail)
{
  if (!avail->GetValue(i))
    {
    vtkIdType orig = i;
    vtkIdType n = avail->GetNumberOfTuples();
    //cout << "N = " << n << "\n";
    for (; i < n; i++)
      {
      if (avail->GetValue(i))
        {
        break;
        }
      }
    if (i == n)
      {
      for (i = 0; i < orig; i++)
        {
        if (avail->GetValue(i))
          {
          break;
          }
        }
      if (!avail->GetValue(i))
        {
        return -1;
        }
      }
    }
  avail->SetTuple1(i, 0);
  return i;
}

//----------------------------------------------------------------------------
void vtkDiscreteLookupTable::CreateLookupTable(
  vtkLookupTable *lut, double llimit, double ulimit)
{
  vtkIdType n = lut->GetNumberOfTableValues();
  vtkIdType i;
  double r, g, b;
  for (i = 0; i < n; i++)
    {
    CalcRGB(llimit, ulimit, r, g, b);
 //   cout << "Color: " << i << " : ( " << r << ", " << g << ", " << b << "), l = "
 //        << ( (0.11 *b) + (0.3 *r) + (0.59*g)) << "\n";
    lut->SetTableValue(i, r, g, b);
    }
}

//----------------------------------------------------------------------------
void vtkDiscreteLookupTable::CreateLookupTable(vtkLookupTable *lut)
{
  vtkIdType n = lut->GetNumberOfTableValues();
  vtkIdType i;

  int m = vtkMath::Round(pow(static_cast<double>(n), 1.0/3.0)+0.5);
  vtkDoubleArray* colors = vtkDoubleArray::New();
  colors->SetNumberOfComponents(3);
  colors->SetNumberOfTuples(m*m*m);
  vtkUnsignedCharArray* avail = vtkUnsignedCharArray::New();
  avail->SetNumberOfComponents(1);
  avail->SetNumberOfTuples(m*m*m);
  double color[3];
  double delta, r, g, b;
  int  x, y, z;
  delta = 1.0 / (static_cast<double>(m-1));
  r = 0.0;
  for (x = 0, i = 0; (x < m); x++, r+= delta)
    {
    g = 0.0;
    for (y = 0; y < m; y++, g+= delta)
      {
      b = 0.0;
      for (z = 0; z < m; z++, b+= delta, i++)
        {
        colors->SetTuple3(i,r,g,b);
        avail->SetTuple1(i,1);
 //       cout << i << " (" << r <<"," << g << "," << b << ")\n";
        }
      }
    }


//   for (i = 0; i < n; i++)
//     {
//     colors->GetTupleValue(i, color);
//     lut->SetTableValue(i, color[0], color[1], color[2]);
//     }

  vtkIdType ci, tci;
  for (i = 0; i < n; i++)
    {
    ci = vtkMath::Round(vtkMath::Random(0, n-1));
    tci = GetNextIndex(ci, avail);
 //   cout << "Mapping: " << i << ", " << ci << ", " << tci << "\n";
    colors->GetTupleValue(tci, color);
    lut->SetTableValue(i, color[0], color[1], color[2]);
    }

  colors->Delete();
  avail->Delete();
}

//-----------------------------------------------------------------------------
void vtkDiscreteLookupTable::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "ValueMin: " << this->ValueMin << endl;
  os << indent << "ValueMax: " << this->ValueMax << endl;
}

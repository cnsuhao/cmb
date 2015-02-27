/*=========================================================================

  Program:   ParaView
  Module:    vtkModelFaceActor.cxx

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkModelFaceActor.h"
#include "vtkObjectFactory.h"
//#include "vtkMultiBlockWrapper.h"
#include "vtkMapper.h"
#include "vtkDataSet.h"
#include "vtkIntArray.h"
#include "vtkScalarsToColors.h"
#include "vtkFieldData.h"
#include "vtkProperty.h"

#include <math.h>

//-----------------------------------------------------------------------------
vtkStandardNewMacro(vtkModelFaceActor);

//----------------------------------------------------------------------------
vtkModelFaceActor::vtkModelFaceActor()
{
  this->ModelFaceColorMode = 0;
  this->EnableLOD = 0;
  this->UseBackface = false;
  this->BackfaceColor[0] = this->BackfaceColor[1] = this->BackfaceColor[2] =0;
}

//----------------------------------------------------------------------------
vtkModelFaceActor::~vtkModelFaceActor()
{
}

//----------------------------------------------------------------------------
void vtkModelFaceActor::SetBackfaceColor(double *color)
{
  if (color[0] == this->BackfaceColor[0] &&
      color[1] == this->BackfaceColor[1] &&
      color[2] == this->BackfaceColor[2])
    {
    return;
    }
  memcpy(this->BackfaceColor, color, 3 * sizeof(double));
  if (this->BackfaceProperty)
    {
    this->BackfaceProperty->SetAmbientColor(this->BackfaceColor);
    this->BackfaceProperty->SetDiffuseColor(this->BackfaceColor);
    this->BackfaceProperty->SetColor(this->BackfaceColor);
    }
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkModelFaceActor::SetUseBackface(bool useBackface)
{
  if (useBackface == this->UseBackface)
    {
    return;
    }
  this->UseBackface = useBackface;
  if (this->BackfaceProperty && !useBackface)
    {
    this->SetBackfaceProperty(0);
    }
  else if (!this->BackfaceProperty && useBackface)
    {
    vtkProperty *temp = vtkProperty::New();
    this->SetBackfaceProperty( temp );
    temp->Delete();
    this->BackfaceProperty->SetAmbientColor(this->BackfaceColor);
    this->BackfaceProperty->SetDiffuseColor(this->BackfaceColor);
    this->BackfaceProperty->SetColor(this->BackfaceColor);
    }
}

//----------------------------------------------------------------------------
void vtkModelFaceActor::Render(vtkRenderer *ren, vtkMapper *vtkNotUsed(m))
{
  /*
  if (this->ModelFaceColorMode != 0)
    {
    int BCSindex = -1;
    switch (this->ModelFaceColorMode)
      {
      case 1:
        BCSindex = 2;
        break;
      case 2:
        BCSindex = 0;
        break;
      case 3:
        BCSindex = 1;
        break;
      case 4:
        BCSindex = 3;
        vtkErrorMacro("Boundary Condition Set Mode not supported.");
        break;
      default:
        vtkErrorMacro("Unsupported ModelFaceColorMode Value.");
      }

    if (this->Mapper)
      {
      vtkDataSet *ds = this->Mapper->GetInput();
      if (ds)
        {
        vtkFieldData *fdata = ds->GetFieldData();
        if (fdata)
          {
          int i=0;
          vtkIntArray *idata = vtkIntArray::SafeDownCast(
            fdata->GetArray(vtkMultiBlockWrapper::GetBCSTagName()));
          if (!(idata && idata->GetNumberOfTuples()))
            {
            vtkErrorMacro("Model face information does not exists.");
            }
          else if(BCSindex >=0 && BCSindex < idata->GetNumberOfTuples())
            {
            int val = idata->GetValue(BCSindex);
            vtkScalarsToColors *ltable = this->Mapper->GetLookupTable();
            if (ltable)
              {
              double color[3];
              ltable->GetColor(val, color);
              this->GetProperty()->SetColor(color);
              }
            }
          }
        }
      }
    }
  */
  Superclass::Render(ren,0);
}


void vtkModelFaceActor::ShallowCopy(vtkProp *prop)
{
  vtkModelFaceActor *a = vtkModelFaceActor::SafeDownCast(prop);
  if ( a != NULL )
    {
    this->ModelFaceColorMode = a->ModelFaceColorMode;
    }

  // Now do superclass
  this->vtkPVLODActor::ShallowCopy(prop);
}


//----------------------------------------------------------------------------
void vtkModelFaceActor::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "ModelFaceColorMode: " << this->ModelFaceColorMode << endl;
}

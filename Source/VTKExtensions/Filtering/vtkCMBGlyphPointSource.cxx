//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkCMBGlyphPointSource.h"

#include "vtkPolyData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPoints.h"
#include "vtkUnsignedCharArray.h"
#include "vtkDoubleArray.h"
#include "vtkPointData.h"
#include "vtkBitArray.h"
#include "vtkCellArray.h"
#include "vtkNew.h"
#include "vtkPolyDataWriter.h"
#include "vtkPolyDataReader.h"
#include "vtkTransform.h"
#include "vtkBoundingBox.h"

vtkStandardNewMacro(vtkCMBGlyphPointSource);

//-----------------------------------------------------------------------------
vtkCMBGlyphPointSource::vtkCMBGlyphPointSource()
{
  vtkPointData *pdata;
  this->Source = vtkSmartPointer<vtkPolyData>::New();
  pdata = this->Source->GetPointData();
  this->Points = vtkSmartPointer<vtkPoints>::New();
  this->Points->SetDataTypeToDouble();
  this->Source->SetPoints(this->Points);
  this->CellIds = vtkSmartPointer<vtkCellArray>::New();
  this->Source->SetVerts(this->CellIds);

  // Make sure that there is an element in
  // the cell array
  this->CellIds->InsertNextCell(0);

  // Add color information
  this->Color = vtkSmartPointer<vtkUnsignedCharArray>::New();
  this->Color->SetName("Color");
  this->Color->SetNumberOfComponents(4);
  pdata->AddArray(this->Color);

   // Add Scaling information
  this->Scaling = vtkSmartPointer<vtkDoubleArray>::New();
  this->Scaling->SetName("Scaling");
  this->Scaling->SetNumberOfComponents(3);
  pdata->AddArray(this->Scaling);

   // Add Orientation information
  this->Orientation = vtkSmartPointer<vtkDoubleArray>::New();
  this->Orientation->SetName("Orientation");
  this->Orientation->SetNumberOfComponents(3);
  pdata->AddArray(this->Orientation);

   // Add Visibility information
  this->Visibility = vtkSmartPointer<vtkBitArray>::New();
  this->Visibility->SetName("Visibility");
  this->Visibility->SetNumberOfComponents(1);
  pdata->AddArray(this->Visibility);

   // Add Color Uniqueness information
  this->SelectionMask = vtkSmartPointer<vtkBitArray>::New();
  this->SelectionMask->SetName("UniqueColor");
  this->SelectionMask->SetNumberOfComponents(1);
  pdata->AddArray(this->SelectionMask);

  // Set Colors to be the active scalar array
  pdata->SetActiveScalars("Color");

  // Set the default color to be white
  this->DefaultColor[0] = this->DefaultColor[1] = this->DefaultColor[2] =
    this->DefaultColor[3] = 1.0;

  // Create a tranform that can be re-used
  this->Transform = vtkSmartPointer<vtkTransform>::New();
  this->SetNumberOfInputPorts(0);
}

//-----------------------------------------------------------------------------
vtkCMBGlyphPointSource::~vtkCMBGlyphPointSource()
{
  this->Source = NULL;
  this->Points = NULL;
  this->Color = NULL;
  this->Scaling = NULL;
  this->Orientation = NULL;
  this->Visibility = NULL;
  this->SelectionMask = NULL;
  this->CellIds = NULL; // Array should be deleted with PolyData
  this->Transform = NULL;
}

//-----------------------------------------------------------------------------
int vtkCMBGlyphPointSource::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{
  // get the ouptut
  vtkPolyData *output = vtkPolyData::GetData(outputVector, 0);

  // now move the input through to the output
  output->ShallowCopy( this->Source );
  return 1;
}

//-----------------------------------------------------------------------------
void vtkCMBGlyphPointSource::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Source: " << this->Source << "\n";
  os << indent << "Points: " << this->Points << "\n";
  os << indent << "Color: " << this->Color << "\n";
  os << indent << "Scaling: " << this->Scaling << "\n";
  os << indent << "Orientation: " << this->Orientation << "\n";
  os << indent << "Visibility: " << this->Visibility << "\n";
  os << indent << "SelectionMask: " << this->SelectionMask << "\n";
  os << indent << "Default Color: " << this->DefaultColor[0]
     << ", " << this->DefaultColor[1] << ", " << this->DefaultColor[2]
     << ", " << this->DefaultColor[3] << "\n";
}
//-----------------------------------------------------------------------------
vtkIdType vtkCMBGlyphPointSource::InsertNextPoint(double x, double y, double z)
{
  vtkIdType id = this->Points->InsertNextPoint(x, y, z);
  this->Points->GetBounds(this->GlyphSourceBounds);
  this->Color->InsertNextTuple4((255.0*this->DefaultColor[0])+0.5,
                                (255.0*this->DefaultColor[1])+0.5,
                                (255.0*this->DefaultColor[2])+0.5,
                                (255.0*this->DefaultColor[3])+0.5);
  this->Scaling->InsertNextTuple3(1.0, 1.0, 1.0);
  this->Orientation->InsertNextTuple3(0.0, 0.0, 0.0);
  this->Visibility->InsertNextValue(1);
  this->SelectionMask->InsertNextValue(0);
  // Update the vertices point Ids
  this->CellIds->InsertCellPoint(id);
  this->CellIds->UpdateCellCount(id+1);
  this->Modified();
  return id;
}
//-----------------------------------------------------------------------------
vtkIdType
vtkCMBGlyphPointSource::InsertNextPoint(double x, double y, double z,
                                        double r, double g, double b, double a,
                                        double sx, double sy, double sz,
                                        double ox, double oy, double oz,
                                        int vis)
{
  vtkIdType id = this->Points->InsertNextPoint(x, y, z);
  this->Points->GetBounds(this->GlyphSourceBounds);

  this->Color->InsertNextTuple4((255.0*r)+0.5, (255.0*g)+0.5, (255.0*b)+0.5,
                                (255.0*a)+0.5);
  this->Scaling->InsertNextTuple3(sx, sy, sz);
  this->Orientation->InsertNextTuple3(ox, oy, oz);
  this->Visibility->InsertNextValue(vis);
  // Update the vertices point Ids
  this->CellIds->InsertCellPoint(id);
  this->CellIds->UpdateCellCount(id+1);
  this->SelectionMask->InsertNextValue(1);
  this->Modified();
  return id;
}
//-----------------------------------------------------------------------------
void vtkCMBGlyphPointSource::SetScale(vtkIdType index, double sx, double sy,
                                      double sz)
{
  this->Scaling->SetTuple3(index, sx, sy, sz);
  this->Modified();
}
//-----------------------------------------------------------------------------
void vtkCMBGlyphPointSource::SetOrientation(vtkIdType index, double ox,
                                            double oy, double oz)
{
  this->Orientation->SetTuple3(index, ox, oy, oz);
  this->Modified();
}
//-----------------------------------------------------------------------------
void vtkCMBGlyphPointSource::SetVisibility(vtkIdType index, int flag)
{
  this->Visibility->SetValue(index, flag);
  this->Modified();
}
//-----------------------------------------------------------------------------
void vtkCMBGlyphPointSource::SetColor(vtkIdType index, double r, double g,
                                      double b, double a)
{
  this->Color->SetTuple4(index, (255.0*r)+0.5, (255.0*g)+0.5, (255.0*b)+0.5,
                         (255.0*a)+0.5);
  this->SelectionMask->SetValue(index, 1);
  this->Color->Modified();
  this->Modified();
}
//-----------------------------------------------------------------------------
void vtkCMBGlyphPointSource::UnsetColor(vtkIdType index)
{
  this->Color->SetTuple4(index,
                         (255.0*this->DefaultColor[0])+0.5,
                         (255.0*this->DefaultColor[1])+0.5,
                         (255.0*this->DefaultColor[2])+0.5,
                         (255.0*this->DefaultColor[3])+0.5);
  this->SelectionMask->SetValue(index, 0);
  this->Modified();
}
//-----------------------------------------------------------------------------
void vtkCMBGlyphPointSource::SetDefaultColor(double r, double g,
                                             double b, double a)
{
  this->DefaultColor[0] = r;
  this->DefaultColor[1] = g;
  this->DefaultColor[2] = b;
  this->DefaultColor[3] = a;

  unsigned char rb =  (255.0*r)+0.5;
  unsigned char gb =  (255.0*g)+0.5;
  unsigned char bb =  (255.0*b)+0.5;
  unsigned char ab =  (255.0*a)+0.5;

  vtkIdType i, n = this->Color->GetNumberOfTuples();
  for (i = 0; i < n; i++)
    {
    if (!this->SelectionMask->GetValue(i))
      {
      this->Color->SetTuple4(i, rb, gb, bb, ab);
      }
    }
  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkCMBGlyphPointSource::ApplyTransform(double *odelta, double *pdelta ,
                                            double *sdelta)
{

  double val[3];
  vtkIdType i, n = this->Points->GetNumberOfPoints();
  for (i = 0; i < n; i++)
    {
    this->Points->GetPoint(i, val);
    val[0] += pdelta[0];
    val[1] += pdelta[1];
    val[2] += pdelta[2];
    this->Points->SetPoint(i, val);

    this->Orientation->GetTuple(i, val);
    val[0] += odelta[0];
    val[1] += odelta[1];
    val[2] += odelta[2];
    this->Orientation->SetTuple(i, val);

    this->Scaling->GetTuple(i, val);
    val[0] *= sdelta[0];
    val[1] *= sdelta[1];
    val[2] *= sdelta[2];
    this->Scaling->SetTuple(i, val);
    }
  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkCMBGlyphPointSource::ApplyTransform(vtkIdType i,
                                            double *odelta, double *pdelta,
                                            double *sdelta)
{

  double val[3];
  this->Points->GetPoint(i, val);
  val[0] += pdelta[0];
  val[1] += pdelta[1];
  val[2] += pdelta[2];
  this->Points->SetPoint(i, val);

  this->Orientation->GetTuple(i, val);
  val[0] += odelta[0];
  val[1] += odelta[1];
  val[2] += odelta[2];
  this->Orientation->SetTuple(i, val);

  this->Scaling->GetTuple(i, val);
  val[0] *= sdelta[0];
  val[1] *= sdelta[1];
  val[2] *= sdelta[2];
  this->Scaling->SetTuple(i, val);
  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkCMBGlyphPointSource::ResetColorsToDefault()
{
  unsigned char rb =  (255.0*this->DefaultColor[0])+0.5;
  unsigned char gb =  (255.0*this->DefaultColor[1])+0.5;
  unsigned char bb =  (255.0*this->DefaultColor[2])+0.5;
  unsigned char ab =  (255.0*this->DefaultColor[3])+0.5;

  vtkIdType i, n = this->Color->GetNumberOfTuples();
  for (i = 0; i < n; i++)
    {
    if (this->SelectionMask->GetValue(i))
      {
      this->Color->SetTuple4(i, rb, gb, bb, ab);
      this->SelectionMask->SetValue(i, 0);
      }
    }
  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkCMBGlyphPointSource::SetPoint(vtkIdType index,
                                      double x, double y, double z)
{
  this->Points->SetPoint(index, x, y, z);
  this->Points->GetBounds(this->GlyphSourceBounds);
  this->Modified();
}
//-----------------------------------------------------------------------------
void vtkCMBGlyphPointSource::GetPoint(vtkIdType index,
                                      double *p)
{
  this->Points->GetPoint(index, p);
}
//-----------------------------------------------------------------------------
void vtkCMBGlyphPointSource::GetScale(vtkIdType index,
                                      double *s)
{
  this->Scaling->GetTypedTuple(index, s);
}
//-----------------------------------------------------------------------------
void vtkCMBGlyphPointSource::GetOrientation(vtkIdType index,
                                            double *o)
{
  this->Orientation->GetTypedTuple(index, o);
}
//-----------------------------------------------------------------------------
int vtkCMBGlyphPointSource::GetVisibility(vtkIdType index)
{
  return this->Visibility->GetValue(index);
}
//-----------------------------------------------------------------------------
void vtkCMBGlyphPointSource::GetColor(vtkIdType index,
                                      double *color)
{
  unsigned char v[4];
  this->Color->GetTypedTuple(index, v);
  color[0] = static_cast<double>(v[0]) / 255.0;
  color[1]= static_cast<double>(v[1]) / 255.0;
  color[2] = static_cast<double>(v[2]) / 255.0;
  color[3] = static_cast<double>(v[3]) / 255.0;
}
//-----------------------------------------------------------------------------
void vtkCMBGlyphPointSource::ReadFromFile(const char *fname)
{
  int index;
  vtkPolyDataReader *reader = vtkPolyDataReader::New();
  reader->SetFileName(fname);
  reader->Update();
  this->Source = reader->GetOutput();
  vtkPointData *pdata = this->Source->GetPointData();
  this->Points = this->Source->GetPoints();
  this->CellIds = this->Source->GetVerts();
  this->Color =
    vtkUnsignedCharArray::SafeDownCast(pdata->GetArray("Color", index));
  this->Scaling =
    vtkDoubleArray::SafeDownCast(pdata->GetArray("Scaling", index));
  this->Orientation =
    vtkDoubleArray::SafeDownCast(pdata->GetArray("Orientation", index));
  this->Visibility =
    vtkBitArray::SafeDownCast(pdata->GetArray("Visibility", index));
  this->SelectionMask =
    vtkBitArray::SafeDownCast(pdata->GetArray("UniqueColor", index));
  reader->Delete();
}
//-----------------------------------------------------------------------------
void vtkCMBGlyphPointSource::WriteToFile(const char *fname)
{
  vtkNew<vtkPolyDataWriter> writer;

  // The Color array should be reset to default colors before write to file
  // so that the selection color is not saved
  vtkNew<vtkUnsignedCharArray> tmpColorArray;
  tmpColorArray->DeepCopy(this->Color);
  vtkNew<vtkBitArray> tmpMaskArray;
  tmpMaskArray->DeepCopy(this->SelectionMask);
  this->ResetColorsToDefault();

  writer->SetInputData(this->Source);
  writer->SetFileName(fname);
  writer->SetFileTypeToBinary();
  writer->Write();

  // Reset the Color and SelectionMask array
  this->Color->DeepCopy(tmpColorArray.GetPointer());
  this->SelectionMask->DeepCopy(tmpMaskArray.GetPointer());
}
//-----------------------------------------------------------------------------
double *vtkCMBGlyphPointSource::GetBounds(vtkIdType index)
{
  // Create a transformation based on the glyph instance
  double val[3], pnt[3];
  this->Points->GetPoint(index, pnt);
  this->Transform->Identity();
  this->Transform->PreMultiply();
  this->Transform->Translate(pnt[0], pnt[1], pnt[2]);
  this->Orientation->GetTuple(index, val);
  this->Transform->RotateZ( val[2] );
  this->Transform->RotateX( val[0] );
  this->Transform->RotateY( val[1] );
  this->Scaling->GetTuple(index, val);
  this->Transform->Scale( val );

  // Get the bounds of the glyph
  //this->Source->GetBounds(b);
  double *b = this->GlyphSourceBounds;
  vtkBoundingBox bbox;
  pnt[0] = b[0] ; pnt[1] = b[2]; pnt[2] = b[4];
  this->Transform->TransformPoint(pnt, val);
  bbox.AddPoint(val);

  pnt[0] = b[0] ; pnt[1] = b[3]; pnt[2] = b[4];
  this->Transform->TransformPoint(pnt, val);
  bbox.AddPoint(val);

  pnt[0] = b[0] ; pnt[1] = b[2]; pnt[2] = b[5];
  this->Transform->TransformPoint(pnt, val);
  bbox.AddPoint(val);

  pnt[0] = b[0] ; pnt[1] = b[3]; pnt[2] = b[5];
  this->Transform->TransformPoint(pnt, val);
  bbox.AddPoint(val);

  pnt[0] = b[1] ; pnt[1] = b[2]; pnt[2] = b[4];
  this->Transform->TransformPoint(pnt, val);
  bbox.AddPoint(val);

  pnt[0] = b[1] ; pnt[1] = b[3]; pnt[2] = b[4];
  this->Transform->TransformPoint(pnt, val);
  bbox.AddPoint(val);

  pnt[0] = b[1] ; pnt[1] = b[2]; pnt[2] = b[5];
  this->Transform->TransformPoint(pnt, val);
  bbox.AddPoint(val);

  pnt[0] = b[1] ; pnt[1] = b[3]; pnt[2] = b[5];
  this->Transform->TransformPoint(pnt, val);
  bbox.AddPoint(val);

  bbox.GetBounds(this->TempData);
  return this->TempData;
}
//-----------------------------------------------------------------------------

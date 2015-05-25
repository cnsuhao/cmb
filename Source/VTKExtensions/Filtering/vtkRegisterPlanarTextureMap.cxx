//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkRegisterPlanarTextureMap.h"

#include "vtkCellData.h"
#include "vtkDataSet.h"
#include "vtkDoubleArray.h"
#include "vtkFloatArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"

#define ZERO_TOLERANCE 1.0e-10

vtkStandardNewMacro(vtkRegisterPlanarTextureMap);

//-----------------------------------------------------------------------------
// Construct with s,t range=(0,1) and automatic plane generation turned on.
vtkRegisterPlanarTextureMap::vtkRegisterPlanarTextureMap()
{
  // set to indentity and no clipping
  this->SMap[0] = this->TMap[1] = 1.0;
  this->SMap[1] = this->TMap[0] = SMap[2] = TMap[2] = 0.0;
  this->SRange[0] = 0.0;
  this->SRange[1] = 1.0;

  this->TRange[0] = 0.0;
  this->TRange[1] = 1.0;

  this->ClipXY = 0;
  this->GenerateCoordinates = 1;
  this->NumberOfRegisterPoints = 2;
  this->RegisterXYPoints[0][0] = 0.0;
  this->RegisterXYPoints[0][1] = 0.0;
  this->RegisterXYPoints[1][0] = 1.0;
  this->RegisterXYPoints[1][1] = 0.0;

  this->RegisterSTPoints[0][0] = 0.0;
  this->RegisterSTPoints[0][1] = 0.0;
  this->RegisterSTPoints[1][0] = 1.0;
  this->RegisterSTPoints[1][1] = 0.0;
}

//-----------------------------------------------------------------------------
int vtkRegisterPlanarTextureMap::SetTwoPointRegistration(double info[8])
{
  return this->SetTwoPointRegistration(info, &(info[2]), &(info[4]),
                                       &(info[6]));
}

//-----------------------------------------------------------------------------
void vtkRegisterPlanarTextureMap::GetTwoPointRegistration(double info[8])
{
  int i, j;
  for (i = 0, j = 0; i < 2; i++)
    {
    info[j++] = this->RegisterXYPoints[i][0];
    info[j++] = this->RegisterXYPoints[i][1];
    info[j++] = this->RegisterSTPoints[i][0];
    info[j++] = this->RegisterSTPoints[i][1];
    }
}

//-----------------------------------------------------------------------------
void vtkRegisterPlanarTextureMap::GetThreePointRegistration(double info[12])
{
  int i, j;
  for (i = 0, j = 0; i < 3; i++)
    {
    info[j++] = this->RegisterXYPoints[i][0];
    info[j++] = this->RegisterXYPoints[i][1];
    info[j++] = this->RegisterSTPoints[i][0];
    info[j++] = this->RegisterSTPoints[i][1];
    }
}

//-----------------------------------------------------------------------------
int vtkRegisterPlanarTextureMap::SetTwoPointRegistration(double xy1[2],
                                                         double st1[2],
                                                         double xy2[2],
                                                         double st2[2])
{
  this->NumberOfRegisterPoints = 2;
  this->RegisterXYPoints[0][0] = xy1[0];
  this->RegisterXYPoints[0][1] = xy1[1];
  this->RegisterXYPoints[1][0] = xy2[0];
  this->RegisterXYPoints[1][1] = xy2[1];
  this->RegisterSTPoints[0][0] = st1[0];
  this->RegisterSTPoints[0][1] = st1[1];
  this->RegisterSTPoints[1][0] = st2[0];
  this->RegisterSTPoints[1][1] = st2[1];
  double dxy[2];
  double dst[2];
  dxy[0] = xy2[0] - xy1[0];
  dxy[1] = xy2[1] - xy1[1];
  dst[0] = st2[0] - st1[0];
  dst[1] = st2[1] - st1[1];

  if (fabs(dxy[0]) <= ZERO_TOLERANCE)
    {
    if (fabs(dxy[1]) <= ZERO_TOLERANCE)
      {
      vtkErrorMacro(<< "Both points are the same in XY\n");
      return -1;
      }

    this->SMap[0] = this->TMap[1] = dst[1] / dxy[1];
    this->SMap[1] = dst[0] / dxy[1];
    this->TMap[0] = -this->SMap[1];
    }
  else if (fabs(dxy[1]) <= ZERO_TOLERANCE)
    {
    this->SMap[0] = this->TMap[1] = dst[0] / dxy[0];
    this->SMap[1] = - dst[1] / dxy[0];
    this->TMap[0] = -this->SMap[1];
    }
  else
    {
    this->SMap[1] = ((dxy[1]*dst[0]) - (dxy[0]*dst[1])) /
      ((dxy[0] * dxy[0]) + (dxy[1] * dxy[1]));
    this->SMap[0] = this->TMap[1] = (dst[0] - (this->SMap[1]*dxy[1])) / dxy[0];
    this->TMap[0] = -this->SMap[1];
    }
  this->SMap[2] = st1[0] - ((xy1[0]*this->SMap[0]) + (xy1[1]*this->SMap[1]));
  this->TMap[2] = st1[1] - ((xy1[0]*this->TMap[0]) + (xy1[1]*this->TMap[1]));
  this->Modified();
  return 0;
}

//-----------------------------------------------------------------------------
int vtkRegisterPlanarTextureMap::SetThreePointRegistration(double info[12])
{
  return this->SetThreePointRegistration(info, &(info[2]),
                                         &(info[4]), &(info[6]),
                                         &(info[8]), &(info[10]));
}

//-----------------------------------------------------------------------------
int vtkRegisterPlanarTextureMap::SetThreePointRegistration(double xy1[2],
                                                           double st1[2],
                                                           double xy2[2],
                                                           double st2[2],
                                                           double xy3[2],
                                                           double st3[2])
{
  this->NumberOfRegisterPoints = 3;
  this->RegisterXYPoints[0][0] = xy1[0];
  this->RegisterXYPoints[0][1] = xy1[1];
  this->RegisterXYPoints[1][0] = xy2[0];
  this->RegisterXYPoints[1][1] = xy2[1];
  this->RegisterXYPoints[2][0] = xy3[0];
  this->RegisterXYPoints[2][1] = xy3[1];
  this->RegisterSTPoints[0][0] = st1[0];
  this->RegisterSTPoints[0][1] = st1[1];
  this->RegisterSTPoints[1][0] = st2[0];
  this->RegisterSTPoints[1][1] = st2[1];
  this->RegisterSTPoints[2][0] = st3[0];
  this->RegisterSTPoints[2][1] = st3[1];

  int dx0 = -1;
  int dy0 = -1;
  double dxy[3][2];
  double dst[3][2];
  int i, j;
  for (i = 0, j = 1; i < 3; i++, j++)
    {
    // See if we need to wrap j back to 0
    if ( j == 3)
      {
      j = 0;
      }
    dxy[i][0] = this->RegisterXYPoints[j][0] - this->RegisterXYPoints[i][0];
    dxy[i][1] = this->RegisterXYPoints[j][1] - this->RegisterXYPoints[i][1];
    dst[i][0] = this->RegisterSTPoints[j][0] - this->RegisterSTPoints[i][0];
    dst[i][1] = this->RegisterSTPoints[j][1] - this->RegisterSTPoints[i][1];
    }

  // Now lets see if any of the deltas in xy are zero since this
  // will simplify things
  for (i = 0; i < 3; i++)
    {
    if (fabs(dxy[i][0]) <= ZERO_TOLERANCE)
      {
      if (fabs(dxy[i][1]) <= ZERO_TOLERANCE)
        {
        vtkErrorMacro(<< "At least 2 Points are the same in XY\n");
        return -1;
        }
      if (dx0 == -1)
        {
        dx0 = i;
        }
      else
        {
        vtkErrorMacro(<< "3 points are colinear in X\n");
        return -1;
        }
      }
    if (fabs(dxy[i][1]) <= ZERO_TOLERANCE)
      {
      if (dy0 == -1)
        {
        dy0 = i;
        }
      else
        {
        vtkErrorMacro(<< "3 points are colinear in Y\n");
        return -1;
        }
      }
    }

  if (dx0 != -1)
    {
    this->SMap[1] = dst[dx0][0]/dxy[dx0][1];
    this->TMap[1] = dst[dx0][1]/dxy[dx0][1];
    }

  if (dy0 != -1)
    {
    this->SMap[0] = dst[dy0][0]/dxy[dy0][0];
    this->TMap[0] = dst[dy0][1]/dxy[dy0][0];
    }

  if (dx0 == -1)
    {
    if (dy0 != -1)
      {
      i = (dy0 + 1) % 3; // find pair w/o deltaY == 0
      this->SMap[1] = (dst[i][0] - (this->SMap[0]*dxy[i][0]))/dxy[i][1];
      this->TMap[1] = (dst[i][1] - (this->TMap[0]*dxy[i][0]))/dxy[i][1];
      }
    else
      {
      this->SMap[0] =
        ((dst[1][0] * dxy[0][1]) - (dst[0][0] * dxy[1][1])) /
        ((dxy[1][0] * dxy[0][1]) - (dxy[0][0] * dxy[1][1]));
      this->SMap[1] = (dst[0][0] - (this->SMap[0]*dxy[0][0]))/dxy[0][1];
      this->TMap[0] =
        ((dst[1][1] * dxy[0][1]) - (dst[0][1] * dxy[1][1])) /
        ((dxy[1][0] * dxy[0][1]) - (dxy[0][0] * dxy[1][1]));
      this->TMap[1] = (dst[0][1] - (this->TMap[0]*dxy[0][0]))/dxy[0][1];
      }
    }
  else if (dy0 == -1)
    {
    i = (dx0 + 1) % 3; // find pair w/o deltaX == 0
    this->SMap[0] = (dst[i][0] - (this->SMap[1]*dxy[i][1]))/dxy[i][0];
    this->TMap[0] = (dst[i][1] - (this->TMap[1]*dxy[i][1]))/dxy[i][0];
    }

  this->SMap[2] = st1[0] - ((xy1[0]*this->SMap[0]) + (xy1[1]*this->SMap[1]));
  this->TMap[2] = st1[1] - ((xy1[0]*this->TMap[0]) + (xy1[1]*this->TMap[1]));
  this->Modified();
  return 0;
}

//-----------------------------------------------------------------------------
int vtkRegisterPlanarTextureMap::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and ouptut
  vtkDataSet *input = vtkDataSet::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkDataSet *output = vtkDataSet::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  // First, copy the input to the output as a starting point
  output->CopyStructure( input );

  if (!this->GenerateCoordinates)
    {
    output->GetPointData()->PassData(input->GetPointData());
    output->GetCellData()->PassData(input->GetCellData());
    return 1;
    }

  double tcoords[2];
  vtkIdType numPts;
  vtkFloatArray *newTCoords;
  vtkIdType i;

  double sSf, tSf, p[3];
  int abort=0;
  vtkIdType progressInterval;

  vtkDebugMacro(<<"Generating texture coordinates!");
  numPts=input->GetNumberOfPoints();


  //  Allocate texture data
  //
  newTCoords = vtkFloatArray::New();
  newTCoords->SetName("Texture Coordinates");
  newTCoords->SetNumberOfComponents(2);
  newTCoords->SetNumberOfTuples(numPts);
  progressInterval = numPts/20 + 1;

  // Add field data to make tit easy for a consumer down the line to
  // use the static method to calculate texture coordinates.
  vtkDoubleArray *sRangeArray = vtkDoubleArray::New();
  sRangeArray->SetName("SRange");
  sRangeArray->SetNumberOfComponents(2);
  sRangeArray->SetNumberOfTuples(1);
  sRangeArray->SetTuple2(0, this->SRange[0], this->SRange[1]);
  output->GetFieldData()->AddArray( sRangeArray );
  sRangeArray->FastDelete();

  vtkDoubleArray *tRangeArray = vtkDoubleArray::New();
  tRangeArray->SetName("TRange");
  tRangeArray->SetNumberOfComponents(2);
  tRangeArray->SetNumberOfTuples(1);
  tRangeArray->SetTuple2(0, this->TRange[0], this->TRange[1]);
  output->GetFieldData()->AddArray( tRangeArray );
  tRangeArray->FastDelete();

  vtkDoubleArray *sMapArray = vtkDoubleArray::New();
  sMapArray->SetName("SMap");
  sMapArray->SetNumberOfComponents(2);
  sMapArray->SetNumberOfTuples(1);
  sMapArray->SetTuple2(0, this->SMap[0], this->SMap[1]);
  output->GetFieldData()->AddArray( sMapArray );
  sMapArray->FastDelete();

  vtkDoubleArray *tMapArray = vtkDoubleArray::New();
  tMapArray->SetName("TMap");
  tMapArray->SetNumberOfComponents(2);
  tMapArray->SetNumberOfTuples(1);
  tMapArray->SetTuple2(0, this->TMap[0], this->TMap[1]);
  output->GetFieldData()->AddArray( tMapArray );
  tMapArray->FastDelete();


  sSf = 1.0 / (this->SRange[1] - this->SRange[0]);
  tSf = 1.0 / (this->TRange[1] - this->TRange[0]);

  //  Now can loop over all points, computing parametric coordinates.
  //
  for (i=0; i<numPts && !abort; i++)
    {
    if ( !(i % progressInterval) )
      {
      this->UpdateProgress(static_cast<double>(i)/numPts);
      abort = this->GetAbortExecute();
      }

    output->GetPoint(i, p);

    tcoords[0] = ((this->SMap[0]*p[0]) + (this->SMap[1]*p[1])
                  + this->SMap[2] - this->SRange[0]) * sSf;
    tcoords[1] = ((this->TMap[0]*p[0]) + (this->TMap[1]*p[1])
                  + this->TMap[2] - this->TRange[0]) * tSf;

//    std::cout << tcoords[0] << ", " << tcoords[1] << "\n";
    newTCoords->SetTuple(i,tcoords);
    }

  // Update ourselves
  //
  output->GetPointData()->CopyTCoordsOff();
  output->GetPointData()->PassData(input->GetPointData());
  output->GetCellData()->PassData(input->GetCellData());

  output->GetPointData()->SetTCoords(newTCoords);
  newTCoords->Delete();

  return 1;
}

//-----------------------------------------------------------------------------
void vtkRegisterPlanarTextureMap::ComputeTextureCoordinate(double pt[2],
  double sRange[2], double tRange[2], double sMap[3], double tMap[3],
  double *tCoords)
{
  tCoords[0] = ((sMap[0] * pt[0]) + (sMap[1] * pt[1]) + sMap[2] - sRange[0]) /
    (sRange[1] - sRange[0]);
  tCoords[1] = ((tMap[0] * pt[0]) + (tMap[1] * pt[1]) + tMap[2] - tRange[0]) /
    (tRange[1] - tRange[0]);
}

//-----------------------------------------------------------------------------
void vtkRegisterPlanarTextureMap::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "SMap: (" << this->SMap[0] << ", "
     << this->SMap[1] << ", " << this->SMap[2] << " )\n";

  os << indent << "TMap: (" << this->TMap[0] << ", "
     << this->TMap[1] << ", " << this->TMap[2] << " )\n";

  os << indent << "Number Of RegisterPoints: "
     << this->NumberOfRegisterPoints << "\n";

  int i;
  for (i = 0; i < this->NumberOfRegisterPoints ; i++)
    {
    os << indent << "Registration Point " << i
       << ": (" << this->RegisterXYPoints[i][0] << ", "
       << this->RegisterXYPoints[i][1] << " ) -> ("
       << this->RegisterSTPoints[i][0] << ", "
       << this->RegisterSTPoints[i][1] << " )"
       << "\n";
    }

  os << indent << "S Range: (" << this->SRange[0] << ", "
                               << this->SRange[1] << ")\n";

  os << indent << "T Range: (" << this->TRange[0] << ", "
                               << this->TRange[1] << ")\n";

  os << indent << "X Range: (" << this->XRange[0] << ", "
                               << this->XRange[1] << ")\n";

  os << indent << "Y Range: (" << this->YRange[0] << ", "
                               << this->YRange[1] << ")\n";
  os << indent << "Clipping XY: " <<
                  (this->ClipXY ? "On\n" : "Off\n");
  os << indent << "Generate Texture Coordinates: " <<
                  (this->GenerateCoordinates ? "On\n" : "Off\n");

}
//-----------------------------------------------------------------------------

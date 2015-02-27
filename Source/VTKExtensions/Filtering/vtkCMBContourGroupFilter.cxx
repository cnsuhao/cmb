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
#include "vtkCMBContourGroupFilter.h"

#include "vtkAppendPolyData.h"
#include "vtkCellData.h"
#include "vtkDataSet.h"
#include "vtkDoubleArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkIntArray.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"

vtkStandardNewMacro(vtkCMBContourGroupFilter);
//-----------------------------------------------------------------------------
vtkCMBContourGroupFilter::vtkCMBContourGroupFilter()
{
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);
  this->ActiveGroupIdx = -1;
}

//-----------------------------------------------------------------------------
vtkCMBContourGroupFilter::~vtkCMBContourGroupFilter()
{
  this->RemoveAllContours();
}

//----------------------------------------------------------------------------
int vtkCMBContourGroupFilter::IsActiveGroupValid()
{
  if ( this->ActiveGroupIdx < 0)
    {
    return 0;
    }
  return 1;
}

//----------------------------------------------------------------------------
void vtkCMBContourGroupFilter::SetGroupInvert(int val)
{
  if(this->IsActiveGroupValid() &&
    (this->GroupInvert.find(this->ActiveGroupIdx) == this->GroupInvert.end() ||
    this->GroupInvert[this->ActiveGroupIdx] != val))
    {
    this->GroupInvert[this->ActiveGroupIdx] = val;
    this->Modified();
    }
}

//----------------------------------------------------------------------------
void vtkCMBContourGroupFilter::AddContour(vtkAlgorithm* contour)
{
  if(this->IsActiveGroupValid())
    {
    PolygonInfo* cfInfo = new PolygonInfo();
    cfInfo->Polygon = contour;
    this->Polygons[this->ActiveGroupIdx].push_back(cfInfo);
    if (this->Polygons[this->ActiveGroupIdx].size() == 1)
      {
      this->GroupInvert[this->ActiveGroupIdx] = 0;
      }
    this->Modified();
    }
}

//----------------------------------------------------------------------------
void vtkCMBContourGroupFilter::RemoveContour(vtkAlgorithm* contour)
{
  if(contour && this->IsActiveGroupValid())
    {
    bool removed = false;
    for(std::vector<PolygonInfo*>::iterator itRemove =
      this->Polygons[this->ActiveGroupIdx].begin();
      itRemove !=this->Polygons[this->ActiveGroupIdx].end(); itRemove++)
      {
      if((*itRemove)->Polygon == contour)
        {
        delete *itRemove;
        this->Polygons[this->ActiveGroupIdx].erase(itRemove);
        removed = true;
        break;
        }
      }
    if(removed)
      {
      this->Modified();
      }
    }
}

//----------------------------------------------------------------------------
void vtkCMBContourGroupFilter::RemoveAllContours()
{
  for(std::map<int, std::vector<PolygonInfo*> >::iterator it=this->Polygons.begin();
    it != this->Polygons.end(); it++)
    {
    for(std::vector<PolygonInfo*>::iterator itRemove =
      it->second.begin(); itRemove !=it->second.end(); itRemove++)
      {
      delete *itRemove;
      }
    }
  this->Polygons.clear();
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkCMBContourGroupFilter::SetContourProjectionNormal(int idx, int val)
{
  if(this->IsActiveGroupValid())
    {
    if(idx>=0 && idx<static_cast<int>(this->Polygons[this->ActiveGroupIdx].size()))
      {
      if(this->Polygons[this->ActiveGroupIdx][idx]->ProjectionNormal != val)
        {
        this->Polygons[this->ActiveGroupIdx][idx]->ProjectionNormal = val;
        }
      this->Modified();
      }
    }
}

//----------------------------------------------------------------------------
void vtkCMBContourGroupFilter::SetContourProjectionPosition(int idx, double val)
{
  if(this->IsActiveGroupValid())
    {
    if(idx>=0 && idx<static_cast<int>(this->Polygons[this->ActiveGroupIdx].size()))
      {
      if(this->Polygons[this->ActiveGroupIdx][idx]->ProjectionPosition != val)
        {
        this->Polygons[this->ActiveGroupIdx][idx]->ProjectionPosition = val;
        }
      this->Modified();
      }
    }
}

//-----------------------------------------------------------------------------
int vtkCMBContourGroupFilter::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector ** /*inputVector*/,
  vtkInformationVector *outputVector)
{
  vtkInformation* info = outputVector->GetInformationObject(0);
  vtkMultiBlockDataSet* output = vtkMultiBlockDataSet::SafeDownCast(
    info->Get(vtkDataObject::DATA_OBJECT()));
  if (!output)
    {
    return 0;
    }
  unsigned int numGroups = static_cast<unsigned int>(this->Polygons.size());
  output->SetNumberOfBlocks(numGroups);
  std::map<int, std::vector<PolygonInfo*> >::iterator itMap=this->Polygons.begin();
  for(unsigned int idx=0;
    idx<numGroups && itMap != this->Polygons.end(); itMap++, idx++)
    {
    vtkAppendPolyData *append = vtkAppendPolyData::New();
    vtkIntArray* projNormalArray = vtkIntArray::New();
    projNormalArray->SetName("ProjectionNormal");
    vtkIntArray* invertGroupArray = vtkIntArray::New();
    invertGroupArray->SetName("GroupInvert");
    vtkDoubleArray* projPosArray = vtkDoubleArray::New();
    projPosArray->SetName("ProjectionPosition");
    for(std::vector<PolygonInfo*>::iterator itF =
      itMap->second.begin(); itF !=itMap->second.end(); itF++)
      {
      if((*itF)->Polygon==NULL)
        {
        continue;
        }
      projNormalArray->InsertNextValue((*itF)->ProjectionNormal);
      projPosArray->InsertNextValue((*itF)->ProjectionPosition);
      append->AddInputConnection((*itF)->Polygon->GetOutputPort());
      }
    int invertgroup =
      (this->GroupInvert.find(itMap->first)==this->GroupInvert.end())
      ? 0 : this->GroupInvert[itMap->first];
    invertGroupArray->InsertNextValue(invertgroup);

    append->Update();
    vtkPolyData* blockPoly = vtkPolyData::New();
    blockPoly->ShallowCopy(append->GetOutput());
    blockPoly->GetFieldData()->AddArray(projNormalArray);
    blockPoly->GetFieldData()->AddArray(projPosArray);
    blockPoly->GetFieldData()->AddArray(invertGroupArray);
    output->SetBlock(idx, blockPoly);
    projNormalArray->Delete();
    projPosArray->Delete();
    invertGroupArray->Delete();
    append->Delete();
    blockPoly->Delete();
    }

  return 1;
}

//-----------------------------------------------------------------------------
int vtkCMBContourGroupFilter::FillInputPortInformation(
  int, vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataObject");
  info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
  return 1;
}

//-----------------------------------------------------------------------------
void vtkCMBContourGroupFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  os << indent << "ActiveGroupIdx: " << this->ActiveGroupIdx << "\n";
  os << indent << "NumberOfContourGroups: " << this->Polygons.size() << "\n";
  this->Superclass::PrintSelf(os,indent);
}

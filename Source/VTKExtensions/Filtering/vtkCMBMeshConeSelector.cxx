//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkCMBMeshConeSelector.h"

#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkCMBConeSource.h"
#include "vtkExecutive.h"
#include "vtkFloatArray.h"
#include "vtkGenericCell.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkLine.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkTransform.h"
#include "vtkTriangle.h"
#include "vtkSelectionNode.h"
#include "vtkUnstructuredGrid.h"
#include "vtkSmartPointer.h"

#include <math.h>

vtkStandardNewMacro(vtkCMBMeshConeSelector);
vtkCxxSetObjectMacro(vtkCMBMeshConeSelector, Transform, vtkTransform);
vtkCxxSetObjectMacro(vtkCMBMeshConeSelector, ConeSource, vtkCMBConeSource);

//----------------------------------------------------------------------------
vtkCMBMeshConeSelector::vtkCMBMeshConeSelector()
{
  this->SetNumberOfInputPorts(1);

  this->Transform = 0;
  this->ConeSource = 0;
  this->IsProcessing = false;
  this->SelectionFieldType = CELL;
  this->InsideOut = 0;
  this->SelectConeType = ALL_IN;
  this->SetNumberOfOutputPorts(1);
  this->IsSelectionEmpty = 1;
}

//----------------------------------------------------------------------------
vtkCMBMeshConeSelector::~vtkCMBMeshConeSelector()
{
  this->SetTransform(static_cast<vtkTransform*>(0));
  this->SetConeSource(NULL);
}
//----------------------------------------------------------------------------
// Overload standard modified time function. If ConeSource is modified,
// then this object is modified as well.
unsigned long vtkCMBMeshConeSelector::GetMTime()
{
  unsigned long mTime=this->Superclass::GetMTime();
  unsigned long time;

  if ( this->ConeSource != NULL )
    {
    time = this->ConeSource->GetMTime();
    mTime = ( time > mTime ? time : mTime );
    }

  return mTime;
}
//----------------------------------------------------------------------------
int vtkCMBMeshConeSelector::FillInputPortInformation(
  int port, vtkInformation* info)
{
  if (port==0)
    {
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataObject");
    }
  return 1;
}

//----------------------------------------------------------------------------
int vtkCMBMeshConeSelector::FillOutputPortInformation(
  int port, vtkInformation* info)
{
  if (port == 0)
    {
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkSelection");
    return 1;
    }
  return 0;
}

//-----------------------------------------------------------------------------
void vtkCMBMeshConeSelector::SetTransform(double elements[16])
{
  vtkTransform *tmpTransform = vtkTransform::New();
  tmpTransform->SetMatrix(elements);
  this->SetTransform(tmpTransform);
  tmpTransform->Delete();
}

//----------------------------------------------------------------------------
int vtkCMBMeshConeSelector::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  if(this->IsProcessing)
    {
    vtkErrorMacro(<< "vtkCMBMeshConeSelector::RequestData is called again while it is still processing");
    return 0;
    }
  this->IsProcessing = true;

  // get the input and output
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  // expecting the original mesh input
  vtkUnstructuredGrid *input = vtkUnstructuredGrid::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkIdType numPts=input->GetNumberOfPoints();
  vtkIdType numCells = input->GetNumberOfCells();
  if(numPts==0 || numCells==0)
    {
    vtkErrorMacro(<< "The input data does not have points or cells");
    return 0;
    }

  vtkPoints *inPts=input->GetPoints();
  // Initialize self; create output objects
  //
  if ( numPts < 1 || inPts == NULL)
    {
    vtkWarningMacro(<<"No input data");
    this->IsProcessing = false;
    return 1;
    }
  if ( this->ConeSource == NULL)
    {
    vtkWarningMacro(<<"Cone is not set for selecting cells");
    this->IsProcessing = false;
    return 0;
    }

  // Get the original "Cell ID" array
  vtkIdTypeArray* meshCellIdArray =vtkIdTypeArray::SafeDownCast(
    input->GetCellData()->GetArray("Mesh Cell ID"));
  if ( ! meshCellIdArray )
    {
    vtkErrorMacro(<<"The Mesh Cell ID array is missing from input.");
    this->IsProcessing = false;
    return 0;
    }
  vtkIdTypeArray* meshNodeIdArray =vtkIdTypeArray::SafeDownCast(
    input->GetPointData()->GetArray("Mesh Node ID"));
  if ( ! meshNodeIdArray )
    {
    vtkErrorMacro(<<"The Mesh Node ID array is missing from input.");
    this->IsProcessing = false;
    return 0;
    }

  // get the info objects
  vtkSelection *output = vtkSelection::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkSelectionNode* selNode = vtkSelectionNode::New();
  vtkInformation* oProperties = selNode->GetProperties();
  oProperties->Set(vtkSelectionNode::CONTENT_TYPE(),
    vtkSelectionNode::INDICES);
  oProperties->Set(vtkSelectionNode::FIELD_TYPE(), this->SelectionFieldType);
  output->AddNode(selNode);
  this->IsSelectionEmpty = 1;
  // Create the selection list
  vtkIdTypeArray* outSelectionList = vtkIdTypeArray::New();
  outSelectionList->SetNumberOfComponents(1);
  vtkIdType numSelIds = 0;
  vtkIdType i,npts, *pts;
  // We need to get the unit vector based on the Cone's Axis Direction
  double AxisUnitDir[3];
  AxisUnitDir[0] = this->ConeSource->GetDirection()[0];
  AxisUnitDir[1] = this->ConeSource->GetDirection()[1];
  AxisUnitDir[2] = this->ConeSource->GetDirection()[2];
  vtkMath::Normalize(AxisUnitDir);

  for (i=0; i < numCells; i++ )
    {
    input->GetCellPoints(i, npts, pts);
    if(this->DoCellConeCheck(npts, pts, input,
      this->ConeSource->GetBaseCenter(), AxisUnitDir,
      this->ConeSource->GetHeight(), this->ConeSource->GetBaseRadius(),
      this->ConeSource->GetTopRadius()))
      {
      outSelectionList->InsertNextValue(i);
      }
    }

  if(outSelectionList->GetNumberOfTuples() > 0)
    {
    selNode->SetSelectionList(outSelectionList);
    oProperties->Set(vtkSelectionNode::CONTAINING_CELLS(),0);
    oProperties->Set(vtkSelectionNode::INVERSE(),this->InsideOut);
    if (selNode->GetSelectionList())
      {
      selNode->GetSelectionList()->SetName("IDs");
      }
    }

  numSelIds = outSelectionList->GetNumberOfTuples();
  vtkIdType numIds = this->SelectionFieldType==vtkSelectionNode::CELL ?
    numCells : numPts;
  if((!this->InsideOut && numSelIds>0) ||
    (this->InsideOut && numSelIds<numIds))
    {
    this->IsSelectionEmpty = 0;
    }
  outSelectionList->Delete();
  selNode->Delete();
  this->IsProcessing = false;

  return 1;
}
// The following algorithm is copied from Bob's vtkCMBConeCellClassifier
//----------------------------------------------------------------------------
inline bool IsInsideCone(const double p[3], double BaseCenter[3],
  double AxisUnitDir[3], double Height, double BaseRadius, double TopRadius)
{
  double vec[3];
  vtkMath::Subtract(p, BaseCenter, vec);
  // Get the dot product to see if the
  // projection of p onto the cone axis lies between 0 and coneLength
  double l = vtkMath::Dot(vec,AxisUnitDir);
  if ((l < 0.0) || (l > Height))
    {
    return false; // point is outside of the cone
    }
  // Now see what the radius at that point along the cone
  // should be based on interpolating between the radii of the cone
  double r2 = BaseRadius + ((TopRadius - BaseRadius) * l / Height);
  // Square the result - we will need it in a min.
  r2 *= r2;
  // Calculate the perpendicular distance squared from the point and the cone
  // axis - this is the distance between the point and p0 squared minus the projected
  // length squared
  double dist2 = vtkMath::Dot(vec, vec) - (l*l);
  // if the perp dist is greater than the radius value we calculated then we know the point
  // is outside the cone (else its inside)
  if (dist2 > r2)
    {
    return false; // point is outside of the cone
    }
  return true;
}

//----------------------------------------------------------------------------
inline bool IsAllIn(  vtkIdType npts, vtkIdType *pts, double* point,
  double BaseCenter[3],double AxisUnitDir[3], double Height,
  double BaseRadius, double TopRadius, vtkUnstructuredGrid* input)
{
  for (vtkIdType ptIndex = 0; ptIndex < npts; ptIndex++)
    {
    input->GetPoint(pts[ptIndex], point);
    if(!IsInsideCone(point,BaseCenter,AxisUnitDir,Height,BaseRadius,TopRadius))
      {
      return false;
      }
    }
  return true;
}
//----------------------------------------------------------------------------
inline bool IsTransFormAllIn(  vtkIdType npts, vtkIdType *pts, double* point,
  double BaseCenter[3],double AxisUnitDir[3], double Height,
  double BaseRadius, double TopRadius,
   vtkTransform* transform, double* transpoint,vtkUnstructuredGrid* input)
{
  for (vtkIdType ptIndex = 0; ptIndex < npts; ptIndex++)
    {
    input->GetPoint(pts[ptIndex], point);
    transform->TransformPoint(point, transpoint);
    if(!IsInsideCone(transpoint,BaseCenter,AxisUnitDir,Height,BaseRadius,TopRadius))
      {
      return false;
      }
    }
  return true;
}

inline bool IsAllOut(  vtkIdType npts, vtkIdType *pts, double* point,
  double BaseCenter[3],double AxisUnitDir[3], double Height,
  double BaseRadius, double TopRadius, vtkUnstructuredGrid* input)
{
  for (vtkIdType ptIndex = 0; ptIndex < npts; ptIndex++)
    {
    input->GetPoint(pts[ptIndex], point);
    if(IsInsideCone(point,BaseCenter,AxisUnitDir,Height,BaseRadius,TopRadius))
      {
      return false;
      }
    }
  return true;
}
//----------------------------------------------------------------------------
inline bool IsTransFormAllOut(  vtkIdType npts, vtkIdType *pts, double* point,
  double BaseCenter[3],double AxisUnitDir[3], double Height,
  double BaseRadius, double TopRadius,
  vtkTransform* transform, double* transpoint,vtkUnstructuredGrid* input)
{
  for (vtkIdType ptIndex = 0; ptIndex < npts; ptIndex++)
    {
    input->GetPoint(pts[ptIndex], point);
    transform->TransformPoint(point, transpoint);
    if(IsInsideCone(transpoint,BaseCenter,AxisUnitDir,Height,BaseRadius,TopRadius))
      {
      return false;
      }
    }
  return true;
}
inline bool IsIntersecting(  vtkIdType npts, vtkIdType *pts, double* point,
  double BaseCenter[3],double AxisUnitDir[3], double Height,
  double BaseRadius, double TopRadius, vtkUnstructuredGrid* input)
{
  bool partIn = false;
  bool partOut = false;
  for (vtkIdType ptIndex = 0; ptIndex < npts; ptIndex++)
    {
    input->GetPoint(pts[ptIndex], point);
    if(IsInsideCone(point,BaseCenter,AxisUnitDir,Height,BaseRadius,TopRadius))
      {
      partIn = true;
      }
    else
      {
      partOut = true;
      }
    }
  return partIn && partOut;
}
//----------------------------------------------------------------------------
inline bool IsTransFormIntersecting(vtkIdType npts, vtkIdType *pts, double* point,
  double BaseCenter[3],double AxisUnitDir[3], double Height,
  double BaseRadius, double TopRadius,
  vtkTransform* transform, double* transpoint,vtkUnstructuredGrid* input)
{
  bool partIn = false;
  bool partOut = false;
  for (vtkIdType ptIndex = 0; ptIndex < npts; ptIndex++)
    {
    input->GetPoint(pts[ptIndex], point);
    transform->TransformPoint(point, transpoint);
    if(IsInsideCone(transpoint,BaseCenter,AxisUnitDir,Height,BaseRadius,TopRadius))
      {
      partIn = true;
      }
    else
      {
      partOut = true;
      }
    }
  return partIn && partOut;
}
//----------------------------------------------------------------------------
bool vtkCMBMeshConeSelector::DoCellConeCheck(
  vtkIdType npts, vtkIdType* pts, vtkUnstructuredGrid* input,
  double BaseCenter[3],double AxisUnitDir[3], double Height,
  double BaseRadius, double TopRadius)
{
  double point[3];
  double transPts[3];
  bool bKeep = false;
  if(npts<=0 || !pts || !input)
    {
    return bKeep;
    }
  switch (this->SelectConeType)
    {
    case ALL_IN:
      bKeep = (this->Transform==NULL) ?
        IsAllIn(npts, pts, point, BaseCenter, AxisUnitDir,
          Height,BaseRadius, TopRadius, input) :
        IsTransFormAllIn(npts, pts, point, BaseCenter, AxisUnitDir,
          Height,BaseRadius, TopRadius, this->Transform, transPts,input);
      break;
    case INTERSECT_ONLY:
      bKeep = (this->Transform==NULL) ?
        IsIntersecting(npts, pts, point, BaseCenter, AxisUnitDir,
          Height,BaseRadius, TopRadius, input) :
        IsTransFormIntersecting(npts, pts, point, BaseCenter, AxisUnitDir,
          Height,BaseRadius, TopRadius, this->Transform, transPts,input);
      break;
    case PARTIAL_OR_ALL_IN:
      bKeep = (this->Transform==NULL) ?
        !IsAllOut(npts, pts, point, BaseCenter, AxisUnitDir,
          Height,BaseRadius, TopRadius, input) :
        !IsTransFormAllOut(npts, pts, point, BaseCenter, AxisUnitDir,
          Height,BaseRadius, TopRadius, this->Transform, transPts,input);
      break;
    }
  return bKeep;
}

//----------------------------------------------------------------------------
void vtkCMBMeshConeSelector::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "SelectConeType: " << this->SelectConeType << "\n";
  os << indent << "SelectionFieldType: " << this->SelectionFieldType << "\n";
  os << indent << "InsideOut: " << this->InsideOut << "\n";
  os << indent << "IsSelectionEmpty: " << this->IsSelectionEmpty << "\n";

  if ( this->Transform )
    {
    os << indent << "Transform: " << this->Transform << "\n";
    }
  else
    {
    os << indent << "Transform: (none)\n";
    }
}

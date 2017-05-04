//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkHydroModelSelectionSource.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkSelection.h"
#include "vtkSelectionNode.h"
#include "vtkSelectionSource.h"
#include <set>

class vtkHydroModelSelectionSource::vtkInternal
{
public:
  struct IDType
  {
    vtkIdType Piece;
    vtkIdType ID;
    IDType(vtkIdType piece, vtkIdType id)
    {
      this->Piece = piece;
      this->ID = id;
    }

    bool operator<(const IDType& other) const
    {
      if (this->Piece == other.Piece)
      {
        return (this->ID < other.ID);
      }
      return (this->Piece < other.Piece);
    }
  };

  typedef std::set<IDType> SetOfIDType;

  SetOfIDType IDs;
};

vtkStandardNewMacro(vtkHydroModelSelectionSource);

vtkHydroModelSelectionSource::vtkHydroModelSelectionSource()
{
  this->Internal = new vtkInternal();
  this->Source = vtkSelectionSource::New();
  this->Selection = vtkSelection::New();
  this->SetNumberOfInputPorts(0);
  this->InsideOut = 0;
}

vtkHydroModelSelectionSource::~vtkHydroModelSelectionSource()
{
  this->Source->Delete();
  this->Selection->Delete();
  delete this->Internal;
}

void vtkHydroModelSelectionSource::CopyData(vtkSelection* selection)
{
  if (this->Internal->IDs.size() > 0)
  {
    this->Internal->IDs.clear();
  }

  this->Selection->DeepCopy(selection);
  this->Modified();
}

void vtkHydroModelSelectionSource::CopyData(vtkAlgorithm* algOut)
{
  if (algOut)
  {
    vtkSelection* selOut = vtkSelection::SafeDownCast(algOut->GetOutputDataObject(0));
    if (selOut)
    {
      this->CopyData(selOut);
    }
  }
}

void vtkHydroModelSelectionSource::AddID(vtkIdType piece, vtkIdType id)
{
  if (piece < -1)
  {
    piece = -1;
  }
  this->Internal->IDs.insert(vtkInternal::IDType(piece, id));
  this->Modified();
}

void vtkHydroModelSelectionSource::RemoveAllIDs()
{
  this->RemoveAllIDsInternal();
  this->Modified();
}

void vtkHydroModelSelectionSource::RemoveAllIDsInternal()
{
  if (this->Internal->IDs.size() > 0)
  {
    this->Internal->IDs.clear();
  }
}

int vtkHydroModelSelectionSource::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed(inputVector), vtkInformationVector* outputVector)
{
  // get the ouptut
  vtkSelection* output = vtkSelection::GetData(outputVector);
  output->Initialize();

  if (this->Internal->IDs.size() > 0)
  {
    this->Source->SetContentType(vtkSelectionNode::INDICES);
    this->Source->RemoveAllIDs();
    vtkInternal::SetOfIDType::iterator iter;
    for (iter = this->Internal->IDs.begin(); iter != this->Internal->IDs.end(); ++iter)
    {
      this->Source->AddID(iter->Piece, iter->ID);
    }
    this->Source->Update();
    this->Selection->ShallowCopy(this->Source->GetOutput());
  }

  // now move the input through to the output
  output->ShallowCopy(this->Selection);
  this->RemoveAllIDsInternal();
  return 1;
}

int vtkHydroModelSelectionSource::GetSelectionFieldType()
{
  int fieldType = 0;
  vtkSelectionNode* selNode =
    (this->Selection && this->Selection->GetNumberOfNodes() > 0) ? this->Selection->GetNode(0) : 0;
  if (selNode)
  {
    fieldType = selNode->GetFieldType();
  }
  return fieldType;
}

void vtkHydroModelSelectionSource::InvertSelection(int vtkNotUsed(ignored))
{
  vtkSelectionNode* selNode =
    (this->Selection && this->Selection->GetNumberOfNodes() > 0) ? this->Selection->GetNode(0) : 0;
  if (selNode)
  {
    vtkInformation* oProperties = selNode->GetProperties();
    int insideout = oProperties->Get(vtkSelectionNode::INVERSE());
    oProperties->Set(vtkSelectionNode::INVERSE(), !insideout);
    this->Modified();
  }
}

void vtkHydroModelSelectionSource::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Source: " << this->Source << "\n";
}

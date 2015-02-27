/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkContourPointCollection.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkContourPointCollection.h"
#include "vtkObjectFactory.h"
#include "vtkDebugLeaks.h"
#include "vtkDebugLeaksManager.h"

#include "vtkPoints.h"
#include "vtkMergePoints.h"

#include <map>
#include <set>

namespace
{
  //the set stores contour source id's that this end node is used by
  typedef  std::map<vtkIdType,std::set<vtkIdType> > vtkInternalMapBase;
  typedef  std::set<vtkIdType> vtkInternalSetBase;
}

class vtkContourPointCollection::vtkInternalMap : public vtkInternalMapBase {};
class vtkContourPointCollection::vtkInternalSet : public vtkInternalSetBase {};

//----------------------------------------------------------------------------

vtkContourPointCollection* vtkContourPointCollection::Instance = 0;
vtkContourPointCollectionCleanup vtkContourPointCollection::Cleanup;

//-----------------------------------------------------------------------------
vtkContourPointCollectionCleanup::vtkContourPointCollectionCleanup()
{
}
//-----------------------------------------------------------------------------
vtkContourPointCollectionCleanup::~vtkContourPointCollectionCleanup()
{
  vtkContourPointCollection::SetInstance(NULL);
}

//-----------------------------------------------------------------------------
vtkContourPointCollection::vtkContourPointCollection()
{
  this->Points = vtkPoints::New();
  this->PointLocator = vtkMergePoints::New();
  this->EndNodes = new vtkContourPointCollection::vtkInternalMap();
  this->RegisteredContourIds = new vtkContourPointCollection::vtkInternalSet();
  this->InitLocator();
}

//-----------------------------------------------------------------------------
vtkContourPointCollection::~vtkContourPointCollection()
{
  this->Points->Delete();
  this->PointLocator->Delete();
  delete this->EndNodes;
  delete this->RegisteredContourIds;
}

//-----------------------------------------------------------------------------
void vtkContourPointCollection::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "vtkContourPointCollection Single instance = "
     << static_cast<void*>(vtkContourPointCollection::Instance) << endl;
}

//-----------------------------------------------------------------------------
// Up the reference count so it behaves like New
vtkContourPointCollection* vtkContourPointCollection::New()
{
  vtkContourPointCollection* ret = vtkContourPointCollection::GetInstance();
  ret->Register(NULL);
  return ret;
}

//-----------------------------------------------------------------------------
// Return the single instance of the vtkContourPointCollection
vtkContourPointCollection* vtkContourPointCollection::GetInstance()
{
  if(!vtkContourPointCollection::Instance)
    {
    // Try the factory first
    vtkContourPointCollection::Instance = static_cast<vtkContourPointCollection*>
      (vtkObjectFactory::CreateInstance("vtkContourPointCollection"));
    // if the factory did not provide one, then create it here
    if(!vtkContourPointCollection::Instance)
      {
      // if the factory failed to create the object,
      // then destroy it now, as vtkDebugLeaks::ConstructClass was called
      // with "vtkContourPointCollection", and not the real name of the class
      vtkContourPointCollection::Instance = new vtkContourPointCollection;
      }
    }
  // return the instance
  return vtkContourPointCollection::Instance;
}

//-----------------------------------------------------------------------------
void vtkContourPointCollection::SetInstance(vtkContourPointCollection* instance)
{
  if (vtkContourPointCollection::Instance==instance)
    {
    return;
    }
  // preferably this will be NULL
  if (vtkContourPointCollection::Instance)
    {
    vtkContourPointCollection::Instance->Delete();
    }
  vtkContourPointCollection::Instance = instance;
  if (!instance)
    {
    return;
    }
  // user will call ->Delete() after setting instance
  instance->Register(NULL);
}

//-----------------------------------------------------------------------------
void vtkContourPointCollection::ResetContourPointCollection( )
{
  this->Points->Delete();
  this->PointLocator->Delete();
  delete this->EndNodes;
  delete this->RegisteredContourIds;

  this->Points = vtkPoints::New();
  this->PointLocator = vtkMergePoints::New();
  this->EndNodes = new vtkContourPointCollection::vtkInternalMap();
  this->RegisteredContourIds = new vtkContourPointCollection::vtkInternalSet();
  this->InitLocator();
}

//-----------------------------------------------------------------------------
void vtkContourPointCollection::InitLocator()
{
  //have to set the tolerance before doing init
  if ( this->PointLocator->GetPoints() != this->Points
    && this->Points != NULL)
    {
    this->PointLocator->SetDivisions(100,100,5);
    this->PointLocator->SetNumberOfPointsPerBucket( 1024 );
    this->PointLocator->InitPointInsertion(this->Points,
      this->Points->GetBounds());
    }
}

//-----------------------------------------------------------------------------
void vtkContourPointCollection::SetPoints(vtkPoints *points)
{
  if (this->Points != points)
    {
    vtkPoints* temp = this->Points;
    this->Points = points;
    if (this->Points != NULL)
      {
      this->Points->Register(this);
      }
    if (temp != NULL)
      {
      temp->UnRegister(this);
      }
    this->Modified();
    }
}

//-----------------------------------------------------------------------------
bool vtkContourPointCollection::IsEndNode( const vtkIdType &id ) const
{
  return (this->EndNodes->find(id) != this->EndNodes->end());
}

//-----------------------------------------------------------------------------
void vtkContourPointCollection::SetAsEndNode( const vtkIdType &id,
                                             const vtkIdType &contourId )
{
  if (!this->IsEndNode(id))
    {
    this->EndNodes->insert(
      std::pair<vtkIdType,std::set<vtkIdType> >(id,std::set<vtkIdType>()));
    }
  this->EndNodes->find(id)->second.insert(contourId);
}

//-----------------------------------------------------------------------------
void vtkContourPointCollection::RemoveAsEndNode( const vtkIdType &id,
                                                const vtkIdType &contourId)
{
  if (this->IsEndNode(id))
    {
    this->EndNodes->find(id)->second.erase(contourId);
    if (this->EndNodes->find(id)->second.size() == 0)
      {
      //remove the end node
      this->EndNodes->erase(id);
      }
    }
}

//-----------------------------------------------------------------------------
int vtkContourPointCollection::NumberOfContoursUsingEndNode( const vtkIdType &id ) const
{
  if (!this->IsEndNode(id))
    {
    return 0;
    }
  return static_cast<int>(this->EndNodes->find(id)->second.size());
}

//-----------------------------------------------------------------------------
std::set<vtkIdType> vtkContourPointCollection::ContoursUsingEndNode(const vtkIdType &id) const
{
  if (!this->IsEndNode(id))
    {
    return std::set<vtkIdType>();
    }
  return this->EndNodes->find(id)->second;
}

//-----------------------------------------------------------------------------
void vtkContourPointCollection::RegisterContour( const vtkIdType &id )
{
  this->RegisteredContourIds->insert(id);
}

//-----------------------------------------------------------------------------
void vtkContourPointCollection::UnRegisterContour( const vtkIdType &id )
{
  this->RegisteredContourIds->erase(id);
  if( this->RegisteredContourIds->size() == 0 )
    {
    this->ResetContourPointCollection();
    }
}


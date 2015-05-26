//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkContourPointCollection - base class for writing debug output to a console
// .SECTION Description
// This class is used to encapsulate all text output, so that it will work
// with operating systems that have a stdout and stderr, and ones that
// do not.  (i.e windows does not).  Sub-classes can be provided which can
// redirect the output to a window.

#ifndef __vtkContourPointCollection_h
#define __vtkContourPointCollection_h

#include "vtkCMBGeneralModule.h" // For export macro
#include <set>
#include "vtkObject.h"
#include "cmbSystemConfig.h"

class vtkPoints;
class vtkMergePoints;

//BTX
class VTKCMBGENERAL_EXPORT vtkContourPointCollectionCleanup
{
public:
  vtkContourPointCollectionCleanup();
  ~vtkContourPointCollectionCleanup();
};
//ETX

class VTKCMBGENERAL_EXPORT vtkContourPointCollection : public vtkObject
{
public:
// Methods from vtkObject
  vtkTypeMacro(vtkContourPointCollection,vtkObject);
  // Description:
  // Print ObjectFactor to stream.
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // This is a singleton pattern New.  There will only be ONE
  // reference to a vtkContourPointCollection object per process.  Clients that
  // call this must call Delete on the object so that the reference
  // counting will work.   The single instance will be unreferenced when
  // the program exits.
  static vtkContourPointCollection* New();
  // Description:
  // Return the singleton instance with no reference counting.
  static vtkContourPointCollection* GetInstance();
  // Description:
  // Supply a user defined output window. Call ->Delete() on the supplied
  // instance after setting it.
  static void SetInstance(vtkContourPointCollection *instance);

//BTX
  // use this as a way of memory management when the
  // program exits the SmartPointer will be deleted which
  // will delete the Instance singleton
  static vtkContourPointCollectionCleanup Cleanup;
//ETX

  //Description:
  //Get the points in the collection
  vtkGetObjectMacro(Points, vtkPoints);

  void SetPoints(vtkPoints *points);

  //Description:
  //Get the locator for the points
  vtkGetObjectMacro(PointLocator, vtkMergePoints);

  void InitLocator();

  //Description:
  //Get if the passed in point id is being stored as an end node
  bool IsEndNode( const vtkIdType &id) const;

  //Description:
  //Set the passed in point id as an end node
  //If this point is already an end node, this increments
  //the number of Contours that this end node is used by
  void SetAsEndNode(const vtkIdType &id, const vtkIdType &contourId);

  //Description:
  //Decrements the number of contours this node
  //is used by. If the number hits zero it will remove
  //the id as an end node.
  void RemoveAsEndNode(const vtkIdType &id, const vtkIdType &contourId);

  //Description:
  //Set the passed in point id as an end node
  //If this point is already an end node, this increments
  //the number of Contours that this end node is used by
  int NumberOfContoursUsingEndNode( const vtkIdType &id ) const;

  //BTX
  //Description:
  //Get the Id's of the arcs that are using this end node
  std::set<vtkIdType> ContoursUsingEndNode(const vtkIdType &id) const;
  //ETX

  //Description:
  //Register a contour with the point collection. The purpose of this
  //is that when the contour count reaches zero we can delete the point
  //locator and recreate it with zero points. We don't do this in
  //RemoveAsEndNode as that is used in the undo/redo stack
  void RegisterContour( const vtkIdType &id );

  //Description:
  //Unregister a contour. When this hits zero we will flush
  //the point locator of all points and recreate it.
  void UnRegisterContour( const vtkIdType &id );


  //Description:
  //Call this to Reset the entire class to a new instance
  //Caution: this will wipe all internal storage of contour points, and
  //the end node id to contour id mapping.
  void ResetContourPointCollection();

protected:
  vtkContourPointCollection();
  virtual ~vtkContourPointCollection();

  vtkPoints* Points;
  vtkMergePoints *PointLocator;

  class vtkInternalMap;
  vtkInternalMap *EndNodes;

  class vtkInternalSet;
  vtkInternalSet *RegisteredContourIds;

private:
  static vtkContourPointCollection* Instance;
private:
  vtkContourPointCollection(const vtkContourPointCollection&);  // Not implemented.
  void operator=(const vtkContourPointCollection&);  // Not implemented.
};

#endif

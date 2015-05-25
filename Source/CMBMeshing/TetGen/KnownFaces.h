//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#ifndef TetGen_KnownFaces_h
#define TetGen_KnownFaces_h

#include <algorithm>
#include <vector>

#include <cassert>

namespace detail
{

//this class doesn't need to be exported as it is an internal implementation
//class for TetGenWorker
struct Face
{
  Face()
  {
    this->p1 = -1;
    this->p2 = -1;
    this->p3 = -1;
    this->faceId=-1;
    this->surfaceId=-1;
  }

  Face(int a, int b, int c)
  {
    //create an array one larger than needed so that we can safely specify
    //the last value of the array as the end position for sort
    int p[4]={a,b,c,-1};
    std::sort(&p[0],&p[3]);
    assert(p[3]==-1); //verify we didn't sort the last value

    this->p1 = p[0];
    this->p2 = p[1];
    this->p3 = p[2];

    this->faceId=-1;
    this->surfaceId=-1;
  }

  Face(int a, int b, int c, int f, int s)
  {
    //create an array one larger than needed so that we can safely specify
    //the last value of the array as the end position for sort
    int p[4]={a,b,c,-1};
    std::sort(&p[0],&p[3]);
    assert(p[3]==-1); //verify we didn't sort the last value

    this->p1 = p[0];
    this->p2 = p[1];
    this->p3 = p[2];

    this->faceId=f;
    this->surfaceId=s;
  }

  bool valid() const { return (faceId != -1); }

  //comparison operator
  bool operator <(Face b) const
    {
      return ((this->p1 < b.p1) ||
             (this->p1 == b.p1 && this->p2 < b.p2) ||
             (this->p1 == b.p1 && this->p2 == b.p2 && this->p3 < b.p3));
    }


  bool operator ==(Face b) const
    { return (p1 == b.p1) && (p2 == b.p2) && (p3 == b.p3); }

  int p1,p2,p3; //point ids, sorted from smallest to largest
  int faceId; //face id
  int surfaceId; //surface id
};

//this class doesn't need to be exported as it is an internal implementation
//class for TetGenWorker
struct KnownFaces
{

  //Insert a face into the vector of known faces. Does zero checks to verify
  //that the face being added doesn't already exist in the vector
  bool add( Face f )
    {
    typedef std::vector< Face >::iterator It;
    It loc = std::lower_bound( this->StoredFaces.begin(),
                               this->StoredFaces.end(),
                               f );
    //insert the face
    this->StoredFaces.insert( loc, f );
    return true;
    }

  bool exists( Face f ) const
    { //the vector is sorted so we use binary_search for fast exist check
    return std::binary_search(this->StoredFaces.begin(),this->StoredFaces.end(),f);
    }

  Face get( Face f ) const
    {
    typedef std::vector< Face >::const_iterator cIt;
    cIt first = std::lower_bound(this->StoredFaces.begin(),this->StoredFaces.end(),f);
    if (!(first == this->StoredFaces.end()) && (f == *first))
      { //return first since it has the correct face and surface ids
      return *first;
      }
    return Face();
    }

private:
  //keep a sorted list of faces, basically a map, but faster
  std::vector< Face > StoredFaces;
};

};

#endif
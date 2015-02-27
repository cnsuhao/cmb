//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//=============================================================================

#ifndef TetGen_VTKConverters_h
#define TetGen_VTKConverters_h

#include "vtkTypeTraits.h"
#include "vtkSetGet.h"


namespace detail
{
//----------------------------------------------------------------------------
template< typename ValueType>
inline bool writeToStream(std::ostream& buffer, ValueType* begin, ValueType* end,
                   vtkIdType numTuples, vtkIdType numComponents )
{
  const std::size_t expectedSize(numTuples*numComponents);
  if( std::distance(begin,end) != expectedSize )
    { return false; }

  //we have contiguous storage
  char* csrc = reinterpret_cast<char*>(begin);
  const std::streamsize size = sizeof(ValueType)*(numTuples*numComponents);

  buffer << vtkTypeTraits<ValueType>::VTKTypeID() << std::endl;
  buffer << numTuples << std::endl;
  buffer << numComponents << std::endl;
  buffer.write(csrc,size);
  buffer << std::endl;

  return true;
}

//----------------------------------------------------------------------------
template<typename VTKType, typename DesiredType>
std::vector<DesiredType> makeAndConvertDataArryFromStream(VTKType,
                                                          DesiredType,
                                                          std::istringstream& buffer,
                                                          vtkIdType numTuples, vtkIdType numComponents)
{
  typedef typename std::vector<DesiredType>::iterator DIteratorType;
  typedef typename std::vector<VTKType>::iterator VIteratorType;

  if(buffer.peek()=='\n')
    {buffer.get();}

  const std::size_t numElements(numTuples*numComponents);
  const std::streamsize size = sizeof(VTKType)*numElements;

  std::vector<DesiredType> result;
  result.resize(numElements);

  char* cdest = NULL;
  if( vtkTypeTraits<VTKType>::VTKTypeID() ==
      vtkTypeTraits<DesiredType>::VTKTypeID())
    {
    char* cdest = reinterpret_cast<char*>(&result[0]);
    buffer.read(cdest,size);
    }
  else
    {
    //we have to make a copy first
    std::vector<VTKType> temp_raw;
    temp_raw.resize(numElements);
    char* cdest = reinterpret_cast<char*>(&temp_raw[0]);
    buffer.read(cdest,size);

    //now copy the values over the result vector, this is slow
    DIteratorType dest = result.begin();
    for(VIteratorType i=temp_raw.begin(); i != temp_raw.end(); ++i, ++dest)
      {
      *dest = static_cast<DesiredType>(*i);
      }
    }
  return result;
}

//----------------------------------------------------------------------------
template<typename T>
std::vector<T> readAndConvert_vtkDataArray(std::istringstream& buffer)
{
  int vtkDataType;
  vtkIdType numTuples, numComponents;
  buffer >> vtkDataType;
  buffer >> numTuples;
  buffer >> numComponents;

  std::vector<T> r;
  switch(vtkDataType)
    {
    vtkTemplateMacro(r = makeAndConvertDataArryFromStream(VTK_TT(), T(),
                            buffer, numTuples, numComponents));
    }
  return r;
}

}
#endif
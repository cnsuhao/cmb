//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkCMBPt123Reader - "reader" for the pt123 formats
// .SECTION Description - Functions common to many cmb readers
//


#ifndef __vtkCMBReaderHelperFunctions_h
#define __vtkCMBReaderHelperFunctions_h

#include <vtksys/SystemTools.hxx>
#include <fstream>
#include <sstream>
#include <string>
#include "cmbSystemConfig.h"
//BTX
namespace ReaderHelperFunctions
{
  bool readNextLine(std::ifstream& file, std::stringstream& line);
  bool readNextLine(std::ifstream& file, std::stringstream& line, std::string& card);
}
//ETX
#endif

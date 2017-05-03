//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME Represents the spatial unit of a scene unit.
// .SECTION Description
// .SECTION Caveats

#ifndef __cmbSceneUnits_h
#define __cmbSceneUnits_h

#include "cmbAppCommonExport.h"
#include "cmbSystemConfig.h"
#include <string>

class CMBAPPCOMMON_EXPORT cmbSceneUnits
{
public:
  enum Enum
  {
    Unknown = 0,
    inches,
    feet,
    mm,
    cm,
    m,
    km
  };

  static const double ConvertFromTo[7][7];
  static Enum convertFromString(const char* unit);
  static std::string convertToString(Enum unit);
};

#endif /* __cmbSceneUnits_h */

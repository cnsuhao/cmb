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

#include "cmbSceneUnits.h"

//-----------------------------------------------------------------------------
const double cmbSceneUnits::ConvertFromTo[7][7] =
{{1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
 {1.0, 1.0, 1.0/12.0, 25.4, 2.54, 0.0254, 0.0000254},
 {1.0, 12.0, 1.0, 12.0*25.4, 12.0*2.54, 12.0*0.0254, 12.0*0.0000254},
 {1.0, 1.0/25.4, 1.0/(12.0*25.4), 1.0, 0.1, 0.001, 0.000001},
 {1.0, 1.0/2.54, 1.0/(12.0*2.54), 10.0, 1.0, 0.01, 0.00001},
 {1.0, 100.0/2.54, 100.0/(12.0*2.54), 1000.0, 100.0, 1.0, 0.001},
 {1.0, 100000.0/2.54, 100000.0/(12.0*2.54), 1000000.0, 100000.0, 1000.0, 1.0}};


//-----------------------------------------------------------------------------
cmbSceneUnits::Enum cmbSceneUnits::convertFromString(const char *unit)
{
  std::string s = unit;
  if (s == "inches")
    {
    return cmbSceneUnits::inches;
    }

  if (s == "feet")
    {
    return cmbSceneUnits::feet;
    }

  if (s == "mm")
    {
    return cmbSceneUnits::mm;
    }

  if (s == "cm")
    {
    return cmbSceneUnits::cm;
    }

  if (s == "m")
    {
    return cmbSceneUnits::m;
    }

  if (s == "km")
    {
    return cmbSceneUnits::km;
    }

  return cmbSceneUnits::Unknown;
}

//-----------------------------------------------------------------------------
std::string cmbSceneUnits::convertToString(cmbSceneUnits::Enum unit)
{
  switch (unit)
    {
    case cmbSceneUnits::inches:
      return "inches";
    case cmbSceneUnits::feet:
      return "feet";
    case cmbSceneUnits::mm:
      return "mm";
    case cmbSceneUnits::cm:
      return "cm";
    case cmbSceneUnits::m:
      return "m";
    case cmbSceneUnits::km:
      return "km";
    default:
      return "Unknown";
    }
  return "Unknown";
}
//-----------------------------------------------------------------------------

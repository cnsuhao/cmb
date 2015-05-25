//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkDateFormatter.h"

#include "vtkDateTime.h"
#include "vtkObjectFactory.h"

#include "string"

#include <sstream>
#include <cstring> // strlen()
#include <limits>


//-----------------------------------------------------------------------------
// LOCALE_ constants need to be defined for non-WIN32 compiles...


// LCID values:
#define LOCALE_SYSTEM_DEFAULT 2048
#define LOCALE_USER_DEFAULT 1024
#define LOCALE_NEUTRAL 0
#define LOCALE_INVARIANT 127

// LCTYPE values: (Pre-defined date/time related control panel setting IDs)
#define LOCALE_SDATE                  0x0000001D   // date separator
#define LOCALE_STIME                  0x0000001E   // time separator
#define LOCALE_SSHORTDATE             0x0000001F   // short date format string
#define LOCALE_SLONGDATE              0x00000020   // long date format string
#define LOCALE_STIMEFORMAT            0x00001003   // time format string
#define LOCALE_IDATE                  0x00000021   // short date format ordering
#define LOCALE_ILDATE                 0x00000022   // long date format ordering
#define LOCALE_ITIME                  0x00000023   // time format specifier
#define LOCALE_ITIMEMARKPOSN          0x00001005   // time marker position
#define LOCALE_ICENTURY               0x00000024   // century format specifier (short date)
#define LOCALE_ITLZERO                0x00000025   // leading zeros in time field
#define LOCALE_IDAYLZERO              0x00000026   // leading zeros in day field (short date)
#define LOCALE_IMONLZERO              0x00000027   // leading zeros in month field (short date)
#define LOCALE_S1159                  0x00000028   // AM designator
#define LOCALE_S2359                  0x00000029   // PM designator
#define W3_UTCDATETIME                0x0000002A   // W3 recognized date time.


//-----------------------------------------------------------------------------
std::string
GetControlPanelSetting(int /*lcid*/, int setting)
{
  std::string s;

  switch (setting)
    {
    case LOCALE_SDATE:
      s = "/";
    break;

    case LOCALE_STIME:
      s = ":";
    break;

    // equivalent of C# default value
    // System.Globalization.DateTimeFormatInfo.CurrentInfo.ShortDatePattern:
    case LOCALE_SSHORTDATE:
      s = "M/d/yyyy";
    break;

    // equivalent of C# default value
    // System.Globalization.DateTimeFormatInfo.CurrentInfo.LongDatePattern:
    case LOCALE_SLONGDATE:
      s = "dddd, MMMM dd, yyyy";
    break;

    // equivalent of C# default value
    // System.Globalization.DateTimeFormatInfo.CurrentInfo.LongTimePattern:
    case LOCALE_STIMEFORMAT:
      s = "h:mm:ss tt";
    break;

    case LOCALE_S1159:
      s = "AM";
    break;

    case LOCALE_S2359:
      s = "PM";
    break;

    case W3_UTCDATETIME:
      s = "yyyy-MM-ddThh::mm::ssZ";
    break;

    default:
      std::ostringstream oss;
      oss << "error: GetControlPanelSetting is not implemented for setting='"
        << setting << "' on this platform...";
      s = oss.str();
    break;
    }


  return s;
}

//-----------------------------------------------------------------------------
int FormatStringLCID = LOCALE_USER_DEFAULT;

std::string LongTimeFormatString;
std::string ShortTimeFormatString;
std::string LongDateFormatString;
std::string ShortDateFormatString;
std::string W3UTCDateTimeFormatString;
std::string TimeAMString;
std::string TimePMString;

//-----------------------------------------------------------------------------
int vtkDateFormatter::GetFormatStringLCID()
{
  return FormatStringLCID;
}

//-----------------------------------------------------------------------------
void vtkDateFormatter::SetFormatStringLCID(int lcid)
{
  FormatStringLCID = lcid;
}


//-----------------------------------------------------------------------------
std::string vtkDateFormatter::GetLongTimeFormatString()
{
  std::string s(LongTimeFormatString);

  if (s.empty())
    {
    s = GetControlPanelSetting(GetFormatStringLCID(), LOCALE_STIMEFORMAT);
    }

  return s;
}

//-----------------------------------------------------------------------------
void vtkDateFormatter::SetLongTimeFormatString(const char* format)
{
  LongTimeFormatString = format;
}

//-----------------------------------------------------------------------------
std::string vtkDateFormatter::GetShortTimeFormatString()
{
  std::string s(ShortTimeFormatString);

  if (s.empty())
    {
    // No LOCALE_ constant for short time...
    // There is no "short time" control panel setting...
    // Use a hard-coded default (that happens to be the same as the C# default
    // System.Globalization.DateTimeFormatInfo.CurrentInfo.ShortTimePattern)
    //
    s = "h:mm tt";
    }

  return s;
}

//-----------------------------------------------------------------------------
void vtkDateFormatter::SetShortTimeFormatString(const char* format)
{
  ShortTimeFormatString = format;
}

//-----------------------------------------------------------------------------
std::string vtkDateFormatter::GetLongDateFormatString()
{
  std::string s(LongDateFormatString);

  if (s.empty())
    {
    s = GetControlPanelSetting(GetFormatStringLCID(), LOCALE_SLONGDATE);
    }

  return s;
}

//-----------------------------------------------------------------------------
void vtkDateFormatter::SetLongDateFormatString(const char* format)
{
  LongDateFormatString = std::string(format);
}

//-----------------------------------------------------------------------------
std::string vtkDateFormatter::GetShortDateFormatString()
{
  std::string s(ShortDateFormatString);

  if (s.empty())
    {
    s = GetControlPanelSetting(GetFormatStringLCID(), LOCALE_SSHORTDATE);
    }

  return s;
}

//-----------------------------------------------------------------------------
std::string vtkDateFormatter::GetW3UTCDateTimeFormatString()
{
  std::string s(W3UTCDateTimeFormatString);

  if (s.empty())
    {
    s = GetControlPanelSetting(GetFormatStringLCID(), W3_UTCDATETIME);
    }

  return s;
}

//-----------------------------------------------------------------------------
void vtkDateFormatter::SetW3UTCDateTimeFormatString(const char* format)
{
  W3UTCDateTimeFormatString = std::string(format);
}

//-----------------------------------------------------------------------------
void vtkDateFormatter::SetShortDateFormatString(const char* format)
{
  ShortDateFormatString = std::string(format);
}

//-----------------------------------------------------------------------------
std::string vtkDateFormatter::GetTimeAMString()
{
  std::string s(TimeAMString);

  if (s.empty())
    {
    s = GetControlPanelSetting(GetFormatStringLCID(), LOCALE_S1159);
    }

  return s;
}

//-----------------------------------------------------------------------------
void vtkDateFormatter::SetTimeAMString(const char* value)
{
  TimeAMString = std::string(value);
}

//-----------------------------------------------------------------------------
std::string vtkDateFormatter::GetTimePMString()
{
  std::string s(TimePMString);

  if (s.empty())
    {
    s = GetControlPanelSetting(GetFormatStringLCID(), LOCALE_S2359);
    }

  return s;
}

//-----------------------------------------------------------------------------
void vtkDateFormatter::SetTimePMString(const char* value)
{
  TimePMString = std::string(value);
}

//-----------------------------------------------------------------------------
static std::string GetShortWeekDayName(int dow)
{
  std::string s;

  switch(dow)
    {
    case 0: s = "Sun"; break;
    case 1: s = "Mon"; break;
    case 2: s = "Tue"; break;
    case 3: s = "Wed"; break;
    case 4: s = "Thu"; break;
    case 5: s = "Fri"; break;
    case 6: s = "Sat"; break;
    default: s = "error: no such day of week"; break;
    }

  return s;
}

//-----------------------------------------------------------------------------
static std::string GetLongWeekDayName(int dow)
{
  std::string s;

  switch(dow)
    {
    case 0: s = "Sunday"; break;
    case 1: s = "Monday"; break;
    case 2: s = "Tuesday"; break;
    case 3: s = "Wednesday"; break;
    case 4: s = "Thursday"; break;
    case 5: s = "Friday"; break;
    case 6: s = "Saturday"; break;
    default: s = "error: no such day of week"; break;
    }

  return s;
}

//-----------------------------------------------------------------------------
static std::string GetShortMonthName(int month)
{
  std::string s;

  switch(month)
    {
    case  1: s = "Jan"; break;
    case  2: s = "Feb"; break;
    case  3: s = "Mar"; break;
    case  4: s = "Apr"; break;
    case  5: s = "May"; break;
    case  6: s = "Jun"; break;
    case  7: s = "Jul"; break;
    case  8: s = "Aug"; break;
    case  9: s = "Sep"; break;
    case 10: s = "Oct"; break;
    case 11: s = "Nov"; break;
    case 12: s = "Dec"; break;
    default: s = "error: no such month"; break;
    }

  return s;
}

//-----------------------------------------------------------------------------
static std::string GetLongMonthName(int month)
{
  std::string s;

  switch(month)
    {
    case  1: s = "January"; break;
    case  2: s = "February"; break;
    case  3: s = "March"; break;
    case  4: s = "April"; break;
    case  5: s = "May"; break;
    case  6: s = "June"; break;
    case  7: s = "July"; break;
    case  8: s = "August"; break;
    case  9: s = "September"; break;
    case 10: s = "October"; break;
    case 11: s = "November"; break;
    case 12: s = "December"; break;
    default: s = "error: no such month"; break;
    }

  return s;
}

//-----------------------------------------------------------------------------
// For later, retrieve user chosen values from Control Panel using the Windows
// API functions...
//
//-----------------------------------------------------------------------------
// WIN32 Platform SDK info:
//
// GetLocaleInfo (or GetLocaleInfoEx, Vista and later only) gets formatting
// string from Control Panel settings...
//   http://msdn2.microsoft.com/en-us/library/ms776270.aspx
//   http://msdn2.microsoft.com/en-us/library/ms776365.aspx
//
// LCTYPE constants
//   http://msdn2.microsoft.com/en-us/library/bb507201.aspx
// (LOCALE_SLONGDATE, LOCALE_SSHORTDATE, LOCALE_STIME, LOCALE_STIMEFORMAT)
//
// GetDateFormat - formats a date using a formatting string...
//   http://msdn2.microsoft.com/en-us/library/ms776293.aspx
// (GetDateFormatEx, Vista and later only)
//
// GetTimeFormat - formats a time using a formatting string...
//   http://msdn2.microsoft.com/en-us/library/ms776299.aspx
// (GetTimeFormatEx, Vista and later only)
//

//-----------------------------------------------------------------------------
static int CountRepeatedChars(const char *c, int i , int n)
{
  // Given a string (c), an initial offset into it (i) and its length (n),
  // return the number of times that c[i] occurs in a row. 1 if c[i+1] is
  // different than c[i], 2 if c[i+2] is different, etc.
  //
  int j = i + 1;
  while (j < n && c[i] == c[j])
    {
    ++j;
    }
  return j - i;
}

//-----------------------------------------------------------------------------
std::string vtkDateFormatter::FormatDate(const vtkDateTime& date,
                                            const char* format)
{
  // For more info on .NET System.DateTime format strings, see:
  // http://msdn2.microsoft.com/en-us/library/system.globalization.datetimeformatinfo.aspx

  // Handle special cases first, paying no attention to "format":
  //
  if (date.GetIsNotADate())
    {
    return "NotADate";
    }
  else if (date.GetIsNegativeInfinity())
    {
    return "NegativeInfinity";
    }
  else if (date.GetIsPositiveInfinity())
    {
    return "PositiveInfinity";
    }
  // else... format it according to "format":
  //

  std::string s;
  std::string formatStr(format);
  const char* c = formatStr.c_str();
  int i = 0;
  int n = static_cast<int>(strlen(c));
  char formatChar = 0;
  int repeatCount = 0;
  std::string component;

  for (i= 0; i<n; )
    {
    repeatCount = CountRepeatedChars(c, i , n);
    formatChar = c[i];
    component = formatStr.substr(i, repeatCount);

    s += std::string(FormatDateComponent(date, component.c_str(), formatChar, repeatCount));

    i += repeatCount;
    }

  return s;
}

//-----------------------------------------------------------------------------
static bool IsNumericalComponent(const char* comp)
{
  std::string component(comp);
  // d and M are only numeric for 1 and 2 digit format strings:
  //
  if (
    component == "d" ||
    component == "dd" ||
    component == "M" ||
    component == "MM"
    )
    {
    return true;
    }

  // These are always numeric:
  //
  char formatChar = component.c_str()[0];
  switch(formatChar)
    {
    case 'y':
    case 'h':
    case 'H':
    case 'm':
    case 's':
    case 'f':
    case 'v':
      return true;
    break;
    }

  // No other format chars are numeric:
  //
  return false;
}

//-----------------------------------------------------------------------------
std::string vtkDateFormatter::FormatDateComponent(const vtkDateTime& date,
                                                     const char* component,
                                                     char formatChar,
                                                     int repeatCount)
{

  std::string componentStr (component);
  std::string s;
//  vtkTypeInt64 componentValue = VE_INT64_MIN;
  vtkTypeInt64 componentValue = std::numeric_limits<vtkTypeInt64>::min();
  bool forceUseComponentValue = false;

  switch(formatChar)
    {
    case 'd':
      // Day of month or day of week:
      //
      if (componentStr == "d" || componentStr == "dd")
        {
        componentValue = date.CalcDayOfMonth();
        }
      else if (componentStr == "ddd")
        {
        s = GetShortWeekDayName(date.CalcDayOfWeek());
        }
      else if (componentStr == "dddd")
        {
        s = GetLongWeekDayName(date.CalcDayOfWeek());
        }
    break;

    case 'g':
      // Era designation (CE, BCE)
      if (date.CalcYear() <= 0)
        {
        s = "BCE";
        }
      else
        {
        s = "CE";
        }
    break;

    case 'M':
      // GetMonth
      if (componentStr == "M" || componentStr == "MM")
        {
        componentValue = date.CalcMonth();
        }
      else if (componentStr == "MMM")
        {
        s = GetShortMonthName(date.CalcMonth());
        }
      else if (componentStr == "MMMM")
        {
        s = GetLongMonthName(date.CalcMonth());
        }
    break;

    case 'y':
      // GetYear
      componentValue = date.CalcYear();
    break;

    case 'h':
      // GetHour (12 hour value ---> 1 thru 12)
      componentValue = date.CalcHour() % 12;
      if (0 == componentValue)
        {
        componentValue = 12;
        }
    break;

    case 'H':
      // GetHour (24 hour value ---> 0 thru 23)
      componentValue = date.CalcHour();
    break;

    case 'm':
      // GetMinute
      componentValue = date.CalcMinute();
    break;

    case 's':
      // GetSecond
      componentValue = date.CalcSecond();
    break;

    case 'f':
      // GetMillisecond / 10
      //
      // f == fraction of a second: centiseconds in VE...
      //
      // 'f' should *only* be used like this for VE dates:
      // s.ff
      //
      componentValue = date.CalcMillisecond() / 10;
    break;

    case 't':
      // AM/PM designator
      if (date.GetHour() > 11)
        {
        s = GetTimePMString();
        }
      else
        {
        s = GetTimeAMString();
        }
    break;

    case 'v':
      // GetTicks - 'v' for "VE Ticks"
      componentValue = date.GetTicks();
      forceUseComponentValue = true;
    break;

    default:
      s = componentStr;
    break;
    }

  // If s was not assigned in the above switch statement, then fill
  // it in here based on componentValue and repeatCount:
  //
  if (s.empty())
    {
    if (forceUseComponentValue || (componentValue != std::numeric_limits<vtkTypeInt64>::min()))
      {
      std::ostringstream ossCmp;

      // Use leading zero chars?
      //
      if (repeatCount > 1)
        {
        ossCmp.fill('0');
        ossCmp.width(repeatCount);
        }

      ossCmp << componentValue;

      s = ossCmp.str();
      }
    }

  return s;
}

//-----------------------------------------------------------------------------
vtkDateTime* vtkDateFormatter::ParseDate(const char* format,
                                         const char* rep)
{
  // Handle special cases first, paying no attention to "format":

  const std::string comparableRep(rep);
  if (comparableRep == "NotADate")
    {
    return vtkDateTime::NotADate();
    }
  else if (comparableRep == "NegativeInfinity")
    {
    return vtkDateTime::NegativeInfinity();
    }
  else if (comparableRep == "PositiveInfinity")
    {
    return vtkDateTime::PositiveInfinity();
    }
  // else... parse it according to "format":
  //

  const char* c = format;
  int i = 0;
  int n = static_cast<int>(strlen(c));
  char formatChar = 0;
  int repeatCount = 0;
  std::string reconstructedFormat;
  std::string component;
  vtkTypeInt64 extractedComponent = 0;
  vtkTypeInt64 extractedYear = 1;
  int extractedMonth = 1;
  int extractedDay = 1;
  int extractedHour = 0;
  int extractedMinute = 0;
  int extractedSecond = 0;
  int extractedCentiSecond = 0;
  vtkTypeInt64 extractedTicks = 0;
  bool useTicks = false;
  std::istringstream iss(rep);

  for (i= 0; i<n; )
    {
    repeatCount = CountRepeatedChars(c, i , n);
    formatChar = c[i];
    component = std::string(format).substr(i, repeatCount);

    if (IsNumericalComponent(component.c_str()))
      {
      iss >> extractedComponent;

      switch (formatChar)
        {
        case 'd':
          extractedDay = static_cast<int>(extractedComponent);
        break;
        case 'M':
          extractedMonth = static_cast<int>(extractedComponent);
        break;
        case 'y':
          extractedYear = extractedComponent;
        break;
        case 'h':
        case 'H':
          extractedHour = static_cast<int>(extractedComponent);
        break;
        case 'm':
          extractedMinute = static_cast<int>(extractedComponent);
        break;
        case 's':
          extractedSecond = static_cast<int>(extractedComponent);
        break;
        case 'f':
          extractedCentiSecond = static_cast<int>(extractedComponent);
        break;
        case 'v':
          extractedTicks = extractedComponent;
          useTicks = true;
        break;
        }
      }
    else
      {
      extractedComponent = 0;
      //switch (formatChar)
      //  {
      //  case 'g':
      //  break;
      //  case 't':
      //  break;
      //  default:
          iss.ignore(repeatCount);
      //  break;
      //  }
      }

    i += repeatCount;
    }


  if (useTicks)
    {
    vtkDateTime* d (vtkDateTime::New());
    d->SetTicks(extractedTicks);
    return d;
    }
  else
    {
    vtkDateTime* d = vtkDateTime::New();
    d->SetYear(extractedYear);
    d->SetMonth(extractedMonth);
    d->SetDay(extractedDay);
    d->SetHour(extractedHour);
    d->SetMinute(extractedMinute);
    d->SetSecond(extractedSecond);

    if (extractedCentiSecond != 0)
      {
      if (extractedCentiSecond < 100)
        {
        vtkDateTime* dt = d->CreateNewAddTicks(extractedCentiSecond);
        d->Delete();
        d = dt;
        }
      else
        {
        std::cerr << "Error: vtkDateFormatter::ParseDate: "
          << "invalid value for extracted centisecond - "
          << "valid values for 'ff' are 0 to 99 inclusive" << std::endl;
        }
      }

    return d;
    }
}

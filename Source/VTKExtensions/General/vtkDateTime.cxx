//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================

#include "vtkDateTime.h"

#include "vtkDateFormatter.h"
#include "vtkObjectFactory.h"

#include <cmath> // floor()
#include <sstream> // ostringstream
#include <climits> // UINT_MAX

vtkStandardNewMacro(vtkDateTime);

//-----------------------------------------------------------------------------
class vtkDateTimeInternal
{
public:
  std::string dateTimeStr;
};

//-----------------------------------------------------------------------------
vtkDateTime::vtkDateTime() : vtkObject(),
  Ticks   (0),
  Year    (0),
  Month   (0),
  Day     (0),
  Hour    (0),
  Minute  (0),
  Second  (0),
  IsNegativeInfinity(false),
  IsPositiveInfinity(false),
  Internal(0)
{
  this->Internal = new vtkDateTimeInternal();
}

//-----------------------------------------------------------------------------
vtkDateTime::~vtkDateTime()
{
  if(this->Internal)
    {
    delete this->Internal;
    this->Internal = 0x0;
    }
}

//-----------------------------------------------------------------------------
void vtkDateTime::PrintSelf(ostream &os, vtkIndent indent)
{
  os << indent << "Ticks: "   << this->Ticks  << std::endl;
  os << indent << "Year: "    << this->Year   << std::endl;
  os << indent << "Month: "   << this->Month  << std::endl;
  os << indent << "Day: "     << this->Day    << std::endl;
  os << indent << "Hour: "    << this->Hour   << std::endl;
  os << indent << "Minute: "  << this->Minute << std::endl;
  os << indent << "Second: "  << this->Second << std::endl;
}

//-----------------------------------------------------------------------------
void vtkDateTime::Set(vtkTypeInt64 year, int month, int day, int hour, int min, int sec)
{
  if(year < 0 && month < 0 && day < 0 && hour < 0 && min < 0 && sec < 0)
    {
      vtkErrorMacro("Cannot have negative arguments.");
      return;
    }

  this->Year    = year;
  this->Month   = month;
  this->Day     = day;
  this->Hour    = hour;
  this->Minute  = min;
  this->Second  = sec;

  this->Modified();
}

//-----------------------------------------------------------------------------
vtkTypeInt64 vtkDateTime::GetTicks() const
{
  if (this->IsNegativeInfinity)
    {
    vtkErrorWithObjectMacro(const_cast<vtkDateTime*>(this),
      "GetTicks is meaningless when IsNegativeInfinity is true");

    return -1;
    }

  if (this->IsPositiveInfinity)
    {
    vtkErrorWithObjectMacro(const_cast<vtkDateTime*>(this),
      "GetTicks is meaningless when IsPositiveInfinity is true");
    return -1;
    }

  return this->Ticks;
}

//-----------------------------------------------------------------------------
vtkTypeInt64 vtkDateTime::CalcYear() const
{
  if (this->IsNegativeInfinity || this->IsPositiveInfinity)
    {
    vtkErrorWithObjectMacro(const_cast<vtkDateTime*>(this),
      "GetYear is meaningless for an Infinity vtkDateTime");
    }

  if (this->Ticks < 0)
    {
    return (this->Ticks / 3155692599LL) + 1;
    }

  vtkTypeInt64 y;
  int m, d;
  this->ConvertFromRataDie(y, m, d);
  return y;
}

//-----------------------------------------------------------------------------
int vtkDateTime::CalcMonth() const
{
  ThrowIfInfiniteOrNegative(*this, "GetMonth");

  vtkTypeInt64 y;
  int m, d;
  this->ConvertFromRataDie(y, m, d);

  ThrowIfIntOutOfRange("vtkDateTime::GetMonth", m, 1, 12);

  return m;
}

//-----------------------------------------------------------------------------
int vtkDateTime::CalcDayOfYear() const
{
  ThrowIfInfiniteOrNegative(*this, "GetDayOfYear");

  // RataDie number for this vtkDateTime:
  vtkTypeInt64 rdThis = (this->Ticks / 8640000) + 1;

  // Compute the year of this vtkDateTime:
  vtkTypeInt64 y;
  int m, d;
  this->ConvertFromRataDie(y, m, d);

  // RataDie number for Jan. 1 in this vtkDateTime's year:
  vtkTypeInt64 jan1stSameYear = ConvertToRataDie(y, 1, 1);

  // DayOfYear is "the difference + 1" so that Jan. 1 computes to 1:
  int doy = static_cast<int>(rdThis - jan1stSameYear) + 1;

  ThrowIfIntOutOfRange("vtkDateTime::GetDayOfYear", doy, 1, 366);

  return doy;
}

//-----------------------------------------------------------------------------
int vtkDateTime::CalcDayOfMonth() const
{
  ThrowIfInfiniteOrNegative(*this, "GetDayOfMonth");

  vtkTypeInt64 y;
  int m, d;
  this->ConvertFromRataDie(y, m, d);

  ThrowIfIntOutOfRange("vtkDateTime::GetDayOfMonth", d, 1, 31);

  return d;
}

//-----------------------------------------------------------------------------
int vtkDateTime::CalcDayOfWeek() const
{
  ThrowIfInfiniteOrNegative(*this, "GetDayOfWeek");

  // RataDie number for this vtkDateTime:
  vtkTypeInt64 rdThis = (this->Ticks / 8640000) + 1;

  // 0 == Sunday, 1 == Monday, ... 6 == Saturday
  int dow = static_cast<int>(rdThis % 7);

  ThrowIfIntOutOfRange("vtkDateTime::GetDayOfWeek", dow, 0, 6);

  return dow;
}

//-----------------------------------------------------------------------------
int vtkDateTime::CalcHour() const
{
  ThrowIfInfiniteOrNegative(*this, "GetHour");

  int h = static_cast<int>((this->Ticks / 360000) % 24);

  ThrowIfIntOutOfRange("vtkDateTime::GetHour", h, 0, 23);

  return h;
}

//-----------------------------------------------------------------------------
int vtkDateTime::CalcMinute() const
{
  ThrowIfInfiniteOrNegative(*this, "GetMinute");

  int m = static_cast<int>((this->Ticks / 6000) % 60);

  ThrowIfIntOutOfRange("vtkDateTime::GetMinute", m, 0, 59);

  return m;
}

//-----------------------------------------------------------------------------
int vtkDateTime::CalcSecond() const
{
  ThrowIfInfiniteOrNegative(*this, "GetSecond");

  int s = static_cast<int>((this->Ticks / 100) % 60);

  ThrowIfIntOutOfRange("vtkDateTime::GetSecond", s, 0, 59);

  return s;
}

//-----------------------------------------------------------------------------
int vtkDateTime::CalcMillisecond() const
{
  ThrowIfInfiniteOrNegative(*this, "GetMillisecond");

  int ms = 10 * static_cast<int>(this->Ticks % 100);

  ThrowIfIntOutOfRange("vtkDateTime::GetMillisecond", ms, 0, 999);

  return ms;
}

//-----------------------------------------------------------------------------
bool vtkDateTime::operator==(const vtkDateTime & that) const
{
  return this->Ticks == that.Ticks &&
    this->IsNegativeInfinity == that.IsNegativeInfinity &&
    this->IsPositiveInfinity == that.IsPositiveInfinity;
}

//-----------------------------------------------------------------------------
bool vtkDateTime::operator!=(const vtkDateTime & that) const
{
  return !(*this == that);
}

//-----------------------------------------------------------------------------
bool vtkDateTime::operator<(const vtkDateTime & that) const
{
  if (this->GetIsNotADate() || that.GetIsNotADate())
    {
    return false;
    }

  if (this->IsNegativeInfinity)
    {
    if (that.IsNegativeInfinity)
      {
      return false;
      }

    return true;
    }

  if (this->IsPositiveInfinity)
    {
    return false;
    }

  if (that.IsPositiveInfinity)
    {
    return true;
    }

  if (that.IsNegativeInfinity)
    {
    return false;
    }

  return (this->Ticks < that.Ticks);
}

//-----------------------------------------------------------------------------
bool vtkDateTime::operator<=(const vtkDateTime & that) const
{
  return (*this < that) || (*this == that);
}

//-----------------------------------------------------------------------------
bool vtkDateTime::operator>=(const vtkDateTime & that) const
{
  return (*this > that) || (*this == that);
}

//-----------------------------------------------------------------------------
bool vtkDateTime::operator>(const vtkDateTime & that) const
{
  return (that < *this);
}

//-----------------------------------------------------------------------------
bool vtkDateTime::GetIsInfinite() const
{
  return (this->IsNegativeInfinity || this->IsPositiveInfinity);
}

//-----------------------------------------------------------------------------
bool vtkDateTime::GetIsNotADate() const
{
  return (this->IsNegativeInfinity && this->IsPositiveInfinity);
}

//-----------------------------------------------------------------------------
int vtkDateTime::GetDayOfYear() const
{
  ThrowIfInfiniteOrNegative(*this, "GetDayOfYear");

  // RataDie number for this Date:
  vtkTypeInt64 rdThis = (this->Ticks / 8640000) + 1;

  // Compute the year of this Date:
  vtkTypeInt64 y;
  int m, d;
  this->ConvertFromRataDie(y, m, d);

  // RataDie number for Jan. 1 in this Date's year:
  vtkTypeInt64 jan1stSameYear = ConvertToRataDie(y, 1, 1);

  // DayOfYear is "the difference + 1" so that Jan. 1 computes to 1:
  int doy = static_cast<int>(rdThis - jan1stSameYear) + 1;

  ThrowIfIntOutOfRange("Date::GetDayOfYear", doy, 1, 366);

  return doy;
}

//-----------------------------------------------------------------------------
int vtkDateTime::GetDayOfMonth() const
{
  ThrowIfInfiniteOrNegative(*this, "GetDayOfMonth");

  vtkTypeInt64 y;
  int m, d;
  this->ConvertFromRataDie(y, m, d);

  ThrowIfIntOutOfRange("Date::GetDayOfMonth", d, 1, 31);

  return d;
}

//-----------------------------------------------------------------------------
int vtkDateTime::GetDayOfWeek() const
{
  ThrowIfInfiniteOrNegative(*this, "GetDayOfWeek");

  // RataDie number for this Date:
  vtkTypeInt64 rdThis = (this->Ticks / 8640000) + 1;

  // 0 == Sunday, 1 == Monday, ... 6 == Saturday
  int dow = static_cast<int>(rdThis % 7);

  ThrowIfIntOutOfRange("Date::GetDayOfWeek", dow, 0, 6);

  return dow;
}

//-----------------------------------------------------------------------------
int vtkDateTime::GetMillisecond() const
{
  ThrowIfInfiniteOrNegative(*this, "GetMillisecond");

  int ms = 10 * static_cast<int>(this->Ticks % 100);

  ThrowIfIntOutOfRange("Date::GetMillisecond", ms, 0, 999);

  return ms;
}

//-----------------------------------------------------------------------------
int vtkDateTime::ThrowIfInfiniteOrNegative(const vtkDateTime& dateTime,
                                           const char* method)
{
  if (dateTime.GetIsNegativeInfinity() || dateTime.GetIsPositiveInfinity())
    {
    std::ostringstream oss;
    oss << std::string(method) << " is meaningless for an Infinity vtkDateTime"
      << std::ends;
    std::cerr << oss.str() << std::endl;
    return 0;
    }

  if (dateTime.GetTicks() < 0)
    {
    std::ostringstream oss;
    oss << method << " is meaningless for negative Ticks values" << std::ends;
    std::cerr << oss.str() << std::endl;
    return 0;
    }

  return 1;
}

//-----------------------------------------------------------------------------
int vtkDateTime::ThrowIfIntOutOfRange(const char* context,
                                      int value,
                                      int Minute,
                                      int max)
{
  if (value < Minute || value > max)
    {
    std::ostringstream oss;
    oss << "error: value is outside expected range. (" << context << ")"
      << std::endl;
    oss << "  value: " << value << std::endl;
    oss << "  Minute: " << Minute << std::endl;
    oss << "  max: " << max << std::endl;
    oss << std::ends;
    std::cerr << oss.str() << std::endl;
    return 0;
    }
  return 1;
}

//-----------------------------------------------------------------------------
void vtkDateTime::SetToNegativeInfinity()
{
  this->Ticks = 0;
  this->IsNegativeInfinity = true;
  this->IsPositiveInfinity = false;

  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkDateTime::SetToPositiveInfinity()
{
  this->Ticks = 0;
  this->IsNegativeInfinity = false;
  this->IsPositiveInfinity = true;

  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkDateTime::SetToNotADate()
{
  this->Ticks = 0;
  this->IsNegativeInfinity = true;
  this->IsPositiveInfinity = true;

  this->Modified();
}

//-----------------------------------------------------------------------------
const char* vtkDateTime::ToLongTimeString() const
{
  std::string format(vtkDateFormatter::GetLongTimeFormatString());
  return this->ToString(format.c_str());
}

//-----------------------------------------------------------------------------
const char* vtkDateTime::ToShortTimeString() const
{
  std::string format(vtkDateFormatter::GetShortTimeFormatString());
  return this->ToString(format.c_str());
}

//-----------------------------------------------------------------------------
const char* vtkDateTime::ToLongDateString() const
{
  std::string format(vtkDateFormatter::GetLongDateFormatString());
  return this->ToString(format.c_str());
}

//-----------------------------------------------------------------------------
const char* vtkDateTime::ToShortDateString() const
{
  std::string format(vtkDateFormatter::GetShortDateFormatString());
  return this->ToString(format.c_str());
}

//-----------------------------------------------------------------------------
const char* vtkDateTime::ToW3UTCDateTimeString() const
{
  std::string format(vtkDateFormatter::GetW3UTCDateTimeFormatString());
  return this->ToString(format.c_str());
}

//-----------------------------------------------------------------------------
const char* vtkDateTime::ToString(const char* format) const
{
  this->Internal->dateTimeStr = vtkDateFormatter::FormatDate(*this, format);
  return this->Internal->dateTimeStr.c_str();
}

//-----------------------------------------------------------------------------
vtkDateTime* vtkDateTime::Parse(const char* format, const char* rep)
{
  return vtkDateFormatter::ParseDate(format, rep);
}

//-----------------------------------------------------------------------------
vtkDateTime* vtkDateTime::CreateNewAddYears(vtkTypeInt64 delta) const
{
  return CreateNewAddTicks(3155692599LL*delta);
}

//-----------------------------------------------------------------------------
vtkDateTime* vtkDateTime::CreateNewAddDays(vtkTypeInt64 delta) const
{
  return CreateNewAddTicks(8640000*delta);
}

//-----------------------------------------------------------------------------
vtkDateTime* vtkDateTime::CreateNewAddHours(vtkTypeInt64 delta) const
{
  return CreateNewAddTicks(360000*delta);
}

//-----------------------------------------------------------------------------
vtkDateTime* vtkDateTime::CreateNewAddMinutes(vtkTypeInt64 delta) const
{
  return CreateNewAddTicks(6000*delta);
}

//-----------------------------------------------------------------------------
vtkDateTime* vtkDateTime::CreateNewAddSeconds(vtkTypeInt64 delta) const
{
  return CreateNewAddTicks(100*delta);
}

//-----------------------------------------------------------------------------
vtkDateTime* vtkDateTime::CreateNewAddTicks(vtkTypeInt64 delta) const
{
  if (this->IsNegativeInfinity || this->IsPositiveInfinity)
    {
    vtkErrorWithObjectMacro(const_cast<vtkDateTime*>(this),
      "CreateNewAddTicks cannot be used on an Infinity vtkDateTime");
    }

  vtkDateTime* d (vtkDateTime::New());
  d->DeepCopy(this);
  d->Ticks += delta;
  return d;
}

//-----------------------------------------------------------------------------
int vtkDateTime::DeepCopy(const vtkDateTime* src)
{
  if(!src)
    {
    return 0;
    }

  this->Ticks   = src->GetTicks();
  this->Year    = src->GetYear();
  this->Month   = src->GetMonth();
  this->Day     = src->GetDay();
  this->Hour    = src->GetHour();
  this->Minute  = src->GetMinute();
  this->Second  = src->GetSecond();

  this->IsNegativeInfinity = src->GetIsNegativeInfinity();
  this->IsPositiveInfinity = src->GetIsPositiveInfinity();

  this->Modified();

  return 1;
}

//-----------------------------------------------------------------------------
vtkDateTime* vtkDateTime::FromYear(vtkTypeInt64 year)
{
  vtkDateTime* d (vtkDateTime::New());
  d->IsNegativeInfinity = false;
  d->IsPositiveInfinity = false;
  if (year <= 0 || (year > static_cast<vtkTypeInt64>(UINT_MAX)))
    {
    vtkTypeInt64 a = year - 1;
    // based on 1 year = 365.242199 days
    d->Ticks = a * 3155692599LL;
    }
  else
    {
    // year 1 to UINT_MAX uses the RataDie conversion
    // so that for years in this range, this call is equivalent
    // to the year-month-day constructor for January 1 in year...

    // casting year to unsigned int will not lose data because
    // its value is in [1..UINT_MAX]

    // Convert to Rata Die
    vtkTypeInt64 rd = ConvertToRataDie(static_cast<unsigned int>(year), 1, 1);
    // Convert days to centiseconds
    d->Ticks = (rd - 1) * 8640000;
    }
  return d;
}

//-----------------------------------------------------------------------------
vtkDateTime* vtkDateTime::NegativeInfinity()
{
  vtkDateTime* d (vtkDateTime::New());
  d->SetToNegativeInfinity();
  return d;
}

//-----------------------------------------------------------------------------
vtkDateTime* vtkDateTime::PositiveInfinity()
{
  vtkDateTime* d (vtkDateTime::New());
  d->SetToPositiveInfinity();
  return d;
}

//-----------------------------------------------------------------------------
vtkDateTime* vtkDateTime::NotADate()
{
  vtkDateTime* d (vtkDateTime::New());
  d->SetToNotADate();
  return d;
}

//-----------------------------------------------------------------------------
vtkTypeInt64 vtkDateTime::ConvertToRataDie(vtkTypeInt64 year,
                                           int month,
                                           int day)
{
  ThrowIfIntOutOfRange("vtkDateTime::ConvertToRataDie - month", month, 1, 12);
  ThrowIfIntOutOfRange("vtkDateTime::ConvertToRataDie - day", day, 1, 31);

  if (month < 3)
    {
    month += 12;
    year -= 1;
    }
  vtkTypeInt64 rd = day + (153 * month - 457) / 5 + 365 * year;
  vtkTypeInt64 a = year / 4;
  rd += a;
  a = year / 100;
  rd -= a;
  a = year / 400;
  rd += a - 306;
  return rd;
}

//-----------------------------------------------------------------------------
vtkTypeInt64 vtkDateTime::ConvertFromRataDie(vtkTypeInt64& ticks,
                                             vtkTypeInt64& year,
                                             int& month,
                                             int& day)
{
  // Convert to RataDie and ticks
  vtkTypeInt64 rd, remainingTicks, z, a, b, c;
  double g;
  rd = (ticks / 8640000) + 1;
  remainingTicks = ticks % 8640000;
  z = rd + 306; // Convert to Julian Day
  g = z - 0.25;
  a = static_cast<vtkTypeInt64>(floor(g / 36524.25)); // Number of full centuries
  // Number of days within the whole centuries (w/r Julian Calendar)
  b = a - static_cast<vtkTypeInt64>(floor(a*0.25));
  year = static_cast<vtkTypeInt64>(floor((b+g)/ 365.25));
  c = b + z - static_cast<vtkTypeInt64>(floor(365.25 * year));
  month = static_cast<int>((5*c+456) / 153);
  day = static_cast<int>(c - static_cast<int>((153*month-457)/5));
  if (month > 12)
    {
    month -= 12;
    year++;
    }
  return remainingTicks;
}

//-----------------------------------------------------------------------------
vtkTypeInt64 vtkDateTime::ConvertFromRataDie(vtkTypeInt64& year,
                                             int& month,
                                             int& day) const
{
  if (this->IsNegativeInfinity || this->IsPositiveInfinity)
    {
    vtkErrorWithObjectMacro(const_cast<vtkDateTime*>(this),
      "Cannot convert an Infinity vtkDateTime");
    }

  vtkTypeInt64 tmpTicks (this->Ticks);

  return vtkDateTime::ConvertFromRataDie(tmpTicks, year, month, day);
}

/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkDateTime.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkDateTime - Allows specifying date and time within VTK toolkit.
//
// .SECTION Description
//
// .SECTION Notes
//

#ifndef __vtkDateTime_h
#define __vtkDateTime_h

// VTK includes.
#include "vtkCMBGeneralModule.h" // For export macro
#include "vtkObject.h"
#include "cmbSystemConfig.h"

// Forward declarations.
class vtkDateTimeInternal;

class VTKCMBGENERAL_EXPORT vtkDateTime : public vtkObject
{
public:
  //BTX
  enum DateTimeUnit
    {
    YEAR,
    MONTH,
    DAY,
    HOUR,
    MINUTE,
    SECOND
    };
  //ETX

  static vtkDateTime* New();

  vtkTypeMacro(vtkDateTime, vtkObject);

  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set date and time.
  void Set(vtkTypeInt64 year=0, int month=0, int day=0,
           int hour=0, int min=0, int sec=0);

  // Description:
  // Get the number of ticks from starting point.
  vtkTypeInt64 GetTicks() const;
  vtkSetMacro(Ticks, vtkTypeInt64);

  // Description:
  // Get/Set year at the beginning.
  vtkTypeInt64 GetYear() const {return this->Year;}
  vtkSetMacro(Year, vtkTypeInt64);

  // Description:
  // Get/Set month at the beginning.
  int GetMonth() const {return this->Month;}
  vtkSetMacro(Month, int);

  // Description:
  // Get/Set day at the beginning.
  int GetDay() const {return this->Day;}
  vtkSetMacro(Day, int);

  // Description:
  // Get/Set hour at the beginning.
  int GetHour() const {return this->Hour;}
  vtkSetMacro(Hour, int);

  // Description:
  // Get/Set minute at the beginning.
  int GetMinute() const {return this->Minute;}
  vtkSetMacro(Minute, int);

  // Description:
  // Get/Set second at the beginning.
  int GetSecond() const {return this->Second;}
  vtkSetMacro(Second, int);

  // Description:
  // Retrive a particular portion of the date.
  vtkTypeInt64 CalcYear()       const;
  int          CalcMonth()      const;
  int          CalcDayOfYear()  const;
  int          CalcDayOfMonth() const;
  int          CalcDayOfWeek()  const;
  int          CalcHour()       const;
  int          CalcMinute()     const;
  int          CalcSecond()     const;
  int          CalcMillisecond()const;

  // Description.
  // Convert to RateDie and store the time as number of ticks.
  // This is mostly required to be called only once.
  void Update()
    {
    vtkTypeInt64 rd = this->ConvertToRataDie(this->Year, this->Month,
                                             this->Day);
    this->Ticks = (((((((rd - 1) * 24) + this->Hour ) * 60) +
                     this->Minute) * 60) + this->Second) * 100;

    this->Modified();
    }

  // Description:
  // An operator for comparing dates.
  bool operator ==  (const vtkDateTime & that) const;
  bool operator !=  (const vtkDateTime & that) const;
  bool operator <   (const vtkDateTime & that) const;
  bool operator <=  (const vtkDateTime & that) const;
  bool operator >=  (const vtkDateTime & that) const;
  bool operator >   (const vtkDateTime & that) const;

  // Description:
  // Retrieve the boolean indicating whether the data represents
  // negative or positive infinity.
  bool GetIsPositiveInfinity() const {return this->IsPositiveInfinity;}
  vtkSetMacro(IsPositiveInfinity, bool);

  bool GetIsNegativeInfinity() const {return this->IsNegativeInfinity;}
  vtkSetMacro(IsNegativeInfinity, bool);

  // Description:
  // A method to determine whether the vtkDateTime represents negative or
  // positive infinity.
  bool GetIsInfinite() const;
  bool GetIsNotADate() const;

  // Description:
  // Retrieve a particular portion of the vtkDateTime
  // (e.g., minutes, seconds, etc.)
  int GetDayOfYear  () const;
  int GetDayOfMonth () const;
  int GetDayOfWeek  () const;
  int GetMillisecond() const;

  static int ThrowIfInfiniteOrNegative(const vtkDateTime& dateTime,
                                       const char* method);
  static int ThrowIfIntOutOfRange     (const char* context,
                                       int value,
                                       int Minute,
                                       int max);

  // Description:
  // Convert the current data to a string.
  const char* ToLongTimeString     () const;
  const char* ToShortTimeString    () const;
  const char* ToLongDateString     () const;
  const char* ToShortDateString    () const;
  const char* ToW3UTCDateTimeString() const;

  const char* ToString(const char* format) const;

  // Description:
  // Parse a string to produce the current vtkDateTime.
  static vtkDateTime* Parse(const char* format, const char* rep);

  // Description:
  // Methods to add various time periods to the current vtkDateTime.
  vtkDateTime* CreateNewAddYears  (vtkTypeInt64 delta)  const;
  vtkDateTime* CreateNewAddDays   (vtkTypeInt64 delta)  const;
  vtkDateTime* CreateNewAddHours  (vtkTypeInt64 delta)  const;
  vtkDateTime* CreateNewAddMinutes(vtkTypeInt64 delta)  const;
  vtkDateTime* CreateNewAddSeconds(vtkTypeInt64 delta)  const;
  vtkDateTime* CreateNewAddTicks  (vtkTypeInt64 delta)  const;

  // Description:
  // Copy data values from the src.
  int DeepCopy(const vtkDateTime* src);

  // Description:
  // Object factories to create dates for various circumstances.
  static vtkDateTime* FromYear        (vtkTypeInt64 year);
  static vtkDateTime* NegativeInfinity();
  static vtkDateTime* PositiveInfinity();
  static vtkDateTime* NotADate        ();

  // Description:
  // Methods to convert the current data to and from a RataDie representation.
  static vtkTypeInt64 ConvertToRataDie  (vtkTypeInt64 year, int month, int day);
  static vtkTypeInt64 ConvertFromRataDie(vtkTypeInt64& ticks, vtkTypeInt64& year,
                                         int &month, int &day);

protected:
  vtkDateTime();
  virtual ~vtkDateTime();

  void SetToNegativeInfinity();
  void SetToPositiveInfinity();
  void SetToNotADate        ();

  vtkTypeInt64 ConvertFromRataDie(vtkTypeInt64& year,
                                  int& month,
                                  int& day) const;

private:
  vtkTypeInt64   Ticks;
  vtkTypeInt64   Year;

  int   Month;
  int   Day;
  int   Hour;
  int   Minute;
  int   Second;

  bool  IsNegativeInfinity;
  bool  IsPositiveInfinity;

  vtkDateTimeInternal* Internal;

private:
  vtkDateTime   (const vtkDateTime&);     // Not implemented.
  void operator=(const vtkDateTime&);  // Not implemented.
};

#endif // __vtkDateTime_h

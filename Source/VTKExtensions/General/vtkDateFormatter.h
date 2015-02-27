/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkDateFormatter.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkDateFormatter - Provides interface to convert vtkDateTime
// into a particular formatted string.
//
// .SECTION Description
//
// .SECTION Notes
//

#ifndef __vtkDateFormatter_h
#define __vtkDateFormatter_h

#include "vtkCMBGeneralModule.h" // For export macro
#include "vtkExporter.h"
#include "cmbSystemConfig.h"

// Forware declaration.
class vtkDateTime;

// Helper class to format vtkDateTime objects into string representations.
// This class is intended as a container of static methods only. It is not
// intended to be instantiated and it holds only static data.
class VTKCMBGENERAL_EXPORT vtkDateFormatter
{
//BTX
public:

  // Description:
  // Given a data instance; produce a formatted string representing the date.
  static std::string FormatDate(const vtkDateTime& date,
                                   const char* format);

  // Description:
  // Given a date represented as a string, format a component of the string
  // with the specified repreat count.
  static std::string FormatDateComponent(const vtkDateTime& date,
                                            const char* component,
                                            char formatChar,
                                            int repeatCount);

  // Description:
  // Given a format specification and a string representing the date, produce
  // a instance of the date class.
  static vtkDateTime* ParseDate(const char* format, const char* rep);

  // Description:
  // Method to retrieve the format string in various forms.
  static int            GetFormatStringLCID         ();
  static std::string GetLongTimeFormatString     ();
  static std::string GetShortTimeFormatString    ();
  static std::string GetLongDateFormatString     ();
  static std::string GetShortDateFormatString    ();
  static std::string GetW3UTCDateTimeFormatString();

  // Description:
  // Method to retrieve the AM and PM indicators as strings.
  static std::string GetTimeAMString();
  static std::string GetTimePMString();

  // Description:
  // Methods to specify various formatting strings for the date.
  static void SetFormatStringLCID         (int lcid);
  static void SetLongTimeFormatString     (const char* format);
  static void SetShortTimeFormatString    (const char* format);
  static void SetLongDateFormatString     (const char* format);
  static void SetShortDateFormatString    (const char* format);
  static void SetW3UTCDateTimeFormatString(const char* format);

  // Description:
  // Method to specify the AM and PM string formats.
  static void SetTimeAMString(const char* value);
  static void SetTimePMString(const char* value);

private:
  vtkDateFormatter(); //Not implemented
  vtkDateFormatter(const vtkDateFormatter&); //Not implemented
  void operator=(const vtkDateFormatter&); //Not implemented
//ETX
};

#endif // __vtkDateFormatter_h

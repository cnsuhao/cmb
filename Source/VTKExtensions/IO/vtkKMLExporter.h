//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// .NAME vtkKMLExporter - Exports VTK data as KML.
//
// .SECTION Description
// There are various options to control how to convert VTK data in KML
// specific format.
//
// .SECTION Notes
//

#ifndef __vtkKMLExportter_h
#define __vtkKMLExportter_h
#include "vtkCMBIOModule.h" // For export macro
#include <algorithm>
#include "cmbSystemConfig.h"

// VTK include.
#include "vtkExporter.h"

// Forward declarations.
class vtkActor;
class vtkDateTime;
class vtkKMLExporterInternal;

class VTKCMBIO_EXPORT vtkKMLExporter : public vtkExporter
{
public:
  static vtkKMLExporter *New();
  vtkTypeMacro(vtkKMLExporter, vtkExporter);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Specify the name of the KML file to write.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  // Description:
  // Specify if the \c vtkPolyData should be rendered as an image. This
  // will convert \c vtkPolyData into KML ground overlay.
  // Default is set to \c true.
  vtkGetMacro(RenderPolyDataAsImage, bool);
  vtkSetMacro(RenderPolyDataAsImage, bool);

  // Description:
  // Specify if each actor needs to be written separately.
  // Turning if off leads to convert vtk data into KML data as polygons,
  // lines or as an image.
  // Default is \c true.
  vtkGetMacro(RenderScene, bool);
  vtkSetMacro(RenderScene, bool);

  // Description:
  // Specify if the incoming data has time stamps and needs
  // to be shown as KML animation.
  // Default is set to \c false.
  vtkGetMacro(WriteTimeSeriesData, bool);
  vtkSetMacro(WriteTimeSeriesData, bool);

  // Description:
  // If set to true the height of the legend will be
  // adjusted to keep the same aspect ration as the original.
  vtkGetMacro(KeepLegendAspectRatio, bool);
  vtkSetMacro(KeepLegendAspectRatio, bool);

  // Description:
  // Set/Get state date and time for the animation.
  // This is required when exporting KML animations.
  vtkGetMacro(StartDateTime, vtkDateTime*);
  void SetStartDateTime(vtkDateTime* dateTime);

  // Description:
  // This flag is used only when exporting KML animations.
  // This flag if set to \c true, set the guideline for the KML
  // exporter not to specify end time for each time step.
  // This could be useful if the data is not overlapping.
  // Default is set to \c false.
  vtkGetMacro(ShowPrevious, bool);
  vtkSetMacro(ShowPrevious, bool);

  // Description:
  // Specify the unit for the interval.
  vtkGetMacro(DeltaDateTimeUnit, int);
  // = 0 = YEARS
  // = 1 = MONTHS
  // = 2 = DAYS
  // = 3 = HOURS
  // = 4 = MINUTES
  // = 5 = SECONDS
  vtkSetClampMacro(DeltaDateTimeUnit, int, 0, 5);

  // Description:
  // Specify how the data should be placed on the terrain / sea floor.
  vtkGetMacro(AltitudeMode, int);

  // 0 = CLAMPTOGROUND
  // 1 = RELATIVETOGROUND,
  // 2 = ABSOLUTE
  // 3 = CLAMPTOSEAFLOOR
  // 4 = RELATIVETOSEAFLOOR
  vtkSetClampMacro(AltitudeMode, int, 0, 4);

  // Description:
  // Set the inteval for date / time.
  vtkGetMacro(DeltaDuration, vtkTypeInt64);
  vtkSetMacro(DeltaDuration, vtkTypeInt64);


  // Description:
  // The magnification of the current render window. Initial value is 1.
  vtkSetClampMacro(Magnification,int,1,2048);
  vtkGetMacro(Magnification,int);

  // Description:
  // Get/Set the legend scale. This is the reduction factor
  // that gets applied to the legend image.
  // See also \c KeepLegendAspectRatio.
  vtkGetMacro(LegendScale, float);
  vtkSetClampMacro(LegendScale, float, 0, 1);


  // Description:
  // Set the threshold for KML above which
  // polydata will be converted to an image.
  vtkGetMacro(CellCountThreshold, int);
  vtkSetMacro(CellCountThreshold, int);

  // Description:
  // This is used when exporting KML animations.Instead of setting
  // delta date / time interval and unit, start date time for each time
  // step can be added to the list. If \c ShowPrevious is set to false
  // then for any given time step then next time step value will be used
  // to set the end time for the current time step.
  void AddStartDateTime(vtkDateTime* dt);

  // Description:
  // SuperKML produces KML that references KML from each time step for KML
  // animations.
  int WriteAnimation();

protected:

  vtkKMLExporter();
  virtual ~vtkKMLExporter();

  virtual void WriteData();

  char*         FileName;

  bool          RenderPolyDataAsImage;
  bool          RenderScene;
  bool          ShowPrevious;
  bool          WriteTimeSeriesData;
  bool          KeepLegendAspectRatio;

  vtkTypeInt64  DeltaDuration;
  int           DeltaDateTimeUnit;

  int           AltitudeMode;

  int           Magnification;

  float         LegendScale;

  vtkDateTime*  StartDateTime;

  int           CellCountThreshold;

  vtkKMLExporterInternal* Implementation;

private:
  vtkKMLExporter(const vtkKMLExporter&);  // Not implemented.
  void operator=(const vtkKMLExporter&);  // Not implemented.
};

#endif // __vtkKMLExportter_h

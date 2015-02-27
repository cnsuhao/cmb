/*=========================================================================

Copyright (c) 1998-2012 Kitware Inc. 28 Corporate Drive,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
=========================================================================*/

#include "vtkKMLExporter.h"

// KML includes.
#include "kml/dom.h"
#include "kml/dom/kml22.h"
#include "kml/base/color32.h"
#include "kml/engine/kmz_file.h"
#include "kml/engine/kml_file.h"

// VTK includes.
#include "vtkAssemblyPath.h"
#include "vtkCamera.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkCompositeDataGeometryFilter.h"
#include "vtkCompositeDataSet.h"
#include "vtkDataSetSurfaceFilter.h"
#include "vtkDateTime.h"
#include "vtkIdList.h"
#include "vtkImageData.h"
#include "vtkLookupTable.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPointSet.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkPNGWriter.h"
#include "vtkRendererCollection.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkSmartPointer.h"
#include "vtkTransformCoordinateSystems.h"
#include "vtkUnstructuredGrid.h"
#include "vtkWindowToImageFilter.h"

// VTK system includes.
#include "vtksys/SystemTools.hxx"
#include "vtksys/String.hxx"

// STL includes.
#include <sstream>
#include <cstdlib>
#include <iostream>

using std::stringstream;
using kmldom::ColorStylePtr;
using kmldom::CoordinatesPtr;
using kmldom::DocumentPtr;
using kmldom::FolderPtr;
using kmldom::GroundOverlayPtr;
using kmldom::IconPtr;
using kmldom::InnerBoundaryIsPtr;
using kmldom::KmlPtr;
using kmldom::KmlFactory;
using kmldom::LatLonBoxPtr;
using kmldom::LinearRingPtr;
using kmldom::LineStringPtr;
using kmldom::LineStylePtr;
using kmldom::LodPtr;
using kmldom::MultiGeometryPtr;
using kmldom::OuterBoundaryIsPtr;
using kmldom::OverlayXYPtr;
using kmldom::PlacemarkPtr;
using kmldom::PointPtr;
using kmldom::PolygonPtr;
using kmldom::PolyStylePtr;
using kmldom::RegionPtr;
using kmldom::ScreenOverlayPtr;
using kmldom::ScreenXYPtr;
using kmldom::SizePtr;
using kmldom::StylePtr;
using kmldom::NetworkLinkPtr;
using kmldom::LinkPtr;
using kmldom::TimeSpanPtr;


vtkStandardNewMacro(vtkKMLExporter);

//-----------------------------------------------------------------------------
class vtkKMLExporterInternal
{
public:
  static const int MAX_NO_ALLOWED  = 10000;

  vtkKMLExporterInternal(vtkKMLExporter* exp) :
    wroteOverlay  (false),
    exporter      (exp),
    fileNamePrefix(""),
    sysPath       (""),
    ovrlFileName  ("")
    {
    }

  ~vtkKMLExporterInternal()
    {
    for(size_t i=0; i < startDateTimeList.size(); ++i)
      {
      if(startDateTimeList.at(i))
        {
        startDateTimeList.at(i)->Delete();
        }
      } // end for...

      startDateTimeList.clear();
    }

  string ConvertRGBtoHex(int num);
  string ConvertRGBtoHex(int r, int g, int b, int a);

  vtkImageData* ConvertToImage  (vtkActor* anActor);
  vtkImageData* ConvertToImage  (vtkRenderWindow* renWin, bool render3D=true);

  int           WritePolygons   (DocumentPtr doc, vtkPolyData* polyData);

  int           AddLine         (DocumentPtr doc, CoordinatesPtr coordinates,
                                 string name, string styleName);
  int           WriteLines      (DocumentPtr doc, vtkPolyData* polyData);

  int           DoWriteGeometry (DocumentPtr doc, vtkPolyData* polyData);
  int           WriteGeometry   (vtkPolyData* polyData, int index=-1);

  int           DoWriteMultiGeometry
                                (DocumentPtr doc,
                                 vtkPolyData* polyData, int index=-1);
  int           WriteMultiGeometry
                                (vtkPolyData* polyData, int index=-1);

  int           WriteImage      (vtkImageData* imageData, int index=-1);
  int           WriteAnActor    (vtkActor *anActor, int index=-1);

  int           AddOverlay      (DocumentPtr doc);
  int           WriteOverlay    (vtkImageData* imageData);

  int           WriteCurrentTimeStep();
  int           WriteAnimation  ();


  std::string GetNextDateTimeString (int index);

  vtkRenderer*  GetRenderer     (vtkRenderWindow* renWin, int index);

  bool              wroteOverlay;

  vtkKMLExporter*   exporter;
  std::ofstream  outputFile;


  std::string                  fileNamePrefix;
  std::string                  sysPath;
  std::string                  ovrlFileName;
  std::vector<std::string>  timeStepFileNames;
  std::vector<std::string>  dataFileNames;
  std::vector<vtkDateTime*>    startDateTimeList;
};


//-----------------------------------------------------------------------------
// Utility functions to convert RGB to hexadecimal string.
string vtkKMLExporterInternal::ConvertRGBtoHex(int num)
{
    static string hexDigits = "0123456789ABCDEF";
    string argb;

    for (int i=(4*2) - 1; i>=0; i--)
      {
            argb += hexDigits[((num >> i*4) & 0xF)];
      }

    return argb;
}

//-----------------------------------------------------------------------------
string vtkKMLExporterInternal::ConvertRGBtoHex(int r, int g, int b, int a)
{
  int rgbNum = ((a & 0xff) << 24)
              | ((r & 0xff) << 16)
              | ((g & 0xff) << 8)
              | (b & 0xff);

  return ConvertRGBtoHex(rgbNum);
}

//-----------------------------------------------------------------------------
vtkImageData* vtkKMLExporterInternal::ConvertToImage(vtkActor* anActor)
{
  // @note: This assumes that all other actors are set to be not visible.
  if(!anActor)
    {
    return 0;
    }

  vtkRenderWindow* renWin = this->exporter->GetRenderWindow();
  vtkImageData* newImage = this->ConvertToImage(renWin);
  return newImage;
}

//-----------------------------------------------------------------------------
vtkImageData* vtkKMLExporterInternal::ConvertToImage(vtkRenderWindow* renWin,
                                                     bool render3D)
{
  if(!renWin)
    {
    std::cerr << "Invalid vtkRenderWindow. " << std::endl;
    return 0;
    }

  // Get the 3d renderer.
  vtkRenderer* ren(0);

  if(render3D)
    {
    ren = this->GetRenderer(renWin, 0);
    }
  else
    {
    ren = this->GetRenderer(renWin, 2);
    }

  if(!ren)
    {
    std::cerr << "Invalid vtkRenderer. " << std::endl;
    return 0;
    }

  renWin->SetAlphaBitPlanes(1);
  renWin->Render();

  if(render3D)
    {
    vtkCamera* cam = ren->GetActiveCamera();

    if(!cam)
      {
      std::cerr << "Invalid vtkCamera. " << std::endl;
      return 0;
      }

    ren->ResetCamera();

    int curPrlPrj = cam->GetParallelProjection();
    double* curViewUp = cam->GetViewUp();
    double* curPos = cam->GetPosition();

    cam->SetParallelProjection(1);
    // We want the camera to look in -Z, with up vector pointing in +Y.
    cam->SetViewUp(0.0, 1.0, 0.0);
    double* fpoint = cam->GetFocalPoint();
    cam->SetPosition(fpoint[0], fpoint[1], fpoint[2] + 1.0);
    ren->ResetCamera();

    // @note: This should be an ivar.
    int* winSize = renWin->GetSize();
    const double IMAGE_DIM_X (static_cast<double>(winSize[0]));
    const double IMAGE_DIM_Y (static_cast<double>(winSize[1]));

//    std::cout << "DIM_X is: " << IMAGE_DIM_X << std::endl;
//    std::cout << "DIM_X is: " << IMAGE_DIM_Y << std::endl;

    // @note: This should be based on the ivar.
    vtkNew<vtkPoints> pts;
    pts->InsertNextPoint(0.0, 0.0, 0.0);
    pts->InsertNextPoint(IMAGE_DIM_X, 0.0, 0.0);
    pts->InsertNextPoint(IMAGE_DIM_X, IMAGE_DIM_Y, 0.0);
    pts->InsertNextPoint(0.0, IMAGE_DIM_Y, 0.0);

    vtkNew<vtkPolyData> ps;
    ps->SetPoints(pts.GetPointer());

    vtkNew<vtkTransformCoordinateSystems> tcs;
    tcs->SetInputCoordinateSystemToDisplay();
    tcs->SetOutputCoordinateSystemToWorld();
    tcs->SetInputData(ps.GetPointer());
    tcs->SetViewport(this->GetRenderer(renWin, 0));
    tcs->Update();

    vtkPointSet* ops = tcs->GetOutput();
    vtkPoints* opts = ops->GetPoints();

    double newBounds[4];
    newBounds[0] = opts->GetPoint(0)[0];
    newBounds[1] = opts->GetPoint(2)[0];
    newBounds[2] = opts->GetPoint(0)[1];
    newBounds[3] = opts->GetPoint(2)[1];

    vtkNew<vtkWindowToImageFilter> winToImg;

    // @note: This is important to have transparency in the final image.
    winToImg->SetInputBufferTypeToRGBA();
    winToImg->SetMagnification(this->exporter->GetMagnification());
    winToImg->SetInput(renWin);
    winToImg->Update();

    double spacingX = ((newBounds[1] - newBounds[0]) / IMAGE_DIM_X);
    double spacingY = ((newBounds[3] - newBounds[2]) / IMAGE_DIM_Y);

    vtkImageData* newImage (vtkImageData::New());
    newImage->ShallowCopy(winToImg->GetOutput());
    newImage->SetOrigin(newBounds[0], newBounds[2], 0.0);
    newImage->SetSpacing(spacingX, spacingY, 0.0);

    // Restore.
    if(!curPrlPrj)
      {
      cam->SetParallelProjection(0);
      }

    cam->SetPosition(curPos);
    cam->SetViewUp(curViewUp);

    return newImage;
    }
  else
    {

    vtkNew<vtkWindowToImageFilter> winToImg;

    // @note: This is important to have transparency in the final image.
    winToImg->SetInputBufferTypeToRGBA();
    winToImg->SetInput(renWin);
    winToImg->Update();

    vtkImageData* newImage (vtkImageData::New());
    newImage->ShallowCopy(winToImg->GetOutput());
    return newImage;
    }
}

//-----------------------------------------------------------------------------
int vtkKMLExporterInternal::WritePolygons(DocumentPtr doc,
                                          vtkPolyData* polyData)
{
  if(!doc)
    {
    std::cerr << "Invalid KML Document." << std::endl;
    return 0;
    }

  if(!polyData)
    {
    std::cerr << "Invalid vtkPolyData." << std::endl;
    return 0;
    }

  vtkPoints* points = polyData->GetPoints();
  if(!points)
    {
    std::cerr << "Failed to get vtkPoints." << std::endl;
    return 0;
    }

  vtkCellArray* polys = polyData->GetPolys();
  if(!polys)
    {
    std::cerr << "Failed to get vtkCellArray." << std::endl;
    return 0;
    }
  polys->InitTraversal();

  KmlFactory* factory = KmlFactory::GetFactory();

  // Check if the incoming data has color array.
  // Check if each line requires different color.
  vtkDataArray* rgbData(0);
  vtkCellData* cellData = polyData->GetCellData();
  if(cellData)
    {
    rgbData = cellData->GetArray("RGB");
    }

  for(int i=0; i < polys->GetNumberOfCells(); ++i)
    {
    CoordinatesPtr coordinates = factory->CreateCoordinates();

    //-- Style stuff.
    std::ostringstream ostr;
    ostr << static_cast<unsigned int>(i);
    string styleName ("poly_style_");
    styleName.append(ostr.str());

    PolyStylePtr polyStyle = factory->CreatePolyStyle();
    polyStyle->set_fill(1);
    polyStyle->set_outline(0);

    if(rgbData)
      {
      double* color = rgbData->GetTuple(i);
      if(color)
        {
        polyStyle->set_color(ConvertRGBtoHex(static_cast<int>(color[0]),
                                             static_cast<int>(color[1]),
                                             static_cast<int>(color[2]),
                                             255));
        }
      }

    StylePtr style = factory->CreateStyle();
    style->set_id(styleName);
    style->set_polystyle(polyStyle);

    // Add style to the list.
    doc->add_styleselector(style);
    //--

    vtkIdList* list (vtkIdList::New());
    polys->GetNextCell(list);

    double firstPoint[3];
    for(int j=0; j < list->GetNumberOfIds(); ++j)
      {
      if(points)
        {
        double* point = points->GetPoint(list->GetId(j));

        if(j==0)
          {
          firstPoint[0] = point[0];
          firstPoint[1] = point[1];
          firstPoint[2] = point[2];
          }

        coordinates->add_latlngalt(point[1], point[0], point[2]);
        }
      }

    coordinates->add_latlngalt(firstPoint[1], firstPoint[0], firstPoint[2]);

    LinearRingPtr linearring = factory->CreateLinearRing();
    linearring->set_coordinates(coordinates);

    OuterBoundaryIsPtr outerboundaryis = factory->CreateOuterBoundaryIs();
    outerboundaryis->set_linearring(linearring);

    PolygonPtr polygon = factory->CreatePolygon();
    polygon->set_extrude(false);
    polygon->set_tessellate(true);
    if(this->exporter->GetAltitudeMode() > 2)
      {
      // @note: This will work only in google earth 5 and onwards.
      polygon->set_gx_altitudemode((this->exporter->GetAltitudeMode() - 3));
      }
    else
      {
      polygon->set_altitudemode(this->exporter->GetAltitudeMode());
      }
    polygon->set_outerboundaryis(outerboundaryis);

    PlacemarkPtr placemark = factory->CreatePlacemark();

    // \NOTE: For now use the name set on the document.
    placemark->set_name(doc->get_name());
    placemark->set_id(doc->get_name());

    placemark->set_geometry(polygon);
    placemark->set_styleurl(string("#").append(styleName));

    doc->add_feature(placemark);

    list->Delete();
    }

  return 1;
}

//-----------------------------------------------------------------------------
int vtkKMLExporterInternal::AddLine(DocumentPtr doc, CoordinatesPtr coordinates,
                                    string name, string styleName)
{
  if(!doc)
    {
    std::cerr << "Invalid KML Document." << std::endl;
    return 0;
    }

  if(!coordinates)
    {
    std::cerr << "Invalid KML Coordinates." << std::endl;
    return 0;
    }

  KmlFactory* factory = KmlFactory::GetFactory();

  LineStringPtr linestr = factory->CreateLineString();
  linestr->set_coordinates(coordinates);
  linestr->set_extrude(false);
  linestr->set_tessellate(true);
  if(this->exporter->GetAltitudeMode() > 2)
    {
    // @note: This will work only in google earth 5 and onwards.
    linestr->set_gx_altitudemode((this->exporter->GetAltitudeMode() - 3));
    }
  else
    {
    linestr->set_altitudemode(this->exporter->GetAltitudeMode());
    }

  PlacemarkPtr placemark = factory->CreatePlacemark();
  placemark->set_name(name);
  placemark->set_id(name);
  placemark->set_styleurl(string("#").append(styleName));

  placemark->set_geometry(linestr);

  doc->add_feature(placemark);

  return 1;
}

//-----------------------------------------------------------------------------
int vtkKMLExporterInternal::WriteLines(DocumentPtr doc, vtkPolyData* polyData)
{
  if(!doc)
    {
    std::cerr << "Invalid KML Document. " << std::endl;
    return 0;
    }

  if(!polyData)
    {
    std::cerr << "Invalid vtkPolyData. " << std::endl;
    return 0;
    }

  vtkPoints* points = polyData->GetPoints();
  if(!points)
    {
    std::cerr << "Failed to get vtkPoints." << std::endl;
    return 0;
    }

  vtkCellArray* lines = polyData->GetLines();
  if(!lines)
    {
    std::cerr << "Failed to get vtkCellArray." << std::endl;
    return 0;
    }

  KmlFactory* factory = KmlFactory::GetFactory();

  // Check if each line requires different color.
  vtkDataArray* rgbData(0);
  vtkCellData* cellData = polyData->GetCellData();
  if(cellData)
    {
    rgbData = cellData->GetArray("RGB");
    }

  // Loop over all the lines in the data.
  for(int i=0; i < lines->GetNumberOfCells(); ++i)
    {
    vtkIdList* list (vtkIdList::New());
    lines->GetNextCell(list);

    std::ostringstream ostr;
    ostr << i;
    string styleName ("line_style_");
    styleName.append(ostr.str());

    CoordinatesPtr coordinates = factory->CreateCoordinates();
    LineStylePtr lineStyle = factory->CreateLineStyle();
    lineStyle->set_width(1.0);

    if(rgbData)
      {
      double* color = rgbData->GetTuple(i);
      if(color)
        {
        lineStyle->set_color(ConvertRGBtoHex(static_cast<int>(color[0]),
                                             static_cast<int>(color[1]),
                                             static_cast<int>(color[2]),
                                             255));
        }
      }

    StylePtr style = factory->CreateStyle();
    style->set_id(styleName);
    style->set_linestyle(lineStyle);

    // Add style to the list.
    doc->add_styleselector(style);

    int count = 0;
    for(int j=0; j < list->GetNumberOfIds(); ++j)
      {
      if(points)
        {
        double* point = points->GetPoint(list->GetId(j));

        coordinates->add_latlngalt(point[1], point[0], point[2]);

        //@note: This is a arbitrary limit set. I could not find a documentation
        // on what exactly the limit is.

        if(count > exporter->GetCellCountThreshold())
          {
          if(!AddLine(doc, coordinates, string("contour_").append(ostr.str()),
                  styleName))
            {
            return 0;
            }
          // Use a new one.
          coordinates = factory->CreateCoordinates();

          // Re enter the current point for continuation.
          coordinates->add_latlngalt(point[1], point[0], point[2]);

          count = 0;
          continue;
          }

          ++count;
        }
      }

    AddLine(doc, coordinates, string("contour_").append(ostr.str()), styleName);

    list->Delete();
    }

  return 1;
}

//-----------------------------------------------------------------------------
int vtkKMLExporterInternal::DoWriteGeometry(DocumentPtr doc,
                                            vtkPolyData* polyData)
{
  if(!doc)
    {
    std::cerr << "Invalid KML Document. " << std::endl;
    return 0;
    }

  if(!polyData)
    {
    std::cerr << "Invalid vtkPolyData. " << std::endl;
    return 0;
    }

  if(polyData->GetPolys())
    {
    return WritePolygons(doc, polyData);
    }

  if(polyData->GetLines())
    {
    return WriteLines(doc, polyData);
    }

  return 0;
}

//-----------------------------------------------------------------------------
int vtkKMLExporterInternal::WriteGeometry(vtkPolyData* polyData,
                                          int index)
{
  if(!polyData)
    {
    std::cerr << "Invalid vtkPolyData. " << std::endl;
    return 0;
    }

  std::string name = this->fileNamePrefix;
  if(index >= 0)
    {
    std::ostringstream ostr;
    ostr << index;
    name.append(ostr.str());
    }
  std::string geomFilename (name + "_geom.kml");

  KmlFactory* factory = KmlFactory::GetFactory();

  // Style.
  PolyStylePtr pstyle = factory->CreatePolyStyle();
  pstyle->set_fill(true);
  pstyle->set_outline(false);

  StylePtr style = factory->CreateStyle();
  style->set_id("style");
  style->set_polystyle(pstyle);

  // LOD.
  LodPtr lod = factory->CreateLod();
  lod->set_id("lod");
  lod->set_minlodpixels(100);
  lod->set_maxlodpixels(-1);

  // Region.
  RegionPtr region = factory->CreateRegion();
  region->set_id("region");
  region->set_lod(lod);

  DocumentPtr doc = factory->CreateDocument();
  doc->set_region(region);
  doc->add_styleselector(style);
  doc->set_name(geomFilename);

  if(!DoWriteGeometry(doc, polyData))
    {
    return 0;
    }

  KmlPtr kml = factory->CreateKml();
  kml->set_feature(doc);


  std::ofstream outFile ((this->sysPath + geomFilename).c_str());

  if(outFile.good())
    {
    std::string str (kmldom::SerializePretty(kml));
    outFile << str;
    outFile.flush();
    outFile.close();

    this->dataFileNames.push_back(geomFilename);

    return 1;
    }
  else
    {
    outFile.close();
    std::cerr << "Error writing data to output stream." << std::endl;
    return 0;
    }
}

// @note: Not tested yet.
//-----------------------------------------------------------------------------
int vtkKMLExporterInternal::DoWriteMultiGeometry(DocumentPtr doc,
                                                 vtkPolyData* polyData,
                                                 int /*index*/)
{
  if(!doc)
    {
    std::cerr << "Invalid KML Document." << std::endl;
    return 0;
    }

  if(!polyData)
    {
    std::cerr << "Invalid vtkPolyData." << std::endl;
    return 0;
    }

  vtkPoints* points = polyData->GetPoints();
  if(!points)
    {
    std::cerr << "Invalid vtkPoints." << std::endl;
    return 0;
    }

  vtkCellArray* polys = polyData->GetPolys();
  if(!polys)
    {
    std::cerr << "Invalid vtkCellArray." << std::endl;
    return 0;
    }

  KmlFactory* factory = KmlFactory::GetFactory();

  MultiGeometryPtr multigeom = factory->CreateMultiGeometry();
  PlacemarkPtr placemark = factory->CreatePlacemark();
  placemark->set_name(exporter->GetFileName());
  placemark->set_id(exporter->GetFileName());
  placemark->set_geometry(multigeom);
  placemark->set_styleurl(string("#").append("style"));

  doc->add_feature(placemark);

  for(int i=0; i < polys->GetNumberOfCells(); ++i)
    {
    CoordinatesPtr coordinates = factory->CreateCoordinates();

    vtkIdList* list (vtkIdList::New());
    polys->GetNextCell(list);

    double firstPoint[3];
    for(int j=0; j < list->GetNumberOfIds(); ++j)
      {
      if(points)
        {
        double* point = points->GetPoint(list->GetId(j));

        if(j==0)
          {
          firstPoint[0] = point[0];
          firstPoint[1] = point[1];
          firstPoint[2] = point[2];
          }

        coordinates->add_latlngalt(point[1], point[0], point[2]);
        }
      }

    coordinates->add_latlngalt(firstPoint[1], firstPoint[0], firstPoint[2]);

    LinearRingPtr linearring = factory->CreateLinearRing();
    linearring->set_coordinates(coordinates);

    OuterBoundaryIsPtr outerboundaryis = factory->CreateOuterBoundaryIs();
    outerboundaryis->set_linearring(linearring);

    PolygonPtr polygon = factory->CreatePolygon();
    polygon->set_extrude(true);
    polygon->set_tessellate(false);
    if(this->exporter->GetAltitudeMode() > 2)
      {
      // @note: This will work only in google earth 5 and onwards.
      polygon->set_gx_altitudemode((this->exporter->GetAltitudeMode() - 3));
      }
    else
      {
      polygon->set_altitudemode(this->exporter->GetAltitudeMode());
      }
    polygon->set_outerboundaryis(outerboundaryis);

    multigeom->add_geometry(polygon);

    list->Delete();
    }

  return 1;
}

// @note: Not tested yet.
//-----------------------------------------------------------------------------
int vtkKMLExporterInternal::WriteMultiGeometry(vtkPolyData* polyData, int index)
{
  if(!polyData)
    {
    std::cerr << "Invalid vtkPolyData. " << std::endl;
    return 0;
    }

  KmlFactory* factory = KmlFactory::GetFactory();

  // Style.
  PolyStylePtr pstyle = factory->CreatePolyStyle();
  pstyle->set_fill(true);
  pstyle->set_outline(false);

  StylePtr style = factory->CreateStyle();
  style->set_id("style");
  style->set_polystyle(pstyle);

  // LOD.
  LodPtr lod = factory->CreateLod();
  lod->set_id("lod");
  lod->set_minlodpixels(100);
  lod->set_maxlodpixels(-1);

  // Region.
  RegionPtr region = factory->CreateRegion();
  region->set_id("region");
  region->set_lod(lod);

  DocumentPtr doc = factory->CreateDocument();
  doc->set_region(region);
  doc->add_styleselector(style);

  if(!DoWriteMultiGeometry(doc, polyData))
    {
    return 0;
    }

  KmlPtr kml = factory->CreateKml();
  kml->set_feature(doc);

  std::string name (this->fileNamePrefix);

  if(index >= 0)
    {
    std::ostringstream ostr;
    ostr << index;
    name.append(ostr.str());
    }

  std::string mulGeomFileName (name + "_mgeom.kml");
  std::ofstream outFile((this->sysPath + mulGeomFileName).c_str());

  if(outFile.good())
    {
    doc->set_name(vtksys::SystemTools::GetFilenameName(mulGeomFileName));

    std::string str(kmldom::SerializePretty(kml));
    outFile << str;
    outFile.flush();
    outFile.close();

    // Add this to the list of data files generated for this time.
    this->dataFileNames.push_back(
      vtksys::SystemTools::GetFilenameName(mulGeomFileName));

    return 1;
    }
  else
    {
    outFile.close();
    std::cerr << "Error writing data to output stream." << std::endl;
    return 0;
    }

  return 1;
}

//-----------------------------------------------------------------------------
int vtkKMLExporterInternal::WriteImage(vtkImageData* imageData, int index)
{
  if(!imageData)
    {
    std::cerr << "Invalid or empty image data. " << std::endl;
    return 0;
    }

  double origin[3];
  double spacing[3];
  int    dim[3];

  imageData->GetOrigin(origin);
  imageData->GetDimensions(dim);
  imageData->GetSpacing(spacing);

  double magFactor = static_cast<double>(this->exporter->GetMagnification());
  double north = origin[1] + spacing[1] * ((dim[1] / magFactor) - 1);
  double south = origin[1];
  double east  = origin[0] + spacing[0] * ((dim[0] / magFactor) - 1);
  double west  = origin[0];

  // Currently lets not apply rotation (we need to deal with this later though).
  KmlFactory* factory = KmlFactory::GetFactory();

  // Lat lon bbox for the image.
  LatLonBoxPtr latLonBB = factory->CreateLatLonBox();
  latLonBB->set_north(north);
  latLonBB->set_south(south);
  latLonBB->set_east(east);
  latLonBB->set_west(west);

  std::string name = this->fileNamePrefix;

  if(index >= 0)
    {
    std::ostringstream ostr;
    ostr << index;
    name.append(ostr.str());
    }

  // Set the file name for png.
  std::string pngFileName (name);
  pngFileName.append(".png");

  vtkNew<vtkPNGWriter> pngWriter;
  pngWriter->SetInputData(imageData);

  pngWriter->SetFileName(std::string(this->sysPath + pngFileName).c_str());
  pngWriter->Update();
  pngWriter->Write();

  IconPtr icon = factory->CreateIcon();
  icon->set_href(std::string(pngFileName));

  GroundOverlayPtr grndOvrl = factory->CreateGroundOverlay();
  grndOvrl->set_name(name.c_str());
  grndOvrl->set_latlonbox(latLonBB);
  if(this->exporter->GetAltitudeMode() > 2)
    {
    grndOvrl->set_gx_altitudemode((this->exporter->GetAltitudeMode() - 3));
    }
  else
    {
    grndOvrl->set_altitudemode(this->exporter->GetAltitudeMode());
    }
  grndOvrl->set_icon(icon);
  grndOvrl->set_visibility(1);
  grndOvrl->set_draworder(0);

  // LOD.
  LodPtr lod = factory->CreateLod();
  lod->set_id("lod");
  lod->set_minlodpixels(100);
  lod->set_maxlodpixels(-1);

  // Region.
  RegionPtr region = factory->CreateRegion();
  region->set_id("region");
  region->set_lod(lod);

  DocumentPtr doc = factory->CreateDocument();
  doc->set_region(region);
  doc->add_feature(grndOvrl);

  KmlPtr kml = factory->CreateKml();
  kml->set_feature(doc);

  std::string imgFileName (name + "_img.kml");
  std::ofstream outFile((this->sysPath + imgFileName).c_str());

  if(outFile.good())
    {
    doc->set_name(vtksys::SystemTools::GetFilenameName(imgFileName));

    std::string str(kmldom::SerializePretty(kml));
    outFile << str;
    outFile.flush();
    outFile.close();

    // Add this to the list of data files generated for this time.
    this->dataFileNames.push_back(
      vtksys::SystemTools::GetFilenameName(imgFileName));

    return 1;
    }
  else
    {
    outFile.close();
    std::cerr << "Error writing data to output stream." << std::endl;
    return 0;
    }
}

//-----------------------------------------------------------------------------
int vtkKMLExporterInternal::WriteAnActor(vtkActor* anActor, int index)
{
  if(!anActor)
    {
    std::cerr << "Invalid vtkActor." << std::endl;
    return 0;
    }

  vtkDataSet*     ds;
  vtkPolyData*    pd;
  vtkImageData*   id;
  vtkLookupTable* lut;

  // see if the actor has a mapper. it could be an assembly
  if (anActor->GetMapper() == NULL)
    {
    return 0;
    }

  if (anActor->GetVisibility() == 0)
    {
    return 0;
    }

  vtkSmartPointer<vtkDataSet> tempDs;

  vtkDataObject *dObj = anActor->GetMapper()->GetInputDataObject(0, 0);
  vtkCompositeDataSet* cd = vtkCompositeDataSet::SafeDownCast(dObj);
  if(cd)
    {
    vtkNew<vtkCompositeDataGeometryFilter> gf;
    gf->SetInputData(cd);
    gf->Update();
    tempDs = gf->GetOutput();
    ds = tempDs;
    }
  else
    {
    ds = anActor->GetMapper()->GetInput();
    }

  if(!ds)
    {
    std::cerr << "No input to the mapper." << std::endl;
    return 0;
    }

  lut = vtkLookupTable::SafeDownCast(anActor->GetMapper()->GetLookupTable());

  if(ds->GetDataObjectType() == VTK_POLY_DATA)
    {
    pd = vtkPolyData::SafeDownCast(ds);

    // We have found that more than 10,000 polygons causes GE to crash or
    // stuck while rendering.
    if(pd->GetNumberOfCells() > exporter->GetCellCountThreshold() ||
       this->exporter->GetRenderPolyDataAsImage())
      {
      vtkSmartPointer<vtkImageData> imgData;
      imgData.TakeReference(this->ConvertToImage(anActor));

      if(!this->WriteImage(imgData, index))
        {
        return 0;
        }
      }
    else
      {
      if(!this->WriteGeometry(pd, index))
        {
        return 0;
        }
      }
    }
  else if(ds->GetDataObjectType() == VTK_IMAGE_DATA)
    {
    // @note: Not implemented.
    id = static_cast<vtkImageData*>(ds);
    if(!this->WriteImage(id))
      {
      return 0;
      }
    }
  else
    {
    return 0;
    }

  return 1;
}


int vtkKMLExporterInternal::AddOverlay(DocumentPtr doc)
{
  if(!doc)
    {
    std::cerr << "Invalid KML Document. " << std::endl;
    return 0;
    }

  if(this->ovrlFileName.empty())
    {
    std::cerr << "Empty overlay file name. " << std::endl;
    return 0;
    }

  KmlFactory* factory = KmlFactory::GetFactory();
  LinkPtr lnk = factory->CreateLink();
  lnk->set_href(vtksys::SystemTools::GetFilenameName(this->ovrlFileName));
  NetworkLinkPtr netLnk = factory->CreateNetworkLink();
  netLnk->set_flytoview(0);
  netLnk->set_link(lnk);
  doc->add_feature(netLnk);

  this->ovrlFileName.clear();

  return 1;
}


//-----------------------------------------------------------------------------
int vtkKMLExporterInternal::WriteOverlay(vtkImageData *imageData)
{
  if(!imageData)
    {
    std::cerr << "Invalid vtkImageData. " << std::endl;
    return 0;
    }

  KmlFactory* factory = KmlFactory::GetFactory();
  ScreenOverlayPtr scrnOvrl = factory->CreateScreenOverlay();

  // Image space.
  OverlayXYPtr ovrXY = factory->CreateOverlayXY();
  ovrXY->set_x(0.0);
  ovrXY->set_y(1.0);
  ovrXY->set_xunits(0);
  ovrXY->set_yunits(0);

  // Screen space.
  ScreenXYPtr scrXY  = factory->CreateScreenXY();
  scrXY->set_x(0.05);   // Leaving some space for padding.
  scrXY->set_y(0.95);   // Leaving some space for padding.
  scrXY->set_xunits(0);
  scrXY->set_yunits(0);

  //
  SizePtr size = factory->CreateSize();
  size->set_x(this->exporter->GetLegendScale());

  if(this->exporter->GetKeepLegendAspectRatio())
    {
    size->set_y(0.0);
    }
  else
    {
    size->set_y(this->exporter->GetLegendScale());
    }
  size->set_xunits(0);
  size->set_yunits(0);

  scrnOvrl->set_overlayxy(ovrXY);
  scrnOvrl->set_screenxy(scrXY);
  scrnOvrl->set_size(size);

  vtkNew<vtkPNGWriter> pngWriter;
  pngWriter->SetInputData(imageData);

  std::string compOvrlPNGFileName (this->sysPath + this->fileNamePrefix +
                                      "_overlay.png");
  pngWriter->SetFileName(compOvrlPNGFileName.c_str());
  pngWriter->Update();
  pngWriter->Write();

  IconPtr icon = factory->CreateIcon();
  icon->set_href(vtksys::SystemTools::GetFilenameName(compOvrlPNGFileName));
  scrnOvrl->set_icon(icon);

  DocumentPtr doc = factory->CreateDocument();
  doc->add_feature(scrnOvrl);

  KmlPtr kml = factory->CreateKml();
  kml->set_feature(doc);

  std::string ovrlayFileName(this->fileNamePrefix + "_overlay.kml");
  std::ofstream fs((this->sysPath + ovrlayFileName).c_str());
  if(fs.good())
    {
    doc->set_name(ovrlayFileName);

    fs << kmldom::SerializePretty(kml);
    fs.flush();
    fs.close();

    this->ovrlFileName = ovrlayFileName;

    return 1;
    }
  else
    {
    std::cerr << "Failed to write overlay KML. " << std::endl;
    return 0;
    }
}

//-----------------------------------------------------------------------------
int vtkKMLExporterInternal::WriteCurrentTimeStep()
{
  KmlFactory* factory = KmlFactory::GetFactory();
  DocumentPtr doc = factory->CreateDocument();

  for(size_t i=0; i < this->dataFileNames.size(); ++i)
      {
      LinkPtr lnk = factory->CreateLink();
      lnk->set_href(vtksys::SystemTools::GetFilenameName(this->dataFileNames[i]));
      NetworkLinkPtr netLnk = factory->CreateNetworkLink();
      netLnk->set_flytoview(0);
      netLnk->set_link(lnk);
      doc->add_feature(netLnk);
      }

  if(!this->exporter->GetWriteTimeSeriesData())
    {
    this->AddOverlay(doc);
    }

  KmlPtr kml = factory->CreateKml();
  kml->set_feature(doc);

  std::string dataFileName (this->sysPath + this->fileNamePrefix + ".kml");
  std::ofstream fs(dataFileName.c_str());
  if(fs.good())
    {
    doc->set_name(vtksys::SystemTools::GetFilenameName(dataFileName));

    fs << kmldom::SerializePretty(kml);
    fs.flush();
    fs.close();

    this->dataFileNames.clear();
    return 1;
    }
  else
    {
    std::cerr << "Failed to write current time step data. " << std::endl;
    return 0;
    }
}

//-----------------------------------------------------------------------------
int vtkKMLExporterInternal::WriteAnimation()
{
  KmlFactory* factory = KmlFactory::GetFactory();
  DocumentPtr doc = factory->CreateDocument();

  for(size_t i=0; i < this->timeStepFileNames.size(); ++i)
    {
    LinkPtr lnk = factory->CreateLink();
    lnk->set_href(vtksys::SystemTools::GetFilenameName(
      this->timeStepFileNames[i]));

    TimeSpanPtr timeSpan = factory->CreateTimeSpan();

    // If user has specified start date time for each time step.
    if(this->startDateTimeList.size() > 0)
      {
      if(this->startDateTimeList.size() != this->timeStepFileNames.size())
        {
        std::cerr << "Number of files and number of start date / time "
          << "does not match." << std::endl;
        return 0;
        }

      std::string out = startDateTimeList.at(i)->ToW3UTCDateTimeString();
      timeSpan->set_begin(out);

      if(!this->exporter->GetShowPrevious())
        {
        if((i + 1) < startDateTimeList.size())
          {
          std::string out = startDateTimeList.at(i + 1)->ToW3UTCDateTimeString();
          timeSpan->set_end(out);
          }
        }
      }
    else if(this->exporter->GetStartDateTime())
      {
      if(this->exporter->GetDeltaDuration() != -1)
        {
        if(!this->exporter->GetShowPrevious())
          {
          timeSpan->set_begin(this->GetNextDateTimeString(static_cast<int>(i)));
          timeSpan->set_end(this->GetNextDateTimeString(static_cast<int>(i) + 1));
          }
        else
          {
          timeSpan->set_begin(this->GetNextDateTimeString(static_cast<int>(i)));
          }
        }
      }
    else
      {
      std::cerr << "Need atleast one start date time. " << std::endl;
      return 0;
      }

    NetworkLinkPtr netLnk = factory->CreateNetworkLink();
    netLnk->set_flytoview(0);
    netLnk->set_timeprimitive(timeSpan);
    netLnk->set_link(lnk);

    doc->add_feature(netLnk);
    }

  this->AddOverlay(doc);

  KmlPtr kml = factory->CreateKml();
  kml->set_feature(doc);

  size_t pos  = this->timeStepFileNames[0].find("0.");
  std::string rootKML (this->timeStepFileNames[0].substr(0, pos));
  rootKML.append((".kml"));

  std::ofstream fs(rootKML.c_str());
  if(fs.good())
    {
    doc->set_name(vtksys::SystemTools::GetFilenameName(rootKML));
    fs << kmldom::SerializePretty(kml);
    fs.flush();
    fs.close();

    this->timeStepFileNames.clear();
    return 1;
    }
  else
    {
    std::cerr << "Failed to write super KML. " << std::endl;
    return 0;
    }
}

//-----------------------------------------------------------------------------
std::string vtkKMLExporterInternal::GetNextDateTimeString(int index)
{
  vtkDateTime* dateTime(0);
  std::string out("");

  if(this->exporter->GetDeltaDuration() != -1)
    {
    switch(this->exporter->GetDeltaDateTimeUnit())
      {
      case 0:
        dateTime = this->exporter->GetStartDateTime()->CreateNewAddYears(
          this->exporter->GetDeltaDuration() * index);
        break;
      case 1:
        return out;
      case 2:
        dateTime = this->exporter->GetStartDateTime()->CreateNewAddDays(
          this->exporter->GetDeltaDuration() * index);
        break;
      case 3:
        dateTime = this->exporter->GetStartDateTime()->CreateNewAddHours(
          this->exporter->GetDeltaDuration() * index);
        break;
      case 4:
        dateTime = this->exporter->GetStartDateTime()->CreateNewAddMinutes(
          this->exporter->GetDeltaDuration() * index);
        break;
      case 5:
        dateTime = this->exporter->GetStartDateTime()->CreateNewAddSeconds(
          this->exporter->GetDeltaDuration() * index);
        break;
      default:
        return out;
      }

    out = dateTime->ToW3UTCDateTimeString();
    }

  return out;
}

//-----------------------------------------------------------------------------
vtkRenderer* vtkKMLExporterInternal::GetRenderer(vtkRenderWindow *renWin, int index)
{
  vtkRendererCollection* rc (renWin->GetRenderers());
  vtkCollectionSimpleIterator rcIt;
  vtkRenderer* ren(0);
  for(rc->InitTraversal(rcIt); (ren = rc->GetNextRenderer(rcIt));)
    {
    if(ren->GetLayer() == index)
      {
      return ren;
      }
    else
      {
      continue;
      }
    }

  return ren;
}

//-----------------------------------------------------------------------------
void vtkKMLExporter::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkExporter::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
vtkKMLExporter::vtkKMLExporter() : vtkExporter(),
  FileName              (0),
  RenderPolyDataAsImage (true),
  RenderScene           (true),
  ShowPrevious          (false),
  WriteTimeSeriesData   (false),
  KeepLegendAspectRatio (true),
  DeltaDuration         (-1),
  DeltaDateTimeUnit     (-1),
  AltitudeMode          (0),
  Magnification         (1),
  LegendScale           (0.5),
  StartDateTime         (0),
  CellCountThreshold    (10000)
{
  Implementation = new vtkKMLExporterInternal(this);

  this->StartDateTime = vtkDateTime::New();
  this->StartDateTime->SetYear(1990);
  this->StartDateTime->SetMonth(1);
  this->StartDateTime->SetDay(1);
  this->SetDeltaDuration(0);
  this->SetDeltaDateTimeUnit(0);
  this->StartDateTime->Update();
}

//-----------------------------------------------------------------------------
vtkKMLExporter::~vtkKMLExporter()
{
  if(this->StartDateTime)
    {
    this->StartDateTime->Delete();
    this->StartDateTime = 0;
    }

  if(this->Implementation)
    {
    delete this->Implementation;
    this->Implementation = 0x0;
    }
}

//-----------------------------------------------------------------------------
void vtkKMLExporter::SetStartDateTime(vtkDateTime* dateTime)
{
  if(!dateTime)
    {
    vtkErrorMacro("Invalid vtkDateTime.");
    return;
    }

  if(this->StartDateTime == dateTime)
    {
    return;
    }

  if(this->StartDateTime)
    {
    this->StartDateTime->UnRegister(this);
    }

  this->StartDateTime = dateTime;
  this->StartDateTime->Register(this);
  this->StartDateTime->Update();
}

//-----------------------------------------------------------------------------
void vtkKMLExporter::AddStartDateTime(vtkDateTime *dt)
{
  if(!dt)
    {
    vtkErrorMacro("Not a valid date time unit.");
    return;
    }

  this->Implementation->startDateTimeList.push_back(dt);
}

//-----------------------------------------------------------------------------
int vtkKMLExporter::WriteAnimation()
{
  return this->Implementation->WriteAnimation();
}

//-----------------------------------------------------------------------------
void vtkKMLExporter::WriteData()
{
  vtkRenderer*        ren;
  vtkActorCollection* ac;
  vtkActor*           anActor;
  vtkActor*           aPart;

  if(this->FileName == NULL)
    {
    vtkErrorMacro("Please specify the file name to use. ");
    return;
    }

  // Get the 3d renderer (its on layer 0).
  ren = this->Implementation->GetRenderer(this->GetRenderWindow(), 0);

  if(ren->GetActors()->GetNumberOfItems() < 1)
    {
    vtkErrorMacro("No actors found for writing MKL file.");
    return;
    }

  std::string path =
    vtksys::SystemTools::GetFilenamePath(this->GetFileName());

  if(!path.empty())
    {
  // Get the system path.
#if defined(_WIN32) || defined (_WIN64)
    this->Implementation->sysPath = std::string(path + "\\");
#else
    this->Implementation->sysPath = std::string(path + "/");
#endif
    }
  else
    {
    this->Implementation->sysPath.clear();
    }

  // Get the file name prefix.
  this->Implementation->fileNamePrefix =
    vtksys::SystemTools::GetFilenameWithoutExtension(this->FileName);

  //@note: Not tested.
  if(!this->RenderScene)
    {
    int count = 1;
    std::vector<vtkActor*> actors;
    std::vector<size_t>    invIndices;
    ac = ren->GetActors();
    vtkAssemblyPath *apath;
    for (ac->InitTraversal(); (anActor = ac->GetNextActor()); )
      {
      for (anActor->InitPathTraversal(); (apath=anActor->GetNextPath()); )
        {
        aPart=static_cast<vtkActor *>(apath->GetLastNode()->GetViewProp());
        actors.push_back(aPart);
        }
      }

    for(size_t i=0; i < actors.size(); ++i)
      {

      // First make sure all other visible actors are set to not visible.
      for(size_t j=0; j < actors.size(); ++j)
        {
        if(i != j && actors[j]->GetVisibility())
          {
          actors[j]->SetVisibility(0);
          invIndices.push_back(j);
          }
        }

      // Write the current actor.
      this->Implementation->WriteAnActor(actors[i], count++);

      // Restore state.
      for(size_t j=0; j < invIndices.size(); ++j)
        {
        actors[ invIndices[j] ]->SetVisibility(1);
        }
      }
    }
  else
    {
    // Here first disable the 3d renderer and get the image.
    // Then disable the legend renderer and get the image.
    vtkRenderWindow* renWin = this->GetRenderWindow();

    vtkRenderer* twoDRen = this->Implementation->GetRenderer(renWin, 2);
    int twoDRenSt (1);
    if(twoDRen)
      {
      // Save the current state.
      twoDRenSt = twoDRen->GetDraw();
      twoDRen->DrawOff();
      }

    // Draw 3d scene as image.
    vtkSmartPointer<vtkImageData> imageData;
    imageData.TakeReference(
      this->Implementation->ConvertToImage(this->RenderWindow));

    if(twoDRen)
      {
      // Restore.
      twoDRen->SetDraw(twoDRenSt);
      }

    if(!this->Implementation->WriteImage(imageData))
      {
      vtkErrorMacro("Failed to write KML.");
      return;
      }

    if(!this->Implementation->wroteOverlay && twoDRen)
      {
      // Draw 2d stuff as image.
      int threeDRenSt (1);
      vtkRenderer* threeDRen = this->Implementation->GetRenderer(renWin, 0);
      if(threeDRen)
        {
        // Save the current state.
        threeDRenSt = threeDRen->GetDraw();
        threeDRen->SetDraw(0);
        threeDRen->Clear();
        }

      vtkSmartPointer<vtkImageData> twoDImageData;
      twoDImageData.TakeReference(
        this->Implementation->ConvertToImage(this->RenderWindow, false));

      if(threeDRen)
        {
        // Restore.
        threeDRen->SetDraw(threeDRenSt);
        }

      this->Implementation->WriteOverlay(twoDImageData);

      this->Implementation->wroteOverlay = true;
      } // writtenOverlay.

      // Force render again.
      renWin->Render();
    }

  // Finish writing current time step.
  this->Implementation->WriteCurrentTimeStep();

  // Push the filenames written in current time step to the list of files
  // for animations.
  if(this->WriteTimeSeriesData)
    {
    this->Implementation->timeStepFileNames.push_back(this->FileName);
    }
}

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkKMLReader.h"

// KML includes.
#include <kml/dom.h>

#include <kml/engine/kml_file.h>
#include <kml/engine/style_resolver.h>

// C++ includes.
#include <ios>
#include <iostream>
#include <string>
#include <sstream>

// C includes.
#include <stdio.h>

// VTK includes.
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkDoubleArray.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkIntArray.h>
#include <vtkLine.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataReader.h>
#include <vtkPolygon.h>
#include <vtkPolyLine.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkVertex.h>
#include <vtkXMLPolyDataWriter.h>

#include <vtksys/SystemTools.hxx>

// VTKExtensions includes.
#include "smtk/extension/vtk/meshing/vtkCMBPrepareForTriangleMesher.h"
#include "smtk/extension/vtk/meshing/vtkCMBTriangleMesher.h"

// Import types.
using kmldom::AbstractViewPtr;
using kmldom::CoordinatesPtr;
using kmldom::DocumentPtr;
using kmldom::ElementPtr;
using kmldom::FeaturePtr;
using kmldom::FolderPtr;
using kmldom::GeometryPtr;
using kmldom::IconStylePtr;
using kmldom::InnerBoundaryIsPtr;
using kmldom::KmlPtr;
using kmldom::LinearRingPtr;
using kmldom::LineStringPtr;
using kmldom::LineStylePtr;
using kmldom::LookAtPtr;
using kmldom::MultiGeometryPtr;
using kmldom::OuterBoundaryIsPtr;
using kmldom::PlacemarkPtr;
using kmldom::PointPtr;
using kmldom::PolygonPtr;
using kmldom::PolyStylePtr;
using kmldom::StylePtr;
using kmldom::StyleSelectorPtr;

using kmlengine::KmlFilePtr;

// @NOTE: Not supporting KMZ.
//using kmlengine::KmzFilePtr;

vtkStandardNewMacro(vtkKMLReader);

//-----------------------------------------------------------------------------
// Implementation class.
class vtkKMLReaderInternal
{
public:
  vtkKMLReaderInternal();
 ~vtkKMLReaderInternal();


  FeaturePtr  GetRootFeature(ElementPtr elem);
  DocumentPtr GetDocument   (FeaturePtr feature);

  void ConvertHexToRGB      (std::string hexStr,
                             int& r, int& g, int& b);

  vtkPolyData*
       HandleFile           (const char* fileName);

  // Currently supported KML tags.
  void HandleDocument       (DocumentPtr documentPtr);
  void HandleFeature        (FeaturePtr featurePtr);
  void HandleFolder         (FolderPtr folderPtr);
  void HandleGeometry       (GeometryPtr geometryPtr, StylePtr stylePtr);
  void HandleLineString     (LineStringPtr linePtr, LineStylePtr lineStylePtr);
  void HandleLookAt         (LookAtPtr lookAtPtr);
  void HandleMultiGeometry  (MultiGeometryPtr multiGeometryPtr, StylePtr stylePtr);
  void HandlePlacemark      (PlacemarkPtr placemarkPtr);
  void HandlePoint          (PointPtr pointPtr, IconStylePtr iconStylePtr);
  void HandlePolygon        (PolygonPtr polygonPtr, PolyStylePtr polyStylePtr);
  void HandleStyle          (StylePtr stylePtr);
  void HandleStyleSelector  (StyleSelectorPtr styleSelectorPtr);
  void HandleStyleUrl       ();

  void Triangulate          (vtkPolyData* polyData);

  // Global variables.
  ElementPtr kmlFile;

  std::map<std::string, StylePtr>             styles;
  typedef std::map<std::string, StylePtr>::const_iterator
                                              StylesItr;

  vtkSmartPointer<vtkCellArray> polys;
  vtkSmartPointer<vtkCellArray> lines;
  vtkSmartPointer<vtkCellArray> vertices;
  vtkSmartPointer<vtkPoints>    points;
  vtkSmartPointer<vtkPolyData>  polyData;
  vtkSmartPointer<vtkUnsignedCharArray>
                                colors;

  vtkSmartPointer<vtkIntArray>  polyDataIds;
  vtkSmartPointer<vtkIntArray>  polygonIds;

  struct KMLInternalPt
    {
    double x,y,z;
    friend bool operator < (const KMLInternalPt& l,const KMLInternalPt& r)
      {
      //Makes sure that points can't be equal unless all 3 components
      //are the same. Ordering doesn't really matter
      return l.x != r.x ? (l.x < r.x) : l.y != r.y ? (l.y < r.y) : l.z < r.z;
      }
    KMLInternalPt(double _x, double _y, double _z):x(_x),y(_y),z(_z){};
    };
};

//-----------------------------------------------------------------------------
vtkKMLReaderInternal::vtkKMLReaderInternal()
{
 points  = vtkSmartPointer<vtkPoints>::New();
 polys   = vtkSmartPointer<vtkCellArray>::New();
 lines   = vtkSmartPointer<vtkCellArray>::New();
 vertices= vtkSmartPointer<vtkCellArray>::New();
 colors  = vtkSmartPointer<vtkUnsignedCharArray>::New();
 colors->SetNumberOfComponents(3);

 polyDataIds = vtkSmartPointer<vtkIntArray>::New();
 polyDataIds->SetName("PolyDataIds");
 polyDataIds->SetNumberOfComponents(1);
 polyDataIds->Initialize();

 polygonIds = vtkSmartPointer<vtkIntArray>::New();
 polygonIds->SetName("PolygonIds");
 polygonIds->SetNumberOfComponents(1);
 polygonIds->Initialize();

 polyData =vtkSmartPointer<vtkPolyData>::New();
 polyData->SetPoints(points);
 polyData->SetPolys(polys);
 polyData->SetLines(lines);
 polyData->SetVerts(vertices);
}

//-----------------------------------------------------------------------------
vtkKMLReaderInternal::~vtkKMLReaderInternal()
{
}

//-----------------------------------------------------------------------------
void vtkKMLReaderInternal::ConvertHexToRGB(std::string hexStr,
                                           int& r, int& g, int& b)
{
  std::istringstream ss (hexStr);
  int ihex;
  ss >> std::hex >> ihex;
  r = (ihex >> 16)  & 0xFF;
  g = (ihex >> 8)   & 0xFF;
  b = (ihex)        & 0xFF;
}

//-----------------------------------------------------------------------------
void vtkKMLReaderInternal::HandleFeature(FeaturePtr featurePtr)
{
  if(!featurePtr)
    {
    return;
    }

  if(kmldom::AsPlacemark(featurePtr))
    {
    this->HandlePlacemark(kmldom::AsPlacemark(featurePtr));
    }
  if(kmldom::AsDocument(featurePtr))
    {
    this->HandleDocument(kmldom::AsDocument(featurePtr));
    }
  if(kmldom::AsFolder(featurePtr))
    {
    this->HandleFolder(kmldom::AsFolder(featurePtr));
    }
}

//-----------------------------------------------------------------------------
vtkPolyData* vtkKMLReaderInternal::HandleFile(const char* fileName)
{
  if(!fileName)
    {
    return 0;
    }

  std::string ext = vtksys::SystemTools::GetFilenameExtension(fileName);

  // @NOTE: Not supporting KMZ as of now.
  if(ext.compare(".kmz") == 0)
     {
//     std::vector<string> kmls;
//     KmzFilePtr kmzFile = kmlengine::KmzFile::OpenFromFile(fileName);
//     kmzFile->List(&kmls);

//     for(size_t i=0; i < kmls.size(); ++i)
//       {
//       std::string output;
//       kmzFile->ReadKml(&output);
//       std::cout << output << std::endl;
//       kmlFile = kmldom::ParseKml(output);
//       if(!kmlFile)
//         {
//         std::cerr << "Error parsing KML. " << std::endl;
//         std::exit(0);
//         }

//       FeaturePtr feature = GetRootFeature(kmlFile);
//       this->HandleFeature(feature);
//       }
     }
  else
    {
    // Parse the file.
    FILE* filePtr;
    long  length;
    char* buffer;

    filePtr = fopen(fileName,"rb");
    fseek(filePtr,0,SEEK_END);        //go to end

    length = ftell(filePtr);          //get position at end (length)
    fseek(filePtr,0,SEEK_SET);        //go to beg.

    buffer=static_cast<char *>(malloc(length));                      //malloc buffer
    /*size_t bytes_read =*/ fread(buffer,length,1,filePtr); //read into buffer

    fclose(filePtr);

    buffer[length-1] = '\0'; //Make sure the last character is a null terminator

    std::string xmlStr (buffer);
    std::string errors;

    this->kmlFile = kmldom::Parse(xmlStr, &errors);
    if(!this->kmlFile)
       {
       std::cerr << "Error parsing KML. " << std::endl;
       std::cerr << "Error message: " << errors << std::endl;
       return 0;
       }

     FeaturePtr feature = this->GetRootFeature(this->kmlFile);
     this->HandleFeature(feature);
     }

  // Now insert cell data.
  for(int i=0; i < vertices->GetNumberOfCells();++i)
    {
    polyDataIds->InsertNextValue(-1);
    }
  for(int i=0; i < lines->GetNumberOfCells();++i)
    {
    polyDataIds->InsertNextValue(-1);
    }
  for(int i=0; i < polygonIds->GetNumberOfTuples();++i)
    {
    polyDataIds->InsertNextValue(polygonIds->GetValue(i));
    }

  polyData->GetCellData()->AddArray(polyDataIds);

  return polyData;
}


//-----------------------------------------------------------------------------
FeaturePtr vtkKMLReaderInternal::GetRootFeature(ElementPtr elem)
{
  if(!elem)
    {
    return 0;
    }

  KmlPtr kml = kmldom::AsKml(elem);
  if(kml)
    {
    return kml->get_feature();
    }
  else
    {
    return kmldom::AsFeature(elem);
    }
}

//-----------------------------------------------------------------------------
DocumentPtr vtkKMLReaderInternal::GetDocument(FeaturePtr feature)
{
  if(!feature)
    {
    return 0;
    }

  return kmldom::AsDocument(feature);
}

//-----------------------------------------------------------------------------
void vtkKMLReaderInternal::HandlePlacemark(PlacemarkPtr placemarkPtr)
{
  if(!placemarkPtr)
    {
    return;
    }

  // This the case when style is referenced.
  StylePtr stylePtr (0);
  if(placemarkPtr->has_styleurl())
    {
    std::string styleURL = placemarkPtr->get_styleurl();
    StylesItr itr = styles.find(styleURL);
    if(itr != styles.end())
      {
      stylePtr = itr->second;
      if(stylePtr)
        {
        // Do nothing.
        }
      }
    else
      {
        // Do nothing.
      }

    // @TODO: Handle style maps here?
    }

  // This is the case when style is within the placemark.
  if(placemarkPtr->has_styleselector())
    {
    PolyStylePtr polyStyle = kmldom::AsStyle(placemarkPtr->get_styleselector())->get_polystyle();
    if(polyStyle)
      {
      // Do nothing.
      }
    }

  if(placemarkPtr->has_geometry())
    {
    this->HandleGeometry(placemarkPtr->get_geometry(), stylePtr);
    }
}

//-----------------------------------------------------------------------------
void vtkKMLReaderInternal::HandleDocument(DocumentPtr documentPtr)
{
  if(!documentPtr)
    {
    return;
    }

  // Documents can have more than one style selector.
  size_t numberOfStyles = documentPtr->get_styleselector_array_size();
  for(size_t i=0; i < numberOfStyles; ++i)
    {
    this->HandleStyleSelector(documentPtr->get_styleselector_array_at(i));
    }

  size_t numberOfFeatures = documentPtr->get_feature_array_size();
  for(size_t i=0; i < numberOfFeatures; ++i)
    {
    FeaturePtr feature = documentPtr->get_feature_array_at(i);
    this->HandleFeature(feature);
    }
}

//-----------------------------------------------------------------------------
void vtkKMLReaderInternal::HandleFolder(FolderPtr folderPtr)
{
  if(!folderPtr)
    {
    return;
    }

  if(folderPtr->has_styleselector())
    {
    this->HandleStyleSelector(folderPtr->get_styleselector());
    }

  for(size_t i=0; i < folderPtr->get_feature_array_size(); ++i)
    {
    FeaturePtr feature = folderPtr->get_feature_array_at(i);
    this->HandleFeature(feature);
    }
}

//-----------------------------------------------------------------------------
void vtkKMLReaderInternal::HandleGeometry(GeometryPtr geometryPtr, StylePtr stylePtr)
{
  if(MultiGeometryPtr multiGeometryPtr = kmldom::AsMultiGeometry(geometryPtr))
    {
    this->HandleMultiGeometry(multiGeometryPtr, stylePtr);
    }

  if(PolygonPtr poly = kmldom::AsPolygon(geometryPtr))
    {
    if(stylePtr)
      {
      this->HandlePolygon(poly, stylePtr->get_polystyle());
      }
    else
      {
      this->HandlePolygon(poly, 0);
      }
    }

  if(LineStringPtr line = kmldom::AsLineString(geometryPtr))
    {
    if(stylePtr)
      {
      this->HandleLineString(line, stylePtr->get_linestyle());
      }
    else
      {
      this->HandleLineString(line, 0);
      }
    }
  if(PointPtr pnt = kmldom::AsPoint(geometryPtr))
    {
    if(stylePtr)
      {
      this->HandlePoint(pnt, stylePtr->get_iconstyle());
      }
    else
      {
      this->HandlePoint(pnt, 0);
      }
    }
}

//-----------------------------------------------------------------------------
void vtkKMLReaderInternal::HandleMultiGeometry(MultiGeometryPtr multiGeometryPtr, StylePtr stylePtr)
{
  if(!multiGeometryPtr)
    {
    return;
    }

  size_t sizeOfGeometryArray = multiGeometryPtr->get_geometry_array_size();
  for(size_t i=0; i < sizeOfGeometryArray; ++i)
    {
    GeometryPtr geom = multiGeometryPtr->get_geometry_array_at(i);
    this->HandleGeometry(geom, stylePtr);
    }
}

//-----------------------------------------------------------------------------
void vtkKMLReaderInternal::HandleLookAt(LookAtPtr /*lookAtPtr*/)
{
  // Not implemented yet.
}

//-----------------------------------------------------------------------------
void vtkKMLReaderInternal::HandleStyleUrl()
{
  // Not implemented yet.
}

//-----------------------------------------------------------------------------
void vtkKMLReaderInternal::HandleStyle(StylePtr stylePtr)
{
  if(!stylePtr)
    {
    return ;
    }

  std::string id = stylePtr->get_id();
  std::string url("#");
  url.append(id);

  styles.insert(std::pair<std::string, StylePtr>(url, stylePtr));
}

//-----------------------------------------------------------------------------
void vtkKMLReaderInternal::HandleStyleSelector(StyleSelectorPtr styleSelectorPtr)
{
  if(!styleSelectorPtr)
    {
    return;
    }

  if( kmldom::StylePtr stylePtr = kmldom::AsStyle(styleSelectorPtr) )
    {
    this->HandleStyle(stylePtr);
    }
  if( kmldom::StyleMapPtr styleMapPtr = kmldom::AsStyleMap(styleSelectorPtr) )
    {
    // Not implemented yet.
    // @TODO: Implement this if required.
    }
}

//-----------------------------------------------------------------------------
// @TODO: What to do with the IconStyle?
void vtkKMLReaderInternal::HandlePoint(PointPtr pointPtr, IconStylePtr /*iconStylePtr*/)
{
  if(!pointPtr)
    {
    return;
    }

  vtkSmartPointer<vtkVertex> vtx (vtkSmartPointer<vtkVertex>::New());
  if(pointPtr->has_coordinates())
    {
    CoordinatesPtr coordinatesPtr = pointPtr->get_coordinates();
    size_t size = coordinatesPtr->get_coordinates_array_size();
    for(size_t i=0; i < size; ++i)
      {
      kmlbase::Vec3 vec = coordinatesPtr->get_coordinates_array_at(i);
      points->InsertNextPoint(vec.get_longitude(), vec.get_latitude(), vec.get_altitude());
      vtx->GetPointIds()->InsertNextId(points->GetNumberOfPoints() - 1);
      }

    vertices->InsertNextCell(vtx);
    }
}

//-----------------------------------------------------------------------------
void vtkKMLReaderInternal::HandleLineString(LineStringPtr linePtr, LineStylePtr lineStylePtr)
{
  if(!linePtr)
    {
    return;
    }

  bool hasColor(false);

  if(linePtr->has_coordinates())
    {
    vtkSmartPointer<vtkPolyLine>    polyLine (vtkSmartPointer<vtkPolyLine>::New());

    if(lineStylePtr)
      {
      // Handle color property.
      if(lineStylePtr->has_color())
        {
        kmlbase::Color32 color = lineStylePtr->get_color();
        unsigned char vtkColor[3] = {static_cast<unsigned char>(color.get_red()),
                                     static_cast<unsigned char>(color.get_green()),
                                     static_cast<unsigned char>(color.get_blue())
                                     };
        colors->InsertNextTupleValue(vtkColor);
        }
      }

    if(!hasColor)
      {
      unsigned char vtkColor[3] = {static_cast<unsigned char>(255),
                                   static_cast<unsigned char>(255),
                                   static_cast<unsigned char>(255)};
      colors->InsertNextTupleValue(vtkColor);
      }

    CoordinatesPtr coordinatesPtr = linePtr->get_coordinates();

    size_t coordinatesArraySize = coordinatesPtr->get_coordinates_array_size();

    int numberOfPoints = points->GetNumberOfPoints();

    for(size_t i=0; i < coordinatesArraySize; ++i)
      {
      kmlbase::Vec3 vec = coordinatesPtr->get_coordinates_array_at(i);
      points->InsertNextPoint(vec.get_longitude(),
                              vec.get_latitude(),
                              vec.get_altitude());

      polyLine->GetPointIds()->InsertNextId(numberOfPoints + i);
      }

    lines->InsertNextCell(polyLine);
    }
}

//-----------------------------------------------------------------------------
void vtkKMLReaderInternal::HandlePolygon(PolygonPtr polygonPtr, PolyStylePtr polyStylePtr)
{
  if(!polygonPtr)
    {
    return;
    }

  vtkSmartPointer<vtkPolyData> tempPoly;
  vtkSmartPointer<vtkCellArray>tempCells;
  vtkSmartPointer<vtkPoints>   tempPoints;

  typedef smtk::vtk::vtkCMBPrepareForTriangleMesher vtkPrepareForMesher;
  vtkNew<vtkPrepareForMesher> mapInterface;

  std::map<KMLInternalPt, vtkIdType> pt2Id; //prevent duplicate points

  // @NOTE: Currently we support fill and color only.
  vtkSmartPointer<vtkCell>  cell(0);
  bool                      renderAsPolyLine(false);
  bool                      hasColor(false);
  bool                      triangulate(false);

  if(polyStylePtr)
    {
    // Handle fill property.
    if(polyStylePtr->has_fill())
      {
      if(!polyStylePtr->get_fill())
        {
        renderAsPolyLine = true;
        cell = vtkSmartPointer<vtkPolyLine>::New();
        }
      }

    // Handle color property.
    if(polyStylePtr->has_color())
      {
      kmlbase::Color32 color = polyStylePtr->get_color();
      unsigned char vtkColor[3] = {static_cast<unsigned char>(color.get_red()),
                                   static_cast<unsigned char>(color.get_green()),
                                   static_cast<unsigned char>(color.get_blue())};
      colors->InsertNextTupleValue(vtkColor);
      hasColor = true;
      }
    }

    if(!hasColor)
      {
      unsigned char vtkColor[3] = {static_cast<unsigned char>(255),
                                   static_cast<unsigned char>(255),
                                   static_cast<unsigned char>(255)};
      colors->InsertNextTupleValue(vtkColor);
      }

  if(!renderAsPolyLine)
    {
    cell = vtkSmartPointer<vtkPolygon>::New();
    }

  size_t sizeOfInnerBoundaryArray = polygonPtr->get_innerboundaryis_array_size();
  if(sizeOfInnerBoundaryArray > 0)
    {
    triangulate = true;
    }

  if(polygonPtr->has_outerboundaryis())
    {
    OuterBoundaryIsPtr outerBoundaryPtr = polygonPtr->get_outerboundaryis();
    if(outerBoundaryPtr->has_linearring())
      {
      LinearRingPtr linearRingPtr = outerBoundaryPtr->get_linearring();
      if(linearRingPtr->has_coordinates())
        {
        CoordinatesPtr coordinatesPtr = linearRingPtr->get_coordinates();

        size_t size = coordinatesPtr->get_coordinates_array_size();

        // If number of points in coordinate array is more than 3 than we could
        // be dealing with concave polygons or points not in the same plane. Though
        // it is overkill to triagulate 4 - small number of points but for time being
        // we are doing this.
        // @TODO: Optimize this.
        if(size > 3)
          {
          triangulate = true;
          }

        if(!triangulate)
          {
          int numberOfPoints = points->GetNumberOfPoints();

          for(size_t i=0; i < size; ++i)
            {
            kmlbase::Vec3 vec = coordinatesPtr->get_coordinates_array_at(i);
            points->InsertNextPoint((vec.get_longitude()),
                                    (vec.get_latitude()),
                                    (vec.get_altitude()));

            cell->GetPointIds()->InsertNextId(numberOfPoints + i);
            }

          if(!renderAsPolyLine)
            {
            polys->InsertNextCell(cell);
            }
          else
            {
            lines->InsertNextCell(cell);
            }
          } // triangulate.
        else
          {
          tempPoly    = vtkSmartPointer<vtkPolyData>::New();
          tempCells   = vtkSmartPointer<vtkCellArray>::New();
          tempPoints  = vtkSmartPointer<vtkPoints>::New();

          tempPoints->Initialize();
          tempPoints->SetDataTypeToDouble();

          tempPoly->SetPoints(tempPoints);
          tempPoly->SetLines(tempCells);

          mapInterface->SetPolyData(tempPoly);
          //TODO it would be nice to know how many arcs / loops
          //are going to be added. Not sure if possible or not
          mapInterface->InitializeNewMapInfo();

          vtkIdType ptIds[2];
          int count = 0;

          for(size_t i=0; i < size; ++i)
            {
            kmlbase::Vec3 vec = coordinatesPtr->get_coordinates_array_at(i);
            //Use pt2Id to see if this point is a duplicate
            KMLInternalPt ptToInsert(vec.get_longitude(),
                                     vec.get_latitude(),
                                     vec.get_altitude());
            std::map<KMLInternalPt, vtkIdType>::iterator pt2IdIter = pt2Id.find(ptToInsert);
            if(pt2IdIter != pt2Id.end())
              {//Pt has been seen
              ptIds[count] = (*pt2IdIter).second;
              }
            else
              { //Pt hasn't been seen
              ptIds[count] = tempPoints->InsertNextPoint(ptToInsert.x,
                                                         ptToInsert.y,
                                                         ptToInsert.z);
              pt2Id[ptToInsert] = ptIds[count];
              }

            if(i != 0)
              {
              tempCells->InsertNextCell(2, ptIds);
              ptIds[0] = ptIds[count];
              count = 0;
              }
            ++count;
            }

          //Assuming VTK_LINES, so 3 times numCells i
          vtkIdType loopId = mapInterface->AddLoop(1,-1);
          mapInterface->AddArc(0, (tempCells->GetNumberOfCells())*3,loopId,loopId,-1,0,0);
          }
        } // has_coordinates()
      } // has_linearring()
    } // has_outerboundaryis()

  for(size_t i=0; i < sizeOfInnerBoundaryArray; ++i)
    {
    InnerBoundaryIsPtr innerBoundaryPtr = polygonPtr->get_innerboundaryis_array_at(i);
    if(innerBoundaryPtr &&  innerBoundaryPtr->has_linearring())
      {
      LinearRingPtr linearRingPtr = innerBoundaryPtr->get_linearring();
      if(linearRingPtr->has_coordinates())
        {
        CoordinatesPtr coordinatesPtr = linearRingPtr->get_coordinates();

        size_t size = coordinatesPtr->get_coordinates_array_size();

        vtkIdType ptIds[2];
        int count = 0;

        vtkIdType firstCellPos = (tempCells->GetNumberOfCells())*3;
        for(size_t j=0; j < size; ++j)
          {
          kmlbase::Vec3 vec = coordinatesPtr->get_coordinates_array_at(j);
          //Use pt2Id to see if this point is a duplicate
          KMLInternalPt ptToInsert(vec.get_longitude(),
                                   vec.get_latitude(),
                                   vec.get_altitude());

          std::map<KMLInternalPt, vtkIdType>::iterator pt2IdIter = pt2Id.find(ptToInsert);
          if(pt2IdIter != pt2Id.end())
            {//Pt has been seen
            ptIds[count] = (*pt2IdIter).second;
            }
          else
            {//Pt hasn't been seen
            ptIds[count] = tempPoints->InsertNextPoint(ptToInsert.x,
                                                       ptToInsert.y,
                                                       ptToInsert.z);
            pt2Id[ptToInsert] = ptIds[count];
            }
          if(j != 0)
            {
            tempCells->InsertNextCell(2, ptIds);

            ptIds[0] = ptIds[count];
            count = 0;
            }
          ++count;
          }
          vtkIdType loopId = mapInterface->AddLoop(-1,1);
          mapInterface->AddArc(firstCellPos,(tempCells->GetNumberOfCells())*3,loopId,-1,loopId,1+i,1+i);

          } // has_coordinates()
        } // has_linearring()
      } // has_innerboundaryis()

  mapInterface->FinalizeNewMapInfo();
  if(triangulate)
    {
    this->Triangulate(tempPoly);
    }
}

//-----------------------------------------------------------------------------
void vtkKMLReaderInternal::Triangulate(vtkPolyData* polyDataIn)
{
  typedef smtk::vtk::vtkCMBTriangleMesher vtkTriangleMesher;
  vtkNew<vtkTriangleMesher> cmbMapMesher;
  cmbMapMesher->SetMaxAreaMode(vtkTriangleMesher::NoMaxArea);
  cmbMapMesher->SetInputData(polyDataIn);
  cmbMapMesher->Update();
  vtkPolyData* output = cmbMapMesher->GetOutput();

  //when meshing fails you can't just expect to get a null data, you
  //can get an empty mesh instead.
  if(output &&
     output->GetNumberOfPoints() > 0 && output->GetNumberOfCells() > 0)
    {
    vtkPoints* outPnts = output->GetPoints();
    std::vector<int> exists(outPnts->GetNumberOfPoints(), -1);

    int lastPolygonId (0);
    if(polygonIds->GetNumberOfTuples() > 0)
      {
      //If there already were polygons start where they left off
      lastPolygonId = polygonIds->GetValue(polygonIds->GetNumberOfTuples() - 1);
      }

    int numberOfCells = output->GetNumberOfCells();
    for(int i=0; i < numberOfCells; ++i)
      {
      vtkSmartPointer<vtkIdList> list (vtkSmartPointer<vtkIdList>::New());
      output->GetCellPoints(i, list);
      int numberOfIds = list->GetNumberOfIds();
      //If the number of points is greater than 2 it is a triangle
      if(numberOfIds > 2)
        {
        vtkSmartPointer<vtkPolygon> polygon (vtkSmartPointer<vtkPolygon>::New());
        for(int j=0; j < numberOfIds; ++j)
          {
          int id = -1;
          if(exists[list->GetId(j)] == -1)
            {
            double pt[3];
            polyDataIn->GetPoints()->GetPoint(list->GetId(j),pt);
            vtkIdType ptId = points->InsertNextPoint(pt);
            exists[list->GetId(j)] = ptId;
            id = ptId;
            }
          else
            {
            id = exists[list->GetId(j)];
            }

          polygon->GetPointIds()->InsertNextId(id);
          }
        polys->InsertNextCell(polygon);

        // We want to insert the same value for all the cells.
        polygonIds->InsertNextValue(lastPolygonId + 1);
        }
     }
   }
  else
    {
    std::cerr << "Failed to tesselate the polygon." << std::endl;
    }
}


//-----------------------------------------------------------------------------
void vtkKMLReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  (this->FileName ?
    (os << indent << "FileName: " << this->FileName) :
    (os << indent << "FileName: NULL"));
}

//-----------------------------------------------------------------------------
vtkKMLReader::vtkKMLReader()
{
  this->Implementation = new vtkKMLReaderInternal();

  this->FileName = 0;

  // This is a source.
  this->SetNumberOfInputPorts(0);
}

//-----------------------------------------------------------------------------
vtkKMLReader::~vtkKMLReader()
{
  this->SetFileName(0);

  delete this->Implementation;
}

//-----------------------------------------------------------------------------
int vtkKMLReader::RequestData(vtkInformation* vtkNotUsed(request),
                              vtkInformationVector** vtkNotUsed(inputVector),
                              vtkInformationVector* outputVector)
{
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  if(!outInfo)
    {
    vtkErrorMacro("Invalid output information.\n");
    return 1;
    }

  vtkDataObject* outDataObj = outInfo->Get(vtkDataObject::DATA_OBJECT());
  if(!outDataObj)
    {
    vtkErrorMacro("Invalid output data object.\n");
    return 1;
    }

  vtkPolyData* outPolyData  = vtkPolyData::SafeDownCast(outDataObj);
  if(!outPolyData)
    {
    vtkErrorMacro("Invalid polydata.\n");
    return 1;
    }

  if(!this->FileName)
    {
    vtkErrorMacro("Requires input filename.\n");
    return 1;
    }

  vtkPolyData* out =
    this->Implementation->HandleFile(this->FileName);
  if(!out)
    {
    vtkErrorMacro("Failed to create valid output.\n");
    return 1;
    }

  outPolyData->ShallowCopy(out);

  return 1;
}

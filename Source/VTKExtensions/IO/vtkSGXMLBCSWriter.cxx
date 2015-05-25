//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkSGXMLBCSWriter.h"

#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkDoubleArray.h"
#include "vtkObjectFactory.h"

#include "vtkSmartPointer.h"
#include "vtkStringArray.h"
#include "vtkXMLDataElement.h"
#include "vtkXMLUtilities.h"

#include <iomanip>
#include <sstream>
#include <vtksys/SystemTools.hxx>



vtkStandardNewMacro(vtkSGXMLBCSWriter);
vtkCxxSetObjectMacro(vtkSGXMLBCSWriter, Coords, vtkDoubleArray);
vtkCxxSetObjectMacro(vtkSGXMLBCSWriter, ModelVertexIds, vtkIdTypeArray);

//----------------------------------------------------------------------------
vtkSGXMLBCSWriter::vtkSGXMLBCSWriter()
{
  this->SetNumberOfInputPorts(0);
  this->Coords = 0;
  this->ModelVertexIds = 0;
}

//----------------------------------------------------------------------------
vtkSGXMLBCSWriter::~vtkSGXMLBCSWriter()
{
  this->SetCoords(0);
  this->SetModelVertexIds(0);
}

//----------------------------------------------------------------------------
const char* vtkSGXMLBCSWriter::GetDefaultFileExtension()
{
  return "bcs";
}

//----------------------------------------------------------------------------
int vtkSGXMLBCSWriter::GetDataSetMajorVersion()
{
  return 2;
}

//----------------------------------------------------------------------------
int vtkSGXMLBCSWriter::GetDataSetMinorVersion()
{
  return 0;
}

//----------------------------------------------------------------------------
int vtkSGXMLBCSWriter::WriteData()
{
  if(!this->Coords || this->Coords->GetNumberOfComponents() != 3 ||
     !this->ModelVertexIds || this->ModelVertexIds->GetNumberOfComponents() != 1)
    {
    vtkErrorMacro("Problem with Coords and/or ModelVertexIds.");
    return 0;
    }

  vtkIndent indent;
  vtkIndent indent2 = indent.GetNextIndent();
  vtkIndent indent3 = indent2.GetNextIndent();
  vtkIndent indent4 = indent3.GetNextIndent();

  vtkSmartPointer<vtkXMLDataElement> bcsfile =
    vtkSmartPointer<vtkXMLDataElement>::New();
  bcsfile->SetName("BCSFile");
  std::stringstream version;
  version << this->GetDataSetMajorVersion() << "." << this->GetDataSetMinorVersion();
  bcsfile->SetAttribute("Version", version.str().c_str());
  // current objects are Points, MeshFacets, ModelFaces, Materials,
  // ModelRegions, and BoundaryConditionSets
  bcsfile->SetIntAttribute("NumberOfObjects", 6);

  //
  // write out the point data
  //

  vtkSmartPointer<vtkXMLDataElement> points =
    vtkSmartPointer<vtkXMLDataElement>::New();
  points->SetName("Points");
  points->SetAttribute("Description",
                       "Implicitly ordered array of point locations (x,y,z tuples) starting at 0 and numbered consecutively. Optionally may contain information for mapping from point Ids in this grid to another grid (e.g. mapping from point Ids in the surface mesh to point Ids in the volumetric mesh that the surface mesh was extracted from.");
  points->SetIntAttribute("NumberOfObjects", 1);
  bcsfile->AddNestedElement(points);
  vtkSmartPointer<vtkXMLDataElement> locations =
    vtkSmartPointer<vtkXMLDataElement>::New();
  locations->SetName("Data");
  locations->SetIntAttribute("NumberOfValues", this->Coords->GetNumberOfTuples()*3);
  locations->SetAttribute("type", "Float64");
  locations->SetAttribute("Description", "The point location tuples.");
  locations->SetIntAttribute("NumberOfObjects", this->Coords->GetNumberOfTuples());
  vtkIdType i;
  std::stringstream data;
  data << "\n";
  char str[1024];
  double loc[3];
  for(i=0;i<this->Coords->GetNumberOfTuples();i++)
    {
    this->Coords->GetTuple(i, loc);

    // make sure we write sufficient precision
    sprintf (str, "%.16lg %.16lg %.16lg\n", loc[0], loc[1], loc[2]);
    data << indent4 << str;
    //cout << i << " is the point " << data.str().c_str() << " " << data.gcount() << endl;
    }
  data << indent3;
  locations->AddCharacterData(data.str().c_str(), data.str().length());
  points->AddNestedElement(locations);

  //
  // write out the cell/mesh edge information
  //

  vtkSmartPointer<vtkIdTypeArray> VertexMarker =
    vtkSmartPointer<vtkIdTypeArray>::New();
  VertexMarker->SetNumberOfTuples(this->Coords->GetNumberOfTuples());
  for(i=0;i<this->Coords->GetNumberOfTuples();i++)
    {
    VertexMarker->SetValue(i, 0);
    }
  for(i=0;i<this->ModelVertexIds->GetNumberOfTuples();i++)
    {
    vtkIdType PointId = this->ModelVertexIds->GetValue(i);
    if(PointId >= 0 && PointId < this->Coords->GetNumberOfTuples())
      {
      VertexMarker->SetValue(PointId, 1);
      }
    else
      {
      vtkWarningMacro("Ignoring bad value in ModelVertexIds.");
      }
    }

  vtkIdType StartIndex = -1;
  for(i=0;i<VertexMarker->GetNumberOfTuples();i++)
    {
    if(VertexMarker->GetValue(i) != 0)
      {
      StartIndex = i;
      break;
      }
    }

  vtkSmartPointer<vtkXMLDataElement> MeshEdges =
    vtkSmartPointer<vtkXMLDataElement>::New();
  MeshEdges->SetName("MeshEdges");
  MeshEdges->SetIntAttribute("NumberOfObjects", 1);
  MeshEdges->SetAttribute("Description",
                             "Implicitly ordered cell data starting from 0 and numbered consecutively with each line being an instance of cell data.  The first number is the number of points of the cell, the second is the model edge the cell belongs to and the subsequent numbers are the point ids of the cell.");
  bcsfile->AddNestedElement(MeshEdges);
  vtkSmartPointer<vtkXMLDataElement> connectivity =
    vtkSmartPointer<vtkXMLDataElement>::New();
  connectivity->SetName("Data");
  // assume that there are only straight edges with 2 points right now
  connectivity->SetIntAttribute("NumberOfValues", VertexMarker->GetNumberOfTuples()*4);
  connectivity->SetIntAttribute("NumberOfObjects", VertexMarker->GetNumberOfTuples());
  connectivity->SetAttribute("type", "Int64");

  MeshEdges->AddNestedElement(connectivity);

  if(i != StartIndex)
    {
    vtkWarningMacro("Could not find a starting index -- assuming a single model edge.");
    StartIndex = 0; // single model edge so we arbitrarily set StartIndex as 0
    VertexMarker->SetValue(0, 1);
    }
  std::stringstream conn;
  conn << "\n";
  unsigned int CurrentModelEdgeId = 0;
  for(i=StartIndex;i<VertexMarker->GetNumberOfTuples()-1;i++)
    {
    conn << indent4 << " 2 " << CurrentModelEdgeId << " " << i << " "
         << i+1 << endl;
    if(VertexMarker->GetValue(i+1))
      {
      CurrentModelEdgeId++;
      }
    }
  // deal with the last point which connects to the first point
  conn << indent4 << " 2 " << CurrentModelEdgeId << " "
       << VertexMarker->GetNumberOfTuples()-1 << " 0\n";
  if(VertexMarker->GetValue(0))
    { // we need this here otherwise we'll end up with one less model edge
    // than we actually have
    CurrentModelEdgeId++;
    }
  // now finish up with the last model edge
  for(i=0;i<StartIndex;i++)
    {
    conn << indent4 << " 2 " << CurrentModelEdgeId << " " << i << " "
         << i+1 << endl;
    if(VertexMarker->GetValue(i+1))
      {
      CurrentModelEdgeId++;
      }
    }
  conn << indent3;
  connectivity->AddCharacterData(conn.str().c_str(), conn.str().length());
  vtkIdType NumberOfModelEdges = CurrentModelEdgeId;

  //
  // write out the material data -- there is only a single material for now
  //
  vtkSmartPointer<vtkXMLDataElement> materials =
    vtkSmartPointer<vtkXMLDataElement>::New();
  materials->SetName("Materials");
  materials->SetIntAttribute("NumberOfObjects", 1);
  materials->SetAttribute("Description",
                          "Implicitly ordered materials starting at 0 and numbered consecutively along with a Name of the material that can be set and a Unique Persistent Id of the material.");
  bcsfile->AddNestedElement(materials);

  int MaterialUniquePersistentId = 0;
  vtkSmartPointer<vtkXMLDataElement> material =
    vtkSmartPointer<vtkXMLDataElement>::New();
  material->SetName("Material");
  material->SetIntAttribute("UniquePersistentId", MaterialUniquePersistentId);
  material->SetAttribute("Name", "SingleMaterial");
  materials->AddNestedElement(material);

  //
  // write out the model face data
  //
  vtkSmartPointer<vtkXMLDataElement> modelFaces =
    vtkSmartPointer<vtkXMLDataElement>::New();
  modelFaces->SetName("ModelFaces");
  modelFaces->SetIntAttribute("NumberOfObjects", 1);
  modelFaces->SetAttribute("Description",
                           "Implicitly ordered model faces starting at 0 and numbered consecutively.  A model face is an aggregation of mesh facets that is used to specify information collectively for the mesh facets.  The model face has two associated model regions and User specified Name. ModelRegionId1 corresponds to the side of the model face that the mesh facets' normals point towards (using a counterclockwise ordering). A -1 indicates that there is no associated model region for the indicated side of the model face. If the problem is 2D then there are no associated model regions but there is an associated material instead.");
  bcsfile->AddNestedElement(modelFaces);
  vtkSmartPointer<vtkXMLDataElement> modelFace =
    vtkSmartPointer<vtkXMLDataElement>::New();
  modelFace->SetName("ModelFace");
  modelFace->SetIntAttribute("MaterialId", 0);
  modelFace->SetIntAttribute("ModelRegionId0", -1);
  modelFace->SetIntAttribute("ModelRegionId1", -1);
  modelFace->SetAttribute("Name", "SingleModelFace");
  modelFaces->AddNestedElement(modelFace);

  //
  // write out the model edge data
  //
  vtkSmartPointer<vtkXMLDataElement> modelEdges =
    vtkSmartPointer<vtkXMLDataElement>::New();
  modelEdges->SetName("ModelEdges");
  modelEdges->SetIntAttribute("NumberOfObjects", NumberOfModelEdges);
  modelEdges->SetAttribute("Description",
                           "Implicitly ordered model edges starting at 0 and numbered consecutively.  A model edge is an aggregation of mesh edges that is used to specify information collectively for the mesh edges.  The model edge has multiple associated model faces (at most 2 for a 2D problem though) that are stored in the Data nested XML element and a User specified Name. The model face ids that are stored are using their implicit numbering.");
  bcsfile->AddNestedElement(modelEdges);
  for(i=0;i<NumberOfModelEdges;i++)
    {
    vtkSmartPointer<vtkXMLDataElement> modelEdge =
      vtkSmartPointer<vtkXMLDataElement>::New();
    modelEdge->SetName("ModelEdge");

    char ModelEdgeName[100];
    int ii = i;
    sprintf(ModelEdgeName, "ModelEdge%d", ii);
    modelEdge->SetAttribute("Name", ModelEdgeName);
    // add in as inline data the number of adjacent model faces
    std::stringstream FaceIds;
    vtkSmartPointer<vtkXMLDataElement> EdgeFaces =
      vtkSmartPointer<vtkXMLDataElement>::New();
    EdgeFaces->SetName("Data");
    EdgeFaces->SetIntAttribute("NumberOfObjects", 1);
    EdgeFaces->SetAttribute("type", "Int64");
    modelEdge->AddNestedElement(EdgeFaces);
    FaceIds << endl << indent4 << " 0 " << endl << indent4;  //there is only 1 face for now
    EdgeFaces->AddCharacterData(FaceIds.str().c_str(), data.str().length());

    modelEdges->AddNestedElement(modelEdge);
    }

  //
  // write out the boundary condition set data
  //
  vtkSmartPointer<vtkXMLDataElement> BCSs =
    vtkSmartPointer<vtkXMLDataElement>::New();
  BCSs->SetName("BoundaryConditionSets");
  BCSs->SetIntAttribute("NumberOfObjects", 0);
  BCSs->SetAttribute("Description",
                     "Implicitly ordered boundary condition sets (BCSs) starting at 0 and numbered consecutively along with a Name that can be set and a Unique Persistent Id.  Each BCS element also has a nested Data element used to store the Ids of the model faces it is applied over.");
  bcsfile->AddNestedElement(BCSs);

  vtkXMLUtilities::WriteElementToFile(bcsfile, this->GetFileName(), &indent);

  return 1;
}

//-----------------------------------------------------------------------------
const char* vtkSGXMLBCSWriter::GetDataSetName()
{
  return 0;
}

//-----------------------------------------------------------------------------
void vtkSGXMLBCSWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "Coords: " << this->Coords << "\n";
  os << indent << "ModelVertexIds: " << this->ModelVertexIds << "\n";
}

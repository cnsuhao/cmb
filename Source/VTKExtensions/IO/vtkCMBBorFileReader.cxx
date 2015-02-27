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
#include "vtkCMBBorFileReader.h"
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkDoubleArray.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include "vtkIntArray.h"
#include <vtkMultiBlockDataSet.h>
#include <vtkNew.h>
#include <vtkPolyData.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include "vtkPoints.h"
#include "vtkStringArray.h"
#include "vtkTriangleFilter.h"
#include "vtkTubeFilter.h"
#include <vtksys/SystemTools.hxx>
#include <string>

#include <fstream>

#include "vtkXMLPolyDataWriter.h"

//for file dumping in debug mode
//#define WRITE_BOREHOLE_FILES


class XContact
{
public:
  XContact()
  {
  this->matId=this->hguId=this->horzId=-1;
  this->xyz[0]=this->xyz[1]=this->xyz[2]=0.0;
  }
  double xyz[3];
  int matId;
  int hguId;
  int horzId;
};
class BorHoleInfo
{
public:
  BorHoleInfo(){}

  std::string Name;
  std::string GUID;
  std::vector< XContact> CSContacts;
  bool InitPoly(vtkPolyData* outPoly)
    {
    int npts = static_cast<int>(this->CSContacts.size());
    // it has to have at least two points
    if(npts<=1)
      {
      return false;
      }
    vtkNew<vtkPolyData> bhPoly;
    // Cell Arrays
    vtkNew<vtkIntArray> matArray;
    matArray->SetName("mat");
    matArray->SetNumberOfComponents(1);
    matArray->SetNumberOfTuples(npts-1);
    vtkNew<vtkIntArray> hguArray;
    hguArray->SetName("hgu");
    hguArray->SetNumberOfComponents(1);
    hguArray->SetNumberOfTuples(npts-1);
    vtkNew<vtkIntArray> horzArray;
    horzArray->SetName("horz");
    horzArray->SetNumberOfComponents(1);
    horzArray->SetNumberOfTuples(npts-1);
    // cell and point data
    vtkNew<vtkPoints> points;
    vtkNew<vtkCellArray> lines;
    vtkIdType id0, id1;
    vtkNew<vtkIdList> linepts;
    // Warning: this first horz should not be zero,
    // otherwise the color map will be wrong.
    int lastNoneZeroHor=this->CSContacts[0].horzId;
    for (int i=0; i < npts-1; ++i)
      {
      id0 = points->InsertNextPoint(
        this->CSContacts[i].xyz);
      id1 = points->InsertNextPoint(
        this->CSContacts[i+1].xyz);
      linepts->Reset();
      linepts->InsertNextId(id0);
      linepts->InsertNextId(id1);
      lines->InsertNextCell(linepts.GetPointer());
      matArray->SetValue(i, this->CSContacts[i].matId);
      hguArray->SetValue(i, this->CSContacts[i].hguId);
      lastNoneZeroHor = this->CSContacts[i].horzId==0 ?
        lastNoneZeroHor : this->CSContacts[i].horzId;
      horzArray->SetValue(i, lastNoneZeroHor);
      }
    bhPoly->SetPoints(points.GetPointer());
    bhPoly->SetLines(lines.GetPointer());
    bhPoly->GetCellData()->AddArray(matArray.GetPointer());
    bhPoly->GetCellData()->AddArray(hguArray.GetPointer());
    bhPoly->GetCellData()->AddArray(horzArray.GetPointer());
    outPoly->ShallowCopy(bhPoly.GetPointer());
    vtkNew<vtkStringArray> nameArray;
    nameArray->SetName("ObjectName");
    nameArray->InsertNextValue(this->Name.c_str());
    outPoly->GetFieldData()->AddArray(nameArray.GetPointer());

    return true;
    }

};
struct CSLocation
{
  double positions[3];
};
struct CSNode
{
  int Id;
  int BedLayerMat;
  int BedLayerBoundary;
  CSLocation Location;
};
struct CSArc
{
public:
  CSArc(){this->ReverseNodeDirection=0;}
  int ReverseNodeDirection;
  int Node1_Id;
  int Node2_Id;
  int BedLayerMat;
  int BedLayerBoundary;
  int Id;
  std::vector<CSLocation> Vertices;
};
struct CSPolygon
{
  int Id;
  int MatId;

  std::vector<int> ArcIds;
  std::vector<int> HArcIds; // Hole arcs
};
class BorCrossSection
{
public:
  BorCrossSection(){}
  bool InitPoly(vtkPolyData* csPoly,
    BorHoleInfo& BH1, BorHoleInfo& BH2)
    {
    int ncells = static_cast<int>(this->Polygons.size());
    // it has to have at least two points
    if(ncells<1)
      {
      return false;
      }
    vtkNew<vtkPolyData> tmpPolyData;
    double x0=BH1.CSContacts[0].xyz[0];
    double y0=BH1.CSContacts[0].xyz[1];
    double xDis=BH2.CSContacts[0].xyz[0]-x0;
    double yDis=BH2.CSContacts[0].xyz[1]-y0;
    // cell and point data
    vtkNew<vtkPoints> points;
    vtkNew<vtkCellArray> polys;
    vtkNew<vtkCellArray> lines;
    vtkNew<vtkCellArray> verts;
    std::vector<int> polyMatIds;
    int minMatID=VTK_INT_MAX;
    std::map<int, CSPolygon>::iterator csit=
      this->Polygons.begin();
    vtkIdType pid;
    double pointpos[3];
    for(; csit!=this->Polygons.end(); ++csit)
      {
      std::vector<int> orderedArcs;
      this->GetOrderedPolygonArcs(csit->second, orderedArcs);
      if(orderedArcs.size()>0)
        {
        //******Polygon Cells********
        // vtkPolygon is a concrete implementation of vtkCell to represent a 2D
        // n-sided polygon. The polygons cannot have any internal holes, and cannot
        // self-intersect. Define the polygon with n-points ordered in the counter-
        // clockwise direction; do not repeat the last point.
        vtkNew<vtkIdList> cellPts;
        vtkNew<vtkIdList> linePts;

        // The first arc
        // Points are ordered counter-clockwise
        int arcId = orderedArcs[0];
        int nodeId;
        for(int v=0; v<static_cast<int>(this->Arcs[arcId].Vertices.size()); v++)
          {
          pid=this->InsertVTKNextPoint(points.GetPointer(),
            this->Arcs[arcId].Vertices[v].positions,
            pointpos, x0, y0, xDis, yDis);
          cellPts->InsertNextId(pid);
          verts->InsertNextCell(1, &pid);
        //          matArray->InsertNextValue(csit->second.MatId);
          }
        nodeId = this->Arcs[arcId].Node2_Id;
        pid=this->InsertVTKNextPoint(points.GetPointer(),
          this->Nodes[nodeId].Location.positions,
          pointpos, x0, y0, xDis, yDis);
        cellPts->InsertNextId(pid);
        verts->InsertNextCell(1, &pid);
        //         matArray->InsertNextValue(csit->second.MatId);

        // The other arcs after the first arc, Counter-Clockwise
        for(int i=1; i<static_cast<int>(orderedArcs.size()); i++)
          {
          arcId = orderedArcs[i];
          if(this->Arcs[arcId].ReverseNodeDirection)
            {
            for(int v=static_cast<int>(this->Arcs[arcId].Vertices.size()-1); v>=0; v--)
              {
              pid=this->InsertVTKNextPoint(points.GetPointer(),
                this->Arcs[arcId].Vertices[v].positions,
                pointpos, x0, y0, xDis, yDis);
              cellPts->InsertNextId(pid);
              verts->InsertNextCell(1, &pid);
              //matArray->InsertNextValue(0);
              }
            nodeId = this->Arcs[arcId].Node1_Id;
            }
          else
            {
            for(size_t v=0; v<this->Arcs[arcId].Vertices.size(); v++)
              {
              pid=this->InsertVTKNextPoint(points.GetPointer(),
                this->Arcs[arcId].Vertices[v].positions,
                pointpos, x0, y0, xDis, yDis);
              cellPts->InsertNextId(pid);
              verts->InsertNextCell(1, &pid);
              //matArray->InsertNextValue(0);
              }
            nodeId = this->Arcs[arcId].Node2_Id;
            }
          pid=this->InsertVTKNextPoint(points.GetPointer(),
            this->Nodes[nodeId].Location.positions,
            pointpos, x0, y0, xDis, yDis);
          cellPts->InsertNextId(pid);
          verts->InsertNextCell(1, &pid);
          //matArray->InsertNextValue(0);
          }

        polys->InsertNextCell(cellPts.GetPointer());
        polyMatIds.push_back(csit->second.MatId);
        minMatID = csit->second.MatId<minMatID ?
          csit->second.MatId : minMatID;
        //matArray->InsertNextValue(csit->second.MatId);
        //******Line Cells******** //
        lines->InsertNextCell(cellPts.GetPointer());
        //matArray->InsertNextValue(0);
        //break;
        }
      }

    tmpPolyData->SetPoints(points.GetPointer());
    tmpPolyData->SetPolys(polys.GetPointer());
    tmpPolyData->SetLines(lines.GetPointer());
    tmpPolyData->SetVerts(verts.GetPointer());
    // Cell Arrays
    vtkNew<vtkIntArray> matArray;
    matArray->SetName("MATID");
    matArray->SetNumberOfComponents(1);
    vtkIdType numCells = tmpPolyData->GetNumberOfCells();
    matArray->SetNumberOfTuples(numCells);
    matArray->FillComponent(0, static_cast<double>(minMatID));
    vtkIdType numOfNonPolys = lines->GetNumberOfCells()+verts->GetNumberOfCells();
    std::vector<int>::iterator matIt =polyMatIds.begin();
    csit = this->Polygons.begin();
    for(vtkIdType cellId=numOfNonPolys; cellId<numCells; ++cellId,++matIt)
      {
      if(matIt==polyMatIds.end())
        {
        break;
        }
      matArray->SetValue(cellId, *matIt);
      }
    tmpPolyData->GetCellData()->AddArray(matArray.GetPointer());
    vtkNew<vtkTriangleFilter> triangulateF;
    triangulateF->SetInputData( tmpPolyData.GetPointer() );
    triangulateF->SetPassLines(1);
    triangulateF->SetPassVerts(1);
    triangulateF->Update();
    csPoly->ShallowCopy(triangulateF->GetOutput());

    vtkNew<vtkStringArray> nameArray;
    nameArray->SetName("ObjectName");
    nameArray->InsertNextValue(this->Name.c_str());
    csPoly->GetFieldData()->AddArray(nameArray.GetPointer());

    return true;
    }
  // <int arcId> orderedArc
  void GetOrderedPolygonArcs(CSPolygon& polygon,
    std::vector<int>& orderedArcs)
    {
    if(polygon.ArcIds.size()<=0)
    {
    return;
    }
    int firstArc = polygon.ArcIds[0];
    orderedArcs.push_back(firstArc);
    std::vector<int> tempArcIds(polygon.ArcIds);
    std::vector<int>::iterator tmpit=tempArcIds.begin();
    tempArcIds.erase(tmpit);
    int node1Id=this->Arcs[firstArc].Node1_Id;
    int nextId=this->Arcs[firstArc].Node2_Id;
    while(nextId != node1Id && tempArcIds.size()>0)
      {
      for(tmpit=tempArcIds.begin(); tmpit!=tempArcIds.end(); ++tmpit)
        {
        int next1Id = this->Arcs[*tmpit].Node1_Id;
        int next2Id = this->Arcs[*tmpit].Node2_Id;
        // find the id for next arc
        if(next1Id==nextId || next2Id==nextId)
          {
          this->Arcs[*tmpit].ReverseNodeDirection = (next1Id==nextId) ? 0 : 1;
          orderedArcs.push_back(*tmpit);
          nextId = (next1Id==nextId) ? next2Id : next1Id;
          tempArcIds.erase(tmpit);
          break;
          }
        }
      }
    }
  vtkIdType InsertVTKNextPoint(vtkPoints* points,
    double* origPts, double* newptsPos,
    double x0, double y0, double xDistance, double yDistance)
    {
    // origPts[2] is always 0 and should be ignored
    // according to file format document
    newptsPos[0] = x0 + xDistance*origPts[0]; // relative
    newptsPos[1] = y0 + yDistance*origPts[0];
    newptsPos[2] = origPts[1]; // elevation
    return points->InsertNextPoint(newptsPos);
    }
  std::string BorHole1_GUID;
  std::string BorHole2_GUID;
  std::string Name;
  double elevation;
  std::map<int, CSNode> Nodes;
  std::map<int, CSArc> Arcs;
  std::map<int, CSPolygon> Polygons;
};
// -----------------------------------------------------------------------------

vtkStandardNewMacro(vtkCMBBorFileReader);

// -----------------------------------------------------------------------------
vtkCMBBorFileReader::vtkCMBBorFileReader()
{
  this->FileName = 0;
  this->SetNumberOfInputPorts(0);
  this->BHDisplayWidth=0;
  this->BHNumSampleDatasets=0;
  this->NumberOfBoreholes=0;
  this->NumberOfCrossSections=0;
}
// -----------------------------------------------------------------------------
vtkCMBBorFileReader::~vtkCMBBorFileReader()
{
  this->SetFileName( 0 );
  this->CrossSections.clear();
  this->BoreHoles.clear();
}
// -----------------------------------------------------------------------------
void vtkCMBBorFileReader::PrintSelf( ostream& os, vtkIndent indent )
{
  this->Superclass::PrintSelf( os, indent );
  os << indent << "FileName: " << ( this->FileName ? this->FileName : "(null)" ) << endl;
}

// -----------------------------------------------------------------------------
int vtkCMBBorFileReader::ReadBorFile(const char* filename)
{
  ifstream supStream(filename);
  if(!supStream)
    {
    vtkErrorMacro("Unable to open file: " << filename);
    return 0;
    }
  std::string line, cardType;
  int numContacts;
  while (!supStream.eof())
    {
    // Read in the BHOLE
    supStream >> cardType;
    if (cardType == "BHOLE")
      {
      supStream.ignore(80, '\n'); // Skip rest of line
      supStream >> line >> this->BHDisplayWidth;
      supStream.ignore(80, '\n'); // Skip rest of line
      supStream >> line >> this->BHNumSampleDatasets;
      supStream.ignore(80, '\n'); // Skip rest of line
      while (!supStream.eof() && cardType != "XSECT")
        {
        supStream >> cardType;
        if(cardType == "BEGH")
          {
          BorHoleInfo borehole;
          supStream.ignore(80, '\n'); // Skip rest of line
          getline(supStream,line);
          //Find name and Remove quotation marks
          std::string::size_type qIdx=line.find('\"')+1;
          line =line.substr(qIdx,line.rfind('\"')-qIdx);
          borehole.Name=line;
          //supStream >> line >> borehole.Name;
          //supStream.ignore(80, '\n'); // Skip rest of line
          supStream >> line >> borehole.GUID;
          supStream.ignore(80, '\n'); // Skip rest of line
          supStream >> line >> numContacts;
          supStream.ignore(80, '\n'); // Skip rest of line
          for(int i=0; i<numContacts; i++)
            {
            XContact xbc;
            supStream >>xbc.xyz[0]>>xbc.xyz[1]>>xbc.xyz[2]>>xbc.matId>>xbc.hguId>>xbc.horzId;
            supStream.ignore(80, '\n'); // Skip rest of line
            borehole.CSContacts.push_back(xbc);
            }
          this->BoreHoles.insert(std::make_pair(borehole.GUID, borehole));
          supStream >> cardType; // "ENDH"
          supStream.ignore(80, '\n'); // Skip rest of line
          }
        }
      } // end cardType == "BHOLE"
    if (cardType == "XSECT")
      {
      supStream.ignore(80, '\n'); // Skip rest of line
      while (!supStream.eof())
        {
        cardType.clear();
        supStream >> cardType;
        supStream.ignore(80, '\n'); // Skip rest of line
        if(cardType == "BEGX")
          {
          BorCrossSection crosssection;
          supStream >> line; //BHOLE
          supStream.ignore(80, '\n'); // Skip rest of line
          supStream >> line >> crosssection.BorHole1_GUID >> crosssection.BorHole2_GUID;
          supStream.ignore(80, '\n'); // Skip rest of line
          supStream >> line; //BEGCOV
          supStream.ignore(80, '\n'); // Skip rest of line

          getline(supStream,line);
          //Find name and Remove quotation marks
          std::string::size_type qIdx=line.find('\"')+1;
          line=line.substr(qIdx,line.rfind('\"')-qIdx);
          crosssection.Name=line;

          //supStream >> line >> crosssection.Name;
          //supStream.ignore(80, '\n'); // Skip rest of line
          if(crosssection.Name=="new coverage")
            {
            crosssection.Name=
              this->BoreHoles[crosssection.BorHole1_GUID].Name
              + "-" +
              this->BoreHoles[crosssection.BorHole2_GUID].Name;
            }
          supStream >> line >> crosssection.elevation;
          supStream.ignore(80, '\n'); // Skip rest of line
          std::string xcard;
          supStream >> xcard;
          while(xcard=="NODE" && !supStream.eof())
            {
            supStream.ignore(80, '\n'); // Skip rest of line
            CSNode csnode;
            supStream >> line >> csnode.Location.positions[0]
                      >> csnode.Location.positions[1]
                      >> csnode.Location.positions[2];
            supStream.ignore(80, '\n'); // Skip rest of line
            supStream >> line >> csnode.Id;
            supStream.ignore(80, '\n'); // Skip rest of line
            supStream >> line >> csnode.BedLayerMat >> csnode.BedLayerBoundary;
            supStream.ignore(80, '\n'); // Skip rest of line
            supStream >> line; //END
            supStream.ignore(80, '\n'); // Skip rest of line
            xcard.clear();
            supStream >> xcard;
            crosssection.Nodes.insert(std::make_pair(csnode.Id, csnode));
            } //end xcard=="NODE"
          while(xcard=="ARC" && !supStream.eof())
            {
            supStream.ignore(80, '\n'); // Skip rest of line
            CSArc csarc;
            supStream >> line>> csarc.Id;
            supStream.ignore(80, '\n'); // Skip rest of line
            supStream >> line >> csarc.Node1_Id >> csarc.Node2_Id;
            supStream.ignore(80, '\n'); // Skip rest of line
            int val1=0;
            supStream >> line;
            if(line == "ARCVERTICES")
              {
              supStream >> val1;
              supStream.ignore(80, '\n'); // Skip rest of line
              for(int i=0; i<val1; i++)
                {
                CSLocation vert;
                supStream >> vert.positions[0]>>vert.positions[1]>>vert.positions[2];
                csarc.Vertices.push_back(vert);
                supStream.ignore(80, '\n'); // Skip rest of line
                }
              supStream >> line >> csarc.BedLayerMat >> csarc.BedLayerBoundary;
              }
            else if(line == "BELAYERS")
              {
              supStream >> csarc.BedLayerMat >> csarc.BedLayerBoundary;
              }

            supStream.ignore(80, '\n'); // Skip rest of line
            supStream >> line; //END
            supStream.ignore(80, '\n'); // Skip rest of line
            xcard.clear();
            supStream >> xcard;
            crosssection.Arcs.insert(std::make_pair(csarc.Id, csarc));
            }//end xcard=="ARC"
          while(xcard=="POLYGON" && !supStream.eof())
            {
            supStream.ignore(80, '\n'); // Skip rest of line
            CSPolygon cspoly;
            int intValue=0;
            supStream >> line >> intValue;
            supStream.ignore(80, '\n'); // Skip rest of line
            int arcId;
            for(int i=0; i<intValue; i++)
              {
              supStream >> arcId;
              cspoly.ArcIds.push_back(arcId);
              supStream.ignore(80, '\n'); // Skip rest of line
              }
            // figure out if there are HARCS card
            supStream >> line >> intValue;
            supStream.ignore(80, '\n'); // Skip rest of line
            if(line == "HARCS")
              {
              for(int i=0; i<intValue; i++)
                {
                supStream >> arcId;
                cspoly.HArcIds.push_back(arcId);
                supStream.ignore(80, '\n'); // Skip rest of line
                }
              supStream >> line >> cspoly.Id;
              supStream.ignore(80, '\n'); // Skip rest of line
              }
            else if(line == "ID")// it is the "ID" line
              {
              cspoly.Id = intValue;
              supStream >> line >> cspoly.MatId;
              supStream.ignore(80, '\n'); // Skip rest of line
              }
            supStream >> line; //END
            supStream.ignore(80, '\n'); // Skip rest of line
            xcard.clear();
            supStream >> xcard;
            crosssection.Polygons.insert(std::make_pair(cspoly.Id, cspoly));
            }// end xcard=="POLYGON"
          this->CrossSections.push_back(crosssection);
          }// end cardType == "BEGX"
        }// while(!eof)
      } // end cardType == "XSECT"
    }// while(!eof)
  supStream.close();
  return 1;
}

// -----------------------------------------------------------------------------
int vtkCMBBorFileReader::RequestData(
    vtkInformation* /*request*/, vtkInformationVector** /*inputVector*/, vtkInformationVector* outputVector )
{
  // get the info object
  this->UpdateProgress(0);
  if ( ! this->FileName )
    {
    return 0;
    }
  vtkMultiBlockDataSet* output = vtkMultiBlockDataSet::SafeDownCast(
      outputVector->GetInformationObject( 0 )->Get(vtkDataObject::DATA_OBJECT()));
  if ( ! output )
    {
    return 0;
    }
  this->NumberOfBoreholes=0;
  this->NumberOfCrossSections=0;

  if(!this->ReadBorFile(this->FileName))
    {
    return 0;
    }
  if(!this->ProcessBorFileInfo(output))
    {
    return 0;
    }

  return 1 ;
}
// -----------------------------------------------------------------------------
int vtkCMBBorFileReader::RequestInformation(
    vtkInformation* /*request*/, vtkInformationVector** /*inputVector*/, vtkInformationVector* /*outputVector*/ )
{
  if (!this->FileName)
    {
    vtkErrorMacro("FileName has to be specified!");
    return 0;
    }
  return 1;
}
// -----------------------------------------------------------------------------
int vtkCMBBorFileReader::ProcessBorFileInfo(vtkMultiBlockDataSet* output)
{
  output->SetNumberOfBlocks(2);
  vtkNew<vtkMultiBlockDataSet> BoreholeRootBlock;
  vtkNew<vtkMultiBlockDataSet> CrossRootBlock;
  // process borehole info
  BoreholeRootBlock->SetNumberOfBlocks(
    static_cast<unsigned int>(this->BoreHoles.size()));
  std::map<std::string, BorHoleInfo>::iterator bhit=
    this->BoreHoles.begin();
  for(int bix=0; bhit!=this->BoreHoles.end(); ++bhit, ++bix)
    {
    vtkNew<vtkPolyData> bhPoly;
    if(bhit->second.InitPoly(bhPoly.GetPointer()))
      {
      BoreholeRootBlock->SetBlock(bix,bhPoly.GetPointer());

#ifdef WRITE_BOREHOLE_FILES
      vtkNew<vtkXMLPolyDataWriter> Writer;
      Writer->SetInputData(0, bhPoly.GetPointer());
      char outputFileName[256];
      sprintf(outputFileName, "%s-%d.vtp",
        "C:/Users/Temp/testBoreHole", bix);
      Writer->SetFileName(outputFileName);
      //Writer->SetDataMode( this->DataMode );
      Writer->Write();
#endif
      }
    }
  // process cross sections
  CrossRootBlock->SetNumberOfBlocks(
    static_cast<unsigned int>( this->CrossSections.size() ));
  std::vector<BorCrossSection>::iterator csit=
    this->CrossSections.begin();
  for(int cix=0; csit!=this->CrossSections.end(); ++csit, ++cix)
    {
    vtkNew<vtkPolyData> csPoly;
    if(csit->InitPoly(csPoly.GetPointer(),
      this->BoreHoles[csit->BorHole1_GUID],
      this->BoreHoles[csit->BorHole2_GUID]))
      {
      CrossRootBlock->SetBlock(cix,csPoly.GetPointer());
#ifdef WRITE_BOREHOLE_FILES
      vtkNew<vtkXMLPolyDataWriter> Writer;
      Writer->SetInputData(0, csPoly.GetPointer());
      char outputFileName[256];
      sprintf(outputFileName, "%s-%d.vtp",
        "C:/Users/Temp/testCrossSection", cix);
      Writer->SetFileName(outputFileName);
      //Writer->SetDataMode( this->DataMode );
      Writer->Write();
#endif

      }
    }
  this->NumberOfBoreholes=BoreholeRootBlock->GetNumberOfBlocks();
  this->NumberOfCrossSections= CrossRootBlock->GetNumberOfBlocks();
  output->SetBlock(0, BoreholeRootBlock.GetPointer());
  output->SetBlock(1, CrossRootBlock.GetPointer());
  return 1;
}

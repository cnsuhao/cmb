//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkCMBTriangleMultiBlockMesher.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkCell.h"
#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkInformation.h"
#include "vtkDataArray.h"
#include "vtkErrorCode.h"
#include "vtkFloatArray.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkPointData.h"

#include "smtk/extension/vtk/meshing/cmbFaceMesherInterface.h"
#include "smtk/extension/vtk/meshing/cmbFaceMeshHelper.h"
#include "smtk/extension/vtk/meshing/vtkCMBMeshServerLauncher.h"
#include "smtk/extension/vtk/meshing/vtkCMBPrepareForTriangleMesher.h"

#include <sstream>
#include <vtksys/SystemTools.hxx>

vtkStandardNewMacro(vtkCMBTriangleMultiBlockMesher);


using namespace smtk::vtk::CmbFaceMesherClasses;

//--------------------------------------------------------------------
vtkCMBTriangleMultiBlockMesher::vtkCMBTriangleMultiBlockMesher()
{
  MinAngle            = 20.0f;
  UseMinAngle         = false;
  PreserveBoundaries  = true;
  PreserveEdges       = false;
  MaxArea             = 1.0/8.0;
  ComputedMaxArea     = MaxArea;
  UseUniqueAreas      = false;
  MaxAreaMode         = RelativeToBoundsAndSegments;
  VerboseOutput       = false;

  this->SetNumberOfInputPorts(1);
  this->SetNumberOfOutputPorts(1);
}
//--------------------------------------------------------------------
//This multiblockdataset algorithm uses a polydata as input instead of
//multiblockdataset
int vtkCMBTriangleMultiBlockMesher::FillInputPortInformation(int vtkNotUsed(port), vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(),"vtkPolyData");
  return 1;
}
//--------------------------------------------------------------------
vtkCMBTriangleMultiBlockMesher::~vtkCMBTriangleMultiBlockMesher()
{
}
//--------------------------------------------------------------------
void vtkCMBTriangleMultiBlockMesher::PrintSelf(ostream& os, vtkIndent indent)
{
  const char* areaModeType[4] =
    {
    "NoMaxArea",
    "AbsoluteArea",
    "RelativeToBounds",
    "RelativeToBoundsAndSegments"
    };
  os << indent << this->GetClassName() << endl;
  os << indent << "  Use Minimum Angle: " << UseMinAngle << endl;
  os << indent << "      Minimum Angle: " <<    MinAngle << endl;
  os << indent << "Preserve Boundaries: " << PreserveBoundaries << endl;
  os << indent << "     Preserve Edges: " << PreserveEdges << endl;
  os << indent << "           Max Area: " << MaxArea << endl;
  os << indent << "  Computed Max Area: " << ComputedMaxArea << endl;
  os << indent << "      Max Area Mode: " << areaModeType[MaxAreaMode] << endl;
  os << indent << "   Use Unique Areas: " << UseUniqueAreas << endl;
  os << indent << "     Verbose Output: " << VerboseOutput << endl;
  this->Superclass::PrintSelf(os,indent);
}
//--------------------------------------------------------------------
int vtkCMBTriangleMultiBlockMesher::RequestData(vtkInformation * /*request*/,
    vtkInformationVector **inputVector,
    vtkInformationVector *outputVector)
{
  //input is a vtkPolyData with field data specified by the
  //Map interface. This information is used for meshing
  vtkPolyData* input = vtkPolyData::GetData(inputVector[0]);
  vtkMultiBlockDataSet* output = vtkMultiBlockDataSet::GetData(outputVector);

  std::map<vtkIdType, ModelFaceRep* > pid2Face; // Create a face for every polygon

  // Build the data structures required for meshing
  typedef smtk::vtk::vtkCMBPrepareForTriangleMesher vtkPrepareForMesher;
  vtkPrepareForMesher* mapInterface = vtkPrepareForMesher::New();
  mapInterface->SetPolyData(input);
  mapInterface->GetPolyId2ModelFaceRepMap(pid2Face);
  mapInterface->Delete();

  // Find global area information
  double* bnds = input->GetBounds();
  double totalPolyDataArea = (bnds[1]-bnds[0]) * (bnds[3]-bnds[2]);

  output->SetNumberOfBlocks(pid2Face.size());
  unsigned blocknum = 0;
  // Mesh each polygon individually then append all their polydata together
  typedef smtk::vtk::vtkCMBMeshServerLauncher vtkMeshServerLauncher;
  vtkNew<vtkMeshServerLauncher> meshServer;
  std::map<vtkIdType, ModelFaceRep* >::iterator faceIter = pid2Face.begin();
  for(; faceIter != pid2Face.end(); faceIter++)
    {
    vtkPolyData* outputMesh = vtkPolyData::New();
    vtkIdType faceId = (*faceIter).first;
    ModelFaceRep* face = (*faceIter).second;

    // Find local area information
    double currArea = totalPolyDataArea;
    double currNumSeg = input->GetNumberOfLines();

    if (UseUniqueAreas)
      {
      double faceBnds[4];
      face->bounds(faceBnds);
      currArea = (faceBnds[2]-faceBnds[0]) * (faceBnds[3]-faceBnds[1]);
      currNumSeg = face->numberOfEdges();
      }
    switch(MaxAreaMode)
      {
      case NoMaxArea:
        break;
      case AbsoluteArea:
        //If it is absolutly known what the max triangle size should
        //be just set it
        this->ComputedMaxArea = MaxArea;
        break;
      case RelativeToBounds:
        //Use the MaxArea as a ratio if areas are supposed to be
        //calculated relative to bounds
        this->ComputedMaxArea = currArea * MaxArea;
        break;
      case RelativeToBoundsAndSegments:
        // For added fidelity you can incorporate how coarse
        // or complicated a polygon is by dividing area by the
        // number of line segments in the polygon
        this->ComputedMaxArea = currArea / currNumSeg * MaxArea;
        break;
      default:
        vtkErrorMacro("ERROR: Invalid Max Area Mode");
        break;
      }

    smtk::vtk::cmbFaceMesherInterface ti(face->numberOfVertices(),
                                                      face->numberOfEdges(),
                                                      face->numberOfHoles(),
                                                      this->PreserveEdges);
    ti.setUseMaxArea(this->MaxAreaMode != NoMaxArea);
    ti.setMaxArea(this->ComputedMaxArea);
    ti.setUseMinAngle(this->UseMinAngle);
    ti.setMinAngle(this->MinAngle);
    ti.setPreserveBoundaries(this->PreserveBoundaries);
    ti.setVerboseOutput(this->VerboseOutput);
    ti.setOutputMesh(outputMesh);
    face->fillTriangleInterface(&ti);
    ti.buildFaceMesh(meshServer.GetPointer(),faceId,0);
    output->SetBlock(blocknum++, outputMesh);
    outputMesh->Delete();
    }

  return true;
}

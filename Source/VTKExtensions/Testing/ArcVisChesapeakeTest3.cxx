//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
// Test functionality to initialize a contour widget from chesapeake
// polydata.

//arc related
#include "vtkCMBArc.h"
#include "vtkCMBArcEndNode.h"
#include "vtkCMBArcProvider.h"
#include "vtkCMBArcManager.h"
#include "vtkCMBArcAutoConnectOperator.h"
#include "vtkCMBArcCreateOperator.h"
#include "vtkCMBArcSplitOnPositionOperator.h"
#include "vtkCMBArcDeleteOperator.h"

//meshing related
#include "vtkCMBPolygonFromArcsOperator.h"
#include "vtkCMBArcPolygonProvider.h"

//vtk
#include "vtkBoundedPlanePointPlacer.h"
#include "vtkCellArray.h"
#include "vtkCleanPolyData.h"
#include "vtkContourWidget.h"
#include "vtkProperty.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkCMBArcWidgetRepresentation.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkSmartPointer.h"
#include "vtkTrivialProducer.h"
#include "vtkXMLPolyDataReader.h"
#include "vtkXMLPolyDataWriter.h"


#include "vtkRegressionTestImage.h"

int ArcVisChesapeakeTest3( int argc, char *argv[] )
{
  vtkSmartPointer<vtkTesting> testHelper = vtkSmartPointer<vtkTesting>::New();
  testHelper->AddArguments(argc,const_cast<const char **>(argv));
  if (!testHelper->IsFlagSpecified("-D"))
    {
    std::cerr << "Error: -D /path/to/data was not specified.";
    return 1;
    }

  std::string filename;
  std::string dataRoot = testHelper->GetDataRoot();
  filename = dataRoot + "/ChesapeakeBay/chesapeakebayContour_contour.vtp";

  // Create the RenderWindow, Renderer and both Actors
  //
  vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
  vtkSmartPointer<vtkRenderWindow> renWin =
      vtkSmartPointer<vtkRenderWindow>::New();
  renWin->AddRenderer(renderer);

  vtkSmartPointer<vtkRenderWindowInteractor> iren =
      vtkSmartPointer<vtkRenderWindowInteractor>::New();
  iren->SetRenderWindow(renWin);

  renderer->SetBackground(0.1, 0.2, 0.4);
  renWin->SetSize(600, 600);
  iren->Initialize();

  //read the file
  vtkSmartPointer<vtkXMLPolyDataReader> reader =
      vtkSmartPointer<vtkXMLPolyDataReader>::New();
  reader->SetFileName( filename.c_str() );
  reader->Update();

  //construct a polydata for each line cell, pass that to the contour widget
  //and than make a arc for each.
  vtkCellArray *lines = reader->GetOutput()->GetLines();
  vtkPoints *origPoints = reader->GetOutput()->GetPoints();
  lines->InitTraversal();
  vtkIdType npts,*pts;
  while(lines->GetNextCell(npts,pts))
    {
    //use the modified representation
    vtkSmartPointer<vtkCMBArcWidgetRepresentation> contourRep =
        vtkSmartPointer<vtkCMBArcWidgetRepresentation>::New();
    //set up the line interpolation
    contourRep->SetLineInterpolator( NULL );
    //set up the point place
    vtkSmartPointer<vtkBoundedPlanePointPlacer> bppp =
        vtkSmartPointer<vtkBoundedPlanePointPlacer>::New();
    contourRep->SetPointPlacer( bppp );

    vtkSmartPointer<vtkContourWidget> contourWidget =
        vtkSmartPointer<vtkContourWidget>::New();
    contourWidget->SetInteractor(iren);
    contourWidget->SetRepresentation(contourRep);
    contourWidget->On();

    //construct a polydata for that cell
    vtkSmartPointer<vtkPolyData> temp =
        vtkSmartPointer<vtkPolyData>::New();
    vtkPoints *points = vtkPoints::New();
    vtkCellArray *cell = vtkCellArray::New();

    cell->InsertNextCell(npts);
    double point[3];
    for (vtkIdType i=0; i < npts; ++i)
      {
      origPoints->GetPoint(pts[i],point);
      vtkIdType id = points->InsertNextPoint(point);
      cell->InsertCellPoint(id);
      }

    temp->SetPoints(points);
    temp->SetLines(cell);

    cell->FastDelete();
    points->FastDelete();

    vtkTrivialProducer *tvp = vtkTrivialProducer::New();
    tvp->SetOutput(temp);

    vtkCleanPolyData *clean = vtkCleanPolyData::New();
    clean->SetInputConnection(tvp->GetOutputPort());
    clean->SetAbsoluteTolerance(0.0);
    clean->SetToleranceIsAbsolute(1);
    clean->SetPointMerging(1);
    clean->Update();

    contourWidget->Initialize(clean->GetOutput());
    contourWidget->Render();

    //we can now cleanup the rendering objects
    renderer->ResetCamera();
    renWin->Render();

    //now we have to test the server side code
    vtkCMBArcCreateOperator *createOp = vtkCMBArcCreateOperator::New();
    createOp->Operate(contourRep->GetContourRepresentationAsPolyData());
    createOp->Delete();
    clean->Delete();
    tvp->Delete();
    }

  renderer->RemoveAllViewProps();

  renderer->ResetCamera();
  renWin->Render();

  vtkSmartPointer<vtkCMBArcSplitOnPositionOperator> split =
      vtkSmartPointer<vtkCMBArcSplitOnPositionOperator>::New();

  //split the left side at the channel twice and create
  //two arcs, that we can connect the right side
  vtkIdType leftArc = 2;

  split->SetSplitPosition(-75.8524,39.4552,0.0);
  split->SetPositionTolerance(0.0002);
  bool valid = split->Operate(leftArc);
  if (!valid)
    {
    cerr << "first split failed" << endl;
    return 1;
    }

  vtkIdType firstNewArc = split->GetCreatedArcId();
  split->SetSplitPosition(-75.8524,39.4541,0.0);
  split->SetPositionTolerance(0.0002);
  valid = split->Operate(firstNewArc);
  if (!valid)
    {
    cerr << "second split failed" << endl;
    return 1;
    }
  vtkIdType bottomArc = split->GetCreatedArcId();

  split->SetSplitPosition(-75.5893,39.4727,0.0);
  split->SetPositionTolerance(0.0002);
  valid = split->Operate(bottomArc);
  if (!valid)
    {
    cerr << "third split failed" << endl;
    return 1;
    }
  vtkIdType secondNewArc = split->GetCreatedArcId();

  split->SetSplitPosition(-75.5895,39.4819,0.0);
  split->SetPositionTolerance(0.0002);
  valid = split->Operate(secondNewArc);
  if (!valid)
    {
    cerr << "fourth split failed" << endl;
    return 1;
    }
  vtkIdType rightArc = split->GetCreatedArcId();

  //now remove the firstNewArc and secondNewArc so we can use auto connect
  vtkCMBArcDeleteOperator* del = vtkCMBArcDeleteOperator::New();
  del->Operate(firstNewArc);
  del->Operate(secondNewArc);
  del->Delete();

    //now auto connect left and right
  vtkCMBArcAutoConnectOperator *autoConnect = vtkCMBArcAutoConnectOperator::New();
  valid = autoConnect->Operate(rightArc,leftArc);
  if (!valid)
    {
    cerr << "unable to do auto connect between right and left arcs" << endl;
    return 1;
    }
  vtkIdType numArcs = autoConnect->GetCreatedArcId() + 1;
  autoConnect->Delete();


  vtkCMBArc *bottom = vtkCMBArcManager::GetInstance()->GetArc(bottomArc);
  double pos[3];
  bottom->GetEndNode(0)->GetPosition(pos);
  bottom->MoveEndNode(1,pos);
  if (!bottom->IsClosedArc())
    {
    cerr << "bottom arc should be closed after the move" << endl;
    return 1;
    }

  //now it is time to create a polygon!
  vtkSmartPointer<vtkCMBPolygonFromArcsOperator> polygonDetector =
      vtkSmartPointer<vtkCMBPolygonFromArcsOperator>::New();

  for (vtkIdType i=0; i < numArcs; i++)
    {
    if (vtkCMBArcManager::GetInstance()->GetArc(i) != NULL)
      {
      //only add arcs that still exist in the system
      polygonDetector->AddArcId(i);
      }
    }
  valid = polygonDetector->Operate();
  if(!valid)
    {
    cerr << "Detection of the polygon structure failed" <<endl;
    return 1;
    }

  vtkSmartPointer<vtkCMBArcPolygonProvider> polygon =
    vtkSmartPointer<vtkCMBArcPolygonProvider>::New();

  //now get the outer loop
  polygon->SetOuterLoopArcIds(polygonDetector->GetOuterLoop());
  vtkIdType size = polygonDetector->GetNumberOfInnerLoops();
  for (vtkIdType idx=0;idx<size;++idx)
    {
    polygon->AddInnerLoopArcIds(polygonDetector->GetInnerLoop(idx));
    }

  vtkSmartPointer<vtkPolyDataMapper> mapper =
    vtkSmartPointer<vtkPolyDataMapper>::New();
  mapper->SetInputConnection( polygon->GetOutputPort() );

  vtkSmartPointer<vtkActor> actor =
    vtkSmartPointer<vtkActor>::New();
  actor->SetMapper( mapper );
  actor->GetProperty()->SetRepresentationToWireframe();
  renderer->AddViewProp( actor );


  renderer->ResetCamera();
  renWin->Render();

  int retVal = vtkRegressionTestImage(renWin);
  if (retVal == vtkRegressionTester::DO_INTERACTOR)
    {
    iren->Start();
    return 0;
    }

  return !retVal;
}

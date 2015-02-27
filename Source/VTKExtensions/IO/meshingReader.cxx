#include "vtkPolyFileReader.h"
#include "vtkPolylineTriangulator.h"
#include "vtkDiscoverRegions.h"
#include "vtkCMBTriangleMesher.h"
#include "vtkXMLPolyDataWriter.h"
#include "vtkPolyDataNormals.h"
#include "vtkPassArrays.h"

#include "vtkNew.h"

int main(int argc, char* argv[])
{
  vtkNew<vtkPolyFileReader> rdr;
  vtkNew<vtkPolylineTriangulator> tmp;
  vtkNew<vtkXMLPolyDataWriter> wri;

  // Read in the polygonal data as polyline strips
  rdr->SetFileName(argc > 1 ? argv[1] : "tet.poly");
  rdr->SetSimpleMeshFormat(-1);
  rdr->FacetMarksAsCellDataOn();
  rdr->Update();

  wri->SetInputConnection(rdr->GetOutputPort());
  wri->SetFileName("polylineFacets.vtp");
  wri->SetDataModeToAscii();
  wri->Write();

  if (rdr->GetOutput(1)->GetNumberOfPoints())
    {
    wri->SetInputConnection(rdr->GetOutputPort(1));
    wri->SetFileName("facetHolePoints.vtp");
    wri->SetDataModeToAscii();
    wri->Write();
    }

  if (rdr->GetOutput(2)->GetNumberOfPoints())
    {
    wri->SetInputConnection(rdr->GetOutputPort(2));
    wri->SetFileName("volumeHolePoints.vtp");
    wri->SetDataModeToAscii();
    wri->Write();
    }

  if (rdr->GetOutput(3)->GetNumberOfPoints())
    {
    wri->SetInputConnection(rdr->GetOutputPort(3));
    wri->SetFileName("regionGroupPoints.vtp");
    wri->SetDataModeToAscii();
    wri->Write();
    }

  // Now triangulate the polyline strips using Triangle
  tmp->SetInputConnection(0, rdr->GetOutputPort(0)); // polygonal loops
  tmp->SetInputConnection(1, rdr->GetOutputPort(1)); // facet hole markers
  tmp->SetModelFaceArrayName(VTK_POLYFILE_MODEL_FACE_ID);
  tmp->Update();

  wri->SetInputConnection(tmp->GetOutputPort(0));
  wri->SetFileName("triangulatedFacets.vtp");
  wri->SetDataModeToAscii();
  wri->Write();

  vtkNew<vtkPolyDataNormals> nrm;
  nrm->SetInputConnection(tmp->GetOutputPort(0));
  nrm->SetConsistency(1);
  nrm->SetNonManifoldTraversal(0);
  nrm->SetSplitting(0);
  nrm->Update();

  wri->SetInputConnection(nrm->GetOutputPort(0));
  wri->SetFileName("orientedFacets.vtp");
  wri->SetDataModeToAscii();
  wri->Write();

  vtkNew<vtkDiscoverRegions> drg;
  drg->SetModelFaceArrayName(argc > 2 ? argv[2] : VTK_POLYFILE_MODEL_FACE_ID);
  drg->ReportRegionsByModelFaceOn();
  drg->SetRegionGroupArrayName(argc > 3 ? argv[3] : VTK_POLYFILE_REGION_GROUP);
  drg->SetInputConnection(0, nrm->GetOutputPort(0));
  drg->SetInputConnection(1, rdr->GetOutputPort(2));
  drg->SetInputConnection(2, rdr->GetOutputPort(3));
  drg->Update();

  // Don't pass point-data normals as they are computed
  // with splitting off, which makes them irritating to look at.
  vtkNew<vtkPassArrays> psa;
  psa->SetInputConnection(drg->GetOutputPort(0));
  psa->UseFieldTypesOn();
  psa->AddFieldType(vtkDataObject::POINT);
  psa->RemoveArraysOn();
  psa->AddPointDataArray("Normals");

  wri->SetInputConnection(psa->GetOutputPort(0));
  wri->SetFileName(argc > 4 ? argv[4] : "discoveredRegions.vtp");
  wri->SetDataModeToAscii();
  wri->Write();

  return 0;
}

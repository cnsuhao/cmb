#include "vtkDoubleArray.h"
#include "vtkIdTypeArray.h"
#include "vtkSGXMLBCSWriter.h"
#include "vtkSmartPointer.h"
#include <string>

// a program to test out the vtkSGXMLBCSWriter class

int main(int argc, char* argv[])
{
  vtkSmartPointer<vtkDoubleArray> Points = vtkSmartPointer<vtkDoubleArray>::New();
  vtkSmartPointer<vtkIdTypeArray> VertexIds = vtkSmartPointer<vtkIdTypeArray>::New();

  int NumberOfPoints = 20;
  // for now the point values don't matter
  Points->SetNumberOfComponents(3);
  Points->SetNumberOfTuples(NumberOfPoints);
  VertexIds->SetNumberOfTuples(3);
  for(int i=0;i<NumberOfPoints;i++)
    {
    double x = static_cast<double>(i);
    double Coord[3] = {x, x, x};
    Points->SetTupleValue(i, Coord);
    }

  VertexIds->SetValue(0, 0);
  VertexIds->SetValue(1, 8);
  VertexIds->SetValue(2, 19);

  std::string FileName = "test2dbcs.bcs";
  if(argc > 1)
    {
    FileName = argv[1];
    FileName.append("/test2dbcs.bcs");
    }
  vtkSGXMLBCSWriter* Writer = vtkSGXMLBCSWriter::New();
  Writer->SetFileName(FileName.c_str());
  Writer->SetCoords(Points);
  Writer->SetModelVertexIds(VertexIds);
  Writer->Update(); //Writer->Write() does not work because it assumes NumberOfInputConnections must be greater than 0

  Writer->Delete();


  cout << "Finished.\n";

  return 0;
}

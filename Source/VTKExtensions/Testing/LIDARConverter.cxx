//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "smtk/extension/vtk/reader/vtkLIDARReader.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include "vtkXMLPolyDataWriter.h"
#include <string>

int main(int argc, char* argv[])
{
  if (argc != 5)
  {
    cerr << "usage:  LIDARConverter inputFileName outputBaseName split/append/index onRatio\n";
    return -1;
  }

  // 1st arguement is input LIDAR filename
  vtkSmartPointer<vtkLIDARReader> reader = vtkSmartPointer<vtkLIDARReader>::New();
  reader->SetFileName(argv[1]);
  //  reader->SetOnRatio( atoi(argv[4]) );

  vtkSmartPointer<vtkXMLPolyDataWriter> writer = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
  writer->SetInputConnection(reader->GetOutputPort());
  writer->SetDataModeToBinary();

  char outputFileName[256];
  if (strcmp(argv[3], "append")) // not append
  {
    //    reader->AppendPiecesOff();
    if (strcmp(argv[3], "split")) // not split, single index
    {
      //      reader->SetPieceIndex( atoi(argv[3]) );
      reader->ReadFileInfo();
      //sprintf(outputFileName, "%s.%d.vtp", argv[2],
      //  atoi(argv[3]) );
      //cerr << outputFileName << endl;
      //writer->SetFileName(outputFileName);
      //writer->Write();
    }
    else
    {
      int i = 0;
      do
      {
        //        reader->SetPieceIndex(i);
        sprintf(outputFileName, "%s.%d.vtp", argv[2], i);
        cerr << outputFileName << endl;
        writer->SetFileName(outputFileName);
        writer->Write();
      } while (++i < reader->GetKnownNumberOfPieces());
    }
  }
  else
  {
    //    reader->AppendPiecesOn();
    sprintf(outputFileName, "%s.vtp", argv[2]);
    cerr << outputFileName << endl;
    writer->SetFileName(outputFileName);
    writer->Write();
  }

  return 0;
}

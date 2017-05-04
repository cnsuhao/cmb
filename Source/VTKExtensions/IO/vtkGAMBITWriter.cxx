//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkGAMBITWriter.h"

#include "vtkCellArray.h"
#include "vtkCompositeDataIterator.h"
#include "vtkCompositeDataSet.h"
#include "vtkDataArray.h"
#include "vtkDataSetAttributes.h"
#include "vtkInformation.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkUnstructuredGrid.h"
#include <iomanip>
#include <vtksys/SystemTools.hxx>

vtkStandardNewMacro(vtkGAMBITWriter);

vtkGAMBITWriter::vtkGAMBITWriter()
{
  this->FileName = 0;
  this->SetInputArrayToProcess(
    0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_CELLS, vtkDataSetAttributes::SCALARS);
  this->InputGrid = 0;
  this->InputPoly = 0;
}

vtkGAMBITWriter::~vtkGAMBITWriter()
{
  this->SetFileName(0);
}

void vtkGAMBITWriter::SetInputData(vtkDataObject* ug)
{
  this->Superclass::SetInputData(ug);
}

ostream* vtkGAMBITWriter::OpenFile()
{
  if (!this->FileName || !this->FileName[0])
  {
    vtkErrorMacro("FileName has to be specified.");
    return 0;
  }

  ostream* fptr = new ofstream(this->FileName, ios::out);
  if (fptr->fail())
  {
    vtkErrorMacro(<< "Unable to open file: " << this->FileName);
    delete fptr;
    return 0;
  }
  return fptr;
}

void vtkGAMBITWriter::CloseFile(ostream* fp)
{
  delete fp;
}

void vtkGAMBITWriter::WriteData()
{
  vtkDataObject* input = this->GetInput();
  vtkCompositeDataSet* mds = vtkCompositeDataSet::SafeDownCast(input);
  if (mds)
  {
    vtkCompositeDataIterator* iter = mds->NewIterator();
    iter->InitTraversal();
    while (!iter->IsDoneWithTraversal())
    {
      this->InputGrid = vtkUnstructuredGrid::SafeDownCast(iter->GetCurrentDataObject());
      if (this->InputGrid)
      {
        break;
      }
      this->InputPoly = vtkPolyData::SafeDownCast(iter->GetCurrentDataObject());
      if (this->InputPoly)
      {
        break;
      }
    }
    iter->Delete();
  }
  else
  {
    this->InputGrid = vtkUnstructuredGrid::SafeDownCast(input);
    if (!this->InputGrid)
    {
      this->InputPoly = vtkPolyData::SafeDownCast(input);
    }
  }
  if (!(this->InputGrid || this->InputPoly))
  {
    vtkErrorMacro("UnstructuredGrid or PolyData input is required.");
    return;
  }

  ostream* file = this->OpenFile();
  if (!file || !this->WriteHeader(*file) || !this->WritePoints(*file) || !this->WriteCells(*file) ||
    !this->WriteGroups(*file))
  {
    vtkErrorMacro("Write failed");
  }
  if (file)
  {
    this->CloseFile(file);
  }
  this->InputGrid = 0;
  this->InputPoly = 0;
}

bool vtkGAMBITWriter::WriteHeader(ostream& fp)
{
  fp << setw(20) << "CONTROL INFO"
     << " 1.2.1" << endl;
  fp << "** GAMBIT NEUTRAL FILE" << endl;
  fp << "Converted VTK Geometry" << endl;
  fp << "PROGRAM:  " << setw(20) << "vtkGAMBITWriter" << setw(0) << "     VERSION:  1.2.1" << endl;
  fp << vtksys::SystemTools::GetCurrentDateTime("%d %b %Y    %H:%M:%S") << endl;
  fp << "     NUMNP     NELEM     NGRPS    NBSETS     NDFCD     NDFVL" << endl;

  // We ned to determine the number of points and cells
  vtkIdType npnts, ncells;
  if (this->InputGrid)
  {
    npnts = this->InputGrid->GetNumberOfPoints();
    ncells = this->InputGrid->GetNumberOfCells();
  }
  else
  {
    npnts = this->InputPoly->GetNumberOfPoints();
    ncells = this->InputPoly->GetPolys()->GetNumberOfCells();
  }
  fp << setw(10) << npnts << setw(10) << ncells << setw(10) << 1 << setw(10) << 0 << setw(10) << 3
     << setw(10) << 3 << endl;
  fp << "ENDOFSECTION" << endl;
  return true;
}

inline void vtkWriteCell(ostream& fp, int id, int elemType, vtkIdType npts, vtkIdType* ptIds)
{
  // Only 7 points can go on one line.  In the case of the Hex we need to put the 8th
  // point on its own line
  // NOTE THAT WE DO NOT DEAL WITH THE CURVED ELEMENTS!!
  vtkIdType np = (npts == 8) ? 7 : npts;
  // Print out the element number, type and number of points
  fp << setw(8) << id + 1 << " " << setw(2) << elemType << " " << setw(2) << npts << " ";
  for (vtkIdType cc = 0; cc < np; ++cc)
  {
    fp << setw(8) << ptIds[cc] + 1;
  }
  fp << endl;
  if (npts == 8)
  {
    fp << "               " << setw(8) << ptIds[7] << endl;
  }
}

bool vtkGAMBITWriter::WriteCells(ostream& fp)
{

  // Prep the header
  fp << setw(20) << "ELEMENTS/CELLS"
     << " 1.2.1" << endl;
  vtkCellArray* cells;
  vtkIdType npts;
  vtkIdType* ptIds;
  vtkIdType cellID = 0;
  if (this->InputGrid)
  {
    cells = this->InputGrid->GetCells();
    cells->InitTraversal();
    while (cells->GetNextCell(npts, ptIds))
    {
      int cellType = this->InputGrid->GetCellType(cellID);
      switch (cellType)
      {
        case VTK_TRIANGLE:
          if (npts == 3)
          {
            vtkWriteCell(fp, cellID, 3, npts, ptIds);
          }
          break;

        case VTK_QUADRATIC_TRIANGLE:
          if (npts == 6)
          {
            // The point ordering of VTK and the file format are different.
            // Re-order to match GAMBITS's
            vtkIdType outIds[6];
            int indices[6] = { 0, 3, 1, 4, 2, 5 };
            for (int i = 0; i < 6; i++)
            {
              outIds[indices[i]] = ptIds[i];
            }
            vtkWriteCell(fp, cellID, 3, npts, outIds);
          }
          break;

        case VTK_QUAD:
          if (npts == 4)
          {
            vtkWriteCell(fp, cellID, 2, npts, ptIds);
          }
          break;

        case VTK_QUADRATIC_QUAD:
          if (npts == 8)
          {
            // The point ordering of VTK and the file format are different.
            // Re-order to match GAMBIT's
            vtkIdType outIds[8];
            int indices[8] = { 0, 4, 1, 5, 2, 6, 3, 7 };
            for (int i = 0; i < 8; i++)
            {
              outIds[indices[i]] = ptIds[i];
            }
            vtkWriteCell(fp, cellID, 2, npts, outIds);
          }
          break;

        case VTK_TETRA:
          if (npts == 4)
          {
            vtkWriteCell(fp, cellID, 6, npts, ptIds);
          }
          break;
        case VTK_PYRAMID:
          if (npts == 6)
          {
            vtkWriteCell(fp, cellID, 7, npts, ptIds);
          }
          break;
        case VTK_WEDGE:
          if (npts == 6)
          {
            vtkWriteCell(fp, cellID, 5, npts, ptIds);
          }
          break;
        case VTK_HEXAHEDRON:
          if (npts == 8)
          {
            vtkWriteCell(fp, cellID, 4, npts, ptIds);
          }
          break;
        default:
          vtkErrorMacro("Non supported cell types are present.");
          return false;
      }
      cellID++;
    }
  }
  else // This is Polydata and we can tell the cell type based on npts
  {
    cells = this->InputPoly->GetPolys();
    cells->InitTraversal();
    while (cells->GetNextCell(npts, ptIds))
    {
      if (npts == 3) // Triangle
      {
        vtkWriteCell(fp, cellID, 3, npts, ptIds);
      }
      else if (npts == 4) // Quad
      {
        vtkWriteCell(fp, cellID, 2, npts, ptIds);
      }
      else
      {
        // Unsupported
        vtkErrorMacro("Non supported cell types are present.");
        return false;
      }
      cellID++;
    }
  }

  fp << setw(0) << "ENDOFSECTION" << endl;
  return true;
}

bool vtkGAMBITWriter::WriteGroups(ostream& fp)
{

  // Prep the header
  fp << setw(20) << "ELEMENT GROUP"
     << " 1.2.1" << endl;
  vtkCellArray* cells;
  if (this->InputGrid)
  {
    cells = this->InputGrid->GetCells();
  }
  else
  {
    cells = this->InputPoly->GetPolys();
  }

  vtkIdType i, n = cells->GetNumberOfCells();

  // We create a single group of Undefined material type and put all the cells into it
  fp << "GROUP: " << setw(10) << 1 << setw(0) << " ELEMENTS: " << setw(10) << n << setw(0)
     << " MATERIAL: " << setw(10) << 0 << setw(0) << " NFLAGS:" << setw(10) << 1 << endl;
  fp << setw(32) << "default" << endl;
  fp << setw(8) << 0 << endl;

  // Remeber we start at 1
  n++;
  for (i = 1; i < n; i++)
  {
    fp << setw(8) << i;
    // We only put 10 on a line
    if (!(i % 10))
    {
      fp << endl;
    }
  }
  // Did we end on a partial line?
  if ((i - 1) % 10)
  {
    fp << endl;
  }
  fp << setw(0) << "ENDOFSECTION" << endl;
  return true;
}

bool vtkGAMBITWriter::WritePoints(ostream& fp)
{
  fp << setw(20) << "NODAL COORDINATES"
     << " 1.2.1" << endl;
  vtkPoints* points;
  if (this->InputGrid)
  {
    points = this->InputGrid->GetPoints();
  }
  else
  {
    points = this->InputPoly->GetPoints();
  }

  vtkIdType numPts = points->GetNumberOfPoints();
  double pt[3];
  fp.setf(ios::scientific);
  for (vtkIdType cc = 0; cc < numPts; cc++)
  {
    points->GetPoint(cc, pt);
    fp << setw(10) << cc + 1 << setw(20) << setprecision(11) << pt[0] << setw(20)
       << setprecision(11) << pt[1] << setw(20) << setprecision(11) << pt[2] << endl;
  }
  fp << setw(0) << "ENDOFSECTION" << endl;
  return true;
}

int vtkGAMBITWriter::FillInputPortInformation(int, vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataObject");
  return 1;
}

void vtkGAMBITWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

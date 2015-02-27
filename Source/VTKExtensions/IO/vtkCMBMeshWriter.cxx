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
#include "vtkCMBMeshWriter.h"

#include "vtkCellArray.h"
#include "vtkCellTypes.h"
#include "vtkCompositeDataIterator.h"
#include "vtkDataSetAttributes.h"
#include "vtkInformation.h"
#include "vtkCompositeDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkUnstructuredGrid.h"
#include "vtkStructuredGrid.h"
#include "vtkPolyData.h"

#include <sstream>
#include <vtksys/SystemTools.hxx>

#define SEPARATOR "  "
#define INVISIBLE_ID -1

vtkStandardNewMacro(vtkCMBMeshWriter);

struct vtkCMBMeshWriterInternals
{
  vtkPointSet* input;
  // specific input pointers
  vtkPolyData* poly;
  vtkStructuredGrid* structured;
  vtkUnstructuredGrid* unstructured;
  // structured grid variables
  vtkUnsignedCharArray* cellVisArray; // non-NULL if invisible cells included
  vtkIdType nVisCells; // valid only if cellVisArray is non-NULL
  vtkIdTypeArray* ptFileIdArray; // non-NULL if invisible points included
  vtkIdType nVisPts; // valid only if ptFileIdArray is non-NULL
  vtkIdList* cellPtList;

  vtkCMBMeshWriterInternals() : input(NULL), poly(NULL), structured(NULL),
    unstructured(NULL), cellVisArray(NULL),
    nVisCells(0), ptFileIdArray(NULL), nVisPts(0), cellPtList(NULL) {}
  ~vtkCMBMeshWriterInternals() {}

  void SetInput(vtkDataObject* data)
    {
    ClearInput();
    this->input = vtkPointSet::SafeDownCast(data);
    if (this->input)
      {
      this->poly = vtkPolyData::SafeDownCast(data);
      InitPoly();
      this->structured = vtkStructuredGrid::SafeDownCast(data);
      InitStructured();
      this->unstructured = vtkUnstructuredGrid::SafeDownCast(data);
      }
    }

  bool HaveInput()
    {
    return (this->input && (this->poly || this->structured || this->unstructured));
    }

  void ClearInput()
    {
    this->input = NULL;
    this->poly = NULL;
    this->unstructured = NULL;
    ClearStructured();
    }

  void InitPoly()
    {
    if (poly)
      {
      // make sure cells are built.
      this->poly->BuildCells();
      }
    }

  void InitStructured()
    {
    if (!this->structured)
      {
      return;
      }
    // cell visibility
    // GetCellBlanking checks for cell and point visibility constraints
    if (this->structured->GetCellBlanking())
      {
      // Create own cell visibility array. This is necessary because a cell can be
      // invisible based on its own constraint or its points' individual constraints.
      // Calling GetCellVisibilityArray is not enough. Also count the number of
      // visible cells
      this->cellVisArray = vtkUnsignedCharArray::New();
      vtkIdType ncells = this->structured->GetNumberOfCells();
      this->cellVisArray->SetNumberOfValues(ncells);
      unsigned char val;
      this->nVisCells = 0;
      for (vtkIdType cc = 0; cc < ncells; ++cc)
        {
        val = (this->structured->IsCellVisible(cc) ? 1 : 0);
        this->cellVisArray->SetValue(cc, val);
        this->nVisCells += val;
        }
      }
    // node visibility
    vtkUnsignedCharArray* ptVisArray = this->structured->GetPointVisibilityArray();
    if (ptVisArray)
      {
      // Create point file id array so point ids are written as a contiguous set
      // ids are zero-based. Also count the number of visible points
      this->ptFileIdArray = vtkIdTypeArray::New();
      this->ptFileIdArray->SetName("Point File IDs");
      vtkIdType npts = this->structured->GetNumberOfPoints();
      this->ptFileIdArray->SetNumberOfValues(npts);
      this->nVisPts = 0;
      for (vtkIdType cc = 0; cc < npts; ++cc)
        {
        this->ptFileIdArray->SetValue(cc,
          ptVisArray->GetValue(cc) ? this->nVisPts++ : INVISIBLE_ID);
        }
      }
    this->cellPtList = vtkIdList::New();
    }

  // cell id is incremented when invisible (blank) cells are skipped
  bool GetNextStructuredCell(vtkIdType & cellId, vtkIdType & npts, vtkIdType *& pts)
    {
    if (!this->structured || cellId >= this->structured->GetNumberOfCells())
      {
      return false;
      }
    // skip invisible (blank) cells with an empty for loop
    for (; this->cellVisArray && !this->cellVisArray->GetValue(cellId); ++cellId);
    this->structured->GetCellPoints(cellId, cellPtList);
    npts = cellPtList->GetNumberOfIds();
    pts = cellPtList->GetPointer(0);
    return true;
    }

  void ClearStructured()
    {
    this->structured = NULL;
    if (this->cellVisArray)
      {
      this->cellVisArray->Delete();
      }
    this->cellVisArray = NULL;
    this->nVisCells = 0;
    if (this->ptFileIdArray)
      {
      this->ptFileIdArray->Delete();
      }
    ptFileIdArray = NULL;
    this->nVisPts = 0;
    if (this->cellPtList)
      {
      this->cellPtList->Delete();
      }
    }
};

// This list should contain the file format names in the same order as the
// enum in vtkCMBMeshWriter.h. Make sure this list is NULL terminated
static const char* vtkCMBMeshFormatStrings[] = {
  "ADH", "PT123", "WASH123D", "XMS", NULL };

//----------------------------------------------------------------------------
vtkCMBMeshWriter::vtkCMBMeshWriter()
{
  this->FileName = NULL;
  this->MeshDimension = MESH3D;
  this->ValidateDimension = true;
  this->FileFormat = PT123;
  this->SetInputArrayToProcess(0, 0, 0,
    vtkDataObject::FIELD_ASSOCIATION_CELLS,
    vtkDataSetAttributes::SCALARS);
  this->WriteMetaInfo = false;
  this->UseScientificNotation = true;
  this->FloatPrecision = 6;
  this->Internals = new vtkCMBMeshWriterInternals;
}

//----------------------------------------------------------------------------
vtkCMBMeshWriter::~vtkCMBMeshWriter()
{
  this->SetFileName(NULL);
  delete this->Internals;
}

//----------------------------------------------------------------------------
void vtkCMBMeshWriter::SetInputData(vtkDataObject* ug)
{
  this->Superclass::SetInputData(ug);
}

//----------------------------------------------------------------------------
ostream* vtkCMBMeshWriter::OpenFile()
{
  if (!this->FileName || !this->FileName[0])
    {
    vtkErrorMacro("FileName has to be specified.");
    return NULL;
    }

  ostream* fp = new ofstream(this->FileName, ios::out);
  if (fp->fail())
    {
    vtkErrorMacro(<< "Unable to open file: "<< this->FileName);
    delete fp;
    return NULL;
    }
  return fp;
}

//----------------------------------------------------------------------------
void vtkCMBMeshWriter::CloseFile(ostream* fp)
{
  if (fp)
    {
    delete fp;
    fp = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkCMBMeshWriter::WriteData()
{
  vtkDataObject* input = this->GetInput();
  vtkCompositeDataSet* mds = vtkCompositeDataSet::SafeDownCast(input);
  if (mds)
    {
    vtkCompositeDataIterator* iter = mds->NewIterator();
    iter->InitTraversal();
    while (!iter->IsDoneWithTraversal())
      {
      this->Internals->SetInput(iter->GetCurrentDataObject());
      if (this->Internals->HaveInput())
        {
        break;
        }
      }
    iter->Delete();
    }
  else
    {
    this->Internals->SetInput(input);
    }

  if (!this->Internals->HaveInput())
    {
    vtkErrorMacro("UnstructuredGrid, StructuredGrid, or compatiable PolyData "
      "input is required.");
    return;
    }

  // Validate mesh before attempting to write file
  if (!ValidateFileFormat())
    {
    vtkErrorMacro("Write failed.");
    return;
    }

  ostream* file = this->OpenFile();
  if (!file ||
      !this->WriteHeader(*file) ||
      !this->WriteCells(*file) ||
      !this->WritePoints(*file) ||
      !this->WriteFooter(*file))
    {
    vtkErrorMacro("Write failed.");
    }
  this->CloseFile(file);
  this->Internals->ClearInput();
}

//----------------------------------------------------------------------------
bool vtkCMBMeshWriter::WriteHeader(ostream& fp)
{
  switch (this->FileFormat)
    {
    case WASH123D:
      switch (this->MeshDimension)
        {
        case MESH1D:
          fp << "WMS1DM" << endl;
          break;
        case MESH2D:
          fp << "WMS2DM" << endl;
          break;
        case MESH3D:
        default:
          fp << "WMS3DM" << endl;
          break;
        }
      fp << "T1" << endl << "T2" << endl << "T3" << endl;
      break;
    case ADH:
    case PT123:
    case XMS:
    default:
      switch (this->MeshDimension)
        {
        case MESH1D:
          fp << "MESH1D" << endl;
          break;
        case MESH2D:
          fp << "MESH2D" << endl;
          break;
        case MESH3D:
        default:
          fp << "MESH3D" << endl;
          break;
        }
      break;
    }
  if (this->WriteMetaInfo)
    {
    vtkIdType ncells = 0, npts = 0;
    if (this->Internals->unstructured)
      {
      ncells = this->Internals->unstructured->GetNumberOfCells();
      npts = this->Internals->unstructured->GetNumberOfPoints();
      }
    else if (this->Internals->poly)
      {
      ncells = this->Internals->poly->GetNumberOfPolys();
      npts = this->Internals->poly->GetNumberOfPoints();
      }
    else if (this->Internals->structured)
      {
      ncells = (this->Internals->cellVisArray ? this->Internals->nVisCells :
        this->Internals->structured->GetNumberOfCells());
      npts = (this->Internals->ptFileIdArray ? this->Internals->nVisPts :
        this->Internals->structured->GetNumberOfPoints());
      }
    fp << "#NELEM" << SEPARATOR << ncells << endl;
    fp << "#NNODE" << SEPARATOR << npts << endl;
    }
  return true;
}

inline unsigned long vtkGetMaterial(vtkDataArray* array, vtkIdType cc)
{
  if (array)
    {
    void* data = array->GetVoidPointer(0);
    int numComps = array->GetNumberOfComponents();
    switch (array->GetDataType())
      {
      vtkTemplateMacro(

        return static_cast<unsigned long>(
          reinterpret_cast<VTK_TT*>(data)[cc*numComps]);
      );
      }
    }
  return 1;
}

bool vtkWriteCell(ostream& fp, vtkIdType npts, vtkIdType* ptIds,
  vtkIdTypeArray* fileIdArray)
{
  // Handle invisible nodes
  if (fileIdArray)
    {
    for (vtkIdType cc=0, id=0; cc < npts; ++cc)
      {
      id = fileIdArray->GetValue(ptIds[cc]);
      if (id == INVISIBLE_ID)
        {
        return false;
        }
      fp << SEPARATOR << id+1;
      }
    }
  else
    {
    for (vtkIdType cc=0; cc < npts; ++cc)
      {
      fp << SEPARATOR << ptIds[cc]+1;
      }
    }
  return true;
}

std::string vtkGetCellCard(int cellType, vtkIdType npts, int fileFormat)
{
  // If the cell type is not supported by the file format then return an empty string.
  // Check the number of points to ensure the cell type is correct.
  switch (fileFormat)
    {
    case vtkCMBMeshWriter::WASH123D:
    case vtkCMBMeshWriter::PT123:
      switch (cellType)
        {
        // 1D Elements
        case VTK_LINE:
          return (npts == 2 ? "GE2" : "");
        // 2D Elements
        case VTK_TRIANGLE:
          return (npts == 3 ? "GE3" : "");
        case VTK_QUAD:
        case VTK_PIXEL:
          return (npts == 4 ? "GE4" : "");
        // 3D Elements
        case VTK_TETRA:
          return (npts == 4 ? "GE4" : "");
        case VTK_WEDGE:
          return (npts == 6 ? "GE6" : "");
        case VTK_HEXAHEDRON:
        case VTK_VOXEL:
          return (npts == 8 ? "GE8" : "");
        }
      break;
    case vtkCMBMeshWriter::ADH:
      switch (cellType)
        {
        // 2D Elements
        case VTK_TRIANGLE:
          return (npts == 3 ? "E3T" : "");
        // 3D Elements
        case VTK_TETRA:
          return (npts == 4 ? "E4T" : "");
        }
      break;
    case vtkCMBMeshWriter::XMS:
    default:
      switch (cellType)
        {
        // 1D Elements
        case VTK_LINE:
          return (npts == 2 ? "E2L" : "");
        case VTK_QUADRATIC_EDGE:
          return (npts == 3 ? "E3L" : "");
        // 2D Elements
        case VTK_TRIANGLE:
          return (npts == 3 ? "E3T" : "");
        case VTK_QUADRATIC_TRIANGLE:
          return (npts == 6 ? "E6T" : "");
        case VTK_QUAD:
        case VTK_PIXEL:
          return (npts == 4 ? "E4Q" : "");
        case VTK_QUADRATIC_QUAD:
          return (npts == 8 ? "E8Q" : "");
        case VTK_BIQUADRATIC_QUAD:
          return (npts == 9 ? "E9Q" : "");
        // 3D ELements
        case VTK_TETRA:
          return (npts == 4 ? "E4T" : "");
        case VTK_PYRAMID:
          return (npts == 5 ? "E5P" : "");
        case VTK_WEDGE:
          return (npts == 6 ? "E6W" : "");
        case VTK_HEXAHEDRON:
        case VTK_VOXEL:
          return (npts == 8 ? "E8H" : "");
        }
      break;
    }
  return "";
}

//----------------------------------------------------------------------------
bool vtkCMBMeshWriter::WriteCells(ostream& fp)
{
  vtkDataArray* materialArray = NULL;
  if (this->FileFormat != PT123)
    {
    materialArray = this->GetInputArrayToProcess(0, this->Internals->input);
    if (!materialArray)
        {
        vtkWarningMacro("Failed to locate material array. Using 1 for material id.");
        }
    }

  vtkCellArray* cells = NULL;
  vtkIdType cellOffset = 0, ncells = 0;
  if (this->Internals->unstructured)
    {
    cells = this->Internals->unstructured->GetCells();
    ncells = this->Internals->unstructured->GetNumberOfCells();
    }
  else if (this->Internals->poly)
    {
    cellOffset = this->Internals->poly->GetNumberOfVerts() +
      this->Internals->poly->GetNumberOfLines();
    cells = this->Internals->poly->GetPolys();
    ncells = this->Internals->poly->GetNumberOfPolys();
    }
  else if (this->Internals->structured)
    {
    ncells = this->Internals->cellVisArray ? this->Internals->nVisCells :
      this->Internals->structured->GetNumberOfCells();
    }
  if (ncells <= 0)
    {
    vtkErrorMacro("No compatiable cells are present for writing out mesh.");
    return false;
    }

  std::string card = "";
  vtkIdType* ptIds = NULL;
  vtkIdType cellId = 0, fileId = 0, npts = 0;
  bool res = true;
  int cellType;

  if (cells)
    {
    cells->InitTraversal();
    }
  while (cells ? cells->GetNextCell(npts, ptIds) :
           this->Internals->GetNextStructuredCell(cellId, npts, ptIds))
    {
    cellType = this->Internals->input->GetCellType(cellId + cellOffset);
    card = ::vtkGetCellCard(cellType, npts, this->FileFormat);
    if (card.empty())
      {
      vtkErrorMacro(<< "Element " << cellId << " (cell ID) is incompatible.");
      return false;
      }
    // Write cell type card and cell's file ID (file format's IDs are one-based and
    // contiguous; cellId could have gaps due to invisible cells)
    fp << card << SEPARATOR << ++fileId;
    // The point ordering of VTK and some of the file format's cell types are different.
    // Re-order to match VTK's if necessary and then write points.
    switch (cellType)
      {
      case VTK_PIXEL:
      case VTK_VOXEL:
        {
        vtkIdType outIds[8];
        int indices[8] = {0, 1, 3, 2, 4, 5, 7, 6};
        for (int i=0; i<npts; ++i)
          {
          outIds[indices[i]] = ptIds[i];
          }
        res = ::vtkWriteCell(fp, npts, outIds, this->Internals->ptFileIdArray);
        }
        break;
      case VTK_QUADRATIC_TRIANGLE:
        {
        vtkIdType outIds[6];
        int indices[6] = {0, 2, 4, 1, 3, 5};
        for (int i=0; i<6; ++i)
          {
          outIds[indices[i]] = ptIds[i];
          }
        res = ::vtkWriteCell(fp, npts, outIds, this->Internals->ptFileIdArray);
        }
        break;
      case VTK_QUADRATIC_QUAD:
      case VTK_BIQUADRATIC_QUAD:
        {
        vtkIdType outIds[9];
        int indices[9] = {0, 2, 4, 6, 1, 3, 5, 7, 8};
        for (int i=0; i < npts; ++i)
          {
          outIds[indices[i]] = ptIds[i];
          }
        res = ::vtkWriteCell(fp, npts, outIds, this->Internals->ptFileIdArray);
        }
        break;
      default:
        res = ::vtkWriteCell(fp, npts, ptIds, this->Internals->ptFileIdArray);
        break;
      }
    if (!res)
      {
      vtkErrorMacro("Visible cell includes invisible point; incompatiable with "
        "file format.");
      return false;
      }
    // Write material
    if (this->FileFormat != PT123)
      {
      fp << SEPARATOR << ::vtkGetMaterial(materialArray, cellId + cellOffset);
      }
    fp << endl;

    ++cellId;
    }

  return true;
}

//----------------------------------------------------------------------------
bool vtkCMBMeshWriter::WritePoints(ostream& fp)
{
  vtkPoints* points = this->Internals->input->GetPoints();
  vtkIdType numPts = points->GetNumberOfPoints();
  std::string card;
  double pts[3];

  switch (this->FileFormat)
    {
    case WASH123D:
    case PT123:
      card = "GN";
      break;
    case ADH:
    case XMS:
    default:
      card = "ND";
      break;
    }
  fp.precision(this->FloatPrecision);
  fp.setf(ios::showpoint);
  fp.setf(UseScientificNotation ? ios::scientific : ios::fixed, ios::floatfield);
  // Handle invisible nodes
  if (this->Internals->ptFileIdArray)
    {
    for (vtkIdType cc=0, id=0; cc < numPts; ++cc)
      {
      id = this->Internals->ptFileIdArray->GetValue(cc);
      if (id == INVISIBLE_ID)
        {
        continue;
        }
      points->GetPoint(cc, pts);
      fp << card << SEPARATOR << id+1 << SEPARATOR << pts[0] << SEPARATOR <<
        pts[1] << SEPARATOR << pts[2] << endl;
      }
    }
  else {
    for (vtkIdType cc=0; cc < numPts; ++cc)
      {
      points->GetPoint(cc, pts);
      fp << card << SEPARATOR << cc+1 << SEPARATOR << pts[0] << SEPARATOR <<
        pts[1] << SEPARATOR << pts[2] << endl;
      }
    }
  return true;
}

//----------------------------------------------------------------------------
bool vtkCMBMeshWriter::WriteFooter(ostream& fp)
{
  switch (this->FileFormat)
    {
    case PT123:
      fp << "ENDR" << endl;
      break;
    case WASH123D:
      fp << "END" << endl;
      break;
    }
  return true;
}

//----------------------------------------------------------------------------
bool vtkCMBMeshWriter::ValidateFileFormat()
{
  if (!this->Internals->input)
    {
    return false;
    }

  vtkCellTypes *types = vtkCellTypes::New();
  this->Internals->input->GetCellTypes(types);
  bool valid = true;
  int type, numTypes = types->GetNumberOfTypes();

  for (int i = 0; i < numTypes && valid; ++i)
    {
    valid = false;
    type = types->GetCellType(i);
    switch (this->FileFormat)
      {
      case ADH:
        switch (type)
          {
          // 2D Elements - 2D Shallow water ADH expects 2D mesh within *.3dm extension
          case VTK_TRIANGLE:
            valid = (this->ValidateDimension ? (this->MeshDimension == MESH2D) : true);
            break;
          // 3D Elements
          case VTK_TETRA:
            valid = (this->ValidateDimension ? (this->MeshDimension == MESH3D) : true);
            break;
          }
        break;
      case PT123:
      case WASH123D:
        switch (type)
          {
          // 1D Elements
          case VTK_LINE:
            valid = (this->ValidateDimension ? (this->MeshDimension == MESH1D) : true);
            break;
          // 2D Elements
          case VTK_TRIANGLE:
          case VTK_QUAD:
          case VTK_PIXEL:
            valid = (this->ValidateDimension ? (this->MeshDimension == MESH2D) : true);
            break;
          // 3D Elements
          case VTK_TETRA:
          case VTK_WEDGE:
          case VTK_HEXAHEDRON:
          case VTK_VOXEL:
            valid = (this->ValidateDimension ? (this->MeshDimension == MESH3D) : true);
            break;
          }
        break;
      case XMS:
        switch (type)
          {
          // 1D Elements
          case VTK_LINE:
          case VTK_QUADRATIC_EDGE:
            valid = (this->ValidateDimension ? (this->MeshDimension == MESH1D) : true);
            break;
          // 2D Elements
          case VTK_TRIANGLE:
          case VTK_QUADRATIC_TRIANGLE:
          case VTK_QUAD:
          case VTK_PIXEL:
          case VTK_QUADRATIC_QUAD:
          case VTK_BIQUADRATIC_QUAD:
            valid = (this->ValidateDimension ? (this->MeshDimension == MESH2D) : true);
            break;
          // 3D ELements
          case VTK_TETRA:
          case VTK_PYRAMID:
          case VTK_WEDGE:
          case VTK_HEXAHEDRON:
          case VTK_VOXEL:
            valid = (this->ValidateDimension ? (this->MeshDimension == MESH3D) : true);
            break;
          }
        break;
      }

      if (!valid)
        {
        if (type == VTK_EMPTY_CELL && this->Internals->structured)
          {
          // Ignore the empty cell only if dealing with structured grid input
          // because invisible cells return this type.
          valid = true;
          continue;
          }
        std::stringstream msg;
        if (this->FileFormat == ADH && this->MeshDimension == MESH1D)
          {
          msg << "ADH does not have a 1D file format.";
          }
        else
          {
          msg << "The " << vtkCMBMeshFormatStrings[this->FileFormat] << " ";
          if (this->ValidateDimension)
            {
            msg << this->MeshDimension << "D ";
            }
          msg << "file format does not support the " <<
            types->GetClassNameFromTypeId(type) << " cell type.";
          }
        vtkErrorMacro(<< msg.str());
        }
    }

  if (valid && !this->ValidateDimension)
    {
    switch (this->FileFormat)
      {
      case PT123:
      case WASH123D:
        if ((types->IsType(VTK_QUAD) || types->IsType(VTK_PIXEL)) &&
            types->IsType(VTK_TETRA))
          {
          vtkWarningMacro("The " << vtkCMBMeshFormatStrings[this->FileFormat] <<
            " file format does not differentiate between 2D quadrilaterals and "
            "3D tetrahedrals.");
          }
        break;
      }
    }

  types->Delete();

  return valid;
}

//----------------------------------------------------------------------------
int vtkCMBMeshWriter::FillInputPortInformation(int,
  vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataObject");
  return 1;
}

//----------------------------------------------------------------------------
void vtkCMBMeshWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "File Name: " <<
    (this->FileName ? this->FileName : "(none)") << endl;
  os << indent << "FileFormat: " <<
    vtkCMBMeshFormatStrings[this->FileFormat] << endl;
  os << indent << "MeshDimension: " << MeshDimension << endl;
  os << indent << "ValidateDimension: " <<
    (this->ValidateDimension ? "On" : "Off") << endl;
  os << indent << "WriteMetaInfo: " <<
    (this->WriteMetaInfo ? "On" : "Off") << endl;
  os << indent << "UseScientificNotation: " <<
    (this->UseScientificNotation ? "On" : "Off") << endl;
  os << indent << "FloatPrecision: " << FloatPrecision << endl;
}

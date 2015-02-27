/*=========================================================================

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
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

#include "vtkCMBSTLReader.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"

#include "vtkCell.h"
#include "vtkCellData.h"
#include "vtkCleanUnstructuredGrid.h"
#include "vtkIdTypeArray.h"
#include "vtkPolyDataConnectivityFilter.h"
#include "vtkSTLReader.h"
#include "vtkMultiBlockWrapper.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"


vtkStandardNewMacro(vtkCMBSTLReader);

vtkCMBSTLReader::vtkCMBSTLReader()
{
  this->FileName = 0;
  this->SetNumberOfInputPorts(0);
}

vtkCMBSTLReader::~vtkCMBSTLReader()
{
  this->SetFileName(0);
}

int vtkCMBSTLReader::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{
  // get the info object
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the ouptut
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  std::ifstream fin(this->FileName);
  if(!fin.good())
    {
    vtkErrorMacro(<< "File " << this->FileName << " not found");
    fin.close();
    return 0;
    }
  fin.close();

  vtkSTLReader *reader = vtkSTLReader::New();
  reader->SetFileName(this->GetFileName());
  reader->ScalarTagsOn();

  //assign each region a different color scalar
  vtkPolyDataConnectivityFilter *seperateRegions = vtkPolyDataConnectivityFilter::New();
  seperateRegions->SetExtractionModeToAllRegions();
  seperateRegions->SetInputConnection( reader->GetOutputPort() );
  seperateRegions->ColorRegionsOn();
  seperateRegions->Update();

  //set the output to the seperated regions
  output->ShallowCopy(seperateRegions->GetOutput(0));
  seperateRegions->Delete();
  reader->Delete();


  vtkIdTypeArray *regions = vtkIdTypeArray::SafeDownCast( output->GetPointData()->GetScalars() );
  if ( !regions )
    {
    vtkErrorMacro("Unable to find any region.");
    return 0;
    }

  //we need to determine which region each cell is in
  //vtkPolyDataConnectivityFilter does not allow a cell to belong to more than one region
  vtkIntArray *regionArray = vtkIntArray::New();
  regionArray->SetNumberOfValues( output->GetNumberOfCells() );
  regionArray->SetName( vtkMultiBlockWrapper::GetShellTagName() );
  vtkIdType id=0;
  for ( vtkIdType i=0; i < output->GetNumberOfCells( ); ++i )
    {
    id = output->GetCell(i)->GetPointId(0);
    regionArray->SetValue( i, regions->GetValue( id ) );
    }

  output->GetCellData()->AddArray( regionArray );
  regionArray->Delete();


  return 1;
}

//----------------------------------------------------------------------------
int vtkCMBSTLReader::RequestInformation(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *vtkNotUsed(outputVector))
{
  if (!this->FileName)
    {
    vtkErrorMacro("FileName has to be specified!");
    return 0;
    }

  return 1;
}

void vtkCMBSTLReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "FileName: " << this->FileName << endl;
}

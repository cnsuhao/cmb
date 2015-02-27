#include "vtkPVGMSRegionArrayInformation.h"

#include "vtkCellData.h"
#include "vtkClientServerStream.h"
#include "vtkDataObject.h"
#include "vtkIntArray.h"
#include "vtkObjectFactory.h"
#include "vtkStringArray.h"
#include "vtkFieldData.h"
#include "vtkAlgorithmOutput.h"
#include "vtkAlgorithm.h"
#include "vtkCompositeDataSet.h"
#include "vtkCompositeDataIterator.h"
#include "vtkMultiBlockWrapper.h"
#include "vtkStringReader.h"
#include "vtkUnstructuredGrid.h"

vtkStandardNewMacro(vtkPVGMSRegionArrayInformation);

//----------------------------------------------------------------------------
vtkPVGMSRegionArrayInformation::vtkPVGMSRegionArrayInformation()
{
  this->RegionArray = NULL;
}

//----------------------------------------------------------------------------
vtkPVGMSRegionArrayInformation::~vtkPVGMSRegionArrayInformation()
{
  if(this->RegionArray)
    {
    this->RegionArray->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkPVGMSRegionArrayInformation::PrintSelf(ostream &os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "RegionArray: " << this->RegionArray << endl;
}

//----------------------------------------------------------------------------
void vtkPVGMSRegionArrayInformation::CopyFromObject(vtkObject* obj)
{
  vtkUnstructuredGrid *dataObject = vtkUnstructuredGrid::SafeDownCast( obj );

  // Handle the case where the a vtkAlgorithmOutput is passed instead of
  // the data object. vtkSMPart uses vtkAlgorithmOutput.
  if (!dataObject)
    {
    vtkAlgorithmOutput* algOutput = vtkAlgorithmOutput::SafeDownCast( obj );
    if (algOutput && algOutput->GetProducer())
      {
      dataObject = vtkUnstructuredGrid::SafeDownCast(
        algOutput->GetProducer()->GetOutputDataObject(
        algOutput->GetIndex() ));
      }
    vtkAlgorithm* alg = vtkAlgorithm::SafeDownCast( obj );
    if (alg)
      {
      dataObject = vtkUnstructuredGrid::SafeDownCast(
        alg->GetOutputDataObject( 0 ));
      }
    if (!dataObject)
      {
      vtkErrorMacro("Unable to get data object from object!");
      return;
      }
  }
  if(this->RegionArray)
    {
    this->RegionArray->Delete();
    this->RegionArray = NULL;
    }
  vtkDataArray* dataArray = dataObject->GetCellData()->GetArray(
    vtkMultiBlockWrapper::GetShellTagName());
  this->RegionArray = vtkIntArray::SafeDownCast(dataArray->NewInstance());
  this->RegionArray->DeepCopy(dataArray);
}

//----------------------------------------------------------------------------
void vtkPVGMSRegionArrayInformation::AddInformation(vtkPVInformation* info)
{
  vtkPVGMSRegionArrayInformation *regionArrayInfo =
    vtkPVGMSRegionArrayInformation::SafeDownCast(info);
  if (regionArrayInfo && regionArrayInfo->GetRegionArray())
    {
    if(this->RegionArray)
      {
      this->RegionArray->Delete();
      this->RegionArray = NULL;
      }
    this->RegionArray = regionArrayInfo->GetRegionArray()->NewInstance();
    this->RegionArray->DeepCopy(regionArrayInfo->GetRegionArray());
    }
}

//----------------------------------------------------------------------------
void
vtkPVGMSRegionArrayInformation::CopyToStream(vtkClientServerStream* css)
{
  //css->Reset();
  //*css << vtkClientServerStream::Reply;
  //*css << vtkClientServerStream::InsertArray(this->RegionArray, 3)
  //     << vtkClientServerStream::End;
}

//----------------------------------------------------------------------------
void
vtkPVGMSRegionArrayInformation::CopyFromStream(const vtkClientServerStream* css)
{
  //double elements[3];
  //css->GetArgument(0, 0, elements, 3);
  //css->GetArgument(0, 1, this->RegionArray, 3);
}

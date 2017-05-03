//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkSMArcOperatorProxy.h"

#include "vtkClientServerStream.h"
#include "vtkObjectFactory.h"
#include "vtkPVXMLElement.h"
#include "vtkProcessModule.h"
#include "vtkSMNewWidgetRepresentationProxy.h"
#include "vtkSMOutputPort.h"
#include "vtkSMSourceProxy.h"

vtkStandardNewMacro(vtkSMArcOperatorProxy);
//----------------------------------------------------------------------------
vtkSMArcOperatorProxy::vtkSMArcOperatorProxy()
{
}

//----------------------------------------------------------------------------
vtkSMArcOperatorProxy::~vtkSMArcOperatorProxy()
{
}

//----------------------------------------------------------------------------
bool vtkSMArcOperatorProxy::Operate(vtkSMSourceProxy* sourceProxy)
{
  this->UpdateVTKObjects();

  vtkClientServerStream stream;
  stream << vtkClientServerStream::Invoke << VTKOBJECT(sourceProxy) << "GetOutput"
         << vtkClientServerStream::End << vtkClientServerStream::Invoke << VTKOBJECT(this)
         << "Operate" << vtkClientServerStream::LastResult << vtkClientServerStream::End;

  this->ExecuteStream(stream);

  vtkClientServerStream result = this->GetLastResult(vtkProcessModule::DATA_SERVER_ROOT);

  bool valid = false;
  result.GetArgument(0, 0, &valid);
  return valid;
}

//----------------------------------------------------------------------------
bool vtkSMArcOperatorProxy::Operate(vtkSMOutputPort* sourceOutputPort)
{
  this->UpdateVTKObjects();

  //From the selection we can get the source proxy
  //The selection of a proxy is stored on source proxy so with the output port
  //number we can now ask for selection proxy on the correct output port from the source proxy
  vtkClientServerStream stream;
  stream << vtkClientServerStream::Invoke << VTKOBJECT(this) << "Operate"
         << VTKOBJECT(sourceOutputPort->GetSourceProxy()->GetSelectionOutput(
              sourceOutputPort->GetPortIndex()))
         << vtkClientServerStream::End;

  this->ExecuteStream(stream);
  vtkClientServerStream result = this->GetLastResult(vtkProcessModule::DATA_SERVER_ROOT);

  bool valid = false;
  result.GetArgument(0, 0, &valid);
  return valid;
}

//----------------------------------------------------------------------------
bool vtkSMArcOperatorProxy::Operate(vtkSMNewWidgetRepresentationProxy* widgetProxy)
{
  this->UpdateVTKObjects();
  vtkSMProxy* repProxy = widgetProxy->GetRepresentationProxy();
  vtkPVXMLElement* hints = repProxy->GetHints();
  if (repProxy && hints && hints->FindNestedElementByName("Output"))
  {
    //currently this filter only supports contour representations
    //the selection is now an int array on the poly data
    vtkClientServerStream stream;
    stream << vtkClientServerStream::Invoke << VTKOBJECT(repProxy)
           << "GetContourRepresentationAsPolyData" << vtkClientServerStream::End
           << vtkClientServerStream::Invoke << VTKOBJECT(this) << "Operate"
           << vtkClientServerStream::LastResult << vtkClientServerStream::End;
    this->ExecuteStream(stream);

    vtkClientServerStream result = this->GetLastResult(vtkProcessModule::DATA_SERVER_ROOT);

    bool valid = false;
    result.GetArgument(0, 0, &valid);
    return valid;
  }
  return false;
}

//----------------------------------------------------------------------------
bool vtkSMArcOperatorProxy::Operate(vtkIdType arcId)
{
  this->UpdateVTKObjects();

  vtkClientServerStream stream;
  stream << vtkClientServerStream::Invoke << VTKOBJECT(this) << "Operate" << arcId
         << vtkClientServerStream::End;

  this->ExecuteStream(stream);

  vtkClientServerStream result = this->GetLastResult(vtkProcessModule::DATA_SERVER_ROOT);

  bool valid = false;
  result.GetArgument(0, 0, &valid);
  return valid;
}

//----------------------------------------------------------------------------
bool vtkSMArcOperatorProxy::Operate(vtkIdType firstArcId, vtkIdType secondArcId)
{
  this->UpdateVTKObjects();

  vtkClientServerStream stream;
  stream << vtkClientServerStream::Invoke << VTKOBJECT(this) << "Operate" << firstArcId
         << secondArcId << vtkClientServerStream::End;

  this->ExecuteStream(stream);

  vtkClientServerStream result = this->GetLastResult(vtkProcessModule::DATA_SERVER_ROOT);

  bool valid = false;
  result.GetArgument(0, 0, &valid);
  return valid;
}

//----------------------------------------------------------------------------
bool vtkSMArcOperatorProxy::Operate()
{
  this->UpdateVTKObjects();

  vtkClientServerStream stream;
  stream << vtkClientServerStream::Invoke << VTKOBJECT(this) << "Operate"
         << vtkClientServerStream::End;

  this->ExecuteStream(stream);

  vtkClientServerStream result = this->GetLastResult(vtkProcessModule::DATA_SERVER_ROOT);

  bool valid = false;
  result.GetArgument(0, 0, &valid);
  return valid;
}

//----------------------------------------------------------------------------
void vtkSMArcOperatorProxy::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

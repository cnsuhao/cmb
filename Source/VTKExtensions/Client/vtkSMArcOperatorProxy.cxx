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
#include "vtkSMArcOperatorProxy.h"

#include "vtkClientServerStream.h"
#include "vtkObjectFactory.h"
#include "vtkSMSourceProxy.h"
#include "vtkSMNewWidgetRepresentationProxy.h"
#include "vtkProcessModule.h"
#include "vtkPVXMLElement.h"
#include "vtkSMOutputPort.h"

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
  stream  << vtkClientServerStream::Invoke
    << VTKOBJECT(sourceProxy) << "GetOutput"
    << vtkClientServerStream::End
    << vtkClientServerStream::Invoke
    << VTKOBJECT(this) << "Operate"
    << vtkClientServerStream::LastResult
    << vtkClientServerStream::End;

  this->ExecuteStream(stream);

  vtkClientServerStream result =
          this->GetLastResult(vtkProcessModule::DATA_SERVER_ROOT);

  bool valid = false;
  result.GetArgument(0,0,&valid);
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
  stream << vtkClientServerStream::Invoke
    << VTKOBJECT(this) << "Operate"
    << VTKOBJECT(sourceOutputPort->GetSourceProxy()->GetSelectionOutput(
                   sourceOutputPort->GetPortIndex()))
    << vtkClientServerStream::End;

  this->ExecuteStream(stream);
  vtkClientServerStream result =
          this->GetLastResult(vtkProcessModule::DATA_SERVER_ROOT);

  bool valid = false;
  result.GetArgument(0,0,&valid);
  return valid;
}

//----------------------------------------------------------------------------
bool vtkSMArcOperatorProxy::Operate(
  vtkSMNewWidgetRepresentationProxy* widgetProxy)
{
  this->UpdateVTKObjects();
  vtkSMProxy *repProxy = widgetProxy->GetRepresentationProxy();
  vtkPVXMLElement* hints = repProxy->GetHints();
  if ( repProxy && hints && hints->FindNestedElementByName("Output"))
    {
    //currently this filter only supports contour representations
    //the selection is now an int array on the poly data
    vtkClientServerStream stream;
    stream  << vtkClientServerStream::Invoke
            << VTKOBJECT(repProxy) << "GetContourRepresentationAsPolyData"
            << vtkClientServerStream::End
            << vtkClientServerStream::Invoke
            << VTKOBJECT(this) << "Operate"
            << vtkClientServerStream::LastResult
            << vtkClientServerStream::End;
    this->ExecuteStream(stream);

    vtkClientServerStream result =
            this->GetLastResult(vtkProcessModule::DATA_SERVER_ROOT);

    bool valid = false;
    result.GetArgument(0,0,&valid);
    return valid;
    }
  return false;
}

//----------------------------------------------------------------------------
bool vtkSMArcOperatorProxy::Operate(vtkIdType arcId)
{
  this->UpdateVTKObjects();

  vtkClientServerStream stream;
  stream  << vtkClientServerStream::Invoke
    << VTKOBJECT(this) << "Operate"
    << arcId
    << vtkClientServerStream::End;

  this->ExecuteStream(stream);

  vtkClientServerStream result =
          this->GetLastResult(vtkProcessModule::DATA_SERVER_ROOT);

  bool valid = false;
  result.GetArgument(0,0,&valid);
  return valid;
}

//----------------------------------------------------------------------------
bool vtkSMArcOperatorProxy::Operate(vtkIdType firstArcId, vtkIdType secondArcId)
{
  this->UpdateVTKObjects();

  vtkClientServerStream stream;
  stream  << vtkClientServerStream::Invoke
    << VTKOBJECT(this) << "Operate"
    << firstArcId
    << secondArcId
    << vtkClientServerStream::End;

  this->ExecuteStream(stream);

  vtkClientServerStream result =
          this->GetLastResult(vtkProcessModule::DATA_SERVER_ROOT);

  bool valid = false;
  result.GetArgument(0,0,&valid);
  return valid;
}

//----------------------------------------------------------------------------
bool vtkSMArcOperatorProxy::Operate()
{
  this->UpdateVTKObjects();

  vtkClientServerStream stream;
  stream  << vtkClientServerStream::Invoke
    << VTKOBJECT(this) << "Operate"
    << vtkClientServerStream::End;

  this->ExecuteStream(stream);

  vtkClientServerStream result =
          this->GetLastResult(vtkProcessModule::DATA_SERVER_ROOT);

  bool valid = false;
  result.GetArgument(0,0,&valid);
  return valid;
}

//----------------------------------------------------------------------------
void vtkSMArcOperatorProxy::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

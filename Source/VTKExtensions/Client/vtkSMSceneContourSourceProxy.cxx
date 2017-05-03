//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkSMSceneContourSourceProxy.h"

#include "pqSMAdaptor.h"
#include "vtkClientServerMoveData.h"
#include "vtkClientServerStream.h"
#include "vtkContourWidget.h"
#include "vtkDoubleArray.h"
#include "vtkIdTypeArray.h"
#include "vtkObjectFactory.h"
#include "vtkOrientedGlyphContourRepresentation2.h"
#include "vtkPVContourRepresentationInfo.h"
#include "vtkPVXMLElement.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkProcessModule.h"
#include "vtkSMNewWidgetRepresentationProxy.h"
#include "vtkSceneContourSource.h"

vtkStandardNewMacro(vtkSMSceneContourSourceProxy);

vtkSMSceneContourSourceProxy::vtkSMSceneContourSourceProxy()
{
}

vtkSMSceneContourSourceProxy::~vtkSMSceneContourSourceProxy()
{
}

void vtkSMSceneContourSourceProxy::CopyData(vtkSMNewWidgetRepresentationProxy* widgetProxy)
{
  if (!widgetProxy)
  {
    return;
  }

  /*vtkProcessModule* pm = */ vtkProcessModule::GetProcessModule();
  vtkSMProxy* repProxy = widgetProxy->GetRepresentationProxy();
  vtkPVXMLElement* hints = repProxy->GetHints();

  if (repProxy && hints && hints->FindNestedElementByName("Output"))
  {
    //currently this filter only supports contour representations
    //the selection is now an int array on the poly data
    vtkClientServerStream stream;
    stream << vtkClientServerStream::Invoke << VTKOBJECT(repProxy)
           << "GetContourRepresentationAsPolyData" << vtkClientServerStream::End;
    this->ExecuteStream(stream);
    stream << vtkClientServerStream::Invoke << VTKOBJECT(this) << "CopyData"
           << this->GetLastResult(vtkProcessModule::DATA_SERVER_ROOT).GetArgument(0, 0)
           << vtkClientServerStream::End;
    this->ExecuteStream(stream);

    //push down the state of the closed loop flag
    stream << vtkClientServerStream::Invoke << VTKOBJECT(repProxy) << "GetClosedLoop"
           << vtkClientServerStream::End;
    this->ExecuteStream(stream);
    stream << vtkClientServerStream::Invoke << VTKOBJECT(this) << "SetClosedLoop"
           << this->GetLastResult(vtkProcessModule::DATA_SERVER_ROOT).GetArgument(0, 0)
           << vtkClientServerStream::End;
    this->ExecuteStream(stream);

    //now we need to push all the selected nodes over
    vtkPVContourRepresentationInfo* info = vtkPVContourRepresentationInfo::New();
    repProxy->GatherInformation(info);

    //delete the pv contour info
    info->Delete();

    this->MarkModified(this);
  }
}

void vtkSMSceneContourSourceProxy::EditData(
  vtkSMNewWidgetRepresentationProxy* widgetProxy, bool& closed)
{
  if (!widgetProxy)
  {
    return;
  }

  //if we are in built in mode get the object directly
  vtkSceneContourSource* contour = vtkSceneContourSource::SafeDownCast(this->GetClientSideObject());
  if (contour)
  {
    vtkContourWidget* widget = vtkContourWidget::SafeDownCast(widgetProxy->GetWidget());

    vtkOrientedGlyphContourRepresentation2* widgetRep =
      vtkOrientedGlyphContourRepresentation2::SafeDownCast(widget->GetRepresentation());

    widgetRep->SetLoggingEnabled(0);

    //reset the widget
    widget->SetEnabled(true);
    widgetRep->ClearAllNodes();
    widget->Initialize();

    //we need to reduce the point data that this object has
    //since it has the points for all objects
    contour->RegenerateEndNodes();

    vtkPolyData* tmp = contour->GetWidgetOutput();
    widget->Initialize(tmp, false);

    closed = static_cast<bool>(contour->GetClosedLoop());
    bool loopClosed = widgetRep->GetClosedLoop();
    if (loopClosed != closed)
    {
      if (closed)
      {
        widget->CloseLoop();
      }
      widgetRep->SetClosedLoop(closed);
    }

    //now set all the selected nodes that the widget had
    vtkIdTypeArray* selected =
      vtkIdTypeArray::SafeDownCast(tmp->GetPointData()->GetArray("selected"));

    for (vtkIdType i = 0; selected && i < selected->GetNumberOfTuples(); ++i)
    {
      if (selected->GetValue(i) == 1)
      {
        widgetRep->SetNthNodeSelected(i);
      }
    }
    tmp->Delete();

    widgetRep->SetLoggingEnabled(1);
  }
  else
  {
    //TODO:
    //we need to create a vtkMoveData to get the object to the client
    vtkPVContourRepresentationInfo* info = vtkPVContourRepresentationInfo::New();
    this->GatherInformation(info);

    //now all the information has been copied to the client from the server
    //we need to push it into the contour widget

    //this method is only vtkOrientedGlyphContourRepresentation2
    pqSMAdaptor::setElementProperty(widgetProxy->GetProperty("EnableLogging"), 0);
    widgetProxy->UpdateProperty("EnableLogging");

    //reset the widget
    widgetProxy->InvokeCommand("ClearAllNodes");
    widgetProxy->InvokeCommand("Initialize");
    widgetProxy->UpdateVTKObjects();

    vtkDoubleArray* nodePositions = info->GetAllNodesWorldPositions();
    if (nodePositions && nodePositions->GetNumberOfTuples() > 0)
    {
      QList<QVariant> values;
      double pointPos[3];
      for (vtkIdType i = 0; i < nodePositions->GetNumberOfTuples(); i++)
      {
        nodePositions->GetTuple(i, pointPos);
        values << pointPos[0] << pointPos[1] << pointPos[2];
      }
      pqSMAdaptor::setMultipleElementProperty(
        widgetProxy->GetRepresentationProxy()->GetProperty("NodePositions"), values);
      //we force push, since with all the clears, the server / client is out of sync
      widgetProxy->GetRepresentationProxy()->UpdateProperty("NodePositions", 1);
    }

    vtkIdTypeArray* selected = info->GetSelectedNodes();
    if (selected && selected->GetNumberOfTuples() > 0)
    {
      QList<QVariant> values;
      for (vtkIdType i = 0; i < selected->GetNumberOfTuples(); i++)
      {
        values << selected->GetValue(i);
      }
      pqSMAdaptor::setMultipleElementProperty(
        widgetProxy->GetRepresentationProxy()->GetProperty("SelectNodes"), values);
      //we force push, since with all the clears, the server / client is out of sync
      widgetProxy->GetRepresentationProxy()->UpdateProperty("SelectNodes", 1);
    }

    widgetProxy->UpdateVTKObjects();

    //update the closed variable with the loop state
    closed = static_cast<bool>(info->GetClosedLoop());
    vtkSMProxy* repProxy = widgetProxy->GetRepresentationProxy();
    repProxy->UpdatePropertyInformation();

    bool loopClosed =
      pqSMAdaptor::getElementProperty(repProxy->GetProperty("ClosedLoopInfo")).toBool();
    if (loopClosed != closed)
    {
      if (closed)
      {
        widgetProxy->InvokeCommand("CloseLoop");
      }
      pqSMAdaptor::setElementProperty(repProxy->GetProperty("ClosedLoop"), closed);
      repProxy->UpdateVTKObjects();
    }

    repProxy->MarkModified(repProxy);
    widgetProxy->MarkModified(widgetProxy);
    this->MarkModified(this);

    info->Delete();

    //this method is only vtkOrientedGlyphContourRepresentation2
    pqSMAdaptor::setElementProperty(widgetProxy->GetProperty("EnableLogging"), 1);
    widgetProxy->UpdateProperty("EnableLogging");
  }
}

void vtkSMSceneContourSourceProxy::ExtractContour(vtkSMSourceProxy* sourceProxy)
{
  if (!sourceProxy)
  {
    return;
  }

  /*vtkProcessModule* pm = */ vtkProcessModule::GetProcessModule();

  vtkClientServerStream stream;
  stream << vtkClientServerStream::Invoke << VTKOBJECT(sourceProxy) << "GetOutput"
         << vtkClientServerStream::End;

  this->ExecuteStream(stream);

  stream << vtkClientServerStream::Invoke << VTKOBJECT(this) << "CopyData"
         << this->GetLastResult(vtkProcessModule::DATA_SERVER_ROOT).GetArgument(0, 0)
         << vtkClientServerStream::End;

  this->ExecuteStream(stream);
}

void vtkSMSceneContourSourceProxy::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

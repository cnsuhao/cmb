//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "cmbForwardingSession.h"

#include "smtk/io/LoadJSON.h"
#include "smtk/io/SaveJSON.h"
#include "smtk/model/EntityRef.h"
#include "smtk/model/RemoteOperator.h"

#include "vtkSMModelManagerProxy.h"

#include "cJSON.h"

cmbForwardingSession::cmbForwardingSession()
{
  this->m_proxy = NULL;
  // Let's clear out the attribute system so there are no
  // pre-existing operation definitions - the operators
  // should come from what ever session we are actually using.
  this->m_operatorCollection = smtk::attribute::Collection::create();
}

cmbForwardingSession::~cmbForwardingSession()
{
  this->setProxy(NULL);
}

std::string cmbForwardingSession::defaultFileExtension(const smtk::model::Model& model) const
{
  cJSON* resp = this->m_proxy->requestJSONFileExtension(model, this->sessionId());
  cJSON* err = NULL;
  cJSON* res;

  if (!resp || (err = cJSON_GetObjectItem(resp, "error")) ||
    !(res = cJSON_GetObjectItem(resp, "result")) || (res->type != cJSON_String))
  {
    if (resp)
      cJSON_Delete(resp);
    if (err && err->valuestring && err->valuestring[0])
    {
      std::cerr << "Unable to determine default file extension for " << model.name() << ": \""
                << err->valuestring << "\"\n";
    }
    return "";
  }

  std::string defaultFileExtension = res->valuestring;
  cJSON_Delete(resp);
  return defaultFileExtension;
}

void cmbForwardingSession::setProxy(vtkSMModelManagerProxy* proxy)
{
  // Unregister old proxy
  if (this->m_proxy)
    this->m_proxy->UnRegister(NULL);

  this->m_proxy = proxy;

  // Register new proxy
  if (this->m_proxy)
    this->m_proxy->Register(NULL);
}

smtk::model::SessionInfoBits cmbForwardingSession::transcribeInternal(
  const smtk::model::EntityRef& entity, smtk::model::SessionInfoBits flags, int vtkNotUsed(depth))
{
  (void)entity;
  (void)flags;
  // TODO.
  return smtk::model::SESSION_NOTHING;
}

bool cmbForwardingSession::ableToOperateDelegate(smtk::model::RemoteOperatorPtr op)
{
  if (!op)
    return false;

  cJSON* resp = this->m_proxy->requestJSONOp(op, "operator-able", this->sessionId());
  cJSON* err = NULL;
  cJSON* res;

  if (!resp || (err = cJSON_GetObjectItem(resp, "error")) ||
    !(res = cJSON_GetObjectItem(resp, "result")) || (res->type != cJSON_True))
  {
    if (resp)
      cJSON_Delete(resp);
    if (err && err->valuestring && err->valuestring[0])
    {
      std::cerr << "Unable to operate: \"" << err->valuestring << "\"\n";
    }
    return false;
  }

  cJSON_Delete(resp);
  return true;
}

smtk::model::OperatorResult cmbForwardingSession::operateDelegate(smtk::model::RemoteOperatorPtr op)
{
  smtk::model::OperatorResult result;
  if (!op)
    return result;

  cJSON* resp = this->m_proxy->requestJSONOp(op, "operator-apply", this->sessionId());
  cJSON* err = NULL;
  cJSON* res;

  if (!resp || (err = cJSON_GetObjectItem(resp, "error")) ||
    !(res = cJSON_GetObjectItem(resp, "result")) ||
    !smtk::io::LoadJSON::ofOperatorResult(res, result, op))
  {
    if (resp)
      cJSON_Delete(resp);
    return op->createResult(smtk::operation::Operator::OPERATION_FAILED);
  }

  cJSON_Delete(resp);
  return result;
}

smtkImplementsModelingKernel(
  MODELBRIDGECLIENT_EXPORT, cmb_forwarding, "", smtk::model::SessionHasNoStaticSetup,
  cmbForwardingSession, false /* do not inherit "universal" operators */
  );

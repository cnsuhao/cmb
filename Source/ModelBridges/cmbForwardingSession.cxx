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

#include "smtk/model/EntityRef.h"
#include "smtk/io/ImportJSON.h"
#include "smtk/io/ExportJSON.h"
#include "smtk/model/RemoteOperator.h"

#include "vtkSMModelManagerProxy.h"

#include "cJSON.h"

cmbForwardingSession::cmbForwardingSession()
{
  this->m_proxy = NULL;
  // lets clear out the attribute so there are no
  // prexesting operation definitions - the operators
  // should come from what ever session we are actually using
  delete this->m_operatorSys;
  this->m_operatorSys = new smtk::attribute::System;
}

cmbForwardingSession::~cmbForwardingSession()
{
  this->setProxy(NULL);
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

bool cmbForwardingSession::ableToOperateDelegate(
  smtk::model::RemoteOperatorPtr op)
{
  if (!op)
    return false;

  cJSON* resp = this->m_proxy->requestJSONOp(op, "operator-able", this->sessionId());
  cJSON* err = NULL;
  cJSON* res;

  if (
    !resp ||
    (err = cJSON_GetObjectItem(resp, "error")) ||
    !(res = cJSON_GetObjectItem(resp, "result")) ||
    (res->type != cJSON_True))
    {
    if(resp)
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

smtk::model::OperatorResult cmbForwardingSession::operateDelegate(
  smtk::model::RemoteOperatorPtr op)
{
  smtk::model::OperatorResult result;
  if (!op)
    return result;

  cJSON* resp = this->m_proxy->requestJSONOp(op, "operator-apply", this->sessionId());
  cJSON* err = NULL;
  cJSON* res;

  if (
    !resp ||
    (err = cJSON_GetObjectItem(resp, "error")) ||
    !(res = cJSON_GetObjectItem(resp, "result")) ||
    !smtk::io::ImportJSON::ofOperatorResult(res, result, op))
    {
    if(resp)
      cJSON_Delete(resp);
    return op->createResult(smtk::model::OPERATION_FAILED);
    }

  cJSON_Delete(resp);
  return result;
}

smtkImplementsModelingKernel(
  MODELBRIDGECLIENT_EXPORT,
  cmb_forwarding,
  "",
  smtk::model::SessionHasNoStaticSetup,
  cmbForwardingSession,
  false /* do not inherit "universal" operators */
);

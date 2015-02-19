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
  const smtk::model::EntityRef& entity, smtk::model::SessionInfoBits flags)
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

  cJSON* req = cJSON_CreateObject();
  cJSON* par = cJSON_CreateObject();
  cJSON_AddItemToObject(req, "jsonrpc", cJSON_CreateString("2.0"));
  cJSON_AddItemToObject(req, "method", cJSON_CreateString("operator-able"));
  cJSON_AddItemToObject(req, "id", cJSON_CreateString("1")); // TODO
  cJSON_AddItemToObject(req, "params", par);
  op->ensureSpecification();
  smtk::io::ExportJSON::forOperator(op->specification(), par);
  // Add the session's session ID so it can be properly instantiated on the server.
  cJSON_AddItemToObject(par, "sessionId", cJSON_CreateString(this->sessionId().toString().c_str()));

  cJSON* resp = this->m_proxy->jsonRPCRequest(req); // This deletes req and par.
  cJSON* err = NULL;
  cJSON* res;

  if (
    !resp ||
    (err = cJSON_GetObjectItem(resp, "error")) ||
    !(res = cJSON_GetObjectItem(resp, "result")) ||
    (res->type != cJSON_True))
    {
    if (err && err->valuestring && err->valuestring[0])
      {
      std::cerr << "Unable to operate: \"" << err->valuestring << "\"\n";
      }
    return false;
    }

  return true;
}

smtk::model::OperatorResult cmbForwardingSession::operateDelegate(
  smtk::model::RemoteOperatorPtr op)
{
  cJSON* req = cJSON_CreateObject();
  cJSON* par = cJSON_CreateObject();
  cJSON_AddItemToObject(req, "jsonrpc", cJSON_CreateString("2.0"));
  cJSON_AddItemToObject(req, "method", cJSON_CreateString("operator-apply"));
  cJSON_AddItemToObject(req, "id", cJSON_CreateString("1")); // TODO
  cJSON_AddItemToObject(req, "params", par);
  op->ensureSpecification();
  smtk::io::ExportJSON::forOperator(op->specification(), par);
  // Add the session's session ID so it can be properly instantiated on the server.
  cJSON_AddItemToObject(par, "sessionId", cJSON_CreateString(this->sessionId().toString().c_str()));

  cJSON* resp = this->m_proxy->jsonRPCRequest(req); // This deletes req and par.
  cJSON* err = NULL;
  cJSON* res;
  smtk::model::OperatorResult result;

  if (
    !resp ||
    (err = cJSON_GetObjectItem(resp, "error")) ||
    !(res = cJSON_GetObjectItem(resp, "result")) ||
    !smtk::io::ImportJSON::ofOperatorResult(res, result, op))
    {
    return op->createResult(smtk::model::OPERATION_FAILED);
    }

  return result;
}

smtkImplementsModelingKernel(
  cmb_forwarding,
  "",
  smtk::model::SessionHasNoStaticSetup,
  cmbForwardingSession,
  false /* do not inherit "universal" operators */
);
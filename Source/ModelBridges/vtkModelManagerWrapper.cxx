#include "vtkModelManagerWrapper.h"

#include "smtk/io/ImportJSON.h"
#include "smtk/io/ExportJSON.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/IntItem.h"

#include "smtk/model/Operator.h"

#include "vtkObjectFactory.h"

#include "cJSON.h"

vtkStandardNewMacro(vtkModelManagerWrapper);

vtkModelManagerWrapper::vtkModelManagerWrapper()
{
  this->ModelMgr = smtk::model::Manager::create();
  this->JSONRequest = NULL;
  this->JSONResponse = NULL;
//  this->ModelEntityID = NULL;
}

vtkModelManagerWrapper::~vtkModelManagerWrapper()
{
  this->SetJSONRequest(NULL);
  this->SetJSONResponse(NULL);
//  this->SetModelEntityID(NULL);
}

void vtkModelManagerWrapper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "JSONRequest:" << this->JSONRequest << "\n";
//  os << indent << "ModelEntityID:" << this->ModelEntityID << "\n";
  os << indent << "JSONResponse:" << this->JSONResponse << "\n";
  os << indent << "ModelMgr:" << this->ModelMgr.get() << "\n";
}

/// Get the SMTK model being displayed.
smtk::model::ManagerPtr vtkModelManagerWrapper::GetModelManager()
{
  return this->ModelMgr;
}

/**\brief Evalate a JSON-RPC 2.0 request.
  *
  * This evaluates the request present in the JSONRequest member
  * and, if the request contains an "id" field, stores a response
  * in the JSONResponse member.
  */
void vtkModelManagerWrapper::ProcessJSONRequest()
{
  cJSON* result = cJSON_CreateObject();
  if (!this->JSONRequest || !this->JSONRequest[0])
    {
    this->GenerateError(result, "No request", "");
    }
  else
    {
    cJSON* req = cJSON_Parse(this->JSONRequest);
    cJSON* spec;
    cJSON* meth;
    if (
      !req ||
      req->type != cJSON_Object ||
      !(meth = cJSON_GetObjectItem(req, "method")) ||
      meth->type != cJSON_String ||
      !meth->valuestring ||
      !(spec = cJSON_GetObjectItem(req, "jsonrpc")) ||
      spec->type != cJSON_String ||
      !spec->valuestring)
      {
      this->GenerateError(
        result, "Malformed request; not an object or missing jsonrpc or method members.", "");
      }
    else
      {
      std::string methStr(meth->valuestring);
      std::string specStr(spec->valuestring);
      std::string reqIdStr;
      cJSON* reqId = cJSON_GetObjectItem(req, "id");
      cJSON* param = cJSON_GetObjectItem(req, "params");
      bool missingIdFatal = true;
      if (reqId && reqId->type == cJSON_String)
        {
        reqIdStr = reqId->valuestring;
        missingIdFatal = false;
        }

      // I. Requests:
      //   search-session-types (available)
      //   list-sessions (instantiated)
      //   create-session
      //   fetch-model
      //   operator-able
      //   operator-apply
      if (methStr == "search-session-types")
        {
        smtk::model::StringList sessionTypeNames = this->ModelMgr->sessionTypeNames();
        cJSON_AddItemToObject(result, "result",
          smtk::io::ExportJSON::createStringArray(sessionTypeNames));
        }
      else if (methStr == "session-filetypes")
        {
        cJSON* bname;
        if (
          !param ||
          !(bname = cJSON_GetObjectItem(param, "session-name")) ||
          bname->type != cJSON_String ||
          !bname->valuestring ||
          !bname->valuestring[0])
          {
          this->GenerateError(result, "Parameters not passed or session-name not specified.", reqIdStr);
          }
        else
          {
          cJSON* typeObj = cJSON_CreateObject();
          smtk::model::StringData sessionFileTypes =
            this->ModelMgr->sessionFileTypes(bname->valuestring);
          for(smtk::model::PropertyNameWithStrings it = sessionFileTypes.begin();
              it != sessionFileTypes.end(); ++it)
            {
            if(it->second.size())
              cJSON_AddItemToObject(typeObj, it->first.c_str(),
                smtk::io::ExportJSON::createStringArray(it->second));
            }
          cJSON_AddItemToObject(result, "result", typeObj);
          }
        }
      else if (methStr == "create-session")
        {
        smtk::model::StringList sessionTypeNames = this->ModelMgr->sessionTypeNames();
        std::set<std::string> sessionSet(sessionTypeNames.begin(), sessionTypeNames.end());
        cJSON* bname;
        if (
          !param ||
          !(bname = cJSON_GetObjectItem(param, "session-name")) ||
          bname->type != cJSON_String ||
          !bname->valuestring ||
          !bname->valuestring[0] ||
          sessionSet.find(bname->valuestring) == sessionSet.end())
          {
          this->GenerateError(result,
            "Parameters not passed or session-name not specified/invalid.",
            reqIdStr);
          }
        else
          {
          smtk::model::SessionRef sref = this->ModelMgr->createSession(bname->valuestring);
          if (!sref.isValid() || !sref.session())
            {
            this->GenerateError(result,
              "Unable to construct session or got NULL session ID.", reqIdStr);
            }
          else
            {
            sref.assignDefaultName();
            cJSON* sess = cJSON_CreateObject();
            smtk::io::ExportJSON::forManagerSession(
              sref.entity(), sess, this->ModelMgr);
            cJSON_AddItemToObject(result, "result", sess);
            //cJSON_AddItemToObject(result, "result",
            //  cJSON_CreateString(session->sessionId().toString().c_str()));
            }
          }
        }
      else if (methStr == "fetch-model")
        {
        cJSON* model = cJSON_CreateObject();
        // Never include session list or tessellation data
        // Until someone makes us.
        smtk::io::ExportJSON::fromModelManager(model, this->ModelMgr,
          static_cast<smtk::io::JSONFlags>(
            smtk::io::JSON_ENTITIES | smtk::io::JSON_PROPERTIES));
        cJSON_AddItemToObject(result, "result", model);
        }
      else if (methStr == "operator-able")
        {
        smtk::model::OperatorPtr localOp;
        if (
          !param ||
          !smtk::io::ImportJSON::ofOperator(param, localOp, this->ModelMgr) ||
          !localOp)
          {
          this->GenerateError(result,
            "Parameters not passed or invalid operator specified.",
            reqIdStr);
          }
        else
          {
          bool able = localOp->ableToOperate();
          cJSON_AddItemToObject(result, "result", cJSON_CreateBool(able ? 1 : 0));
          }
        }
      else if (methStr == "operator-apply")
        {
        smtk::model::OperatorPtr localOp;
        if (
          !param ||
          !smtk::io::ImportJSON::ofOperator(param, localOp, this->ModelMgr) ||
          !localOp)
          {
          this->GenerateError(result,
            "Parameters not passed or invalid operator specified.",
            reqIdStr);
          }
        else
          {
          smtk::attribute::IntItem::Ptr ani = localOp->findInt("assign names");
          ani->setIsEnabled(true);
          ani->setValue(1);
          smtk::model::OperatorResult ores = localOp->operate();
          cJSON* oresult = cJSON_CreateObject();
          smtk::io::ExportJSON::forOperatorResult(ores, oresult);
          cJSON_AddItemToObject(result, "result", oresult);
          }
        }
      // II. Notifications:
      //   delete session
      else if (methStr == "delete-session")
        {
        missingIdFatal &= false; // Notifications do not require an "id" member in the request.

        cJSON* bsess;
        if (
          !param ||
          !(bsess = cJSON_GetObjectItem(param, "session-id")) ||
          bsess->type != cJSON_String ||
          !bsess->valuestring ||
          !bsess->valuestring[0])
          {
          this->GenerateError(result, "Parameters not passed or session-id not specified/invalid.", reqIdStr);
          }
        else
          {
          smtk::model::SessionRef sref(
            this->ModelMgr,
            smtk::common::UUID(bsess->valuestring));
          if (!sref.isValid() || !sref.session())
            {
            this->GenerateError(result, "No session with given session ID.", reqIdStr);
            }
          else
            {
            sref.close();
            }
          }
        }
      if (missingIdFatal)
        {
        this->GenerateError(result, "Method was a request but is missing \"id\".", reqIdStr);
        }
      }
    }
  char* response = cJSON_Print(result);
  cJSON_Delete(result);
  this->SetJSONResponse(response);
  free(response);
}


/**\brief Deserializes a JSON operator description and executes ableToOperate.
  *
  * The return value is a JSON serialization of the OperatorOutcome.
  */
std::string vtkModelManagerWrapper::CanOperatorExecute(const std::string& jsonOperator)
{
  return "{\"outcome\": \"0\"}";
}

/**\brief Deserializes a JSON operator description and executes operate().
  *
  * The return value is a JSON serialization of the OperatorOutcome.
  */
std::string vtkModelManagerWrapper::ApplyOperator(const std::string& jsonOperator)
{
  return "{\"outcome\": \"0\"}";
}

void vtkModelManagerWrapper::GenerateError(cJSON* err, const std::string& errMsg, const std::string& reqId)
{
  cJSON_AddItemToObject(err, "error", cJSON_CreateString(errMsg.c_str()));
  cJSON_AddItemToObject(err, "id", cJSON_CreateString(reqId.c_str()));
}

//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "vtkSMModelManagerProxy.h"

#include "smtk/attribute/Attribute.h"
#include "smtk/attribute/FileItem.h"
#include "smtk/attribute/IntItem.h"
#include "smtk/attribute/StringItem.h"

#include "smtk/model/Manager.h"
#include "smtk/model/Model.h"
#include "smtk/model/RemoteOperator.h"
#include "smtk/io/ImportJSON.h"
#include "smtk/io/ExportJSON.h"

#include "cJSON.h"

#include "vtkObjectFactory.h"
#include "vtkClientServerStream.h"
#include "vtkSMPropertyHelper.h"
#include "vtkSMSession.h"
#include "cmbForwardingSession.h"
#include <vtksys/SystemTools.hxx>

using smtk::common::UUID;
using namespace smtk::common;
using namespace smtk::model;
using namespace smtk::io;

#if defined(_MSC_VER)
#pragma warning(disable:4503)
#endif

vtkStandardNewMacro(vtkSMModelManagerProxy);

vtkSMModelManagerProxy::vtkSMModelManagerProxy()
{
  // This model will be mirrored (topology-only) from the server
  this->m_modelMgr = smtk::model::Manager::create();
  this->m_serverSession = NULL;
}

vtkSMModelManagerProxy::~vtkSMModelManagerProxy()
{
  this->endSessions();
}

/// Print the state of this instance.
void vtkSMModelManagerProxy::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "ModelMgr: " << this->m_modelMgr.get() << "\n"; // m_modelMgr
  os << indent << "ServerSession: " << this->m_serverSession << "\n"; // m_serverSession
}

/// Return the list of session types available on the server (not the local modelManager()'s list).
std::vector<std::string> vtkSMModelManagerProxy::sessionNames(bool forceFetch)
{
  std::vector<std::string> resultVec;
  if (this->m_remoteSessionNames.empty() || forceFetch)
    {
    this->m_remoteSessionNames.clear();
    // Do not report our session names. Report those available on the server.
    std::string reqStr = "{\"jsonrpc\":\"2.0\", \"method\":\"search-session-types\", \"id\":\"1\"}";
    cJSON* result = this->jsonRPCRequest(reqStr);
    cJSON* sarr;
    if (
      !result ||
      result->type != cJSON_Object ||
      !(sarr = cJSON_GetObjectItem(result, "result")) ||
      sarr->type != cJSON_Array)
      {
      // TODO: See if result has "error" key and report it.
      if (result)
        cJSON_Delete(result);
      return std::vector<std::string>();
      }

    smtk::io::ImportJSON::getStringArrayFromJSON(sarr, resultVec);
    cJSON_Delete(result);
    this->m_remoteSessionNames = std::set<std::string>(
      resultVec .begin(), resultVec.end());
    }
  else
    {
    resultVec = std::vector<std::string>(
      this->m_remoteSessionNames.begin(), this->m_remoteSessionNames.end());
    }

  return resultVec;
}

bool vtkSMModelManagerProxy::validSession(
  const smtk::common::UUID& sessionId)
{
  return m_remoteSessionIds.find(sessionId) != m_remoteSessionIds.end();
}

smtk::common::UUID vtkSMModelManagerProxy::beginSession(
  const std::string& sessionName,
  const smtk::common::UUID& newSessionId,
  bool createNew)
{
  if (this->m_remoteSessionNames.find(sessionName) == this->m_remoteSessionNames.end())
    {
    return smtk::common::UUID::null();
    }

  if(!createNew)
    {
    // if there is already a session created, use that session.
    std::map<smtk::common::UUID,std::string>::iterator it;
    for (
      it = this->m_remoteSessionIds.begin();
      it != this->m_remoteSessionIds.end();
      ++it)
      {
      if(( !newSessionId.isNull() && newSessionId == it->first ) ||
         ( newSessionId.isNull() && it->second == sessionName ))
        return it->first;
      }
    }

  std::string sessionParams = sessionName;
  if(!newSessionId.isNull())
    sessionParams += ("\", \"session-id\":\"" + newSessionId.toString());
  //FIXME: Sanitize sessionName!
  std::string reqStr =
    "{\"jsonrpc\":\"2.0\", \"method\":\"create-session\", \"params\":{\"session-name\":\"" +
    sessionParams + "\"}, \"id\":\"1\"}";
  cJSON* result = this->jsonRPCRequest(reqStr);
  cJSON* sessionObj;
  cJSON* sessionIdObj;
  cJSON* opsObj;
  cJSON* nameObj;
  if (
    // Was JSON parsable?
    !result ||
    // Is the result an Object (as required by JSON-RPC 2.0)?
    result->type != cJSON_Object ||
    // Does the result Object have a field named "result" (req'd by JSON-RPC)?
    !(sessionObj = cJSON_GetObjectItem(result, "result")) ||
    // Is the "result" field an Object with a child that is also an object?
    sessionObj->type != cJSON_Object ||
    !(sessionIdObj = sessionObj->child) ||
    sessionIdObj->type != cJSON_Object ||
    // Does the first child have a valid name? (This is the session ID)
    !sessionIdObj->string ||
    !sessionIdObj->string[0] ||
    // Does the first child have fields "name" and "ops" of type String and Array?
    !(nameObj = cJSON_GetObjectItem(sessionIdObj, "name")) ||
    nameObj->type != cJSON_String ||
    !nameObj->valuestring ||
    !nameObj->valuestring[0] ||
    !(opsObj = cJSON_GetObjectItem(sessionIdObj, "ops")) ||
    opsObj->type != cJSON_String ||
    !opsObj->valuestring ||
    !opsObj->valuestring[0]
    )
      {
      // TODO: See if result has "error" key and report it.
      if (result)
        cJSON_Delete(result);
      return smtk::common::UUID::null();
      }

  // OK, construct a special "forwarding" session locally.
  cmbForwardingSession::Ptr session = cmbForwardingSession::create();
  session->setProxy(this);
  // The ImportJSON registers this session with the model manager.
  if (ImportJSON::ofRemoteSession(sessionIdObj, session, this->m_modelMgr))
    {
    // Failure.
    }
  //this->m_modelMgr->registerSession(session);

  cJSON_Delete(result);

  UUID sessionId = session->sessionId();
  this->m_remoteSessionIds[sessionId] = sessionName;
  return sessionId;
}

bool vtkSMModelManagerProxy::endSession(const smtk::common::UUID& sessionId)
{
  std::map<smtk::common::UUID,std::string>::iterator it =
    this->m_remoteSessionIds.find(sessionId);
  if (it == this->m_remoteSessionIds.end())
    return false;

  // Unhook our local cmbForwardingSession representing the remote.
  smtk::model::SessionRef sref(this->m_modelMgr, sessionId);
  sref.close();

  // Tell the server to unregister this session.
  // (Since the server's model manager should hold the only shared pointer
  // to the session, this should kill it.)
  std::string note =
    "{\"jsonrpc\":\"2.0\", \"method\":\"delete-session\", \"params\":{\"session-id\":\"" +
    sessionId.toString() + "\"}}";
  this->jsonRPCNotification(note);

  // Now remove the entry from the proxy's list of sessions.
  this->m_remoteSessionIds.erase(it);

  return true;
}

smtk::model::StringData vtkSMModelManagerProxy::supportedFileTypes(
  const std::string& sessionName)
{
  if(this->m_sessionFileTypes.find(sessionName) != this->m_sessionFileTypes.end())
    return this->m_sessionFileTypes[sessionName];

  smtk::model::StringData retFileTypes;
  smtk::model::StringList bnames;

  if(sessionName.empty())
    {
    // no session name is given, fetch all available session
    bnames = this->sessionNames();
    }
  else
    {
    bnames.push_back(sessionName);
    }

  for (smtk::model::StringList::iterator it = bnames.begin(); it != bnames.end(); ++it)
    {
    std::cout << "Find Session      " << *it << "\n";
    std::string reqStr =
      "{\"jsonrpc\":\"2.0\", \"method\":\"session-filetypes\", "
      "\"params\":{ \"session-name\":\"" + *it + "\"}, "
      "\"id\":\"1\"}";

    cJSON* resObj;
    cJSON* result = this->jsonRPCRequest(reqStr);
//    std::cout << "result File types: " << cJSON_Print(result) <<std::endl;
    if (
      !result ||
      result->type != cJSON_Object ||
      !(resObj = cJSON_GetObjectItem(result, "result")) ||
      resObj->type != cJSON_Object)
      {
      // TODO: See if result has "error" key and report it.
      if (result)
        cJSON_Delete(result);
      continue;
      }

    smtk::model::StringData brFileTypes;
    for (cJSON* sarr = resObj->child; sarr; sarr = sarr->next)
      {
      if (sarr->type == cJSON_Array)
        {
        smtk::model::StringList bftypes;
        smtk::io::ImportJSON::getStringArrayFromJSON(sarr, bftypes);
        retFileTypes[sarr->string].insert(
          retFileTypes[sarr->string].end(), bftypes.begin(), bftypes.end());
        brFileTypes[sarr->string].insert(
          brFileTypes[sarr->string].end(), bftypes.begin(), bftypes.end());
        }
      }

    if(brFileTypes.size())
      {
      this->m_sessionFileTypes[*it] = brFileTypes;
      }

    if (result)
      cJSON_Delete(result);

    }

  return retFileTypes;
}

void vtkSMModelManagerProxy::initFileOperator(
  smtk::model::OperatorPtr fileOp,
  const std::string& fileName,
  const std::string& engineName)
{
  if(!fileOp->ensureSpecification())
    return;
  fileOp->specification()->findFile("filename")->setValue(fileName);
  smtk::attribute::StringItem::Ptr enginetypeItem =
    fileOp->specification()->findString("enginetype");
  if (enginetypeItem)
    {
    enginetypeItem->setValue(engineName);
    }
}

smtk::model::OperatorPtr vtkSMModelManagerProxy::newFileOperator(
  const std::string& fileName,
  smtk::model::SessionPtr session,
  const std::string& engineName)
{
  std::string lastExt =
    vtksys::SystemTools::GetFilenameLastExtension(fileName);
  if( lastExt == ".smtk" )
    {
    OperatorPtr smtkImportOp = session->op("import smtk model");
    // Not all session should have an ImportOperator
    if (smtkImportOp)
      {
      this->initFileOperator(smtkImportOp, fileName, engineName);
      return smtkImportOp;
      }
    }

  OperatorPtr readOp = session->op("read");
  // Assuming all session should have a ReadOperator
  if (!readOp)
    {
    std::cerr
      << "Could not create read operator for session"
      << " \"" << session->name() << "\""
      << " (" << session->sessionId() << ")\n";
    return smtk::model::OperatorPtr();
    }

  this->initFileOperator(readOp, fileName, engineName);
  if (readOp->specification() && readOp->ableToOperate() )
    {
    return readOp;
    }
  else
    {
    std::cout << "No specs for Read Op or Read Op can not operate with the file: "
              << fileName.c_str() << "\n";
    }
  // try "import" if there is one
  OperatorPtr importOp = session->op("import");
  // Not all session should have an ImportOperator
  if (importOp)
    {
    this->initFileOperator(importOp, fileName, engineName);
    return importOp;
    }
  else
    {
    std::cout
      << "Could not create import operator for session"
      << " \"" << session->name() << "\""
      << " (" << session->sessionId() << ")\n";
    }

  return smtk::model::OperatorPtr();
}

smtk::model::OperatorPtr vtkSMModelManagerProxy::newFileOperator(
  const std::string& fileName,
  const std::string& sessionName,
  const std::string& engineName)
{
  std::string actualSessionName = sessionName, actualEngineName = engineName;
  if (sessionName.empty())
    {
    std::set<std::string>::const_iterator bnit;
    for (
      bnit = this->m_remoteSessionNames.begin();
      bnit != this->m_remoteSessionNames.end() && actualSessionName.empty();
      ++bnit)
      {
      smtk::model::StringData fileTypesForSession = this->supportedFileTypes(*bnit);
      smtk::model::PropertyNameWithStrings typeIt;
      for(typeIt = fileTypesForSession.begin(); typeIt != fileTypesForSession.end(); ++typeIt)
        {
        std::vector<std::string>::const_iterator fit;
        for (fit = typeIt->second.begin(); fit != typeIt->second.end(); ++fit)
          {
          std::string::size_type fEnd;
          std::string::size_type eEnd = (*fit).find(' ');
          std::string ext(*fit, 0, eEnd);
          std::cout << "Looking for \"" << ext << "\"\n";
          if ((fEnd = fileName.rfind(ext)) && (fileName.size() - fEnd == eEnd))
            { // matching substring is indeed at end of fileName
            actualSessionName = *bnit;
            actualEngineName = typeIt->first;
            std::cout << "Found session type " << actualSessionName << " for " << fileName << "\n";
            break;
            }
          }
        }
      }
    }

  std::map<smtk::common::UUID,std::string>::iterator it;
  SessionPtr session;
  for (
    it = this->m_remoteSessionIds.begin();
    it != this->m_remoteSessionIds.end();
    ++it)
    {
    SessionPtr tsession = SessionRef(this->m_modelMgr, it->first).session();
    //if (tsession && tsession->name() == actualSessionName)
    if (tsession && it->second == actualSessionName)
      {
      session = tsession;
//      std::cout << "Found session " << session->sessionId() << " (" << actualSessionName << ")\n";
      break;
      }
    }
  if (!session)
    { // No existing session of that type. Create a new remote session.
    smtk::common::UUID sessionId = this->beginSession(actualSessionName);
//    std::cout << "started sessionID: " << sessionId.toString() <<std::endl;

    session = SessionRef(this->m_modelMgr, sessionId).session();
    }
  if (!session)
    {
    std::cerr << "Could not find or create session of type \"" << actualSessionName << "\"\n";
    return OperatorPtr();
    }

//  std::string ext = vtksys::SystemTools::GetFilenameLastExtension(fileName);
//  std::string opName = (ext == ".vtk" || ext == ".exo") ? "import" : "read";
  OperatorPtr fileOp = this->newFileOperator(fileName, session, actualEngineName);
  return fileOp;
}

smtk::model::OperatorPtr vtkSMModelManagerProxy::smtkFileOperator(
  const std::string& fileName)
{
  std::string lastExt =
    vtksys::SystemTools::GetFilenameLastExtension(fileName);
  if( lastExt != ".smtk")
    return OperatorPtr();

  // This is a json/smtk model file, we will look for native model file,
  // and return the session type that can read that file.
  std::ifstream file(fileName.c_str());
  if (!file.good())
    return OperatorPtr();

  std::string data(
    (std::istreambuf_iterator<char>(file)),
    (std::istreambuf_iterator<char>()));

  if (data.empty())
    return OperatorPtr();

  cJSON* root = cJSON_Parse(data.c_str());
  if(!root)
    return OperatorPtr();
  cJSON* sessNode = root->child;
  cJSON* typeObj;
  cJSON* nameObj;
  if (
    !sessNode ||
    sessNode->type != cJSON_Object ||
    // Does the sessNode have a valid session ID?
    !sessNode->string ||
    !sessNode->string[0] ||
    // Does the node have fields "type" and "name" of type String?
    !(typeObj = cJSON_GetObjectItem(sessNode, "type")) ||
    typeObj->type != cJSON_String ||
    !typeObj->valuestring ||
    !typeObj->valuestring[0] ||
    strcmp(typeObj->valuestring, "session") || // must be session type
    !(nameObj = cJSON_GetObjectItem(sessNode, "name")) ||
    nameObj->type != cJSON_String ||
    !nameObj->valuestring ||
    !nameObj->valuestring[0]
    )
    return OperatorPtr();

  smtk::common::UUID fileSessionId(sessNode->string);
  smtk::common::UUID sessionId = this->beginSession(nameObj->valuestring,
                                                    fileSessionId);
  SessionPtr session = SessionRef(this->m_modelMgr, sessionId).session();
  if (!session)
    {
    std::cerr << "Could not find or create session of type \"" << nameObj->valuestring << "\"\n";
    return OperatorPtr();
    }

  OperatorPtr fileOp = this->newFileOperator(fileName, session, std::string());
  return fileOp;
}

std::vector<std::string> vtkSMModelManagerProxy::operatorNames(const smtk::common::UUID& sessionId)
{
  (void)sessionId;
  std::vector<std::string> result;
  return result;
}

smtk::model::ManagerPtr vtkSMModelManagerProxy::modelManager()
{
  return this->m_modelMgr;
}

cJSON* vtkSMModelManagerProxy::requestJSONOp(
  smtk::model::RemoteOperatorPtr op,
  const std::string& strMethod,
  const smtk::common::UUID& fwdSessionId)
{
  if (!op)
    return NULL;

  cJSON* req = cJSON_CreateObject();
  cJSON* par = cJSON_CreateObject();
  cJSON_AddItemToObject(req, "jsonrpc", cJSON_CreateString("2.0"));
  cJSON_AddItemToObject(req, "method", cJSON_CreateString(strMethod.c_str()));
  cJSON_AddItemToObject(req, "id", cJSON_CreateString("1")); // TODO
  cJSON_AddItemToObject(req, "params", par);
  op->ensureSpecification();
  smtk::io::ExportJSON::forOperator(op->specification(), par);
  // Add the session's session ID so it can be properly instantiated on the server.
  cJSON_AddItemToObject(par, "sessionId", cJSON_CreateString(fwdSessionId.toString().c_str()));

  vtkSMProxy* opHelperProxy = NULL;
  smtk::attribute::IntItem::Ptr opProxyIdItem =
    op->specification()->findInt("HelperGlobalID");
  if(opProxyIdItem && opProxyIdItem->value() > 0)
    {
    opHelperProxy = vtkSMProxy::SafeDownCast(
      this->GetSession()->GetRemoteObject(opProxyIdItem->value()));
    }

  cJSON* resp = this->jsonRPCRequest(req, opHelperProxy);
  if(req)
    cJSON_Delete(req);
  return resp;
}

cJSON* vtkSMModelManagerProxy::jsonRPCRequest(cJSON* req, vtkSMProxy* opHelperProxy)
{
  char* reqStr = cJSON_Print(req);
  cJSON* response = this->jsonRPCRequest(reqStr, opHelperProxy);
  free(reqStr);
  return response;
}

cJSON* vtkSMModelManagerProxy::jsonRPCRequest(const std::string& req, vtkSMProxy* opHelperProxy)
{
  if(req.empty())
    return NULL;

  this->jsonRPCNotification(req, opHelperProxy);

  // Now, unlike notifications, we expect a response.
  // Get the json response string and parse it:
  this->UpdatePropertyInformation();
  std::string responseStr = vtkSMPropertyHelper(this, "JSONResponse").GetAsString();
  cJSON* response = cJSON_Parse(responseStr.c_str());
  return response;
}

void vtkSMModelManagerProxy::jsonRPCNotification(cJSON* note, vtkSMProxy* opHelperProxy)
{
  char* noteStr = cJSON_Print(note);
  cJSON_Delete(note);
  this->jsonRPCNotification(noteStr, opHelperProxy);
  free(noteStr);
}

void vtkSMModelManagerProxy::jsonRPCNotification(const std::string& note, vtkSMProxy* opHelperProxy)
{
  if(note.empty())
    return;

  // Check if there is a geometryHelper(for example vtkSMTKOperator) proxy in "params",
  // if yes, pass that to the server too.
  vtkSMPropertyHelper(this, "JSONRequest").Set(note.c_str());
  this->UpdateVTKObjects();
  vtkClientServerStream stream;
  // calls "ProcessJSONRequest" function on object this->GetId() (which gets turned
  // into a pointer on the server)
  if(opHelperProxy)
    {
    stream  << vtkClientServerStream::Invoke
            << VTKOBJECT(this)
            << "ProcessJSONRequest"
            << VTKOBJECT(opHelperProxy)
            << vtkClientServerStream::End;
    }
  else
    {
    stream  << vtkClientServerStream::Invoke
            << VTKOBJECT(this)
            << "ProcessJSONRequest"
            << vtkClientServerStream::End;
    }
  this->ExecuteStream(stream);
}

void vtkSMModelManagerProxy::fetchWholeModel()
{
  cJSON* response = this->jsonRPCRequest(
    "{\"jsonrpc\":\"2.0\", \"id\":\"1\", \"method\":\"fetch-model\"}");
  cJSON* model;
  cJSON* topo;
//  std::cout << " ----- \n\n\n" << cJSON_Print(response) << "\n ----- \n\n\n";
  if (
    response &&
    (model = cJSON_GetObjectItem(response, "result")) &&
    model->type == cJSON_Object &&
    (topo = cJSON_GetObjectItem(model, "topo")))
    ImportJSON::ofManager(topo, this->m_modelMgr);
  cJSON_Delete(response);
}

void vtkSMModelManagerProxy::endSessions()
{
  while (!this->m_remoteSessionIds.empty())
    this->endSession(
      this->m_remoteSessionIds.begin()->first);
}

void vtkSMModelManagerProxy::connectProxyToManager(vtkSMProxy* sourceProxy)
{
  if(!sourceProxy)
    return;

  vtkClientServerStream stream;
  stream  << vtkClientServerStream::Invoke
          << VTKOBJECT(this)
          << "SetModelManagerToSource"
          << VTKOBJECT(sourceProxy)
          << vtkClientServerStream::End;
  this->ExecuteStream(stream);
}

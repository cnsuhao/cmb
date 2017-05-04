//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __vtkSMModelManagerProxy_h
#define __vtkSMModelManagerProxy_h

#include "ModelBridgeClientModule.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/model/StringData.h"
#include "vtkSMProxy.h"

struct cJSON;

class MODELBRIDGECLIENT_EXPORT vtkSMModelManagerProxy : public vtkSMProxy
{
public:
  static vtkSMModelManagerProxy* New();
  vtkTypeMacro(vtkSMModelManagerProxy, vtkSMProxy);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  std::vector<std::string> sessionNames(bool forceFetch = false);

  smtk::common::UUID beginSession(const std::string& sessionName,
    const smtk::common::UUID& sessionId = smtk::common::UUID::null(), bool createNew = false);
  bool endSession(const smtk::common::UUID& sessionId);

  smtk::model::StringData supportedFileTypes(const std::string& sessionName = std::string());
  smtk::model::OperatorPtr newFileOperator(const std::string& fileName,
    const std::string& sessionName = std::string(), const std::string& engineName = std::string());
  std::vector<std::string> fileOperators(const std::string& sessionName);
  smtk::model::OperatorPtr smtkFileOperator(const std::string& fileName);

  std::vector<std::string> operatorNames(const std::string& sessionName);
  std::vector<std::string> operatorNames(const smtk::common::UUID& sessionId);

  void fetchWholeModel();

  smtk::model::ManagerPtr modelManager();
  void endSessions();
  bool validSession(const smtk::common::UUID& sessionId);
  int numberOfRemoteSessions() const { return static_cast<int>(this->m_remoteSessionIds.size()); }

  void connectProxyToManager(vtkSMProxy* sourceProxy);

protected:
  friend class cmbForwardingSession;

  vtkSMModelManagerProxy();
  ~vtkSMModelManagerProxy() override;

  cJSON* requestJSONOp(smtk::model::RemoteOperatorPtr op, const std::string& strMethod,
    const smtk::common::UUID& fwdSessionId);
  cJSON* jsonRPCRequest(cJSON* req, vtkSMProxy* opHelperProxy = NULL);
  cJSON* jsonRPCRequest(const std::string& req, vtkSMProxy* opHelperProxy = NULL);
  void jsonRPCNotification(cJSON* note, vtkSMProxy* opHelperProxy = NULL);
  void jsonRPCNotification(const std::string& note, vtkSMProxy* opHelperProxy = NULL);

  void initFileOperator(
    smtk::model::OperatorPtr fileOp, const std::string& fileName, const std::string& engineName);
  smtk::model::OperatorPtr newFileOperator(
    const std::string& fileName, smtk::model::SessionPtr session, const std::string& engineName);

  smtk::model::ManagerPtr m_modelMgr;
  vtkSMProxy* m_serverSession;
  std::set<std::string> m_remoteSessionNames;
  std::map<smtk::common::UUID, std::string> m_remoteSessionIds;
  /// map for session file types
  /// <sessionName, <engine-name, fileTypesList> >
  std::map<std::string, smtk::model::StringData> m_sessionFileTypes;

private:
  vtkSMModelManagerProxy(const vtkSMModelManagerProxy&); // Not implemented.
  void operator=(const vtkSMModelManagerProxy&);         // Not implemented.
};

#endif // __vtkSMModelManagerProxy_h

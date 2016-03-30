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
#include "vtkSMProxy.h"
#include "smtk/PublicPointerDefs.h"
#include "smtk/model/StringData.h"

struct cJSON;

class MODELBRIDGECLIENT_EXPORT vtkSMModelManagerProxy : public vtkSMProxy
{
public:
  static vtkSMModelManagerProxy* New();
  vtkTypeMacro(vtkSMModelManagerProxy,vtkSMProxy);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  std::vector<std::string> sessionNames(bool forceFetch = false);

  smtk::common::UUID beginSession(
    const std::string& sessionName,
    const smtk::common::UUID& sessionId = smtk::common::UUID::null(),
    bool createNew = false);
  bool endSession(const smtk::common::UUID& sessionId);

  smtk::model::StringData supportedFileTypes(
    const std::string& sessionName = std::string());
  smtk::model::OperatorPtr newFileOperator(
    const std::string& fileName,
    const std::string& sessionName = std::string(),
    const std::string& engineName = std::string());
  std::vector<std::string> fileOperators(
    const std::string& sessionName);
  smtk::model::OperatorPtr smtkFileOperator(
  const std::string& fileName);

  std::vector<std::string> operatorNames(const std::string& sessionName);
  std::vector<std::string> operatorNames(const smtk::common::UUID& sessionId);

  void fetchWholeModel();

  smtk::model::ManagerPtr modelManager();
  void endSessions();
  bool validSession(const smtk::common::UUID& sessionId);

protected:
  friend class cmbForwardingSession;

  vtkSMModelManagerProxy();
  virtual ~vtkSMModelManagerProxy();

  cJSON* jsonRPCRequest(cJSON* req);
  cJSON* jsonRPCRequest(const std::string& req);
  void jsonRPCNotification(cJSON* note);
  void jsonRPCNotification(const std::string& note);

  void initFileOperator(
    smtk::model::OperatorPtr fileOp,
    const std::string& fileName,
    const std::string& engineName);
  smtk::model::OperatorPtr newFileOperator(
    const std::string& fileName,
    smtk::model::SessionPtr session,
    const std::string& engineName);

  smtk::model::ManagerPtr m_modelMgr;
  vtkSMProxy* m_serverSession;
  std::set<std::string> m_remoteSessionNames;
  std::map<smtk::common::UUID,std::string> m_remoteSessionIds;
  /// map for session file types
  /// <sessionName, <engine-name, fileTypesList> >
  std::map<std::string, smtk::model::StringData> m_sessionFileTypes;

private:
  vtkSMModelManagerProxy(const vtkSMModelManagerProxy&); // Not implemented.
  void operator = (const vtkSMModelManagerProxy&); // Not implemented.
};

#endif // __vtkSMModelManagerProxy_h

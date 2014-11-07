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

  std::vector<std::string> bridgeNames(bool forceFetch = false);

  smtk::common::UUID beginBridgeSession(const std::string& bridgeName);
  bool endBridgeSession(const smtk::common::UUID& bridgeSessionId);

  smtk::model::StringData supportedFileTypes(
    const std::string& bridgeName = std::string());
  smtk::model::OperatorResult readFile(
    const std::string& fileName,
    const std::string& bridgeName = std::string(),
    const std::string& engineName = std::string());
  std::vector<std::string> fileOperators(
    const std::string& bridgeName);

  std::vector<std::string> operatorNames(const std::string& bridgeName);
  std::vector<std::string> operatorNames(const smtk::common::UUID& bridgeSessionId);

  smtk::model::OperatorPtr createOperator(
    const smtk::common::UUID& bridgeOrModel, const std::string& opName);
  smtk::model::OperatorPtr createOperator(
    const std::string& bridgeName, const std::string& opName);

  smtk::model::ManagerPtr modelManager();

protected:
  friend class cmbForwardingBridge;

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
  smtk::model::OperatorPtr findFileOperator(
    const std::string& fileName,
    smtk::model::BridgePtr bridge,
    const std::string& engineName);

  void fetchWholeModel();

  smtk::model::ManagerPtr m_modelMgr;
  vtkSMProxy* m_serverSession;
  std::set<std::string> m_remoteBridgeNames;
  std::map<smtk::common::UUID,std::string> m_remoteBridgeSessionIds;
  /// map for bridge file types
  /// <bridgeName, <engine-name, fileTypesList> >
  std::map<std::string, smtk::model::StringData> m_bridgeFileTypes;

private:
  vtkSMModelManagerProxy(const vtkSMModelManagerProxy&); // Not implemented.
  void operator = (const vtkSMModelManagerProxy&); // Not implemented.
};

#endif // __vtkSMModelManagerProxy_h

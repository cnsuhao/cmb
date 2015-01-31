#ifndef __cmbForwardingSession_h
#define __cmbForwardingSession_h

#include "ModelBridgeClientModule.h"
#include "smtk/model/DefaultSession.h"

class vtkSMModelManagerProxy;

class MODELBRIDGECLIENT_EXPORT cmbForwardingSession : public smtk::model::DefaultSession
{
public:
  smtkTypeMacro(cmbForwardingSession);
  smtkCreateMacro(cmbForwardingSession);
  smtkSharedFromThisMacro(Session);
  smtkDeclareModelingKernel();

  virtual ~cmbForwardingSession();

  vtkSMModelManagerProxy* proxy() { return this->m_proxy; }
  virtual void setProxy(vtkSMModelManagerProxy* proxy);

protected:
  cmbForwardingSession();

  virtual smtk::model::SessionInfoBits transcribeInternal(const smtk::model::EntityRef& entity, smtk::model::SessionInfoBits flags);
  virtual bool ableToOperateDelegate(smtk::model::RemoteOperatorPtr op);
  virtual smtk::model::OperatorResult operateDelegate(smtk::model::RemoteOperatorPtr op);

  vtkSMModelManagerProxy* m_proxy;

private:
  cmbForwardingSession(const cmbForwardingSession&); // Not implemented.
  void operator = (const cmbForwardingSession&); // Not implemented.
};

#endif // __cmbForwardingSession_h

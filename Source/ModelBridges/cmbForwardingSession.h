//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
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

  ~cmbForwardingSession() override;

  virtual std::string defaultFileExtension(const smtk::model::Model& model) const;

  vtkSMModelManagerProxy* proxy() { return this->m_proxy; }
  virtual void setProxy(vtkSMModelManagerProxy* proxy);

protected:
  cmbForwardingSession();

  virtual smtk::model::SessionInfoBits transcribeInternal(const smtk::model::EntityRef& entity,
    smtk::model::SessionInfoBits flags, int depth = -1) override;
  bool ableToOperateDelegate(smtk::model::RemoteOperatorPtr op) override;
  smtk::model::OperatorResult operateDelegate(smtk::model::RemoteOperatorPtr op) override;

  vtkSMModelManagerProxy* m_proxy;

private:
  cmbForwardingSession(const cmbForwardingSession&); // Not implemented.
  void operator=(const cmbForwardingSession&);       // Not implemented.
};

#endif // __cmbForwardingSession_h

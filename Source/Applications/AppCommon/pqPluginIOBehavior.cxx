//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#include "pqPluginIOBehavior.h"

#include "pqApplicationCore.h"
#include "pqPluginManager.h"
#include "vtkPVProxyDefinitionIterator.h"
#include "vtkPVXMLElement.h"
#include "vtkSmartPointer.h"
#include "vtkSMProxyManager.h"
#include "vtkSMProxyDefinitionManager.h"
#include "vtkSMSession.h"
#include "vtkSMSessionProxyManager.h"
#include "vtkWeakPointer.h"
#include <vtksys/ios/sstream>
#include <vtksys/SystemTools.hxx>

#include <QByteArray>
#include <QFile>
#include <QMap>
#include <set>

// Mostly this is copy/paste from vtkSMReaderFactory.cxx

class pqPluginIOBehavior::cmbInternals
{
public:
  struct ReaderInfo
    {
//    vtkWeakPointer<vtkSMSession> Session;
    std::string Group;
    std::string Name;
    std::vector<std::string> Extensions;
    std::vector<std::string> FilenamePatterns;
    std::string Description;

    void initialize(vtkPVXMLElement* rfHint,
      const std::string& group, const std::string& name)
      {
      if (!rfHint)
        {
        return;
        }
      this->Group = group;
      this->Name = name;
      this->Extensions.clear();
      const char* exts = rfHint->GetAttribute("extensions");
      if (exts)
        {
        vtksys::SystemTools::Split(exts, this->Extensions,' ');
        }
      const char* filename_patterns = rfHint->GetAttribute("filename_patterns");
      if (filename_patterns)
        {
        vtksys::SystemTools::Split(filename_patterns, this->FilenamePatterns,' ');
        }
      this->Description = rfHint->GetAttribute("file_description");
//      this->Session = session;
      }
    };

  void addReader(const std::string& group, const std::string& name,
    vtkPVXMLElement* hints)
  {
    std::string readerkey = group + name;
    if(this->Readers.find(readerkey) == this->Readers.end())
      {
      this->Readers[readerkey].initialize(hints, group, name);
//      this->Groups.insert(group);
      }
  }

  vtkSMProxy* GetReaderProxy(vtkSMSession* session, const char* groupName, const char* proxyName)
  {
  return session->GetSessionProxyManager()->GetPrototypeProxy(groupName, proxyName);
  }

  // The key is a
  // combination of the prototype name and group.
  typedef std::map<std::string, ReaderInfo> ReaderType;
  ReaderType Readers;
  std::string SupportedFileTypes;
  // The set of groups that are defined from plugins.
  // It is used to search for readers.
  std::set<std::string> Groups;
};

//-----------------------------------------------------------------------------
pqPluginIOBehavior::pqPluginIOBehavior(QObject* parentObject)
  : Superclass(parentObject)
{
  this->Internals = new cmbInternals();
  QObject::connect(pqApplicationCore::instance()->getPluginManager(),
    SIGNAL(pluginsUpdated()),
    this, SLOT(updateResources()));
  this->Internals->Groups.insert("sources");
//  this->updateResources();
}

//-----------------------------------------------------------------------------
pqPluginIOBehavior::~pqPluginIOBehavior()
{
  delete this->Internals;
}

//-----------------------------------------------------------------------------
void pqPluginIOBehavior::updateResources()
{
  vtkSMProxyManager* proxyManager = vtkSMProxyManager::GetProxyManager();
  // when we change the server we may not have a session yet. that's ok
  // since we'll come back here after the proxy definitions are loaded
  // from that session.
  if(vtkSMSession* session = proxyManager->GetActiveSession())
    {
    vtkSMSessionProxyManager* sessionProxyManager = session->GetSessionProxyManager();
    vtkSMProxyDefinitionManager* pdm = sessionProxyManager->GetProxyDefinitionManager();

    for(std::set<std::string>::iterator group=this->Internals->Groups.begin();
        group!=this->Internals->Groups.end();group++)
      {
      vtkPVProxyDefinitionIterator* iter =
        pdm->NewSingleGroupIterator(group->c_str());
      for (iter->GoToFirstItem(); !iter->IsDoneWithTraversal(); iter->GoToNextItem())
        {
        vtkPVXMLElement* hints = sessionProxyManager->GetProxyHints(
          iter->GetGroupName(), iter->GetProxyName());
        if (pqPluginIOBehavior::isPluginReader(hints))
          {
          this->Internals->addReader(
            iter->GetGroupName(), iter->GetProxyName(),
             hints->FindNestedElementByName("ReaderFactory"));
          }
        }
      iter->Delete();
      }
    }
}

//----------------------------------------------------------------------------
const char* pqPluginIOBehavior::supportedFileTypes(vtkSMSession* session)
{
  vtksys_ios::ostringstream all_types;
  size_t i=0, j=0;
  cmbInternals::ReaderType::iterator iter;
  for (iter = this->Internals->Readers.begin();
    iter != this->Internals->Readers.end(); ++iter, ++i)
    {
    if (this->Internals->GetReaderProxy(
        session, iter->second.Group.c_str(), iter->second.Name.c_str()) &&
        iter->second.Extensions.size() > 0)
      {
      vtksys_ios::ostringstream stream;
      stream << iter->second.Description << " (";
      std::vector<std::string>::const_iterator it;
      for (it = iter->second.Extensions.begin();
           it != iter->second.Extensions.end(); ++it, ++j)
        {
        stream << "*." << (*it);
        if (j < iter->second.Extensions.size() -1)
          {
          stream << " ";
          }
        }
      stream << ")";
      all_types << stream.str();
      if (i < this->Internals->Readers.size() -1)
        {
        all_types << ";;";
        }
      }
    }

  this->Internals->SupportedFileTypes = all_types.str();
  return this->Internals->SupportedFileTypes.c_str();
}

//----------------------------------------------------------------------------
pqPluginIOBehavior::FileExtMap pqPluginIOBehavior::fileExtensionMap(
  vtkSMSession* session)
{
  pqPluginIOBehavior::FileExtMap readerMap;
  cmbInternals::ReaderType::iterator iter;
  for (iter = this->Internals->Readers.begin();
    iter != this->Internals->Readers.end(); ++iter)
    {
    std::vector<std::string>::const_iterator it;
    for (it = iter->second.Extensions.begin();
         it != iter->second.Extensions.end(); ++it)
      {
      readerMap.insert((*it).c_str(),
        QPair<QString, QString>(iter->second.Group.c_str(),
             iter->second.Name.c_str()));
      }
    }

  return readerMap;
}

//-----------------------------------------------------------------------------
bool pqPluginIOBehavior::isPluginReader(vtkPVXMLElement* hints)
{
  return (hints && hints->FindNestedElementByName("ReaderFactory") &&
          hints->FindNestedElementByName("CMBPluginReader"));
}

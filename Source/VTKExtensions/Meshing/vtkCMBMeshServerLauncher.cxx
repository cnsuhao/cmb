/*=========================================================================

Copyright (c) 1998-2005 Kitware Inc. 28 Corporate Drive, Suite 204,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced,
distributed,

or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================*/

#include "vtkCMBMeshServerLauncher.h"

#include "vtkObjectFactory.h"

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif

#include <vector>
#include <string>
#include <vtksys/SystemTools.hxx>

#include <remus/server/Server.h>
#include <remus/server/WorkerFactory.h>

namespace resources
{
inline std::string getExecutableLocation()
  {
  //we default our guess of the executable location to be the current working
  //directory
  std::string execLocation(vtksys::SystemTools::GetCurrentWorkingDirectory());
#ifdef __APPLE__
  CFBundleRef mainBundle = CFBundleGetMainBundle();
  if(mainBundle)
    {
    CFURLRef url (CFBundleCopyExecutableURL(mainBundle));
    if(url)
      {
      CFStringRef posixUrl (CFURLCopyFileSystemPath(url,kCFURLPOSIXPathStyle));
      if(posixUrl)
        {
        int len = static_cast<int>(CFStringGetLength(posixUrl));
        int bufferSize = static_cast<int>(CFStringGetMaximumSizeForEncoding(len,
                                          kCFStringEncodingUTF8));
        if(len > 0 && bufferSize > 0)
          {
          //don't use len, that returns the len for utf16, which isn't accurate
          //you need to get the maxium size for the c string encoding
          char* name = new char[bufferSize+1];
          bool nameCopied = CFStringGetCString(posixUrl,name,bufferSize,
                                               kCFStringEncodingUTF8);
          if(nameCopied)
            {
            //we have a valid bundle path, copy to string and use
            //vtksys to drop the executables name.
            std::string execFile = std::string(name);
            execLocation = vtksys::SystemTools::GetFilenamePath(execFile);
            }
          delete[] name;
          }
        CFRelease(posixUrl);
        }
      CFRelease(url);
      }
    }
#endif
  return execLocation;
}

inline std::vector<std::string> locationsToSearch()
{
  std::vector<std::string> locations;
  locations.push_back("bin/");
  locations.push_back("../");
  locations.push_back("../bin/");
  locations.push_back("../../bin/");
  locations.push_back("../../");
  locations.push_back("../../../bin/");
  locations.push_back("../../../");

#ifdef _WIN32
  //only search paths that make sense on window development or dashboard machines
  locations.push_back("../../../bin/Debug/");
  locations.push_back("../../../bin/Release/");
  locations.push_back("../../bin/Debug/");
  locations.push_back("../../bin/Release/");
  locations.push_back("../bin/Debug/");
  locations.push_back("../bin/Release/");
  locations.push_back("bin/Debug/");
  locations.push_back("bin/Release/");
#endif

  return locations;
}

//the goal here is to find all the places that the mesh workers config
//files could be located and give them to the factory.
inline void locationsToSearchForWorkers(
                      boost::shared_ptr<remus::server::WorkerFactory> factory )
{
  //first we search from the executables locations,
  //if than we search the current working directory if it is a different path
  //than the executable location
  std::string execLoc = ::resources::getExecutableLocation();
  factory->addWorkerSearchDirectory(execLoc);

  typedef std::vector<std::string>::const_iterator It;
  std::vector<std::string> locations = ::resources::locationsToSearch();

  for(It i = locations.begin(); i != locations.end();++i)
    {
    std::stringstream buffer;
    buffer << execLoc << "/" << *i;
    factory->addWorkerSearchDirectory(buffer.str());
    }
}

}

vtkStandardNewMacro(vtkCMBMeshServerLauncher)

//-----------------------------------------------------------------------------
vtkCMBMeshServerLauncher::vtkCMBMeshServerLauncher()
{
  //we aren't alive till somebody calls launch
  this->Alive = false;

  boost::shared_ptr<remus::server::WorkerFactory> factory(
                                        new remus::server::WorkerFactory());
  factory->setMaxWorkerCount(2);
  ::resources::locationsToSearchForWorkers(factory);

  //this sets up the server but it isn't accepting any connections,
  //but has bound to the correct ports
  this->Implementation = new remus::server::Server(factory);

  //now setup the server to poll fairly fast, so that when we ask it shutdown,
  //it can in a fairly timely manner. Range is 32ms to 2.5sec for our polling
  remus::server::PollingRates rates(32,2500);
  this->Implementation->pollingRates(rates); //update the server rates

  //launch the server, this must be done before we grab the port info
  this->Launch();

  //grab the host and port info for the client side
  const remus::server::ServerPorts& pinfo =
                                    this->Implementation->serverPortInfo();
  this->HostName = pinfo.client().host();
  this->PortNumber = pinfo.client().port();
}

//-----------------------------------------------------------------------------
vtkCMBMeshServerLauncher::~vtkCMBMeshServerLauncher()
{
  this->Terminate();
  delete this->Implementation;
  this->Implementation = NULL;
}

//-----------------------------------------------------------------------------
int vtkCMBMeshServerLauncher::Launch()
{
  //remus can handle being asked to start brokering even when it is
  //currently brokering
  this->Implementation->startBrokeringWithoutSignalHandling();
  this->Alive = true;
  return 1;
}

//-----------------------------------------------------------------------------
int vtkCMBMeshServerLauncher::IsAlive()
{
  return !!this->Alive;
}

//-----------------------------------------------------------------------------
int vtkCMBMeshServerLauncher::Terminate()
{
  if(this->Alive)
    {
    this->Implementation->stopBrokering();
    this->Alive = false;
    }
  return 1;
}

//-----------------------------------------------------------------------------
void vtkCMBMeshServerLauncher::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "Host Name: " << this->HostName << std::endl;
  os << indent << "Port Number: " << this->PortNumber << std::endl;
  os << indent << "Alive: " << this->Alive << std::endl;
}

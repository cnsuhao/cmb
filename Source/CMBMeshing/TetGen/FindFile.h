/*=========================================================================

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef cmbmesh_omicron_findfile_h
#define cmbmesh_omicron_findfile_h

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif

#include <vector>
#include <string>
#include <vtksys/SystemTools.hxx>

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

//the goal here is to find a file given a base name + ext
//input example:
// name = "foo", ext = "txt" = we search for "foo.txt"
// name = "foo", ext = ".txt" = we search for "foo.txt"
// name = "foo.", ext = ".txt" = we search for "foo..txt"
// name = "foo", ext = "" = we search for foo
// name = "", ext = "txt" = we don't search at all, we require a name
inline remus::common::FileHandle FindFile( std::string name, std::string ext )
{
  remus::common::FileHandle handle("");
  if(name.size() == 0)
    { return handle; }

  //generate the fullname which is "name+ext"
  std::string fullname = name;
  if(ext.size()>0)
    {
    if(ext[0]!='.')
      { fullname += "."; }
    fullname += ext;
    }

  //we have a full file name now, time to search for the file

  std::vector< std::string > fileComponents;
  fileComponents.push_back( ::resources::getExecutableLocation() );
  fileComponents.push_back( fullname );

  std::string possibleFile = vtksys::SystemTools::JoinPath(fileComponents);
  if(vtksys::SystemTools::FileExists(possibleFile.c_str()))
    {
    return remus::common::FileHandle(possibleFile);
    }

  //the file wasn't found beside the executable lets look a little harder.
  //we insert a place holder into file components between the root and filename
  //that will be used for the relative directory from the root to look for the
  //file
  fileComponents.insert(fileComponents.end(), "");
  typedef std::vector<std::string>::const_iterator It;
  std::vector<std::string> locations = ::resources::locationsToSearch();
  for(It i=locations.begin(); i!=locations.end(); ++i)
    {
    fileComponents[1]=*i;
    possibleFile = vtksys::SystemTools::JoinPath(fileComponents);
    if(vtksys::SystemTools::FileExists(possibleFile.c_str()))
      {
      return remus::common::FileHandle(possibleFile);
      }
    }
  return handle;
}

}

#endif
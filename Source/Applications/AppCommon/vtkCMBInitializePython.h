//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __vtkCMBInitializePython_h
#define __vtkCMBInitializePython_h

#include "vtkPythonInterpreter.h"
#include <string>
#include <vtksys/SystemTools.hxx>

/* The maximum length of a file name.  */
#if defined(PATH_MAX)
# define VTK_PYTHON_MAXPATH PATH_MAX
#elif defined(MAXPATHLEN)
# define VTK_PYTHON_MAXPATH MAXPATHLEN
#else
# define VTK_PYTHON_MAXPATH 16384
#endif
namespace
{
  // ParaView should setup Python Path to find the modules in the following
  // locations for various platforms. There are always two cases to handle: when
  // running from build-location and when running from installed-location.

  //----------------------------------------------------------------------------
  void vtkPythonAppInitPrependPythonPath(const std::string& dir)
    {
    if (dir != "")
      {
      std::string collapsed_dir =
        vtksys::SystemTools::CollapseFullPath(dir.c_str());
      if (vtksys::SystemTools::FileIsDirectory(collapsed_dir.c_str()))
        {
        vtkPythonInterpreter::PrependPythonPath(collapsed_dir.c_str());
        }
      }
    }

//#ifndef PARAVIEW_FREEZE_PYTHON
# if defined(_WIN32)
  void vtkPythonAppInitPrependPathWindows(const std::string &SELF_DIR);
# elif defined(__APPLE__)
  void vtkPythonAppInitPrependPathOsX(const std::string &SELF_DIR);
# else
  void vtkPythonAppInitPrependPathLinux(const std::string &SELF_DIR);
# endif
//#endif // ifndef PARAVIEW_FREEZE_PYTHON

  //----------------------------------------------------------------------------
  void vtkCMBPythonAppInitPrependPath(const std::string &SELF_DIR)
    {
    // We don't initialize Python paths when Frozen Python is being used. This
    // avoid unnecessary file-system accesses..
//#ifndef PARAVIEW_FREEZE_PYTHON
# if defined(_WIN32)
    vtkPythonAppInitPrependPathWindows(SELF_DIR);
# elif defined(__APPLE__)
    vtkPythonAppInitPrependPathOsX(SELF_DIR);
# else
    vtkPythonAppInitPrependPathLinux(SELF_DIR);
# endif
//#endif // ifndef PARAVIEW_FREEZE_PYTHON

    // *** The following maybe obsolete. Need to verify and remove. ***

    // This executable does not actually link to the python wrapper
    // libraries, though it probably should now that the stub-modules
    // are separated from them.  Since it does not we have to make
    // sure the wrapper libraries can be found by the dynamic loader
    // when the stub-modules are loaded.  On UNIX this executable must
    // be running in an environment where the main VTK libraries (to
    // which this executable does link) have been found, so the
    // wrapper libraries will also be found.  On Windows this
    // executable may have simply found its .dll files next to itself
    // so the wrapper libraries may not be found when the wrapper
    // modules are loaded.  Solve this problem by adding this
    // executable's location to the system PATH variable.  Note that
    // this need only be done for an installed VTK because in the
    // build tree the wrapper modules are in the same directory as the
    // wrapper libraries.
#if defined(_WIN32)
    static char system_path[(VTK_PYTHON_MAXPATH+1)*10] = "PATH=";
    strcat(system_path, SELF_DIR.c_str());
    if(char* oldpath = getenv("PATH"))
      {
      strcat(system_path, ";");
      strcat(system_path, oldpath);
      }
    putenv(system_path);
#endif // if defined(_WIN32)
    }

//#ifndef PARAVIEW_FREEZE_PYTHON
# if defined(_WIN32)
  //===========================================================================
  // Windows
  // Key:
  //    - SELF_DIR: directory containing the pvserver/pvpython/paraview
  //      executables.
  //---------------------------------------------------------------------------
  //  + BUILD_LOCATION
  //    + ParaView C/C++ library location
  //      - SELF_DIR
  //    + ParaView Python modules
  //      - SELF_DIR/../lib/site-packages    (when CMAKE_INTDIR is not defined).
  //          OR
  //      - SELF_DIR/../../lib/site-packages (when CMAKE_INTDIR is defined).
  //  + INSTALL_LOCATION
  //    + ParaView C/C++ library location
  //      - SELF_DIR
  //      - SELF_DIR/../lib/paraview-<major>.<minor>/
  //    + ParaView Python modules
  //      - SELF_DIR/../lib/paraview-<major>.<minor>/site-packages
  //    + VTK Python Module libraries
  //      - SELF_DIR/../lib/paraview-<major>.<minor>/site-packages/vtk
  //===========================================================================
  void vtkPythonAppInitPrependPathWindows(const std::string& SELF_DIR)
    {
    // For use in MS VS IDE builds we need to account for selection of
    // build configuration in the IDE. CMAKE_INTDIR is how we know which
    // configuration user has selected. It will be one of Debug, Release,
    // ... etc. With in VS builds SELF_DIR will be set like
    // "builddir/bin/CMAKE_INTDIR". PYHTONPATH should have SELF_DIR and
    // "builddir/lib/CMAKE_INTDIR"
    std::string build_dir_site_packages;
#if defined(CMAKE_INTDIR)
    build_dir_site_packages = SELF_DIR + "/../../lib/site-packages";
#else
    build_dir_site_packages = SELF_DIR + "/../lib/site-packages";
#endif
    bool is_build_dir =
      vtksys::SystemTools::FileExists(build_dir_site_packages.c_str());
    if (is_build_dir)
      {
      vtkPythonAppInitPrependPythonPath(SELF_DIR);
#if defined(CMAKE_INTDIR)
      vtkPythonAppInitPrependPythonPath(
        SELF_DIR + "/../../lib/" + std::string(CMAKE_INTDIR));
#else
      vtkPythonAppInitPrependPythonPath(SELF_DIR + "/../lib");
#endif
      vtkPythonAppInitPrependPythonPath(build_dir_site_packages);
      }
    else
      {
      vtkPythonAppInitPrependPythonPath(SELF_DIR);
      vtkPythonAppInitPrependPythonPath(
        SELF_DIR + "/../lib/paraview-" PARAVIEW_VERSION);
      vtkPythonAppInitPrependPythonPath(
        SELF_DIR + "/../lib/paraview-" PARAVIEW_VERSION "/site-packages");
      // BUG #14263 happened with windows installed versions too. This addresses
      // that problem.
      vtkPythonAppInitPrependPythonPath(
        SELF_DIR + "/../lib/paraview-" PARAVIEW_VERSION "/site-packages/vtk");
      }
    }
# elif defined(__APPLE__)
  //===========================================================================
  // OsX
  // Key:
  //    - SELF_DIR: directory containing the pvserver/pvpython/paraview
  //      executables. This is different for app and non-app executables.
  //---------------------------------------------------------------------------
  //  + BUILD_LOCATION
  //    - LIB_DIR
  //      - SELF_DIR/../lib       (when executable is not APP, e.g pvpython)
  //          OR
  //      - SELF_DIR/../../../../lib (when executable is APP, e.g. paraview)
  //    + ParaView C/C++ library location
  //      - LIB_DIR
  //    + ParaView Python modules
  //      - LIB_DIR/site-packages
  //  + INSTALL_LOCATION (APP)
  //    - APP_ROOT
  //      - SELF_DIR/../..        (this is same for paraview and pvpython)
  //    - ParaView C/C++ library location
  //      - APP_ROOT/Contents/Libraries/
  //    - ParaView Python modules
  //      - APP_ROOT/Contents/Python
  //  + INSTALL_LOCATION (UNIX STYLE)
  //    + SELF_DIR is "bin"
  //    + ParaView C/C++ library location
  //      - SELF_DIR/../lib/paraview-<major>.<minor>
  //    + ParaView Python modules
  //      - SELF_DIR/../lib/paraview-<major>.<minor>/site-packages
  //    + VTK Python Module libraries
  //      - SELF_DIR/../lib/paraview-<major>.<minor>/site-packages/vtk
  //===========================================================================
  void vtkPythonAppInitPrependPathOsX(const std::string &SELF_DIR)
    {
    vtkPythonAppInitPrependPythonPath(SELF_DIR + "/../Libraries");
    vtkPythonAppInitPrependPythonPath(SELF_DIR + "/../Python");
    }
# else
  void vtkPythonAppInitPrependPathLinux(const std::string &SELF_DIR);
  //===========================================================================
  // Linux/UNIX (not OsX)
  // Key:
  //    - SELF_DIR: directory containing the pvserver/pvpython/paraview
  //      executables. For installed locations, this corresponds to the "real"
  //      executable, not the shared-forwarded executable (if applicable).
  //---------------------------------------------------------------------------
  // + BUILD_LOCATION
  //    + ParaView C/C++ library location
  //      - SELF_DIR/../lib
  //    + ParaView Python modules
  //      - SELF_DIR/../lib/site-packages
  //  + INSTALL_LOCATION
  //    + ParaView C/C++ library location
  //      - SELF_DIR
  //    + ParaView Python modules
  //      - SELF_DIR/site-packages
  //    + VTK Python Module libraries
  //      - SELF_DIR/site-packages/vtk
  void vtkPythonAppInitPrependPathLinux(const std::string& SELF_DIR)
    {
    vtkPythonAppInitPrependPythonPath(SELF_DIR + "/../lib");
    vtkPythonAppInitPrependPythonPath(SELF_DIR + "/../lib/site-packages");
    // Build directory
    vtkPythonAppInitPrependPythonPath(SELF_DIR + "/../lib/python2.7/site-packages");
    // Package directory
    vtkPythonAppInitPrependPythonPath(SELF_DIR + "/../paraview-4.1/site-packages");
    vtkPythonAppInitPrependPythonPath(SELF_DIR + "/../paraview-4.1/lib/python2.7/site-packages");
    vtkPythonAppInitPrependPythonPath(SELF_DIR + "/../paraview-4.1/lib/python2.7/");
    vtkPythonAppInitPrependPythonPath(SELF_DIR + "/../paraview-4.1/lib/python2.7/plat-linux3");
    vtkPythonAppInitPrependPythonPath(SELF_DIR + "/../paraview-4.1/lib/python2.7/lib-tk");
    vtkPythonAppInitPrependPythonPath(SELF_DIR + "/../paraview-4.1/lib/python2.7/lib-old");
    vtkPythonAppInitPrependPythonPath(SELF_DIR + "/../paraview-4.1/lib/python2.7/lib-dynload");
  }
# endif
}

#endif
// VTK-HeaderTest-Exclude: vtkProcessModuleInitializePython.h

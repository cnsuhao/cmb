# Setup install directories (we use names with VTK_ prefix, since ParaView now
# is built as a custom "VTK" library.

include(CMBVersion)

# Use the new version of the variable names for output of build process.
# These are consistent with what VTK sets.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
if(UNIX)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/cmb-${CMB_VERSION}")
else()
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
endif()
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/cmb-${CMB_VERSION}")


#we need to explicitly set these so that paraview's plugin bad values
#aren't kept
set(VTK_INSTALL_RUNTIME_DIR bin)
set(VTK_INSTALL_LIBRARY_DIR lib/cmb-${CMB_VERSION})
set(VTK_INSTALL_ARCHIVE_DIR lib/cmb-${CMB_VERSION})
set(VTK_INSTALL_INCLUDE_DIR include/cmb-${CMB_VERSION})

if(NOT VTK_INSTALL_DATA_DIR)
  set(VTK_INSTALL_DATA_DIR share/cmb-${CMB_VERSION})
endif()
if(NOT VTK_INSTALL_DOC_DIR)
  set(VTK_INSTALL_DOC_DIR share/doc/cmb-${CMB_VERSION})
endif()
if(NOT VTK_INSTALL_PACKAGE_DIR)
  set(VTK_INSTALL_PACKAGE_DIR "lib/cmake/cmb-${CMB_VERSION}")
endif()
if(NOT VTK_INSTALL_EXPORT_NAME)
  set(VTK_INSTALL_EXPORT_NAME CMBTargets)
endif()

if(NOT PV_INSTALL_PLUGIN_DIR)
  if(WIN32)
    set (PV_INSTALL_PLUGIN_DIR ${VTK_INSTALL_RUNTIME_DIR})
  else ()
    set (PV_INSTALL_PLUGIN_DIR ${VTK_INSTALL_LIBRARY_DIR})
  endif()
endif()

#set (VTK_INSTALL_NO_LIBRARIES TRUE)
#set (VTK_INSTALL_NO_DEVELOPMENT TRUE)

# Disable installing of the Qt Designer plugin. There's no need for it in
# ParaView install rules.
set (VTK_INSTALL_NO_QT_PLUGIN TRUE)

# ParaView install the vtk python modules specifically to appropriate locations.
set (VTK_INSTALL_NO_PYTHON TRUE)
set (VTK_INSTALL_PYTHON_USING_CMAKE TRUE)

# for temporary backwards compatibility.
set (PV_INSTALL_BIN_DIR ${VTK_INSTALL_RUNTIME_DIR})
set (PV_INSTALL_LIB_DIR ${VTK_INSTALL_LIBRARY_DIR})
set (PV_INSTALL_EXPORT_NAME ${VTK_INSTALL_EXPORT_NAME})

# Setting this ensures that "make install" will leave rpaths to external
# libraries (not part of the build-tree e.g. Qt, ffmpeg, etc.) intact on
# "make install". This ensures that one can install a version of ParaView on the
# build machine without any issues. If this not desired, simply specify
# CMAKE_INSTALL_RPATH_USE_LINK_PATH when configuring and
# "make install" will strip all rpaths, which is default behavior.
if (NOT CMAKE_INSTALL_RPATH_USE_LINK_PATH)
  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
endif ()

if(UNIX)
  # When we are building a package on linux we have the problem that the
  # libraries are laid out like this:
  # - lib/
  # -  - /paraview-version/
  # -  - /cmb-version/
  # So we need to teach the forwarding executable that builds the plugins
  # where to find the paraview libraries, as it will already know where to
  # find the cmb libraries
  set (pv_version "${PARAVIEW_VERSION_MAJOR}.${PARAVIEW_VERSION_MINOR}")
  set(PARAVIEW_INSTALL_LIB_DIR "lib" "../lib/paraview-${pv_version}")
  unset(pv_version)
endif()



if (APPLE)
  # If building on apple, we will set cmake rules for ParaView such that "make
  # install" will result in creation of an application bundle by default.
  # The trick we play is as such:
  # 1. We hide the CMAKE_INSTALL_PREFIX variable at force it to a
  #    internal/temporary location. All generic VTK/ParaView install rules
  #    happily go about installing under this CMAKE_INSTALL_PREFIX location.
  # 2. We provide a new vairable MACOSX_APP_INSTALL_PREFIX that user can set to
  #    point where he wants the app bundle to be placed (default is
  #    /Applications).
  # 3. We set CMAKE_INSTALL_NAME_DIR to point to location of runtime libraries
  #    in the app bundle. Thus when the libraries are installed, cmake
  #    automatically cleans up the install_name paths for all shared libs that
  #    we built to point to a location within the app.
  # 4. To make packaging of plugins easier, we install plugins under a directory
  #    named "plugins" in the temporary CMAKE_INSTALL_PREFIX location. This just
  #    a simple trick to avoid having to keep track of plugins we built.
  # 5. Every application that builds an app, then uses the
  #    ParaViewBrandingInstallApp.cmake or something similar to put all the
  #    libraries, plugins, python files etc. within the app bundle itself.
  # 6. Finally, the bundle generated under the temporary location is copied over
  #   to the path specified by MACOSX_APP_INSTALL_PREFIX.
  #
  # In keeping with our "WE INSTALL WHAT WE BUILD" rule, this app bundle is not
  # distributable to others since it does not include Qt, or other external
  # dependencies. For a distributable pacakage, refer to ParaView Super-build
  # instructions.
  set (CMAKE_INSTALL_PREFIX
    ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/__macos_install
    CACHE INTERNAL "" FORCE)
  set (MACOSX_APP_INSTALL_PREFIX
    "/Applications"
    CACHE PATH
    "Location where the *.app bundle must be installed.")
  set(CMAKE_INSTALL_NAME_DIR "@executable_path/../Libraries")
  set(PV_INSTALL_PLUGIN_DIR "Plugins")
endif()

#helper function to install plugins
function(cmb_install_plugin target)
  if(APPLE)
    set(PROGRAMS_TO_INSTALL_PLUGIN
        # GeologyBuilder
        # MeshViewer
        ModelBuilder
        # PointsBuilder
        # ProjectManager
        # SceneBuilder
        )
    foreach(app_name ${PROGRAMS_TO_INSTALL_PLUGIN})
      install(TARGETS ${target}
        DESTINATION "${VTK_INSTALL_LIBRARY_DIR}/${app_name}.app/Contents/Plugins"
        )
    endforeach()
  else()
    install(TARGETS ${target} DESTINATION ${PV_INSTALL_PLUGIN_DIR} )
  endif()
endfunction()

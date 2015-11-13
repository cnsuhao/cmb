
#we have a problem where things designed with the old source layout,
#use this as the root of the project, and new scripts and code expects
#the parent directory to be the root of the project. So to handle both use
#cases we conditional call the project call based on if the project name
#has been set. Project is the default string if the project command hasn't been
#called.
if(NOT ConceptualModelBuilder_Project_Name_Set)
  cmake_minimum_required(VERSION 2.8.11)

  if(POLICY CMP0025)
    #setup policy rules for CMake 3.0 while we have a minimum required of 2.8.X
    #both 42 and 25 were introduced in the same cmake version
    cmake_policy(SET CMP0025 NEW)  # Clang is now Clang and Apple Clang
    cmake_policy(SET CMP0042 OLD)  # Disable @rpath on Apple
  endif()

  project(ConceptualModelBuilder)

  #just like the project name we can only call ctest once, and we want to
  #do it in the root directory of the project
  option(BUILD_TESTING "Build CMB Testing" ON)
  if(BUILD_TESTING)
    enable_testing()
    include(CTest)
  endif()
endif()

#Specify which tests we want to build.
option(BUILD_SHORT_TESTS "Build short tests (10 second timeout tests)" ON)
option(BUILD_MEDIUM_TESTS "Build medium tests (60 second timeout tests)" ON)
option(BUILD_LONG_TESTS "Build long tests (200 second timeout tests)" ON)

option(BUILD_SHARED_LIBS
      "Global flag to cause add_library to create shared libraries if on."
      ON)

#override the default project source and binary dir so that they
#always point to the same locations no matter where the project
#command was called from
set(ConceptualModelBuilder_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(ConceptualModelBuilder_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR})#for system header


#Policy CMP0003 deals with how specify link paths, with the policy set to NEW
#we are stating that we shouldn't split link paths into search paths and
#library names, but instead directly link using the full path. You really
#don't want to remove this policy before we set the cmake_minimum_required to
#be 3.X
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

# Ensure that SUBDIRS is not used in this project:
# (we need in-order processing so the fixup script for the bundle
#  applications works properly...)
#
macro(SUBDIRS)
  message(FATAL_ERROR "error: do not use SUBDIRS in this project. Use ADD_SUBDIRECTORY instead.")
endmacro(SUBDIRS)

set(CMB_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/CMake")
set(CMAKE_MODULE_PATH "${CMB_CMAKE_DIR}" ${CMAKE_MODULE_PATH})

include(CMBVersion)

if(NOT APPLE AND "${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
  if(NOT "${CMAKE_CXX_FLAGS}" MATCHES "-fPIC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
  endif()
  if(NOT "${CMAKE_C_FLAGS}" MATCHES "-fPIC")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
  endif()
endif()

message(STATUS "CMAKE_CXX_COMPILER_ID='${CMAKE_CXX_COMPILER_ID}'")
message(STATUS "CMAKE_CXX_FLAGS='${CMAKE_CXX_FLAGS}'")
message(STATUS "CMAKE_C_FLAGS='${CMAKE_C_FLAGS}'")

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

#########################################################################
# Testing and dashboard setup
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CTestCustom.cmake.in
  ${ConceptualModelBuilder_BINARY_DIR}/CTestCustom.cmake)


if(BUILD_TESTING)
  set(CMB_TEST_DIR ${ConceptualModelBuilder_BINARY_DIR}/Testing/Temporary)
  make_directory(${CMB_TEST_DIR})

  #add the first test which is for checking the copyright
  add_test(NAME CopyrightStatement
    COMMAND ${CMAKE_COMMAND}
        "-DConceptualModelBuilder_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/CMake/CheckCopyright.cmake")
endif()

# Configure data directory
find_path(CMB_TEST_DATA_ROOT cmb-testing-data.marker
  $ENV{CMB_TEST_DATA_ROOT}
  )

get_filename_component(CMB_TEST_DATA_ROOT "${CMB_TEST_DATA_ROOT}" ABSOLUTE)
mark_as_advanced(CMB_TEST_DATA_ROOT)

set(CMB_TEST_PLUGIN_PATHS ${CMB_TEST_PLUGIN_PATHS} CACHE STRING
   "Semi-colon seperated paths for testing plugins.")
mark_as_advanced(CMB_TEST_PLUGIN_PATHS)

#########################################################################

# setup cpack stuff
include(CPackCreate.cmake)
# Setup output directories.
if(NOT DEFINED LIBRARY_OUTPUT_PATH)
  set (LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin CACHE INTERNAL
    "Single output directory for building all libraries.")
else()
  set (LIBRARY_OUTPUT_PATH ${LIBRARY_OUTPUT_PATH} CACHE INTERNAL
    "Single output directory for building all libraries.")
endif()

if(NOT DEFINED EXECUTABLE_OUTPUT_PATH)
  set (EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin CACHE INTERNAL
    "Single output directory for building all executables.")
else()
   set (EXECUTABLE_OUTPUT_PATH ${EXECUTABLE_OUTPUT_PATH} CACHE INTERNAL
    "Single output directory for building all executables.")
endif()

#########################################################################
# include helper functions that projects use
include(CMBHelpers RESULT_VARIABLE CMB_HELPERS_PATH)


#########################################################################
# Search for vxl
find_package(VXL REQUIRED)
if(NOT VXL_RPL_FOUND)
  message(FATAL_ERROR "vxl must be built with BUILD_RPL enabled")
endif()
if(NOT VXL_RPL_RGTL_FOUND)
  message(FATAL_ERROR "vxl must be built with BUILD_RPL_RGTL enabled")
endif()

#########################################################################
# Search for boost
cmb_find_boost()

#########################################################################
# include ParaView here since basically everything links to it
find_package(ParaView REQUIRED)
include("${PARAVIEW_USE_FILE}")
include (ParaViewPlugins)
# We need to add the current value of VTK_MODULES_DIR to the module path
# so that when the plugins are built all the modules can be found. Otherwise,
# modules that aren't loaded as direct dependencies of CMB modules will
# not be found.
list(APPEND CMAKE_MODULE_PATH "${VTK_MODULES_DIR}")


#needs to be before CMBMeshing and pv_process_modules
pv_setup_module_environment("ConceptualModelBuilder")

#------------------------------------------------------------------------------
#We need to call include(CMBInstallRules) as pv_setup_module_environment
#overrides the install variables, and what it sets them too is work. So we
#get to set them all over again.
include(CMBInstallRules)

# This will build rtvl, which is linked from Filtering.
add_subdirectory(LIDAR)

#########################################################################
# Meshing support
#configure meshing components before
add_subdirectory(CMBMeshing)

find_package(SMTK REQUIRED)

#needs to be after CMBMeshing so that we can link to the MeshServer
pv_process_modules()

#process ModelBridge plugin
add_subdirectory(ModelBridges)

add_subdirectory(VTKExtensions/Client)
add_subdirectory(VTKExtensions/Testing)
add_subdirectory(Applications)

option(BUILD_EXAMPLES "Build CMB Examples" ON)
if(BUILD_EXAMPLES)
  add_subdirectory(Examples)
endif()

#------------------------------------------------------------------------------
# Lastly generate the CMBConfig.cmake so that other projects can depend on
# CMB.
# We create two versions of CMBConfig.cmake for the build tree and the
# install tree.

# For build tree.
configure_file(${ConceptualModelBuilder_SOURCE_DIR}/CMBConfig.cmake.in
  ${ConceptualModelBuilder_BINARY_DIR}/../CMBConfig.cmake @ONLY)
configure_file(CMBConfigVersion.cmake.in CMBConfigVersion.cmake @ONLY)


#-----------------------------------------------------------------------------
# Export all targets at once from the build tree in their final configuration.
# We export these again so that we add CMB's targets to the list.
get_property(_vtk_targets GLOBAL PROPERTY VTK_TARGETS)
if (_vtk_targets)
  list(REMOVE_DUPLICATES _vtk_targets)
  export(TARGETS ${_vtk_targets} FILE
    ${ConceptualModelBuilder_BINARY_DIR}/../CMBTargets.cmake)
endif()
unset(_vtk_targets)


# Set project name.
project(CMBMeshWorkers)

option(BUILD_OMICRON_MESH_WORKER "Build Omicron Mesh Worker" OFF)
option(BUILD_TETGEN_MESH_WORKER "Build TetGen Mesh Worker" OFF)
option(BUILD_TRIANGLE_MESH_WORKER "Build Triangle Mesh Worker" OFF)

#Find the Remus package so that we import the targets into this project
#and all projects under this folder
find_package(Remus REQUIRED)
include_directories(${REMUS_INCLUDE_DIRS})

set(ORIG_CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}")
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake" ${CMAKE_MODULE_PATH})

include(RemusRegisterWorker)

if(BUILD_OMICRON_MESH_WORKER)
  add_subdirectory(Omicron)
endif()

if(BUILD_TETGEN_MESH_WORKER)
  add_subdirectory(TetGen)
endif()

if(BUILD_TRIANGLE_MESH_WORKER)
  add_subdirectory(Triangle)
endif()

set(CMAKE_MODULE_PATH "${ORIG_CMAKE_MODULE_PATH}")
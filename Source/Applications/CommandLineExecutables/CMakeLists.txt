project (CMBCommandLineExecutables)

find_package(ParaView REQUIRED)
include(${PARAVIEW_USE_FILE})

if(NOT PARAVIEW_ENABLE_PYTHON)
  #if paraview doesn't have python enabled no
  #point in going further as we won't build
  return()
endif()

find_package(SMTK REQUIRED)

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${SMTK_INCLUDE_DIRS}
  ${ModelBuilder_BINARY_DIR}    # for DefaultExportTemplate.h
  ${ModelBuilder_SOURCE_DIR}/SimBuilder
  ${SimBuilderMesh_SOURCE_DIR}
  ${vtkCMBMeshing_INCLUDE_DIRS}
  ${vtkCmbDiscreteModel_INCLUDE_DIRS}
  ${vtkDiscreteModel_INCLUDE_DIRS}
  ${vtkDiscreteModel_INCLUDE_DIRS}/Model
  ${CMAKE_CURRENT_SOURCE_DIR}/../AppCommon/Resource
  ${CMAKE_CURRENT_BINARY_DIR}/../AppCommon
)


#------------------------------------------------------------------------------
# CommandLine CMB export executable
include_directories (SimBuilder)

# Needs path to python executable
find_package(PythonInterp)
if (PYTHONINTERP_FOUND)
  #message("PYTHON_EXECUTABLE: ${PYTHON_EXECUTABLE}")
  #message("PYTHON_VERSION_STRING: ${PYTHON_VERSION_STRING}")

  set(PYTHON_DEFINES
    CMAKE_PYTHON_EXECUTABLE="${PYTHON_EXECUTABLE}"
    CMAKE_PYTHON_VERSION="${PYTHON_VERSION_STRING}"
  )

  set_property(
    SOURCE export.cxx
    APPEND PROPERTY COMPILE_DEFINITIONS ${PYTHON_DEFINES}
  )
endif()

add_executable(export
  export.cxx
  ${ModelBuilder_SOURCE_DIR}/SimBuilder/smtkModel.cxx
  ${ModelBuilder_SOURCE_DIR}/SimBuilder/smtkModelGroup.cxx
  ${ModelBuilder_SOURCE_DIR}/SimBuilder/smtkModelItem.cxx
  vtkCMBStandaloneMesh.cxx
  vtkCMBStandaloneModelFaceMesh.cxx
)

target_link_libraries(export
  CMBModel_Plugin
  SimBuilderMesh_Plugin
  cmbAppCommon
  SMTKCore
  vtksys
)

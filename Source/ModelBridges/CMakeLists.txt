project(CMBBridge)

find_package(ParaView REQUIRED)
find_package(SMTK REQUIRED)

set(CMAKE_MODULE_PATH "${SMTK_ROOT_DIR}/lib/cmake/SMTK" ${CMAKE_MODULE_PATH})

SET(CmbBridgelFiles
  cmbForwardingBridge.cxx
  vtkSMModelManagerProxy.cxx
  vtkPVSMTKModelInformation
)

SET(CmbBridgelServerFiles
  vtkModelManagerWrapper.cxx
)

SET_SOURCE_FILES_PROPERTIES(
${CmbBridgelFiles}
WRAP_EXCLUDE
)

set(_module_src
  ${CmbBridgelFiles}
  ${CmbBridgelServerFiles}
)
set(vtkCMBModelBridge_NO_HeaderTest 1)
vtk_module_library(vtkCMBModelBridge ${_module_src})

target_link_libraries(vtkCMBModelBridge
                      LINK_PUBLIC
                      SMTKCore
                      vtkSMTK
                      )

target_include_directories(vtkCMBModelBridge
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${SMTK_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}
  )

if(BUILD_TESTING)
#  add_subdirectory(Testing)
endif(BUILD_TESTING)

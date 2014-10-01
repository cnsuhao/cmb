project(ModelBridge)

find_package(ParaView REQUIRED)
find_package(SMTK REQUIRED)

set(CMAKE_MODULE_PATH "${SMTK_ROOT_DIR}/lib/cmake/SMTK" ${CMAKE_MODULE_PATH})
include (ParaViewPlugins)

SET(CmbBridgelFiles
  vtkSMModelManagerProxy.cxx
  vtkPVSMTKModelInformation.cxx
)

SET(CmbBridgelServerFiles
  vtkModelManagerWrapper.cxx
)

SET_SOURCE_FILES_PROPERTIES(
cmbForwardingBridge.cxx
WRAP_EXCLUDE
)

add_paraview_plugin(ModelBridge_Plugin "1.0"
  SERVER_MANAGER_XML smconfigModelBridge.xml
  SERVER_MANAGER_SOURCES
    ${CmbBridgelServerFiles}
    ${CmbBridgelFiles}
  GUI_SOURCES
    cmbForwardingBridge.cxx
)

target_link_libraries(ModelBridge_Plugin
  LINK_PRIVATE
    SMTKCore
    vtkSMTK
    smtkCMBBridge
  )

target_include_directories(ModelBridge_Plugin
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${SMTK_INCLUDE_DIRS}
  )

#now that the target has been created we can do the header generation
generate_export_header(ModelBridge_Plugin BASE_NAME ModelBridgeClient
                                   EXPORT_FILE_NAME ModelBridgeClientModule.h)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/ModelBridgeClientModule.h DESTINATION lib)


cmb_install_plugin(ModelBridge_Plugin)

if(BUILD_TESTING)
#  add_subdirectory(Testing)
endif(BUILD_TESTING)

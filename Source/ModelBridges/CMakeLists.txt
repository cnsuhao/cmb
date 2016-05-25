project(ModelBridge)

find_package(ParaView REQUIRED)
find_package(SMTK REQUIRED)

set(CMAKE_MODULE_PATH "${SMTK_ROOT_DIR}/lib/cmake/SMTK" ${CMAKE_MODULE_PATH})
include (ParaViewPlugins)

SET(CmbBridgelFiles
  vtkSMModelManagerProxy.cxx
  vtkPVSMTKModelInformation.cxx
  vtkPVSMTKMeshInformation.cxx
)

SET(CmbBridgelServerFiles
  vtkModelManagerWrapper.cxx
  vtkPVSMTKModelSource.cxx
  vtkPythonExporter.cxx
  vtkSMTKModelFieldArrayFilter.cxx
  vtkPVSMTKMeshSource.cxx
)

SET_SOURCE_FILES_PROPERTIES(
  cmbForwardingSession.cxx
  WRAP_EXCLUDE
)

# we unset the pv-plugin variable because that will bring in stuff
# that we don't want for client-server library initialization of this plugin
unset(pv-plugin)
add_paraview_plugin(ModelBridge_Plugin "1.0"
  SERVER_MANAGER_XML smconfigModelBridge.xml
  SERVER_MANAGER_SOURCES
    ${CmbBridgelServerFiles}
    ${CmbBridgelFiles}
  GUI_SOURCES
    cmbForwardingSession.cxx
)

target_link_libraries(ModelBridge_Plugin
  LINK_PRIVATE
    smtkCore
    vtkSMTKSourceExt
    vtkSMTKOperatorsExt
  )

target_include_directories(ModelBridge_Plugin
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${SMTK_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
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

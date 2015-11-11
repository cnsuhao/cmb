
if(PARAVIEW_BUILD_QT_GUI)
  include(${QT_USE_FILE})
endif(PARAVIEW_BUILD_QT_GUI)

#include the export header generater since we need to manually export
#some classes so they can be used in the applications
include(GenerateExportHeader)

set(vtkCMBClient_INCLUDE_DIRS
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  )

include_directories(
  ${vtkCMBClient_INCLUDE_DIRS}
  ${vtkCMBGeneral_INCLUDE_DIRS}
  ${vtkCMBFiltering_INCLUDE_DIRS}
  ${vtkCMBGraphics_INCLUDE_DIRS}
  ${vtkCMBMeshing_INCLUDE_DIRS}
  ${vtkCMBIO_INCLUDE_DIRS}
  )

set(PLUGIN_SRC
    vtkCMBArcAutoConnectClientOperator.cxx
    vtkCMBArcCreateClientOperator.cxx
    vtkCMBArcDeleteClientOperator.cxx
    vtkCMBArcEditClientOperator.cxx
    vtkCMBArcFindPickPointOperator.cxx
    vtkCMBArcGrowClientOperator.cxx
    vtkCMBArcMergeArcsClientOperator.cxx
    vtkCMBArcPolygonCreateClientOperator.cxx
    vtkCMBArcSnapClientOperator.cxx
    vtkCMBArcUpdateAndSplitClientOperator.cxx
    vtkCMBSubArcModifyClientOperator.cxx
    vtkPVArcInfo.cxx
    vtkPVCMBSceneV2WriterInformation.cxx
    vtkPVContourGroupInformation.cxx
    vtkPVContourRepresentationInfo.cxx
    vtkPVLASOutputBlockInformation.cxx
    vtkPVMeshDataInformation.cxx
    vtkPVModelVertexObjectInformation.cxx
    vtkPVMultiBlockRootObjectInfo.cxx
    vtkPVSceneGenFileInformation.cxx
    vtkPVSceneGenObjectInformation.cxx
    vtkSMArcOperatorProxy.cxx
    vtkSMCMBConeCellClassifierProxy.cxx
    vtkSMCMBGlyphPointSourceProxy.cxx
    vtkSMMeshSourceProxy.cxx
    vtkSMSceneContourSourceProxy.cxx
)

# objects panels
qt4_wrap_ui(UI_BUILT_SRCS
  pqBandedPolyDataContourControls.ui
  qtPlanarTextureRegistrationDialog.ui
)

#we can't use automoc since paraview plugin macro generated headers
#will be mocced twice
qt4_wrap_cpp(MOC_SRCS
  pqBandedPolyDataContourPanel.h
  pqCMBTabWidgetTranslator.h
  pqCMBMeshReaderPanel.h
  pqCMBPluginStarter.h
  pqCMBStreamTracerPanel.h
  pqGeneralTransferFunctionWidget.h
  pqPlanarTextureRegistrationDialog.h
  pqPlanarTextureRegistrationPanel.h
)

# We need this to call "vtkVTKExtensionsCS_Initialize(pm->GetInterpreter())"
# so that the vtkProcessModule's interpreter is exposed to these classes
# NOTE: This will not work in real cliet-server mode. According to Utkarsh,
# more magic needs to happen for client-server mode to work
#
add_paraview_auto_start(IFACES1 IFACE_SRCS1 CLASS_NAME pqCMBPluginStarter
                        STARTUP onStartup
                        SHUTDOWN onShutdown)

add_paraview_object_panel(IFACES2 IFACE_SRCS2
  CLASS_NAME pqCMBMeshReaderPanel
  XML_NAME CMBMeshReader
  XML_GROUP sources
)
add_paraview_object_panel(IFACES3 IFACE_SRCS3
  CLASS_NAME pqCMBStreamTracerPanel
  XML_NAME CMBStreamTracer
  XML_GROUP filters
)
add_paraview_object_panel(IFACES4 IFACE_SRCS4
  CLASS_NAME pqBandedPolyDataContourPanel
  XML_NAME BandedPolyDataContourFilter
  XML_GROUP filters
)
add_paraview_object_panel(IFACES5 IFACE_SRCS5
  CLASS_NAME pqPlanarTextureRegistrationPanel
  XML_NAME RegisterPlanarTextureMapFilter
  XML_GROUP filters
)

set(PANEL_SRC
  pqBandedPolyDataContourPanel.cxx
  pqCMBTabWidgetTranslator.cxx
  pqCMBMeshReaderPanel.cxx
  pqCMBStreamTracerPanel.cxx
  pqGeneralTransferFunctionWidget.cxx
  pqPlanarTextureRegistrationDialog.cxx
  pqPlanarTextureRegistrationPanel.cxx
)

add_paraview_plugin(CMB_Plugin "1.0"
  SERVER_MANAGER_XML
    smconfig.xml
  SERVER_MANAGER_SOURCES
    ${PLUGIN_SRC}
  GUI_INTERFACES
    ${IFACES1}
    ${IFACES2}
    ${IFACES3}
    ${IFACES4}
    ${IFACES5}
  GUI_SOURCES
    pqCMBPluginStarter.cxx
    ${UI_BUILT_SRCS}
    ${MOC_SRCS}
    ${IFACE_SRCS1}
    ${IFACE_SRCS2}
    ${IFACE_SRCS3}
    ${IFACE_SRCS4}
    ${IFACE_SRCS5}
    ${PANEL_SRC}
  CS_KITS
    vtkSMTKDiscreteMeshingExt
    vtkSMTKDiscreteReaderExt
)

target_link_libraries(CMB_Plugin
  PRIVATE
    vtkCMBGraphics
    vtkSMTKDiscreteMeshingExtCS-6.3
    vtkSMTKDiscreteReaderExtCS-6.3)

#now that the target has been created we can do the header generation
generate_export_header(CMB_Plugin BASE_NAME vtkCMBClient
                                   EXPORT_FILE_NAME vtkCMBClientModule.h)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/vtkCMBClientModule.h DESTINATION lib)

cmb_install_plugin(CMB_Plugin)

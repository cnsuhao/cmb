project(KMLExporter)
set(Project_USE_PARAVIEW ON CACHE BOOL "Use ParaView and ParaView VTK")

# ParaView is needed.
if(ParaView_SOURCE_DIR)
  include_directories(
    ${PARAVIEW_INCLUDE_DIRS}
    ${PARAVIEW_GUI_INCLUDE_DIRS}
    ${PARAVIEW_KWSYS_INCLUDE_DIRS}
    ${VTK_INCLUDE_DIR}
  )
else (ParaView_SOURCE_DIR)
  find_package(ParaView REQUIRED)
  include(${PARAVIEW_USE_FILE})
endif (ParaView_SOURCE_DIR)

if(PARAVIEW_BUILD_QT_GUI)
  include(${QT_USE_FILE})
endif(PARAVIEW_BUILD_QT_GUI)

# KML is needed.
find_package(KML)
include_directories(${KML_INCLUDE_DIRS})


# Include all the required paths.
include_directories(${KML_INCLUDE_DIRS}
                    ${ParaViewExtensions_SOURCE_DIR}/Server
                    ${ParaViewExtensions_SOURCE_DIR}/Plugins
                    ${vtkCMBGeneral_INCLUDE_DIRS}
                    ${vtkCMBIO_INCLUDE_DIRS}
                    )

# Fill in the macro to build a paraview plugin.
set(SM_XML vtkKMLExporter.xml
           vtkSMAnimationSceneKMLWriter.xml
)

set(SM_SRCS
  ${ParaViewExtensions_SOURCE_DIR}/Server/vtkSMAnimationSceneKMLWriter.cxx
)

qt4_wrap_cpp(MOC_SRCS
  ${ParaViewExtensions_SOURCE_DIR}/Plugins/KMLToolbar.h
)

qt4_wrap_ui(UI_SRCS
  ${ParaViewExtensions_SOURCE_DIR}/Plugins/KMLAnimationSettings.ui
)

add_paraview_action_group(IFACES IFACE_SRCS
                          CLASS_NAME KMLToolbar
                          GROUP_NAME "MenuBar/KML"
)

add_paraview_plugin(KMLExporter_Plugin "1.0"
  SERVER_MANAGER_XML
    ${SM_XML}
  SERVER_MANAGER_SOURCES
    ${SM_SRCS}
  GUI_SOURCES
    ${UI_SRCS}
  GUI_INTERFACES
    ${IFACES}
  SOURCES
    ${MOC_SRCS}
    ${IFACE_SRCS}
    ${ParaViewExtensions_SOURCE_DIR}/Plugins/KMLToolbar.cxx
  CS_KITS
    vtkFiltersCore
    vtkIOCore
    vtkPVServerManagerCore
    vtkPVServerImplementationCore
    vtkCMBGeneral
    vtkCMBIO
)

cmb_install_plugin(KMLExporter_Plugin)

# Libraries requires for linking.
target_link_libraries(KMLExporter_Plugin
  LINK_PRIVATE
    ${KML_LIBRARIES}
    vtkCMBGeneral vtkCMBGeneralCS
    vtkCMBIO vtkCMBIOCS
    vtkFiltersCore vtkFiltersCoreCS
    vtkIOCore vtkIOCoreCS
    vtkPVServerManagerCore vtkPVServerManagerCoreCS
    vtkPVServerImplementationCore vtkPVServerImplementationCoreCS
    vtkPVVTKExtensionsCore
  )

# Disable the testing for now.
if(BUILD_TESTING)
  # add_subdirectory(Testing)
endif(BUILD_TESTING)



# Example plugin demonstrating how to add a dock panel to CMB.
# This plugin adds the panel ExampleDockPanel to the main window.

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

IF(NOT ParaView_BINARY_DIR)
  FIND_PACKAGE(ParaView REQUIRED)
  INCLUDE(${PARAVIEW_USE_FILE})
ENDIF(NOT ParaView_BINARY_DIR)

IF(PARAVIEW_BUILD_QT_GUI)
  QT4_WRAP_CPP(MOC_SRCS ExampleDockPanel.h)
  QT4_WRAP_UI(UI_SRCS ExampleDockPanel.ui)

  ADD_PARAVIEW_DOCK_WINDOW(
    OUTIFACES
    OUTSRCS
    CLASS_NAME ExampleDockPanel
    DOCK_AREA Right)

  ADD_PARAVIEW_PLUGIN(ExampleDockPanel "1.0"
                      GUI_INTERFACES ${OUTIFACES}
                      GUI_SOURCES ${OUTSRCS} ${MOC_SRCS} ${UI_SRCS} ExampleDockPanel.cxx)


  target_link_libraries(ExampleDockPanel
    LINK_PUBLIC
      vtkPVServerManagerApplication
      pqApplicationComponents
    LINK_PRIVATE
      vtkPVServerManagerApplicationCS
      vtkRenderingFreeType
      vtkRenderingFreeType${VTK_RENDERING_BACKEND}
  )

  cmb_install_plugin(ExampleDockPanel)
endif()
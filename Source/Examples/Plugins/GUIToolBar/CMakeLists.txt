
# create a plugin that adds a toolbar to CMB main window

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

IF(NOT ParaView_BINARY_DIR)
  FIND_PACKAGE(ParaView REQUIRED)
  INCLUDE(${PARAVIEW_USE_FILE})
ENDIF(NOT ParaView_BINARY_DIR)

IF(PARAVIEW_BUILD_QT_GUI)
  QT4_WRAP_CPP(MOC_SRCS MyToolBarActions.h)

  # we implement a pqConePanel.h for the ConeSource
  ADD_PARAVIEW_ACTION_GROUP(IFACES IFACE_SRCS CLASS_NAME MyToolBarActions
                            GROUP_NAME "ToolBar/MyActions")

  # create a plugin for this panel
  ADD_PARAVIEW_PLUGIN(GUIMyToolBar "1.0"
                      GUI_INTERFACES ${IFACES}
                      SOURCES MyToolBarActions.cxx ${MOC_SRCS} ${IFACE_SRCS})

  target_link_libraries(GUIMyToolBar
    LINK_PUBLIC
      vtkPVServerManagerApplication
      pqApplicationComponents
    LINK_PRIVATE
      vtkPVServerManagerApplicationCS
      vtkInteractionStyle
      vtkRendering${VTK_RENDERING_BACKEND}
      vtkRenderingFreeType
  )

  cmb_install_plugin(GUIMyToolBar)
ENDIF()

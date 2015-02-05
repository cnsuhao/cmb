cmake_minimum_required(VERSION 2.8.8 FATAL_ERROR)
project(PluginExamples)

IF(NOT ParaView_BINARY_DIR)
  FIND_PACKAGE(ParaView REQUIRED)
  INCLUDE(${PARAVIEW_USE_FILE})
ENDIF(NOT ParaView_BINARY_DIR)

# we unset the pv-plugin variable because that will bring in stuff
# which we don't want brought in for client-server library initialization
  unset(pv-plugin)

  add_subdirectory(Plugins/DockWidget)
  add_subdirectory(Plugins/GUIToolBar)
  add_subdirectory(Plugins/MeshReader)
  add_subdirectory(Plugins/PointsReader)

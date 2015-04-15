
# create a plugin that adds ".cmbmesh" readers to the CMB GUI
# The extension is added in the file dialog when doing file opens.

#--------------------------------------------------
# Find and Use ParaView
#--------------------------------------------------
IF (ParaView_SOURCE_DIR)
  INCLUDE_DIRECTORIES(
    ${VTK_INCLUDE_DIRS}
    )
ELSE (ParaView_SOURCE_DIR)
  FIND_PACKAGE(ParaView REQUIRED)
  INCLUDE(${PARAVIEW_USE_FILE})
ENDIF (ParaView_SOURCE_DIR)

set(depend_LIBRARIES
    vtkCommonCore
    vtkCommonDataModel
    vtkCommonExecutionModel
    vtkFiltersCore
    vtksys
    vtkInteractionStyle
    vtkRendering${VTK_RENDERING_BACKEND}
    vtkRenderingFreeType
    vtkRenderingFreeType${VTK_RENDERING_BACKEND}
)

ADD_PARAVIEW_PLUGIN(cmbMeshReaderPlugin "1.0"
  SERVER_MANAGER_SOURCES vtkCMBMeshReader.cxx
  SERVER_MANAGER_XML     CMBMeshReaderSM.xml
  REQUIRED_ON_SERVER)

target_link_libraries(cmbMeshReaderPlugin
  LINK_PRIVATE ${depend_LIBRARIES})

cmb_install_plugin(cmbMeshReaderPlugin)

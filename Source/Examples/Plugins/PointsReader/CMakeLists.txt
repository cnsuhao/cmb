
# create a plugin that adds ".cmbpts" readers to the CMB GUI
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

vtk_module_load(vtkGeovisCore)
INCLUDE_DIRECTORIES(${vtkGeovisCore_INCLUDE_DIRS})
vtk_module_load(vtkInfovisLayout)
INCLUDE_DIRECTORIES(${vtkInfovisLayout_INCLUDE_DIRS})

set(depend_LIBRARIES
    vtkCommonCore
    vtkCommonDataModel
    vtkCommonExecutionModel
    vtkGeovisCore
    vtkInfovisLayout
    vtksys
    vtkInteractionStyle
    vtkRendering${VTK_RENDERING_BACKEND}
    vtkRenderingFreeType
    vtkRenderingFreeType${VTK_RENDERING_BACKEND}
)

ADD_PARAVIEW_PLUGIN(cmbPointsReaderPlugin "1.0"
  SERVER_MANAGER_SOURCES vtkCMBPointsReader.cxx
  SERVER_MANAGER_XML     CMBPointsReaderSM.xml
  REQUIRED_ON_SERVER)

target_link_libraries(cmbPointsReaderPlugin
  LINK_PRIVATE ${depend_LIBRARIES})

cmb_install_plugin(cmbPointsReaderPlugin)

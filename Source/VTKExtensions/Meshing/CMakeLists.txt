
#SMTK and Remus are needed
find_package(SMTK REQUIRED)
find_package(Remus REQUIRED)

set(CMB_Meshing_SRC
    vtkCMBArcPolygonProvider.cxx
    vtkCMBMeshTerrainWithArcs.cxx
    vtkCMBPolygonFromArcsOperator.cxx
    vtkCMBSceneGenPolygon.cxx
    vtkCMBTriangleMultiBlockMesher.cxx
    vtkLoopsMesher.cxx
    vtkMeshModelEdgesFilter.cxx
    )

if(OMICRON_FOUND AND BUILD_OMICRON_MESH_WORKER)
  list(APPEND CMB_Meshing_SRC
      vtkMultiLayerTINStitcher.cxx
      vtkTINStitcher.cxx
      )
endif()

set(Meshing_Libs)
if(OMICRON_FOUND AND BUILD_OMICRON_MESH_WORKER)
  list(APPEND Meshing_Libs
       omicron
       triangle_omicron
       tl_alloc
       )
endif()


set(vtkCMBMeshing_NO_HeaderTest 1)
vtk_module_library(vtkCMBMeshing ${CMB_Meshing_SRC})

#we link against smtkCore so that we get the proper include directories
#for smtk, as those aren't exported with vtkSMTKDiscreteExt
target_link_libraries(vtkCMBMeshing
                      LINK_PRIVATE
                      RemusClient
                      RemusServer
                      smtkCore
                      vtkSMTKMeshingExt
                      ${Meshing_Libs})

target_include_directories(vtkCMBMeshing
                           PRIVATE
                           ${REMUS_INCLUDE_DIRS}
                           ${SMTK_INCLUDE_DIRS}/extension/vtk/meshing/
                           )

if(OMICRON_FOUND AND BUILD_OMICRON_MESH_WORKER)
  target_include_directories(vtkCMBMeshing PRIVATE ${OMICRON_INCLUDE_DIRS})
endif()

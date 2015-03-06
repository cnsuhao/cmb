
#Remus is needed
find_package(Remus REQUIRED)

#for vtkCMBMeshServerLauncher we need the core foundation libraries, so that we
#properly find where the bundle we are installed into is located
if(APPLE)
  find_library(COREFOUNDATION_LIBRARY CoreFoundation )
endif()

set(CMB_Meshing_SRC
    cmbFaceMesherInterface.cxx
    cmbFaceMeshHelper.cxx
    vtkCMBArcPolygonProvider.cxx
    vtkCMBPrepareForTriangleMesher.cxx
    vtkCMBMeshServerLauncher.cxx
    vtkCMBMeshTerrainWithArcs.cxx
    vtkCMBPolygonFromArcsOperator.cxx
    vtkCMBSceneGenPolygon.cxx
    vtkCMBTriangleMesher.cxx
    vtkCMBTriangleMultiBlockMesher.cxx
    vtkDiscoverRegions.cxx
    vtkLoopsMesher.cxx
    vtkPolylineTriangulator.cxx
    vtkMeshModelEdgesFilter.cxx
    vtkDEMToMesh.cxx
    vtkRayIntersectionLocator.cxx
    vtkRegionsToLoops.cxx
    vtkSplitPlanarLines.cxx
    )

if(BUILD_MESH_WORKERS)
  list(APPEND CMB_Meshing_SRC
      vtkMultiLayerTINStitcher.cxx
      vtkTINStitcher.cxx
      )
endif()

set_source_files_properties(
   cmbFaceMesherInterface.cxx
   cmbFaceMeshHelper.cxx
   WRAP_EXCLUDE
 )

set(Meshing_Libs)
if(BUILD_MESH_WORKERS)
  list(APPEND Meshing_Libs
       omicron
       triangle_omicron
       tl_alloc
       )
endif()


set(vtkCMBMeshing_NO_HeaderTest 1)
vtk_module_library(vtkCMBMeshing ${CMB_Meshing_SRC})

target_link_libraries(vtkCMBMeshing
                      LINK_PRIVATE RemusClient RemusServer ${Meshing_Libs})

if(APPLE)
  #we link privately since nobody else needs to know we link to CoreFoundation
  target_link_libraries(vtkCMBMeshing LINK_PRIVATE ${COREFOUNDATION_LIBRARY})
endif()

target_include_directories(vtkCMBMeshing PRIVATE ${REMUS_INCLUDE_DIRS})
if(BUILD_MESH_WORKERS)
  target_include_directories(vtkCMBMeshing PRIVATE ${Omicron_INCLUDE_DIRS})
endif()

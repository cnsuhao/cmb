# Set project name.
project(CMBMeshOmicronWorker)

set(Surface_Worker
  OmicronWorker.cxx
  SurfaceWorker.cxx
  )

set(Volume_Worker
  OmicronWorker.cxx
  VolumeWorker.cxx
  )

if(APPLE)
  find_library(COREFOUNDATION_LIBRARY CoreFoundation )
endif()

add_executable(CMBMeshOmicronSurfaceWorker ${Surface_Worker})
target_link_libraries(CMBMeshOmicronSurfaceWorker
                      LINK_PRIVATE
                      RemusWorker
                      RemusCommon
                      ${Boost_LIBRARIES}
                      )

Register_Mesh_Worker(CMBMeshOmicronSurfaceWorker
                     INPUT_TYPE "SceneFile"
                     OUTPUT_TYPE "Mesh3DSurface"
                     )

add_executable(CMBMeshOmicronModelWorker ${Volume_Worker})
target_link_libraries(CMBMeshOmicronModelWorker
                      LINK_PRIVATE
                      RemusWorker
                      RemusCommon
                      ${Boost_LIBRARIES}
                      vtksys
                      )

Register_Mesh_Worker(CMBMeshOmicronModelWorker
                     INPUT_TYPE "SceneFile"
                     OUTPUT_TYPE "Mesh3D")

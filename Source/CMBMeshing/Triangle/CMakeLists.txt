# Set project name.
project(CMBMeshTriangleWorker)

find_package(Triangle REQUIRED)

set(Triangle_Worker
  TriangleWorker.cxx
  )

add_executable(CMBMeshTriangleWorker ${Triangle_Worker})

target_include_directories(CMBMeshTriangleWorker
                           PRIVATE ${TRIANGLE_INCLUDE_DIRS} )

target_link_libraries(CMBMeshTriangleWorker
                      LINK_PRIVATE
                      RemusWorker
                      RemusCommon
                      ${TRIANGLE_LIBRARIES}
                      ${Boost_LIBRARIES}
                      )

Register_Mesh_Worker(CMBMeshTriangleWorker
                     INPUT_TYPE "Edges"
                     OUTPUT_TYPE "Mesh2D"
                     )

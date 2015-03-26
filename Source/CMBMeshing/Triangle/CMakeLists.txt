# Set project name.
project(CMBMeshTriangleWorker)

set(USE_TL_DEBUG OFF) #we set this to off since tl_free_debug is buggy.
find_package(Omicron REQURIED)

#need to include the omicron directories in this exact order
#so that the proper share_declare.h and triangle.h are found
include_directories(
  ${Omicron_INCLUDE_DIRS}
  )

set(Triangle_Worker
  TriangleWorker.cxx
  )

add_executable(CMBMeshTriangleWorker ${Triangle_Worker})
#have to link omicron to get the helper methods for the triangle
#alloc and free triangle functions
target_link_libraries(CMBMeshTriangleWorker
                      LINK_PRIVATE
                      RemusWorker
                      RemusCommon
                      tl_alloc
                      triangle_omicron
                      omicron
                      ${Boost_LIBRARIES}
                      )

Register_Mesh_Worker(CMBMeshTriangleWorker
                     INPUT_TYPE "Edges"
                     OUTPUT_TYPE "Mesh2D"
                     )

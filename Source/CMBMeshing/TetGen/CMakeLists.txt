# Set project name.
project(CMBMeshTetGenWorker)

find_package(TetGen REQUIRED)

find_package(SMTK REQUIRED)

if(APPLE)
  find_library(COREFOUNDATION_LIBRARY CoreFoundation )
endif()


set(TetGen_Worker
  TetGenMain.cxx
  TetGenWorker.cxx
  )


add_executable(CMBMeshTetGenWorker ${TetGen_Worker})
target_link_libraries(CMBMeshTetGenWorker
                      LINK_PRIVATE
                      RemusWorker
                      RemusCommon
                      smtkCore
                      vtkCommonCore
                      vtksys
                      ${Boost_LIBRARIES}
                      ${TETGEN_LIBRARIES}
                      ${COREFOUNDATION_LIBRARY}
                      )

target_include_directories(CMBMeshTetGenWorker
                          PRIVATE
                          ${TETGEN_INCLUDE_DIRS}
                          ${SMTK_INCLUDE_DIRS}
                          )

Register_Mesh_Worker(CMBMeshTetGenWorker
                    INPUT_TYPE "Mesh3D"
                    OUTPUT_TYPE "Model"
                    FILE_TYPE "XML"
                    FILE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMBMeshTetGenWorkerInput.xml")

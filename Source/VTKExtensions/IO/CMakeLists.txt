
# SMTK, KML, and GDAL is needed.
find_package(SMTK REQUIRED)
find_package(KML)
find_package(GDAL)

set(CMB_IO_SRC
    vtkCMBADHReader.cxx
    vtkCMBBorFileReader.cxx
    vtkCMBICMReader.cxx
    vtkCMBProjectManagerReader.cxx
    vtkCMBProjectManagerWriter.cxx
    vtkCMBPt123ElementVelocityConversionWriter.cxx
    vtkCMBPt123PointsWriter.cxx
    vtkCMBPt123Reader.cxx
    vtkCMBPt123VelocityConversionWriter.cxx
    vtkCMBPt123VelocityWriter.cxx
    vtkCMBPtsWriter.cxx
    vtkCMBSceneV2WriterHelper.cxx
    vtkCMBWriter.cxx
    vtkDelosMeshReader.cxx
    vtkADHHotStartWriter.cxx
    vtkCMBMeshWriter.cxx
    vtkCMBPolyReader.cxx
    vtkGAMBITWriter.cxx
    vtkLIDARPtsWriter.cxx
    vtkOmicronMeshInputWriter.cxx
    vtkOmicronModelInputReader.cxx
    vtkOSDLReader.cxx
    vtkRawDEMReader.cxx
    vtkSceneGenV2ContourWriter.cxx
    vtkSceneGenVegetationClusterReader.cxx
    vtkSceneGenVegetationReader.cxx
    vtkSGXMLBCSWriter.cxx
    vtkStringReader.cxx
    vtkStringWriter.cxx
    vtkTriangulateConcavePolysFilter.cxx
)

set(vtkCMBIO_NO_HeaderTest 1)
vtk_module_library(vtkCMBIO ${CMB_IO_SRC})

#we link against smtkCore so that we get the proper include directories
#for smtk, as those aren't exported with vtkSMTKDiscreteExt
target_link_libraries(vtkCMBIO
                      LINK_PRIVATE
                      smtkCore
                      vtkSMTKReaderExt
                      vtkSMTKMeshingExt
                      ${GDAL_LIBRARY}
                      ${KML_LIBRARIES})


target_include_directories(vtkCMBIO
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
  PRIVATE ${KML_INCLUDE_DIRS}
  )

add_executable(meshingReader
  meshingReader.cxx
  )

target_link_libraries(meshingReader
  vtkCMBMeshing
  vtkCMBIO
  vtkFiltersGeneral
  )


# SMTK, KML, and GDAL is needed.
find_package(SMTK REQUIRED)
find_package(KML)
find_package(GDAL)

# Sources that should not get wrapped - for example methods that
# return std::string
set(UNWRAP_SRC vtkCMBReaderHelperFunctions.cxx)

set(CMB_IO_SRC
    ${UNWRAP_SRC}
    vtkCMBADHReader.cxx
    vtkCMBBorFileReader.cxx
    vtkCMBICMReader.cxx
    vtkCMBMapReader.cxx
    vtkCMBProjectManagerReader.cxx
    vtkCMBProjectManagerWriter.cxx
    vtkCMBPt123ElementVelocityConversionWriter.cxx
    vtkCMBPt123PointsWriter.cxx
    vtkCMBPt123Reader.cxx
    vtkCMBPt123VelocityConversionWriter.cxx
    vtkCMBPt123VelocityWriter.cxx
    vtkCMBPtsWriter.cxx
    vtkCMBSceneV2WriterHelper.cxx
    vtkCMBSTLReader.cxx
    vtkCMBWriter.cxx
    vtkCUBITReader.cxx
    vtkDelosMeshReader.cxx
    vtkCMBGeometryReader.cxx
    vtkCMBGeometry2DReader.cxx
    vtkADHHotStartWriter.cxx
    vtkCMBMeshReader.cxx
    vtkCMBMeshWriter.cxx
    vtkCMBPolyReader.cxx
    vtkExtractRegionEdges.cxx
    vtkGAMBITWriter.cxx
#    vtkGDALRasterReader.cxx # Extents for master
    vtkGMSSolidReader.cxx
    vtkGMSTINReader.cxx
#    vtkKMLExporter.cxx # vtkExporter for OpenGL2
    # vtkKMLReader.cxx
    vtkLASReader.cxx
    vtkLIDARPtsWriter.cxx
    vtkLIDARReader.cxx
    vtkOmicronMeshInputWriter.cxx
    vtkOmicronModelInputReader.cxx
    vtkOSDLReader.cxx
    vtkPolyFileReader.cxx
    vtkRawDEMReader.cxx
    vtkSceneGenV2ContourWriter.cxx
    vtkSceneGenVegetationClusterReader.cxx
    vtkSceneGenVegetationReader.cxx
    vtkSGXMLBCSWriter.cxx
    vtkStringReader.cxx
    vtkStringWriter.cxx
    vtkTriangulateConcavePolysFilter.cxx
)
set_source_files_properties(
  ${UNWRAP_SRC}
  WRAP_EXCLUDE
)

set(vtkCMBIO_NO_HeaderTest 1)
vtk_module_library(vtkCMBIO ${CMB_IO_SRC})

#we link against smtkCore so that we get the proper include directories
#for smtk, as those aren't exported with vtkSMTKDiscreteExt
target_link_libraries(vtkCMBIO
                      LINK_PRIVATE
                      smtkCore
                      vtkSMTKDiscreteReaderExt
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

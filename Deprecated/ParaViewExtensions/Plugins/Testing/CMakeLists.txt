add_executable(testKMLExporter testKMLExporter.cxx)
add_executable(testKMLExporterGeometry testKMLExporterGeometry.cxx)

target_link_libraries(testKMLExporter
  vtkCommonCore
  vtkIOCore
  KMLExporter_Plugin
  vtkCMBIO
  )

target_link_libraries(testKMLExporterGeometry
  vtkCommonCore
  vtkIOCore
  KMLExporter_Plugin
  vtkCMBIO
  )

add_short_test(TestKMLExporterImage
  ${CMAKE_COMMAND}
  -Dcfg=$<CONFIGURATION>
  -Dtest_exe=$<TARGET_FILE:testKMLExporter>
  -Doutput_base=${CMB_TEST_DATA_ROOT}/KMLExporter/testKMLExporter_img.kml
  -Doutput_test=${CMB_TEST_DIR}/testKMLExporter_img.kml
  -P ${CMAKE_CURRENT_SOURCE_DIR}/../CMake/run_test.cmake
  -Dtest_dir=${CMB_TEST_DIR}
)

add_short_test(TestKMLExporterGeometry
  ${CMAKE_COMMAND}
  -Dcfg=$<CONFIGURATION>
  -Dtest_exe=$<TARGET_FILE:testKMLExporterGeometry>
  -Doutput_base=${CMB_TEST_DATA_ROOT}/KMLExporter/testKMLExporterGeometry1_geom.kml
  -Doutput_test=${CMB_TEST_DIR}/testKMLExporterGeometry1_geom.kml
  -P ${CMAKE_CURRENT_SOURCE_DIR}/../CMake/run_test.cmake
  -Dtest_dir=${CMB_TEST_DIR}
)

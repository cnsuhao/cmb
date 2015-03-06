
# I am perfectly happy overlinking tests
set(testing_libraries
  vtkCMBFiltering
  vtkCMBGeneral
  vtkCMBGraphics
  vtkCMBIO
  vtkCMBMeshing
  vtkInteractionStyle
  vtkInteractionWidgets
  vtkRenderingCore
  vtkRenderingFreeType${VTK_RENDERING_BACKEND}
  vtkRendering${VTK_RENDERING_BACKEND}
  vtkTestingRendering
  vtksys
)

#since these tests are going through the vtk test harness, or being built
#as plugins to paraview they have none of the autoinit code that is required
#to properly setup the vtk render window.
#So to properly setup the render window we add compile definitions to all
#executables and libraries that are built in this directory so that they
#now what objects the factories need to load
set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS ${VTK_DEFINITIONS})

include_directories(${vtkCMBGeneral_INCLUDE_DIRS}
                    ${vtkCMBFiltering_INCLUDE_DIRS}
                    ${vtkCMBMeshing_INCLUDE_DIRS}
                    ${vtkCMBGraphics_INCLUDE_DIRS}
                    ${vtkCMBIO_INCLUDE_DIRS})

add_executable(testDiscreteColorLookupTable testDiscreteColorLookupTable.cxx)
add_executable(cmbGambitWriterTest cmbGambitWriterTest.cxx)
add_executable(cmbGambitWriterSolidTest cmbGambitWriterSolidTest.cxx)
add_executable(cmbTextureMappingTest cmbTextureMappingTest.cxx)
add_executable(cmbTextureMappingTest1 cmbTextureMappingTest1.cxx)
add_executable(cmbPolyReaderTest cmbPolyReaderTest.cxx)
add_executable(vtkCMBConeClassifierTest vtkCMBConeClassifierTest.cxx)
add_executable(vtkCMBConeClassifierTest1 vtkCMBConeClassifierTest1.cxx)
add_executable(vtkCMBConeClassifierTest2 vtkCMBConeClassifierTest2.cxx)
add_executable(vtkCMBGlyphMappingTest vtkCMBGlyphMappingTest.cxx)
add_executable(vtkCMBGlyphMappingTest1 vtkCMBGlyphMappingTest1.cxx)
add_executable(vtkCMBADHReaderTest vtkCMBADHReaderTest.cxx)
add_executable(vtkCMBICMReaderTest vtkCMBICMReaderTest.cxx)
add_executable(vtkCMBPt123ReaderTest vtkCMBPt123ReaderTest.cxx)
add_executable(testVTKObjectMethods testVTKObjectMethods.cxx)
add_executable(vtkCMBPt123VelocityWriterTest vtkCMBPt123VelocityWriterTest.cxx)
add_executable(vtkADHHotStartWriterTest vtkADHHotStartWriterTest.cxx)

target_link_libraries(testDiscreteColorLookupTable ${testing_libraries})

target_link_libraries(cmbTextureMappingTest ${testing_libraries})

target_link_libraries(cmbTextureMappingTest1 ${testing_libraries})

target_link_libraries(cmbPolyReaderTest ${testing_libraries})

target_link_libraries(vtkCMBConeClassifierTest ${testing_libraries})

target_link_libraries(vtkCMBConeClassifierTest1 ${testing_libraries})

target_link_libraries(vtkCMBConeClassifierTest2 ${testing_libraries})

target_link_libraries(vtkCMBGlyphMappingTest ${testing_libraries})

target_link_libraries(vtkCMBGlyphMappingTest1 ${testing_libraries})

target_link_libraries(vtkCMBADHReaderTest ${testing_libraries})

target_link_libraries(vtkCMBICMReaderTest ${testing_libraries})

target_link_libraries(vtkCMBPt123ReaderTest ${testing_libraries})

target_link_libraries(cmbGambitWriterTest ${testing_libraries})

target_link_libraries(cmbGambitWriterSolidTest ${testing_libraries})
target_link_libraries(testVTKObjectMethods ${testing_libraries})

target_link_libraries(vtkCMBPt123VelocityWriterTest ${testing_libraries})

target_link_libraries(vtkADHHotStartWriterTest ${testing_libraries})

if(BUILD_MESH_WORKERS)
  add_executable(vtkCMBMapReaderTest vtkCMBMapReaderTest.cxx)
  target_link_libraries(vtkCMBMapReaderTest ${testing_libraries})
endif(BUILD_MESH_WORKERS)


# utility to convert LIDAR data (also used in testing)
add_executable(LIDARConverter LIDARConverter.cxx)
target_link_libraries(LIDARConverter ${testing_libraries})
# not acually a test (as its name suggests), but could/should be made into one?
add_executable(delosMeshReader delosMeshReader.cxx)
target_link_libraries(delosMeshReader ${testing_libraries})

# not acually a test (as its name suggests), but could/should be made into one?
add_executable(testDataReader testDataReader.cxx)
target_link_libraries(testDataReader ${testing_libraries})

add_short_test(DiscreteColorLookupTableTest ${EXECUTABLE_OUTPUT_PATH}/testDiscreteColorLookupTable)

add_short_test(TestLIDARReaderPiece ${EXECUTABLE_OUTPUT_PATH}/LIDARConverter
        ${CMB_TEST_DATA_ROOT}/LIDAR/LIDARTest.pts
        ${CMB_TEST_DIR}/testSplit 3 1)

add_short_test(TestLIDARReaderAppend ${EXECUTABLE_OUTPUT_PATH}/LIDARConverter
        ${CMB_TEST_DATA_ROOT}/LIDAR/LIDARTest.pts
        ${CMB_TEST_DIR}/testAppend append 5)

add_short_test(TestLIDARReaderBinary ${EXECUTABLE_OUTPUT_PATH}/LIDARConverter
        ${CMB_TEST_DATA_ROOT}/LIDAR/smooth_surface.bin
        ${CMB_TEST_DIR}/testBinary append 2)

add_short_test(PrintAllExtensions
  ${EXECUTABLE_OUTPUT_PATH}/testVTKObjectMethods
  ${EXT_CLASS}
  )

add_short_test(cmbGambitWriterTest ${EXECUTABLE_OUTPUT_PATH}/cmbGambitWriterTest
  -D ${CMB_TEST_DATA_ROOT}
  -T ${CMB_TEST_DIR}
  -V Baseline/VTKExtensions/cmbGambitWriterTest.png
  )

add_short_test(cmbGambitWriterSolidTest ${EXECUTABLE_OUTPUT_PATH}/cmbGambitWriterSolidTest
  -D ${CMB_TEST_DATA_ROOT}
  -T ${CMB_TEST_DIR}
  -V Baseline/VTKExtensions/cmbGambitWriterSolidTest.png
  )

add_short_test(cmbTextureMappingTest ${EXECUTABLE_OUTPUT_PATH}/cmbTextureMappingTest
  -D ${CMB_TEST_DATA_ROOT}
  -T ${CMB_TEST_DIR}
  -V Baseline/VTKExtensions/cmbTextureMappingTest.png
  )

add_short_test(cmbTextureMappingTest1 ${EXECUTABLE_OUTPUT_PATH}/cmbTextureMappingTest1
  -D ${CMB_TEST_DATA_ROOT}
  -T ${CMB_TEST_DIR}
  -V Baseline/VTKExtensions/cmbTextureMappingTest1.png
  )

add_short_test(cmbPolyReaderTest ${EXECUTABLE_OUTPUT_PATH}/cmbPolyReaderTest
  -D ${CMB_TEST_DATA_ROOT}
  -T ${CMB_TEST_DIR}
  -V Baseline/VTKExtensions/cmbPolyReaderTest.png
  )

add_short_test(vtkCMBConeClassifierTest ${EXECUTABLE_OUTPUT_PATH}/vtkCMBConeClassifierTest
  -D ${CMB_TEST_DATA_ROOT}
  -T ${CMB_TEST_DIR}
  -V Baseline/VTKExtensions/vtkCMBConeClassifierTest.png
  )

add_short_test(vtkCMBConeClassifierTest1 ${EXECUTABLE_OUTPUT_PATH}/vtkCMBConeClassifierTest1
  -D ${CMB_TEST_DATA_ROOT}
  -T ${CMB_TEST_DIR}
  -V Baseline/VTKExtensions/vtkCMBConeClassifierTest1.png
  )

add_short_test(vtkCMBConeClassifierTest2 ${EXECUTABLE_OUTPUT_PATH}/vtkCMBConeClassifierTest2
  -D ${CMB_TEST_DATA_ROOT}
  -T ${CMB_TEST_DIR}
  -V Baseline/VTKExtensions/vtkCMBConeClassifierTest2.png
  )

add_medium_test(vtkCMBGlyphMappingTest ${EXECUTABLE_OUTPUT_PATH}/vtkCMBGlyphMappingTest
  -D ${CMB_TEST_DATA_ROOT}
  -T ${CMB_TEST_DIR}
  -V Baseline/VTKExtensions/vtkCMBGlyphMappingTest.png
  )

add_short_test(vtkCMBGlyphMappingTest1 ${EXECUTABLE_OUTPUT_PATH}/vtkCMBGlyphMappingTest1
  -D ${CMB_TEST_DATA_ROOT}
  -T ${CMB_TEST_DIR}
  -V Baseline/VTKExtensions/vtkCMBGlyphMappingTest1.png
  )

add_short_test(vtkCMBADHReaderTest ${EXECUTABLE_OUTPUT_PATH}/vtkCMBADHReaderTest
  ${CMB_TEST_DATA_ROOT}/bay.map
  -D ${CMB_TEST_DATA_ROOT}
  -T ${CMB_TEST_DIR}
  -V Baseline/VTKExtensions/vtkCMBADHReaderTest.png)

add_short_test(vtkCMBMapReaderTest ${EXECUTABLE_OUTPUT_PATH}/vtkCMBMapReaderTest
  ${CMB_TEST_DATA_ROOT}/bay.map
  -D ${CMB_TEST_DATA_ROOT}
  -T ${CMB_TEST_DIR}
  -V Baseline/VTKExtensions/vtkCMBMapReaderTest.png)

add_medium_test(vtkCMBICMReaderTest ${EXECUTABLE_OUTPUT_PATH}/vtkCMBICMReaderTest
  ${CMB_TEST_DATA_ROOT}
  -D ${CMB_TEST_DATA_ROOT}
  -T ${CMB_TEST_DIR}
  -V Baseline/VTKExtensions/vtkCMBICMReaderTest.png)

add_short_test(vtkCMBPt123ReaderTest ${EXECUTABLE_OUTPUT_PATH}/vtkCMBPt123ReaderTest
  ${CMB_TEST_DATA_ROOT}
  -D ${CMB_TEST_DATA_ROOT}
  -T ${CMB_TEST_DIR}
  -V Baseline/VTKExtensions/vtkCMBPt123ReaderTest.png)

add_executable(testSGXMLBCSWriter testSGXMLBCSWriter.cxx)
target_link_libraries(testSGXMLBCSWriter ${testing_libraries})

add_short_test(TestSGXMLBCSWriter ${EXECUTABLE_OUTPUT_PATH}/testSGXMLBCSWriter ${CMB_TEST_DIR})

add_short_test(TestSGXMLBCSWriterOutput ${CMAKE_COMMAND}
  -E compare_files
        ${CMB_TEST_DATA_ROOT}/test2dbcs.bcs
        ${CMB_TEST_DIR}/test2dbcs.bcs)

if(BUILD_MESH_WORKERS)
  include_directories(${Omicron_INCLUDE_DIRS})
  add_executable(vtkCMBTriangleMeshTest vtkCMBTriangleMeshTest.cxx)
  target_link_libraries(vtkCMBTriangleMeshTest ${testing_libraries})

  add_short_test(vtkCMBTriangleMeshTest ${EXECUTABLE_OUTPUT_PATH}/vtkCMBTriangleMeshTest
  ${CMB_TEST_DATA_ROOT}/bay.map
  -D ${CMB_TEST_DATA_ROOT}
  -T ${CMB_TEST_DIR}
  -V Baseline/VTKExtensions/vtkCMBTriangleMeshTest.png)
endif(BUILD_MESH_WORKERS)

add_short_test(vtkCMBPt123VelocityWriterTest ${EXECUTABLE_OUTPUT_PATH}/vtkCMBPt123VelocityWriterTest
  ${CMB_TEST_DATA_ROOT}
  -D ${CMB_TEST_DATA_ROOT}
  -T ${CMB_TEST_DIR}
  -V Baseline/VTKExtensions/vtkCMBPt123ReaderTest.png)

add_short_test(vtkADHHotStartWriterTest ${EXECUTABLE_OUTPUT_PATH}/vtkADHHotStartWriterTest
  ${CMB_TEST_DATA_ROOT}
  -D ${CMB_TEST_DATA_ROOT}
  -T ${CMB_TEST_DIR})

# Create a test list of all non graphical arc tests
create_test_sourcelist(CmbArcTestSources vtkCMBArcTests.cxx
  ArcServerAutoConnectTest1.cxx
  ArcServerAutoConnectTest2.cxx
  ArcServerCreateMoveTest.cxx
  ArcServerCreateOperatorTest.cxx
  ArcServerGrowTest.cxx
  ArcServerSplitTest1.cxx
  ArcServerSplitTest2.cxx
  ArcServerSplitTest3.cxx
  ArcServerUpdateTest1.cxx
  ArcServerUpdateTest2.cxx
  ArcServerUndoTest.cxx
  EXTRA_INCLUDE vtkTestDriver.h
)

if(BUILD_MESH_WORKERS)
  set(MESHVISTESTS
  ArcVisChesapeakeTest2.cxx
  ArcVisChesapeakeTest3.cxx
  ArcVisMeshTest1.cxx
  ArcVisMeshTest2.cxx
  )
endif(BUILD_MESH_WORKERS)

# Create a test list of all baseline arc tests
create_test_sourcelist(CmbArcVisTestSources vtkCMBArcVisTests.cxx
  ${MESHVISTESTS}
  ArcVisChesapeakeTest1.cxx
  ArcVisMergeTest1.cxx
  ArcVisMoveTest.cxx
  ArcVisSplitTest.cxx
  ArcVisUpdateTest1.cxx
  ArcVisUpdateTest2.cxx
  EXTRA_INCLUDE vtkTestDriver.h
)


add_executable(CmbArcTests ${CmbArcTestSources})
target_link_libraries(CmbArcTests ${testing_libraries})

SET(CmbArcTestSourcesToRun ${CmbArcTestSources})
REMOVE(CmbArcTestSourcesToRun vtkCMBArcTests.cxx)
FOREACH(test ${CmbArcTestSourcesToRun})
  GET_FILENAME_COMPONENT(TName ${test} NAME_WE)
  add_short_test(${TName} ${EXECUTABLE_OUTPUT_PATH}/CmbArcTests ${TName})
ENDFOREACH(test)

add_executable(CmbArcVisTests ${CmbArcVisTestSources})
target_link_libraries(CmbArcVisTests ${testing_libraries})

SET (CmbArcVisTestsToRun ${CmbArcVisTestSources})
REMOVE (CmbArcVisTestsToRun vtkCMBArcVisTests.cxx)
FOREACH (test ${CmbArcVisTestsToRun})
  GET_FILENAME_COMPONENT(TName ${test} NAME_WE)
  add_short_test(${TName} ${EXECUTABLE_OUTPUT_PATH}/CmbArcVisTests ${TName}
    -D ${CMB_TEST_DATA_ROOT}
    -T ${CMB_TEST_DIR}
    -V Baseline/VTKExtensions/${TName}.png)
ENDFOREACH (test)

#
# Test CMB ParaView plugin ... if we can find ParaView
#
# TODO/FIXME: This is currently unused for 2 reasons
#             (1) there is no possible way to run ParaView with "-dr" and have the plugin loaded
#             (2) there is no identification of the ParaView executable in ${ParaView_USE_FILE}
#
if(Q_WS_MAC)
  #set(PV_TEST_EXECUTABLE ${EXECUTABLE_OUTPUT_PATH}/SceneBuilder.app/Contents/MacOS/SceneBuilder --test-directory=${CMB_TEST_DIR})
  if ( EXISTS ${ParaView_DIR}/bin/paraview.app )
    set( PV_TEST_EXECUTABLE ${ParaView_DIR}/bin/paraview.app/Contents/MacOS/paraview )
  else ( EXISTS ${ParaView_DIR}/bin/paraview.app )
    set( PV_TEST_EXECUTABLE ${EXECUTABLE_OUTPUT_PATH}/paraview.app/Contents/MacOS/paraview )
  endif ( EXISTS ${ParaView_DIR}/bin/paraview.app )
else(Q_WS_MAC)
  #set(PV_TEST_EXECUTABLE ${EXECUTABLE_OUTPUT_PATH}/SceneBuilder --test-directory=${CMB_TEST_DIR})
  if ( EXISTS ${ParaView_DIR}/bin/paraview )
    set( PV_TEST_EXECUTABLE ${ParaView_DIR}/bin/paraview )
  else ( EXISTS ${ParaView_DIR}/bin/paraview )
    set( PV_TEST_EXECUTABLE ${EXECUTABLE_OUTPUT_PATH}/paraview )
  endif ( EXISTS ${ParaView_DIR}/bin/paraview )
endif(Q_WS_MAC)
if ( EXISTS ${PV_TEST_EXECUTABLE} )

  set( PV_TEST_XML_DIR ${CMAKE_CURRENT_SOURCE_DIR} )
  set( PV_TEST_IMG_DIR ${CMB_TEST_DATA_ROOT}/Baseline )

  # Tests that don't work on Apple
  set( APPLE_EXCLUDE_TESTS
  )

  set( TESTS_WITHOUT_BASELINES
  )

  set( XML_TESTS_WITH_BASELINES
    #cmbBandedPolyDataContourFilter
  )

  foreach( test ${TESTS_WITHOUT_BASELINES} )
    # Run the test if it is on the correct platform
    list( FIND APPLE_EXCLUDE_TESTS ${test} APPLE_FIND_RES )

    if ( NOT APPLE OR NOT APPLE_FIND_RES GREATER -1 )
      # FIXME: Running with -dr prevents plugin from loading, but might also cause invalid test images
      add_short_test( ${test}
        ${PV_TEST_EXECUTABLE}
          --test-directory=${CMB_TEST_DIR}
          --test-script=${PV_TEST_XML_DIR}/${test}.xml
          --exit
      )
      add_medium_timeout(${test})
    endif()
  endforeach(test)

  if ( CMB_TEST_DATA_ROOT )
    foreach( test ${XML_TESTS_WITH_BASELINES} )
      # Run the test if it is on the correct platform
      list( FIND APPLE_EXCLUDE_TESTS ${test} APPLE_FIND_RES )
      if ( NOT APPLE OR NOT APPLE_FIND_RES GREATER -1 )
        get_image_threshold_arg( THRESHOLD_CMD ${test} )
        # FIXME: Running with -dr prevents plugin from loading, but might also cause invalid test images
        add_short_test( ${test}
          ${PV_TEST_EXECUTABLE}
            --test-script=${PV_TEST_XML_DIR}/${test}.xml
            --test-directory=${CMB_TEST_DIR}
            --test-baseline=${PV_TEST_IMG_DIR}/${test}.png
            ${THRESHOLD_CMD}
            --exit
        )
      endif()
    endforeach(test)
  endif( CMB_TEST_DATA_ROOT )
endif ( EXISTS ${PV_TEST_EXECUTABLE} )


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
  vtkRenderingFreeType
  vtkRendering${VTK_RENDERING_BACKEND}
  vtkTestingRendering
  vtksys
  vtkSMTKReaderExt
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
add_executable(testVTKObjectMethods testVTKObjectMethods.cxx)

target_link_libraries(testDiscreteColorLookupTable ${testing_libraries})

target_link_libraries(testVTKObjectMethods ${testing_libraries})

# utility to convert LIDAR data (also used in testing)
add_executable(LIDARConverter LIDARConverter.cxx)
target_link_libraries(LIDARConverter ${testing_libraries})
# not acually a test (as its name suggests), but could/should be made into one?
add_executable(delosMeshReader delosMeshReader.cxx)
target_link_libraries(delosMeshReader ${testing_libraries})

# not acually a test (as its name suggests), but could/should be made into one?
add_executable(testDataReader testDataReader.cxx)
target_link_libraries(testDataReader ${testing_libraries})

add_short_test(DiscreteColorLookupTableTest testDiscreteColorLookupTable)

add_short_test(TestLIDARReaderPiece LIDARConverter
        ${CMB_TEST_DATA_ROOT}/LIDAR/LIDARTest.pts
        ${CMB_TEST_DIR}/testSplit 3 1)

add_short_test(TestLIDARReaderAppend LIDARConverter
        ${CMB_TEST_DATA_ROOT}/LIDAR/LIDARTest.pts
        ${CMB_TEST_DIR}/testAppend append 5)

add_short_test(TestLIDARReaderBinary LIDARConverter
        ${CMB_TEST_DATA_ROOT}/LIDAR/smooth_surface.bin
        ${CMB_TEST_DIR}/testBinary append 2)

add_short_test(PrintAllExtensions
  testVTKObjectMethods
  ${EXT_CLASS}
  )

# Create a test list of all non graphical arc tests
create_test_sourcelist(CmbArcTestSources
  CmbArcTestsDriver.cxx
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

add_executable(CmbArcTests ${CmbArcTestSources})
target_link_libraries(CmbArcTests ${testing_libraries})

list(REMOVE_AT CmbArcTestSources 0)

SET(CmbArcTestSourcesToRun ${CmbArcTestSources})
FOREACH(test ${CmbArcTestSourcesToRun})
  GET_FILENAME_COMPONENT(TName ${test} NAME_WE)
  add_short_test(${TName} CmbArcTests ${TName})
ENDFOREACH(test)

#
# Test CMB ParaView plugin ... if we can find ParaView
#
# TODO/FIXME: This is currently unused for 2 reasons
#             (1) there is no possible way to run ParaView with "-dr" and have the plugin loaded
#             (2) there is no identification of the ParaView executable in ${ParaView_USE_FILE}
#
if ( TARGET paraview )

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
        $<TARGET_FILE:paraview>
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
          $<TARGET_FILE:paraview>
            --test-script=${PV_TEST_XML_DIR}/${test}.xml
            --test-directory=${CMB_TEST_DIR}
            --test-baseline=${PV_TEST_IMG_DIR}/${test}.png
            ${THRESHOLD_CMD}
            --exit
        )
      endif()
    endforeach(test)
  endif( CMB_TEST_DATA_ROOT )
endif ()

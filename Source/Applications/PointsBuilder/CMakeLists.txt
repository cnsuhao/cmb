project (PointsBuilder)

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${cmbAppCommon_SOURCE_DIR}
  ${cmbAppCommon_SOURCE_DIR}/Scene
  ${cmbAppCommon_BINARY_DIR}
  ${test_source_directories}
  ${test_build_directories}
  )

# Add Qt UI files here
set(UI_FORMS
  qtLIDARPanel.ui
  qtFilterDialog.ui
  qtSaveScatterData.ui
  qtDEMExporter.ui
)

qt4_wrap_ui(UI_BUILT_SOURCES
  ${UI_FORMS}
  )

# Add Qt resource files here
set(UI_RESOURCES
  ../Resources/Resources.qrc
  )

qt4_add_resources(RCS_SOURCES
  ${UI_RESOURCES}
  )

if(WIN32)
  set(EXE_ICON ${CMAKE_CURRENT_SOURCE_DIR}/LIDARViewer.rc)
  set(EXE_RES ${CMAKE_CURRENT_BINARY_DIR}/LIDARViewer.res)
  add_custom_command(
    OUTPUT ${EXE_RES}
    COMMAND rc.exe
    ARGS /fo ${EXE_RES} ${EXE_ICON}
    )
endif(WIN32)

source_group("Resources" FILES
  ${UI_RESOURCES}
  ${UI_FORMS}
  ${EXE_ICON}
  )

source_group("Generated" FILES
  ${MOC_BUILT_SOURCES}
  ${RCS_SOURCES}
  ${UI_BUILT_SOURCES}
  )

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmbLIDARConfig.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/cmbLIDARConfig.h
  ESCAPE_QUOTES)


#we include headers, so that MSVC on "Go To Header File" context menu  works properly
set(PointsBuilder_SRCS
pqCMBPointsBuilderMainWindow.cxx
pqCMBPointsBuilderMainWindow.h
  pqCMBPointsBuilderMainWindowCore.cxx
pqCMBPointsBuilderMainWindowCore.h
  pqCMBLIDARTerrainExtractionManager.cxx
pqCMBLIDARTerrainExtractionManager.h
  qtCMBLIDARPanelWidget.cxx
qtCMBLIDARPanelWidget.h
  pqCMBLIDARReaderManager.cxx
pqCMBLIDARReaderManager.h
  qtCMBLIDARFilterDialog.cxx
qtCMBLIDARFilterDialog.h
  pqCMBLIDARSaveDialog.cxx
pqCMBLIDARSaveDialog.h
  pqCMBLIDARContourTree.cxx
pqCMBLIDARContourTree.h
  pqCMBContourTreeItem.cxx
pqCMBContourTreeItem.h
  pqDEMExporterDialog.cxx
pqDEMExporterDialog.h
)

set(PointsBuilder_ALL_SRCS
  ${PointsBuilder_SRCS}
  ${MOC_BUILT_SOURCES}
  ${RCS_SOURCES}
  ${UI_BUILT_SOURCES}
  ${EXE_RES}
  ${apple_bundle_sources}
  )

#------------------------------------------------------------------------------
# Build the client
build_paraview_client(PointsBuilder
  TITLE "Points Builder ${CMB_VERSION_MAJOR}.${CMB_VERSION_MINOR}.${CMB_VERSION_PATCH}"
  ORGANIZATION  "Kitware"
  VERSION_MAJOR ${CMB_VERSION_MAJOR}
  VERSION_MINOR ${CMB_VERSION_MINOR}
  VERSION_PATCH ${CMB_VERSION_PATCH}
  SPLASH_IMAGE "${CMAKE_CURRENT_SOURCE_DIR}/../Resources/Icons/PointsBuilderSplash.png"
  PVMAIN_WINDOW pqCMBPointsBuilderMainWindow
  PVMAIN_WINDOW_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/pqCMBPointsBuilderMainWindow.h
  BUNDLE_ICON   "${CMAKE_CURRENT_SOURCE_DIR}/MacIcon.icns"
  APPLICATION_ICON  "${CMAKE_CURRENT_SOURCE_DIR}/lidarviewerappico.ico"
  SOURCES ${PointsBuilder_ALL_SRCS}
  COMPRESSED_HELP_FILE "${CMAKE_CURRENT_BINARY_DIR}/../Help/cmbsuite.qch"
  INSTALL_BIN_DIR "${VTK_INSTALL_RUNTIME_DIR}"
  INSTALL_LIB_DIR "${VTK_INSTALL_LIBRARY_DIR}"
)

#let cmake do what qt4_wrap_cpp used to do automatically
set_target_properties(PointsBuilder PROPERTIES AUTOMOC TRUE)

#we need to explicitly state that you shouldn't build PointsBuilder
#before the help has been generated
add_dependencies(PointsBuilder CMBSuiteHelp)

target_link_libraries(PointsBuilder
LINK_PRIVATE
  cmbAppCommon
  )

#------------------------------------------------------------------------------
# For Macs, we add install rule to package everything that's built into a single
# App. Look at the explanation of MACOSX_APP_INSTALL_PREFIX in the top-level
# CMakeLists.txt file for details.
if (APPLE)
  # add install rules to generate the App bundle.
  install(CODE "
   include(\"${CMB_CMAKE_DIR}/CMBInstallApp.cmake\")

   #fillup bundle with all the libraries and plugins.
   cleanup_bundle(
     \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${VTK_INSTALL_RUNTIME_DIR}/PointsBuilder.app/Contents/MacOS/PointsBuilder\"
     \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${VTK_INSTALL_RUNTIME_DIR}/PointsBuilder.app\"
     \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${VTK_INSTALL_LIBRARY_DIR}\")

   # Place the App at the requested location.
   file(INSTALL DESTINATION \"${MACOSX_APP_INSTALL_PREFIX}\"
        TYPE DIRECTORY FILES
          \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${VTK_INSTALL_RUNTIME_DIR}/PointsBuilder.app\"
        USE_SOURCE_PERMISSIONS)
   "
   COMPONENT Runtime)
endif()

########################################################################
if(BUILD_TESTING)
  set(LIDARZoomToBox_THRESHOLD 14)
  set(LIDARDisplayProperties_THRESHOLD 14)
endif(BUILD_TESTING)

if(BUILD_TESTING)
  set(TEST_XML ${PointsBuilder_SOURCE_DIR}/Testing/XML)
  set(TEST_IMAGE ${CMB_TEST_DATA_ROOT}/Baseline)

  #Tests that don't work on Apple
  #
#    set (APPLE_EXCLUDE_TESTS
#     LIDARTerrainExtraction
#      LIDARTerrainExtraction2
#    LIDARThresholdFilter
#      )

  set (XML_TESTS_WITH_BASELINES
    # XXX: nodata(LIDARTest.pts): LIDAROpenFile
    # XXX: nodata(kandahar_tile10_crop_A_classified.las): LIDAROpenLAS
    # XXX: nodata(LIDARTest.pts): LIDARThresholdFilter
    # XXX: nodata(kandahar_tile10_crop_A_classified.las): LIDARElevationFilter
    # XXX: nodata(kandahar_tile10_crop_A_classified.las): LIDARDisplayProperties
    # XXX: nodata(LIDARTest.pts): LIDARSaveLoadContours
    # XXX: nodata(clipped5Subset.pts): LIDARTerrainExtraction
    # XXX: nodata(LIDARTest.pts): LIDARZoomToBox
    LIDARRotation
    MercedRiver
    TestFarPoints
    # XXX: nodata(LIDARTest.pts): SelectPieces
    # XXX: nodata(LIDARTest.pts): LIDARThresholdSave
    # XXX: nodata(clipped5Subset.pts): MultiLIDARFiles
    # XXX: nodata(LIDARTest.pts): AddContourIfFocusPointIsAtInfinity
    # XXX: nodata(JercSite_100x40_30.bin): EditPointsAddArc
    # XXX: nodata(JercSite_100x40_30.bin): EditPointsApplyArcAgain
    # XXX: nodata(JercSite_100x40_30.bin): EditPointsChangeDisplacementProfile
    # XXX: nodata(JercSite_100x40_30.bin): EditPointsChangeWeightingProfile
    # XXX: nodata(JercSite_100x40_30.bin): EditPointsCloseContour
    # XXX: nodata(JercSite_100x40_30.bin): EditPointsDatasetApply
    # XXX: nodata(JercSite_100x40_30.bin): EditPointsDeleteAllNodes
    # XXX: nodata(JercSite_100x40_30.bin): EditPointsDeleteLastAddedNode
    # XXX: nodata(JercSite_100x40_30.bin): EditPointsModifyArc
    # XXX: nodata(JercSite_100x40_30.bin): EditPointsMakeArcStraightLine
  )

  set (TESTS_WITHOUT_BASELINES
  AboutDialog
  # XXX: nodata(kandahar_tile10_crop_A_classified.las): LIDARTerrainExtraction2
  HelpDialog
  )

  foreach(test ${TESTS_WITHOUT_BASELINES})
    #Run the test if it is on the correct platform
    #
    list(FIND APPLE_EXCLUDE_TESTS ${test} APPLE_FIND_RES)

    if(NOT APPLE OR NOT APPLE_FIND_RES GREATER -1)
      add_long_test(PointsBuilder${test}
        PointsBuilder
        -dr
        --test-directory=${CMB_TEST_DIR}
        --test-script=${TEST_XML}/${test}.xml --exit)
    endif()
  endforeach(test)

  if(CMB_TEST_DATA_ROOT)
  foreach(test ${XML_TESTS_WITH_BASELINES})
    #If this is an apple machine and it is a
    #platform specific test add the Apple suffix
    #
    list(FIND APPLE_EXCLUDE_TESTS ${test} APPLE_FIND_RES)
    if(NOT APPLE OR NOT APPLE_FIND_RES GREATER -1)
      get_image_threshold_arg(THRESHOLD_CMD ${test})
      add_long_test(PointsBuilder${test}
        PointsBuilder
        -dr
        --test-directory=${CMB_TEST_DIR}
        --test-script=${TEST_XML}/${test}.xml
        --test-baseline=${TEST_IMAGE}/${test}.png
        ${THRESHOLD_CMD}
        --exit
        )
    endif()
  endforeach(test)
  endif(CMB_TEST_DATA_ROOT)
  add_subdirectory(Testing)
endif(BUILD_TESTING)

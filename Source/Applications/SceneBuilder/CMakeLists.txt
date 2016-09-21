project (SceneBuilder)

find_package(SMTK REQUIRED)
find_package(Remus REQUIRED)

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${cmbAppCommon_SOURCE_DIR}
  ${cmbAppCommon_BINARY_DIR}
  ${cmbAppCommon_SOURCE_DIR}/Scene
  ${SMTK_INCLUDE_DIR}
  )

# Add Qt UI files here
set(UI_FORMS
  qtCMBSceneBuilderPanel.ui
  qtSceneBuilderOptions.ui
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
  set(EXE_ICON ${CMAKE_CURRENT_SOURCE_DIR}/cmbSceneBuilder.rc)
  set(EXE_RES ${CMAKE_CURRENT_BINARY_DIR}/cmbSceneBuilder.res)
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
  ${RCS_SOURCES}
  ${UI_BUILT_SOURCES}
  )

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmbSceneBuilderConfig.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/cmbSceneBuilderConfig.h
  ESCAPE_QUOTES)

set(SceneBuilder_SOURCE_FILES
  pqCMBSceneBuilderMainWindowCore.cxx
  pqCMBSceneBuilderMainWindow.cxx
  qtCMBSceneBuilderPanelWidget.cxx
  qtCMBSceneBuilderContextMenuBehavior.cxx
  qtCMBSceneBuilderContextMenuBehavior.h
  qtSceneBuilderOptions.cxx
  qtSceneBuilderOptions.h

  ${RCS_SOURCES}
  ${UI_BUILT_SOURCES}
  ${EXE_RES}
  ${apple_bundle_sources}
  )

#------------------------------------------------------------------------------
# Build the client
build_paraview_client(SceneBuilder
  TITLE "Scene Builder ${CMB_VERSION_MAJOR}.${CMB_VERSION_MINOR}.${CMB_VERSION_PATCH}"
  ORGANIZATION  "Kitware"
  VERSION_MAJOR ${CMB_VERSION_MAJOR}
  VERSION_MINOR ${CMB_VERSION_MINOR}
  VERSION_PATCH ${CMB_VERSION_PATCH}
  SPLASH_IMAGE "${CMAKE_CURRENT_SOURCE_DIR}/../Resources/Icons/SceneBuilderSplash.png"
  PVMAIN_WINDOW pqCMBSceneBuilderMainWindow
  PVMAIN_WINDOW_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/pqCMBSceneBuilderMainWindow.h
  BUNDLE_ICON   "${CMAKE_CURRENT_SOURCE_DIR}/MacIcon.icns"
  APPLICATION_ICON  "${CMAKE_CURRENT_SOURCE_DIR}/scenebuilderappico.ico"
  SOURCES ${SceneBuilder_SOURCE_FILES}
  COMPRESSED_HELP_FILE "${CMAKE_CURRENT_BINARY_DIR}/../Help/cmbsuite.qch"
  INSTALL_BIN_DIR "${VTK_INSTALL_RUNTIME_DIR}"
  INSTALL_LIB_DIR "${VTK_INSTALL_LIBRARY_DIR}"
)

#let cmake do what qt4_wrap_cpp used to do automatically
set_target_properties(SceneBuilder PROPERTIES AUTOMOC TRUE)

#we need to explicitly state that you shouldn't build SceneBuilder
#before the help has been generated
add_dependencies(SceneBuilder CMBSuiteHelp)

target_link_libraries(SceneBuilder
LINK_PRIVATE
  #CMBModel_Plugin
  cmbAppCommon
  smtkDiscreteSession
  vtkSMTKDiscreteModel
  RemusClient
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
     \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${VTK_INSTALL_RUNTIME_DIR}/SceneBuilder.app/Contents/MacOS/SceneBuilder\"
     \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${VTK_INSTALL_RUNTIME_DIR}/SceneBuilder.app\"
     \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${VTK_INSTALL_LIBRARY_DIR}\")

   # Place the App at the requested location.
   file(INSTALL DESTINATION \"${MACOSX_APP_INSTALL_PREFIX}\"
        TYPE DIRECTORY FILES
          \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${VTK_INSTALL_RUNTIME_DIR}/SceneBuilder.app\"
        USE_SOURCE_PERMISSIONS)
   "
   COMPONENT Runtime)
endif()

########################################################################
if(BUILD_TESTING)
  set(ContourWidgetTest_THRESHOLD 14)
  set(CubeAxesTest_THRESHOLD 14)
  set(LightingTest_THRESHOLD 14)
  set(ObjectPropertiesTest_THRESHOLD 14)
endif(BUILD_TESTING)

if(BUILD_TESTING)
  set(TEST_XML ${SceneBuilder_SOURCE_DIR}/Testing/XML)
  set(TEST_IMAGE ${CMB_TEST_DATA_ROOT}/baseline/cmb_application)

  #Tests that don't work on Apple
  #
  set (APPLE_EXCLUDE_TESTS
    )

set (XML_TESTS_WITH_BASELINES
  ApplyBathymetry
  ArcAutoConnect1
  ArcCreate1
  ArcGrow
   ArcSnap
  ArcSplit1
  ArcSplit2
  # XXX: nodata(SceneGen.osd.txt): CameraOrientationTest
  ConeSceneTest
  # XXX: nodata(SceneGen.osd.txt): ContextMenuTest
  # XXX: nodata(SceneGen.osd.txt): CreateLine
  # XXX: nodata(SceneGen.osd.txt): CubeAxesTest
  # XXX: nodata(SceneGen.osd.txt): DisplayTypeTest
  # XXX: nodata(SceneGen.osd.txt): DuplicateNodeTest
  # XXX: nodata(Ballard_veg_mask.jpg): EditTexturePointsTest
  GeotiffOpenTest
  # XXX: nodata(SmallCeosoteCorrected.vtp): GlyphSelectionTest_Playback
  # XXX: nodata(SmallCeosoteCorrected.vtp): GlyphTest
  # XXX: nodata(SmallCeosoteCorrected.vtp): GlyphTest_Playback
  # XXX: nodata(Ballard_veg_mask.jpg): GlyphTextureTest_Playback
  # XXX: nodata(SmallCeosoteCorrected.vtp): GlyphVOITest_Playback
  # XXX: nodata(stone3.2dm): ImportingAsGlyphs_Playback
  # XXX: nodata(TINHigh.vtk): ImportObjectTest
  # XXX: nodata(SceneGen.osd.txt): LightingTest
  LoadSaveBathymetry
  MercedRiverElevationTest
  # XXX: nodata(cyl_frame.2dm): NewTest
  # XXX: nodata(SceneGen.osd.txt): ObjectPropertiesTest
  # XXX: nodata(SampleScene.sg): ObjectReadersTest
  # XXX: nodata(SceneGen.osd.txt): OpenFileTest
  # XXX: nodata(bay.map): OpenMapFileTestSB
  # XXX: nodata(cyl_frame.2dm): ResetCameraWhenCreatingFirstSceneObject
  # XXX: nodata(sac_clay_29_5_ft_blanket_rootZone_2d.poly): SceneBuilder2DPolyFileTest
  SceneBuilderContourExtraction
  # XXX: nodata(SceneGen.osd.txt): SelectionModeTest
  # XXX: nodata(SceneGen.osd.txt): SelectionTest
  # XXX: nodata(SceneGen.osd.txt): SnapObjectTest
  # XXX: nodata(TINHigh.vtk): stitchTest
  # XXX: nodata(TINHigh.vtk): StitchWithPointInsertionTest
  # XXX: nodata(SceneGen.osd.txt): StyleTest
  SubArcEdit1
  # XXX: nodata(SceneGen.osd.txt): TurnOffObjectsTest
  # XXX Need Mesher TerrainMeshTest
  # XXX Need MesherTerrainMeshTest2
  )

  set (TESTS_WITHOUT_BASELINES
  # XXX: nodata(SceneGen.osd.txt): AbortMesherTest
  AboutTest
  HelpTest
  # XXX: nodata(SceneGen.osd.txt): ReadWriteTest
  # XXX: nodata(SceneGen.osd.txt): SceneInspectorTest
  # XXX: nodata(SceneGen.osd.txt): WriteOmicronInputTest
  # XXX: nodata(SceneGen.osd.txt): LIDARDialogTest
  # XXX: nodata(SceneGen.osd.txt): DuplicateRandomTest
  #ExportBCSTest
  # XXX: nodata(SceneGen.osd.txt): EditTextureTest
  # XXX: nodata(SceneGen.osd.txt): RejectMeshTest
  # XXX: nodata(SceneGen.osd.txt): CreateMeshTest
  # XXX: nodata(SceneGen.osd.txt): InfoPanelTest
    )

  foreach(test ${TESTS_WITHOUT_BASELINES})
    #Run the test if it is on the correct platform
    #
    list(FIND APPLE_EXCLUDE_TESTS ${test} APPLE_FIND_RES)

    if(NOT APPLE OR NOT APPLE_FIND_RES GREATER -1)
      add_long_test(SceneBuilder${test}
        SceneBuilder
        -dr
        --test-directory=${CMB_TEST_DIR}
        --test-script=${TEST_XML}/${test}.xml --exit)
    endif()
  endforeach(test)

  function(test_with_text_file_output testname tempfile baselinefile exepath1 exe1 args1 exepath2 exe2 args2)
    file(WRITE "${CMAKE_BINARY_DIR}/Run${testname}.cmake"
"set(fullexe1 \"${exepath1}/${exe1}\")
if(NOT EXISTS \${fullexe1})
set(fullexe1 \"${exepath1}/\${cfg}/${exe1}\")
endif()
set(fullexe2 \"${exepath2}/${exe2}\")
if(NOT EXISTS \${fullexe2})
set(fullexe2 \"${exepath2}/\${cfg}/${exe2}\")
endif()
FILE(REMOVE \"${tempfile}\")
SET(ARGS1 \"${args1}\")
SET(ARGS2 \"${args2}\")
execute_process(COMMAND \${fullexe1} \${ARGS1} RESULT_VARIABLE rv1)
if(NOT rv1 EQUAL 0)
message(FATAL_ERROR \"SceneBuilder executable return value was \${rv1}\")
endif()
execute_process(COMMAND \${fullexe2} \${ARGS2} RESULT_VARIABLE rv2)
if(NOT rv2 EQUAL 0)
message(FATAL_ERROR \"ModelBuilder executable return value was \${rv2}\")
endif()"
)
    add_long_test(SceneBuilder${testname} COMMAND ${CMAKE_COMMAND} -Dcfg=$<CONFIGURATION> -P "${CMAKE_BINARY_DIR}/Run${testname}.cmake")
  endfunction()

  set(PROJECT_EXE ${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX})

  if(CMB_TEST_DATA_ROOT)
  foreach(test ${XML_TESTS_WITH_BASELINES})
    #If this is an apple machine and it is a
    #platform specific test add the Apple suffix
    #
    list(FIND APPLE_EXCLUDE_TESTS ${test} APPLE_FIND_RES)
    if(NOT APPLE OR NOT APPLE_FIND_RES GREATER -1)
      get_image_threshold_arg(THRESHOLD_CMD ${test})
      add_long_test(SceneBuilder${test}
      SceneBuilder
      -dr
      --test-script=${TEST_XML}/${test}.xml
      --test-directory=${CMB_TEST_DIR}
      --test-baseline=${TEST_IMAGE}/${test}.png
      ${THRESHOLD_CMD}
      --exit
      )
    endif()
  endforeach(test)
  endif(CMB_TEST_DATA_ROOT)
  add_subdirectory(Testing)
endif(BUILD_TESTING)

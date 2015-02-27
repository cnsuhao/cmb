project (SceneBuilder)

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ../../CMBModel/Plugin
  ${cmbAppCommon_SOURCE_DIR}
  ${cmbAppCommon_BINARY_DIR}
  ${cmbAppCommon_SOURCE_DIR}/Scene
  ${vtkCmbDiscreteModel_INCLUDE_DIRS}
  ${vtkDiscreteModel_INCLUDE_DIRS}
  ${vtkDiscreteModel_INCLUDE_DIRS}/Model
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
  CMBModel_Plugin
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
  set(ZoomToBoxTest_THRESHOLD 36)
  set(CubeAxesTest_THRESHOLD 14)
  set(LightingTest_THRESHOLD 14)
  set(ObjectPropertiesTest_THRESHOLD 14)
endif(BUILD_TESTING)

if(BUILD_TESTING)
  if(Q_WS_MAC)
  set(TEST_BINARY ${EXECUTABLE_OUTPUT_PATH}/SceneBuilder.app/Contents/MacOS/SceneBuilder --test-directory=${CMB_TEST_DIR})
  else(Q_WS_MAC)
   set(TEST_BINARY ${EXECUTABLE_OUTPUT_PATH}/SceneBuilder --test-directory=${CMB_TEST_DIR})
  endif(Q_WS_MAC)

  set(TEST_XML ${SceneBuilder_SOURCE_DIR}/Testing/XML)
  set(TEST_IMAGE ${CMB_TEST_DATA_ROOT}/Baseline)

  #Tests that don't work on Apple
  #
  set (APPLE_EXCLUDE_TESTS
    )

  set (XML_TESTS_WITH_BASELINES
  2DTest
  ApplyBathymetry
  ArcAutoConnect1
  ArcAutoConnect2
  ArcCreate1
  ArcGrow
  ArcMerge1
  ArcMerge2
  ArcMesh1
  ArcMesh2
  ArcSnap
  ArcSplit1
  ArcSplit2
  AssignColorsTest
  CameraOrientationTest
  ChangeNumberOfPoints
  ConeSceneTest
  ContextMenuTest
  CreateLine
  CubeAxesTest
  DeleteNodeTest
  DisplayTypeTest
  DuplicateNodeTest
  EditTexturePointsTest
  ExportArcSetsTest
  GlyphSelectionTest_Playback
  GlyphTest
  GlyphTest_Playback
  GlyphTextureTest_Playback
  GlyphVOITest_Playback
  ImportingAsGlyphs_Playback
  ImportObjectTest
  LightingTest
  LoadSaveBathymetry
  MercedRiverElevationTest
  NewTest
  ObjectPropertiesTest
  ObjectReadersTest
  OpenFileTest
  OpenMapFileTestSB
  ResetCameraWhenCreatingFirstSceneObject
  SceneBuilder2DPolyFileTest
  SceneBuilderContourExtraction
  SceneBuilderTextureMappedGroundPlane
  SelectionModeTest
  SelectionTest
  SnapObjectTest
  stitchTest
  StitchWithPointInsertionTest
  StyleTest
  SubArcEdit1
  TurnOffObjectsTest
  TerrainMeshTest
  TerrainMeshTest2
  ZoomToBoxTest
  ZoomToDataTest
  )

  set (TESTS_WITHOUT_BASELINES
  AbortMesherTest
  AboutTest
  HelpTest
  ReadWriteTest
  SceneInspectorTest
  WriteOmicronInputTest
  LIDARDialogTest
  DuplicateRandomTest
  #ExportBCSTest
  EditTextureTest
  ProcessOutputTest
  RejectMeshTest
  CreateMeshTest
  InfoPanelTest
  ChesapeakeBayToCMB
    )

  foreach(test ${TESTS_WITHOUT_BASELINES})
    #Run the test if it is on the correct platform
    #
    list(FIND APPLE_EXCLUDE_TESTS ${test} APPLE_FIND_RES)

    if(NOT APPLE OR NOT APPLE_FIND_RES GREATER -1)
      add_long_test(SceneBuilder${test}
        ${TEST_BINARY}
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
  if(WIN32)
    test_with_text_file_output(CreateMeshWithSubmergedSolidTest ${CMB_TEST_DIR}/SubmergedSolidTest.cmb
      ${CMB_TEST_DATA_ROOT}/SubmergedSolidTest.cmb
      ${EXECUTABLE_OUTPUT_PATH} ${PROJECT_EXE} "-dr;--test-directory=${CMB_TEST_DIR};--test-script=${TEST_XML}/CreateMeshWithSubmergedSolidTest.xml;--exit"
      ${EXECUTABLE_OUTPUT_PATH} ModelBuilder "-dr;--test-directory=${CMB_TEST_DIR};--test-script=${TEST_XML}/CreateMeshWithSubmergedSolidTestPart2.xml;--test-baseline=${TEST_IMAGE}/CreateMeshWithSubmergedSolidTest.png;--exit")
  else (WIN32)
    if (Q_WS_MAC)
    test_with_text_file_output(CreateMeshWithSubmergedSolidTest ${CMB_TEST_DIR}/SubmergedSolidTest.cmb
      ${CMB_TEST_DATA_ROOT}/SubmergedSolidTest.cmb
      ${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}.app/Contents/MacOS ${PROJECT_NAME} "-dr;--test-directory=${CMB_TEST_DIR};--test-script=${TEST_XML}/CreateMeshWithSubmergedSolidTest.xml;--exit"
      ${EXECUTABLE_OUTPUT_PATH}/ModelBuilder.app/Contents/MacOS ModelBuilder "-dr;--test-directory=${CMB_TEST_DIR};--test-script=${TEST_XML}/CreateMeshWithSubmergedSolidTestPart2.xml;--test-baseline=${TEST_IMAGE}/CreateMeshWithSubmergedSolidTest.png;--exit")
    endif (Q_WS_MAC)
  endif (WIN32)

  if(CMB_TEST_DATA_ROOT)
  foreach(test ${XML_TESTS_WITH_BASELINES})
    #If this is an apple machine and it is a
    #platform specific test add the Apple suffix
    #
    list(FIND APPLE_EXCLUDE_TESTS ${test} APPLE_FIND_RES)
    if(NOT APPLE OR NOT APPLE_FIND_RES GREATER -1)
      get_image_threshold_arg(THRESHOLD_CMD ${test})
      add_long_test(SceneBuilder${test}
      ${TEST_BINARY}
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

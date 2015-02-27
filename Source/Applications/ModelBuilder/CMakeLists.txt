project (ModelBuilder)

find_package(Remus REQUIRED)

IF(NOT ParaView_BINARY_DIR)
  FIND_PACKAGE(ParaView REQUIRED)
  INCLUDE(${PARAVIEW_USE_FILE})
ENDIF(NOT ParaView_BINARY_DIR)

find_package(SMTK REQUIRED)

include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

# Add Qt UI files here
set(UI_FORMS
  pqCMBImportShapefile.ui
  qtCMBLegacyMesherDialog.ui
  qtVolumeMesherSelector.ui
  pqCMBModelBuilderOptions.ui
  SimBuilder/qtSimBuilderExportADHDialog.ui
  SimBuilder/qtSimBuilderExportDialog.ui
  SimBuilder/qtSimBuilderEMSExportDialog.ui
  )

qt4_wrap_ui(UI_BUILT_SOURCES
  ${UI_FORMS}
  )

# Add Qt resource files here
set(UI_RESOURCES
  ../Resources/Resources.qrc
  icon.qrc
  )

qt4_add_resources(RCS_SOURCES
  ${UI_RESOURCES}
  )

if(WIN32)
  set(EXE_ICON ${CMAKE_CURRENT_SOURCE_DIR}/CMB.rc)
  set(EXE_RES ${CMAKE_CURRENT_BINARY_DIR}/CMB.res)
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

# Add SimBuilder files here
set(SimBuilder_SOURCES
  SimBuilder/SimBuilderCore.cxx
  SimBuilder/SimBuilderCustomExportDialog.cxx
  SimBuilder/SimBuilderEMSExportDialog.cxx
  SimBuilder/qtSimBuilderUIPanel.cxx
  SimBuilder/pqSMTKUIManager.cxx
  SimBuilder/vtkSBFunctionParser.cxx
  SimBuilder/pqSMTKUIHelper.h
  ${CMAKE_CURRENT_BINARY_DIR}/DefaultExportTemplate.h
  )
source_group("SimBuilder" FILES
  ${SimBuilder_SOURCES}
  )

#------------------------------------------------------------------------------
# Generate default export template header file
# Read default template file (.sbt)
file(READ
  ${CMAKE_CURRENT_SOURCE_DIR}/SimBuilder/XML/DefaultExportTemplate.sbt
  ExportTemplate
)

# Convert string to list (per Dave Cole's method)
STRING(REGEX REPLACE ";" "\\\\;" ExportTemplate "${ExportTemplate}")
STRING(REGEX REPLACE "\n" ";" ExportTemplate "${ExportTemplate}")

# Escape all quote signs
STRING(REGEX REPLACE "\"" "\\\\\"" ExportTemplate "${ExportTemplate}")

# Build new string with each line contained in quotes.
# This produces a c++ multiline string literal.
set(QuotedTemplate)
foreach(line ${ExportTemplate})
  set(QuotedTemplate "${QuotedTemplate}  \"${line}\\n\"\n")
endforeach()

# Use QuotedTemplate to configure header file
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/SimBuilder/DefaultExportTemplate.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/DefaultExportTemplate.h
  @ONLY
)


#------------------------------------------------------------------------------
# Specify template files to install
SET(SIMBUILDER_TEMPLATE_NAMES
  AdHShallowWater.crf
  AdHShallowWater.sbt
  AdHSurfaceWater.crf
  AdHSurfaceWaterDefinitions.sbt
  AdHSurfaceWater.sbt
  AdHSurfaceWaterViews.sbt
  CTB.crf
  CTB.sbt
  EMS.crf
  EMS.sbt
  ProteusPoisson.crf
  ProteusPoisson.sbt
  Proteus_Template.sbt
  )
 foreach(name ${SIMBUILDER_TEMPLATE_NAMES})
    list(APPEND SIMBUILDER_TEMPLATE_FILES "SimBuilder/XML/${name}")
 endforeach()

if(NOT APPLE)
  #windows or unix
  install(DIRECTORY Icons DESTINATION "${VTK_INSTALL_RUNTIME_DIR}/Resources")
  install(FILES ${SIMBUILDER_TEMPLATE_FILES} DESTINATION "${VTK_INSTALL_RUNTIME_DIR}/Resources//SimBuilderTemplateFiles/")
  install(DIRECTORY SimBuilder/PythonExporters DESTINATION "${VTK_INSTALL_RUNTIME_DIR}/Resources")
else()
  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/../Resources/qt.conf
    DESTINATION "${VTK_INSTALL_RUNTIME_DIR}/${PROJECT_NAME}.app/Contents/Resources")
  install(DIRECTORY Icons DESTINATION "${VTK_INSTALL_RUNTIME_DIR}/${PROJECT_NAME}.app/Contents/Resources")
  install(FILES ${SIMBUILDER_TEMPLATE_FILES}   DESTINATION "${VTK_INSTALL_RUNTIME_DIR}/${PROJECT_NAME}.app/Contents/Resources/SimBuilderTemplateFiles/")
  install(DIRECTORY SimBuilder/PythonExporters DESTINATION "${VTK_INSTALL_RUNTIME_DIR}/${PROJECT_NAME}.app/Contents/Resources")
endif()

# Copy certain files that are used for tests.
# At some point, we should rewrite those tests.

# Set up the Icons directory and symlink/copy all icons to it.
# Note: Windows lacks symlinks and so the files must be copied.
if(WIN32)
  set(icon_cmd "copy_if_different")
  set(icon_path "${EXECUTABLE_OUTPUT_PATH}")
elseif(APPLE)
  set(icon_cmd "create_symlink")
  set(icon_path "${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}.app/Contents")
elseif(UNIX)
  set(icon_cmd "create_symlink")
  set(icon_path "${EXECUTABLE_OUTPUT_PATH}")
endif()

make_directory("${icon_path}/Resources/Icons")
file(GLOB SIMBUILDER_ICON_FILES RELATIVE "${ModelBuilder_SOURCE_DIR}/Icons" "Icons/*.png")
foreach(iconFile ${SIMBUILDER_ICON_FILES})
  set(from "${ModelBuilder_SOURCE_DIR}/Icons/${iconFile}")
  set(to   "${icon_path}/Resources/Icons/${iconFile}")
  execute_process(COMMAND ${CMAKE_COMMAND} -E ${icon_cmd} "${from}" "${to}")
endforeach()

make_directory("${icon_path}/Resources/SimBuilderTemplateFiles")
SET(SIMBUILDER_TEMPLATE_DIR ${icon_path}/Resources/SimBuilderTemplateFiles/)
foreach(name ${SIMBUILDER_XML_NAMES})
  file(COPY ${ModelBuilder_SOURCE_DIR}/SimBuilder/XML/${name}
  DESTINATION ${SIMBUILDER_TEMPLATE_DIR})
endforeach()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmbModelBuilderConfig.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/cmbModelBuilderConfig.h
  ESCAPE_QUOTES)

set(ModelBuilder_SOURCE_FILES
  pqCMBLegacyMesherDialog.cxx
  pqCMBModelBuilderMainWindow.cxx
  pqCMBModelBuilderMainWindowCore.cxx
  pqModelBuilderViewContextMenuBehavior.cxx
  pqModelBuilderViewContextMenuBehavior.h
  qtRemusVolumeMesherSubmitter.h
  qtRemusVolumeMesherSubmitter.cxx
  qtRemusVolumeMesherSelector.h
  qtRemusVolumeMesherSelector.cxx
  pqCMBModelBuilderOptions.cxx
  pqCMBModelBuilderOptions.h
  pqCMBImportShapefile.h
  pqCMBImportShapefile.cxx
  pqSMTKModelPanel.h
  pqSMTKModelPanel.cxx
  pqCMBModelManager.h
  pqCMBModelManager.cxx

  ${RCS_SOURCES}
  ${UI_BUILT_SOURCES}
  ${EXE_RES}
  ${apple_bundle_sources}
  ${SimBuilder_SOURCES}
  )

#------------------------------------------------------------------------------
# Build the client
build_paraview_client(ModelBuilder
  TITLE "Model Builder ${CMB_VERSION_MAJOR}.${CMB_VERSION_MINOR}.${CMB_VERSION_PATCH}"
  ORGANIZATION  "Kitware"
  VERSION_MAJOR ${CMB_VERSION_MAJOR}
  VERSION_MINOR ${CMB_VERSION_MINOR}
  VERSION_PATCH ${CMB_VERSION_PATCH}
  SPLASH_IMAGE "${CMAKE_CURRENT_SOURCE_DIR}/../Resources/Icons/ModelBuilderSplash.png"
  PVMAIN_WINDOW pqCMBModelBuilderMainWindow
  PVMAIN_WINDOW_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/pqCMBModelBuilderMainWindow.h
  BUNDLE_ICON   "${CMAKE_CURRENT_SOURCE_DIR}/MacIcon.icns"
  APPLICATION_ICON   "${CMAKE_CURRENT_SOURCE_DIR}/cmbappico.ico"
  SOURCES ${ModelBuilder_SOURCE_FILES}
  COMPRESSED_HELP_FILE "${CMAKE_CURRENT_BINARY_DIR}/../Help/cmbsuite.qch"
  OPTIONAL_PLUGINS smtkCGMSession_Plugin
                   smtkDiscreteSession_Plugin
                   smtkExodusSession_Plugin
  INSTALL_BIN_DIR "${VTK_INSTALL_RUNTIME_DIR}"
  INSTALL_LIB_DIR "${VTK_INSTALL_LIBRARY_DIR}"
  )

#let cmake do what qt4_wrap_cpp used to do automatically
set_target_properties(ModelBuilder PROPERTIES AUTOMOC TRUE)

#we need to explicitly state that you shouldn't build ModelBuilder
#before the help has been generated
add_dependencies(ModelBuilder CMBSuiteHelp)

target_link_libraries(ModelBuilder
LINK_PRIVATE
  cmbAppCommon
  QtSMTK
  RemusClient
  SMTKCore
  ModelBridge_Plugin
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
     \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${VTK_INSTALL_RUNTIME_DIR}/ModelBuilder.app/Contents/MacOS/ModelBuilder\"
     \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${VTK_INSTALL_RUNTIME_DIR}/ModelBuilder.app\"
     \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${VTK_INSTALL_LIBRARY_DIR}\")

   # Place the App at the requested location.
   file(INSTALL DESTINATION \"${MACOSX_APP_INSTALL_PREFIX}\"
        TYPE DIRECTORY FILES
          \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${VTK_INSTALL_RUNTIME_DIR}/ModelBuilder.app\"
        USE_SOURCE_PERMISSIONS)
   "
   COMPONENT Runtime)
endif()


########################################################################
if(BUILD_TESTING)
  set(ChangeDetails_THRESHOLD 14)
  set(ChangeDetailsCMB_THRESHOLD 14)
  set(GrowTest_THRESHOLD 14)
  set(GrowTestCMB_THRESHOLD 14)
endif(BUILD_TESTING)

message(STATUS "EXECUTABLE_OUTPUT_PATH=${EXECUTABLE_OUTPUT_PATH}")
set(PROJECT_EXE ${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX})
if(BUILD_TESTING)
  if(Q_WS_MAC)
    set(TEST_BINARY
      "${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}.app/Contents/MacOS/${PROJECT_NAME}"
      --test-directory=${CMB_TEST_DIR})
  else(Q_WS_MAC)
    set(TEST_BINARY "${EXECUTABLE_OUTPUT_PATH}/${PROJECT_EXE}"
      --test-directory=${CMB_TEST_DIR})
  endif(Q_WS_MAC)
 set(TEST_XML ${ModelBuilder_SOURCE_DIR}/Testing/XML)
  set(TEST_IMAGE ${CMB_TEST_DATA_ROOT}/Baseline)
message(STATUS "TEST_BINARY=${TEST_BINARY}")

  #Tests that don't work on Apple

  set (APPLE_EXCLUDE_TESTS
    AboutDialog
    HelpDialog
    ChangeBackgroundColor
    SpawnVolumeMesherCMB
    SpawnVolumeMesher
    GrowTest
    StoneTest
    GrowTestCMB
    # ChangeFaceColors
    )
  set (APPLE_ONLY_TESTS
    SpawnVolumeMesherCMBMac
    SpawnVolumeMesherMac
    GrowTest_Mac
    StoneTest_Mac
    GrowTestCMB_Mac
    )

  set (XML_TESTS_WITH_BASELINES
    # ChangeFaceColors
    2DMeshing
    AxesControls
    OpenFile
    OpenSTLFile
    OpenPolyFile
    GrowTest
    ModifyRegionFaces
    MultipleSelection
    SelectionByMode
    ViewportTest
    SplitFace
    SaveState
    StoneTest
    SelectByTypeTest
    ModifyLighting
    ChangeBackgroundColor
    ReadSolids
    ZoomToSelection
    ChangeDetails
    GrowTestCMB
    ModelBuilderCreateRectangle
    ModelBuilderCreateEllipse
    ModifyLightingCMB
    ModifyRegionFacesCMB
    MultipleSelectionCMB
    SaveStateCMB
    SelectByTypeTestCMB
    SelectionByModeCMB
    SplitFaceCMB
    ViewportTestCMB
    ZoomToSelectionCMB
    ChangeDetailsCMB
    ChangeDetailsPoly
    MaterialColorFront
    MaterialColorBack
    ZoomToBoxCMB
    ReadMapFileCMB
    SimBuilderMeshing
    2DModelModification
    2DModelModificationWithMesh
    BathymetryTest
    ShowEdgeVertex
    Export2DFaceMesh
    2DMeshDomainset
    )

  set (TESTS_WITHOUT_BASELINES
    AddRemoveMaterialTest
    AddRemoveMaterialTestCMB
    AddRemoveMaterialTestPoly
    AboutDialog
    HelpDialog
    ReadWriteCMB
    ExportBCS
    ExportBCSCMB
    ChangeMaterialNames
    ChangeMaterialNamesCMB
    SpawnVolumeMesher
    EnableColumnSorting
    SpawnVolumeMesherCMB
    ChesapeakeBayBathymetry
    )

  foreach(test ${TESTS_WITHOUT_BASELINES})
    #Run the test if it is on the correct platform
    #
    list(FIND APPLE_EXCLUDE_TESTS ${test} APPLE_FIND_RES)

    if(NOT APPLE OR NOT APPLE_FIND_RES GREATER -1)
      add_long_test(${PROJECT_NAME}${test}
    ${TEST_BINARY} -dr
    --test-directory=${CMB_TEST_DIR}
    --test-script=${TEST_XML}/${test}.xml --exit)
      #Because of SpawnVolumeMesher and SpawnVolumeMesherCMB, we need to make all of these
      #are long timeouts.
    endif()
  endforeach(test)

  #Run apple specific tests without baselines
  if (APPLE)
    foreach(test ${APPLE_ONLY_TESTS})
      add_long_test(${PROJECT_NAME}${test}
    ${TEST_BINARY} -dr
    --test-directory=${CMB_TEST_DIR}
    --test-script=${TEST_XML}/${test}.xml --exit)
      #Because of SpawnVolumeMesher and SpawnVolumeMesherCMB, we need to make all of these
      #are long timeouts.
    endforeach(test)
  endif(APPLE)

  if(CMB_TEST_DATA_ROOT)
    foreach(test ${XML_TESTS_WITH_BASELINES})
      #Run the test if it is on the correct platform
      #
      list(FIND APPLE_EXCLUDE_TESTS ${test} APPLE_FIND_RES)

      if(NOT APPLE OR NOT APPLE_FIND_RES GREATER -1)
    get_image_threshold_arg(THRESHOLD_CMD ${test})
    add_medium_test(${PROJECT_NAME}${test}
      ${TEST_BINARY} -dr
      --test-script=${TEST_XML}/${test}.xml
      --test-directory=${CMB_TEST_DIR}
      --test-baseline=${TEST_IMAGE}/${test}.png
      ${THRESHOLD_CMD}
      --exit
      )
      endif()
    endforeach(test)
  endif(CMB_TEST_DATA_ROOT)

  set(PYTHON_PATH "")
  # Use TEST_ENV as list of environment vars to include in testing
  # Formatted as a list of name=value strings
  # Note that since windows env vars use semicolon:
  # *ALWAYS PUT ${TEST_ENV} IN QUOTES, I.E., "${TEST_ENV}"*
  set(TEST_ENV OFF)
  if(UNIX)
    set(PYTHON_PATH "${CMAKE_PREFIX_PATH}/lib/python2.7:${CMAKE_PREFIX_PATH}/lib/python2.7/lib-dynload:${CMAKE_PREFIX_PATH}/lib/python2.7/site-packages:${CMAKE_PREFIX_PATH}/python")
  else() #windows
    # Use escaped-semicolon for PYTHONPATH, so that it can be part of a list (TEST_ENV)
    set(PYTHON_PATH "${CMAKE_PREFIX_PATH}/../python/src/python/lib\;${CMAKE_PREFIX_PATH}/lib/site-packages")
    set(TEST_ENV "PYTHONPATH=${PYTHON_PATH}")
    #message("TEST_ENV ${TEST_ENV}")
  endif()

  # these tests use create the same temp files which need to be deleted before each of them
  # run so they are run in serial so that they don't conflict with each other
  if (APPLE)
    if(BUILD_LONG_TESTS) #ModelBuilderSpawnVolumeMesherMac and ModelBuilderSpawnVolumeMesherCMBMac should be classified as long tests
      set_tests_properties(ModelBuilderSpawnVolumeMesherMac ModelBuilderSpawnVolumeMesherCMBMac PROPERTIES RUN_SERIAL ON)
    endif()
  else (APPLE)
    if(BUILD_LONG_TESTS) #ModelBuilderSpawnVolumeMesher and ModelBuilderSpawnVolumeMesherCMB should be classified as long tests
      set_tests_properties(ModelBuilderSpawnVolumeMesher ModelBuilderSpawnVolumeMesherCMB PROPERTIES RUN_SERIAL ON)
    endif()
  endif()

  # Writes cmake file to run test
  function(test_with_text_file_output testname tempfile baselinefile envlist exepath exe)
    set(testfile "${CMAKE_BINARY_DIR}/Run${testname}.cmake")
    #message("testfile ${testfile} envlist ${envlist}")
    file(WRITE ${testfile}
      "set(fullexe \"${exepath}/${exe}\")
      if(NOT EXISTS \${fullexe})
        set(fullexe \"${exepath}/\${cfg}/${exe}\")
      endif()
      FILE(REMOVE \"${tempfile}\")
      SET(ARGS \"${ARGN}\")
      ")

    # Set any environment variables passed in
    # Each item in list should be name=value
    if(envlist)
      #message('envlist ${envlist}')
      foreach(envitem ${envlist})
        #message("envitem ${envitem}.")
        # Be sure to only use "${envitem}" in double quotes
        # So that semicolons are not lost
        string(REGEX MATCH "^([^=]+)=([^=]+)$" notused "${envitem}")
        #message("SET(${CMAKE_MATCH_1}) ${CMAKE_MATCH_2}")
        file(APPEND ${testfile}
          "SET(ENV{${CMAKE_MATCH_1}} \"${CMAKE_MATCH_2}\")
      ")
      endforeach()
    endif()

    # Write execute_process commands to run test and check results
    file(APPEND ${testfile}
      "execute_process(COMMAND \${fullexe} \${ARGS} RESULT_VARIABLE rv)
      if(NOT rv EQUAL 0)
        message(FATAL_ERROR \"executable return value was \${rv}\")
      endif()
      execute_process(COMMAND \"${CMAKE_COMMAND}\" -E compare_files \"${tempfile}\" \"${baselinefile}\"
        RESULT_VARIABLE failed)
      if(failed)
        message(FATAL_ERROR \"files differ -- \" \${failed})
      endif()
      ")

    add_medium_test(${testname} ${CMAKE_COMMAND} -Dcfg=$<CONFIGURATION> -P "${CMAKE_BINARY_DIR}/Run${testname}.cmake")
  endfunction()

  # SimBuilder tests -- we set PYTHONPATH for them automatically
  # we also check if ImportError is output and if it is we mark
  # the test as failed. additionally we copy the Python code
  # to the build directory to make sure we don't generate pyc
  # files in the source code.
  file(COPY ${CMB_TEST_DATA_ROOT}/SimBuilderGUI/FunctionExpressions.py
    DESTINATION ${CMB_TEST_DIR} )
  set (SIMBUILDER_TESTS_WITHOUT_BASELINES
    Expressions
    )

  foreach(test ${SIMBUILDER_TESTS_WITHOUT_BASELINES})
    add_short_test(SimBuilder${test}
      ${TEST_BINARY} -dr
      --test-directory=${CMB_TEST_DIR}
      --test-script=${TEST_XML}/${test}.xml --exit)
    if(BUILD_SHORT_TESTS)
      set_tests_properties(SimBuilder${test}
        PROPERTIES
        ENVIRONMENT "PYTHONPATH=${PYTHON_PATH}"
        FAIL_REGULAR_EXPRESSION "ImportError"
        )
    endif()
  endforeach(test)

  # All of these tests use test_with_text_file_output(), so they are all medium length tests.
  if(BUILD_MEDIUM_TESTS)
    # SimBuilderADHExporter test
    if(WIN32)
      test_with_text_file_output(SimBuilderADHExport ${CMAKE_CURRENT_BINARY_DIR}/outputADHExport.bc
        ${CMB_TEST_DATA_ROOT}/SimBuilderADHExportTest/BaselineADHOutput.bc
        "${TEST_ENV}"
        ${EXECUTABLE_OUTPUT_PATH} ${PROJECT_EXE} -dr "--test-directory=${CMB_TEST_DIR}" "--test-script=${TEST_XML}/SBADHExport.xml" --exit)
    else (WIN32)
      if (Q_WS_MAC)
        test_with_text_file_output(SimBuilderADHExport ${CMAKE_CURRENT_BINARY_DIR}/outputADHExport.bc
          ${CMB_TEST_DATA_ROOT}/SimBuilderADHExportTest/BaselineADHOutputLinux.bc
          "${TEST_ENV}"
          ${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}.app/Contents/MacOS ${PROJECT_EXE} -dr "--test-directory=${CMB_TEST_DIR}" "--test-script=${TEST_XML}/SBADHExport.xml" --exit)
      else (Q_WS_MAC) # if building with xcode also need debug/release in executable path
        test_with_text_file_output(SimBuilderADHExport ${CMAKE_CURRENT_BINARY_DIR}/outputADHExport.bc ${CMB_TEST_DATA_ROOT}/SimBuilderADHExportTest/BaselineADHOutputLinux.bc
          "${TEST_ENV}"
          ${EXECUTABLE_OUTPUT_PATH} ${PROJECT_EXE} -dr "--test-directory=${CMB_TEST_DIR}" "--test-script=${TEST_XML}/SBADHExport.xml" --exit)
      endif (Q_WS_MAC)
    endif (WIN32)

    # SimBuilderReaderWriter test
    #file(COPY "${ModelBuilder_SOURCE_DIR}/SimBuilder/XML/SimBuilderv2.sbt"
    #  DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
    #if(Q_WS_MAC)
    #  test_with_text_file_output(SimBuilderReaderWriter ${CMAKE_CURRENT_BINARY_DIR}/firstwrite.sbi
    #    ${CMAKE_CURRENT_BINARY_DIR}/secondwrite.sbi
    #    "${TEST_ENV}" ${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}.app/Contents/MacOS ${PROJECT_EXE} -dr "--test-directory=${CMB_TEST_DIR}" "--test-script=${TEST_XML}/SBReaderWriter.xml" --exit)
    #else(Q_WS_MAC)
    #  test_with_text_file_output(SimBuilderReaderWriter ${CMAKE_CURRENT_BINARY_DIR}/firstwrite.sbi
    #    ${CMAKE_CURRENT_BINARY_DIR}/secondwrite.sbi
    #    "${TEST_ENV}" ${EXECUTABLE_OUTPUT_PATH} ${PROJECT_EXE} -dr "--test-directory=${CMB_TEST_DIR}" "--test-script=${TEST_XML}/SBReaderWriter.xml" --exit)
    #endif(Q_WS_MAC)

    # SimBuilderLibraryReaderWriter test
    if(Q_WS_MAC)
      test_with_text_file_output(SimBuilderLibraryReaderWriter ${CMAKE_CURRENT_BINARY_DIR}/libraryfirstwrite.sbi
        ${CMAKE_CURRENT_BINARY_DIR}/librarysecondwrite.sbi
        "${TEST_ENV}"
        ${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}.app/Contents/MacOS ${PROJECT_EXE} -dr "--test-directory=${CMB_TEST_DIR}" "--test-script=${TEST_XML}/SBLibraryReaderWriter.xml" --exit)
    else(Q_WS_MAC)
      test_with_text_file_output(SimBuilderLibraryReaderWriter ${CMAKE_CURRENT_BINARY_DIR}/libraryfirstwrite.sbi
        ${CMAKE_CURRENT_BINARY_DIR}/librarysecondwrite.sbi
        "${TEST_ENV}"
        ${EXECUTABLE_OUTPUT_PATH} ${PROJECT_EXE} -dr "--test-directory=${CMB_TEST_DIR}" "--test-script=${TEST_XML}/SBLibraryReaderWriter.xml" --exit)
    endif(Q_WS_MAC)

    # SimBuilderADH2DExporter test
    if(WIN32)
      test_with_text_file_output(SimBuilderADH2DExport ${CMAKE_CURRENT_BINARY_DIR}/outputADH2DExport.bc
        ${CMB_TEST_DATA_ROOT}/SimBuilderADH2DExportTest/BaselineADH2DOutput.bc
        "${TEST_ENV}"
        ${EXECUTABLE_OUTPUT_PATH} ${PROJECT_EXE} -dr "--test-directory=${CMB_TEST_DIR}" "--test-script=${TEST_XML}/SimBuilderADH2DExport.xml" --exit)
    else (WIN32)
      if (Q_WS_MAC)
        test_with_text_file_output(SimBuilderADH2DExport ${CMAKE_CURRENT_BINARY_DIR}/outputADH2DExport.bc
          ${CMB_TEST_DATA_ROOT}/SimBuilderADH2DExportTest/BaselineADH2DOutputLinux.bc
          "${TEST_ENV}"
          ${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}.app/Contents/MacOS ${PROJECT_EXE} -dr "--test-directory=${CMB_TEST_DIR}" "--test-script=${TEST_XML}/SimBuilderADH2DExport.xml" --exit)
      else (Q_WS_MAC) # if building with xcode also need debug/release in executable path
         test_with_text_file_output(SimBuilderADH2DExport ${CMAKE_CURRENT_BINARY_DIR}/outputADH2DExport.bc ${CMB_TEST_DATA_ROOT}/SimBuilderADH2DExportTest/BaselineADH2DOutputLinux.bc
          "${TEST_ENV}"
          ${EXECUTABLE_OUTPUT_PATH} ${PROJECT_EXE} -dr "--test-directory=${CMB_TEST_DIR}" "--test-script=${TEST_XML}/SimBuilderADH2DExport.xml" --exit)
      endif (Q_WS_MAC)
    endif (WIN32)

    # SimBuilderMeshModel3DRoundTrip test
    if(Q_WS_MAC)
      test_with_text_file_output(SimBuilderMeshModel3DRoundTrip ${CMAKE_CURRENT_BINARY_DIR}/mesh_BoxWithBCS_ADH.bc
        ${CMAKE_CURRENT_BINARY_DIR}/mesh_BoxWithBCS_m2mADH.bc
        "${TEST_ENV}"
        ${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}.app/Contents/MacOS ${PROJECT_EXE} -dr "--test-directory=${CMB_TEST_DIR}" "--test-script=${TEST_XML}/BoxMesh2ModelTest.xml" --exit)
    else(Q_WS_MAC)
      test_with_text_file_output(SimBuilderMeshModel3DRoundTrip ${CMAKE_CURRENT_BINARY_DIR}/mesh_BoxWithBCS_ADH.bc
        ${CMAKE_CURRENT_BINARY_DIR}/mesh_BoxWithBCS_m2mADH.bc
        "${TEST_ENV}"
        ${EXECUTABLE_OUTPUT_PATH} ${PROJECT_EXE} -dr "--test-directory=${CMB_TEST_DIR}" "--test-script=${TEST_XML}/BoxMesh2ModelTest.xml" --exit)
    endif(Q_WS_MAC)

    # SimBuilderMeshModel2DRoundTrip test
    if(Q_WS_MAC)
      test_with_text_file_output(SimBuilderMeshModel2DRoundTrip ${CMAKE_CURRENT_BINARY_DIR}/mesh_test2DWithBCS_ADH.bc
        ${CMAKE_CURRENT_BINARY_DIR}/mesh_test2DWithBCS_m2mADH.bc
        "${TEST_ENV}"
        ${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}.app/Contents/MacOS ${PROJECT_EXE} -dr "--test-directory=${CMB_TEST_DIR}" "--test-script=${TEST_XML}/test2DMesh2ModelTest.xml" --exit)
    else(Q_WS_MAC)
      test_with_text_file_output(SimBuilderMeshModel2DRoundTrip ${CMAKE_CURRENT_BINARY_DIR}/mesh_test2DWithBCS_ADH.bc
        ${CMAKE_CURRENT_BINARY_DIR}/mesh_test2DWithBCS_m2mADH.bc
        "${TEST_ENV}"
        ${EXECUTABLE_OUTPUT_PATH} ${PROJECT_EXE} -dr "--test-directory=${CMB_TEST_DIR}" "--test-script=${TEST_XML}/test2DMesh2ModelTest.xml" --exit)
    endif(Q_WS_MAC)

    # EMSExport test
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/SimBuilder/PythonExporters/EMS.py
      DESTINATION ${CMB_TEST_DIR} )
    if(Q_WS_MAC)
      test_with_text_file_output(SimBuilderEMSExport ${CMAKE_CURRENT_BINARY_DIR}/EMSExportBaseline.txt
        ${CMB_TEST_DATA_ROOT}/EMSExport/EMSExportBaseline.txt
        "${TEST_ENV}"
        ${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}.app/Contents/MacOS ${PROJECT_EXE} -dr "--test-directory=${CMB_TEST_DIR}" "--test-script=${TEST_XML}/EMSExport.xml" --exit)
    else(Q_WS_MAC)
      test_with_text_file_output(SimBuilderEMSExport ${CMAKE_CURRENT_BINARY_DIR}/EMSExportBaseline.txt
        ${CMB_TEST_DATA_ROOT}/EMSExport/EMSExportBaseline.txt
        "${TEST_ENV}"
        ${EXECUTABLE_OUTPUT_PATH} ${PROJECT_EXE} -dr "--test-directory=${CMB_TEST_DIR}" "--test-script=${TEST_XML}/EMSExport.xml" --exit)
    endif(Q_WS_MAC)
    set_tests_properties(SimBuilderEMSExport
      PROPERTIES
      ENVIRONMENT "PYTHONPATH=${PYTHON_PATH}"
      FAIL_REGULAR_EXPRESSION "ImportError"
    )

    # Copy GEOTACSexporter.py to build directory
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/SimBuilder/PythonExporters/CTB.py
      DESTINATION ${CMB_TEST_DIR} )


    # Standalone export test: Shallow Water 2D
    # Copy the Python script to the build so that we don't get
    # pyc files in the source code generated when we run the test.
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/SimBuilder/PythonExporters/AdHSurfaceWater.py
      DESTINATION ${CMB_TEST_DIR} )
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/SimBuilder/PythonExporters/adhcommon.py
      DESTINATION ${CMB_TEST_DIR} )
    set(EXPORT_TEST_ARGS
      -c ${CMB_TEST_DATA_ROOT}/Export/test2DPlusGroups.cmb
      -g ${CMB_TEST_DATA_ROOT}/Export/test2D.2dm
      -m ${CMB_TEST_DATA_ROOT}/Export/test2D.m2m
      -s ${CMB_TEST_DATA_ROOT}/Export/ShallowWater2D.sbi
      -a "\\\"CFD Flow\\\"" "\\\"Constituent Transport\\\""
      -p ${CMB_TEST_DIR}/AdHSurfaceWater.py
      -o ${CMAKE_CURRENT_BINARY_DIR}/surfacewater.bc
    )
    test_with_text_file_output(SimBuilderStandaloneExportAdHSurfaceWater
      ${CMAKE_CURRENT_BINARY_DIR}/surfacewater.bc
      ${CMB_TEST_DATA_ROOT}/Export/ShallowWater2DBaseline.bc
      "${TEST_ENV}"
      ${EXECUTABLE_OUTPUT_PATH}
      export${CMAKE_EXECUTABLE_SUFFIX} ${EXPORT_TEST_ARGS}
    )
    set_tests_properties(SimBuilderStandaloneExportAdHSurfaceWater
      PROPERTIES
      ENVIRONMENT "PYTHONPATH=${PYTHON_PATH}"
      FAIL_REGULAR_EXPRESSION "ImportError"
    )

    # Standalone export test: Hydra-TH
    # Copy the Python script to the build so that we don't get
    # pyc files in the source code generated when we run the test.
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/SimBuilder/PythonExporters/HydraExporter.py
      DESTINATION ${CMB_TEST_DIR} )
    set(EXPORT_TEST_ARGS
      -c ${CMB_TEST_DATA_ROOT}/Export/HydraTHSimpleBox.cmb
      -s ${CMB_TEST_DATA_ROOT}/Export/HydraTH_ahmed_sa.sbi
      -p ${CMB_TEST_DIR}/HydraExporter.py
      -a "Incompressible Navier-Stokes Analysis"
      -o ${CMAKE_CURRENT_BINARY_DIR}/HydraTH_ahmed_sa.cntl
    )
    test_with_text_file_output(SimBuilderStandaloneExportHydraTH
      ${CMAKE_CURRENT_BINARY_DIR}/HydraTH_ahmed_sa.cntl
      ${CMB_TEST_DATA_ROOT}/Export/HydraTHBaseline.cntl
      "${TEST_ENV}"
      ${EXECUTABLE_OUTPUT_PATH}
      export${CMAKE_EXECUTABLE_SUFFIX} ${EXPORT_TEST_ARGS}
    )
    set_tests_properties(SimBuilderStandaloneExportHydraTH
      PROPERTIES
      ENVIRONMENT "PYTHONPATH=${PYTHON_PATH}"
      FAIL_REGULAR_EXPRESSION "ImportError"
    )

    # Standalone export test: ProteusPoisson
    # Two tests: 1 for n file, 1 for p file

    # Copy the Python script to the build so that we don't get
    # pyc files in the source code generated when we run the test.
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/SimBuilder/PythonExporters/ProteusPoisson.py
      DESTINATION ${CMB_TEST_DIR} )
    set(EXPORT_TEST_ARGS
      -c ${CMB_TEST_DATA_ROOT}/Export/cylinder3d.cmb
      -s ${CMB_TEST_DATA_ROOT}/Export/ProteusPoisson.sbi
      -p ${CMB_TEST_DIR}/ProteusPoisson.py
    )
    test_with_text_file_output(SimBuilderStandaloneExportProteusPoissonPhysics
      ${CMAKE_CURRENT_BINARY_DIR}/poisson_p.py
      ${CMB_TEST_DATA_ROOT}/Export/poisson_p.baseline.py
      "${TEST_ENV}"
      ${EXECUTABLE_OUTPUT_PATH}
      export${CMAKE_EXECUTABLE_SUFFIX} ${EXPORT_TEST_ARGS}
    )
    set_tests_properties(SimBuilderStandaloneExportProteusPoissonPhysics
      PROPERTIES
      ENVIRONMENT "PYTHONPATH=${PYTHON_PATH}"
      FAIL_REGULAR_EXPRESSION "ImportError"
    )

    test_with_text_file_output(SimBuilderStandaloneExportProteusPoissonNumerics
      ${CMAKE_CURRENT_BINARY_DIR}/poisson_n.py
      ${CMB_TEST_DATA_ROOT}/Export/poisson_n.baseline.py
      "${TEST_ENV}"
      ${EXECUTABLE_OUTPUT_PATH}
      export${CMAKE_EXECUTABLE_SUFFIX} ${EXPORT_TEST_ARGS}
    )
    set_tests_properties(SimBuilderStandaloneExportProteusPoissonNumerics
      PROPERTIES
      ENVIRONMENT "PYTHONPATH=${PYTHON_PATH}"
      FAIL_REGULAR_EXPRESSION "ImportError"
    )

    # AdHShallowWater extrusion test
    # Use environment var to help test find adh_2d_3d program
    set(env_python "PYTHONPATH=${PYTHON_PATH}")
    set(env_adh2d3d "NOTUSED=notused")
    find_program(ADH_2D_3D_EXE adh_2d_3d${CMAKE_EXECUTABLE_SUFFIX}
      ${CMAKE_BINARY_DIR}/bin
      ${CMAKE_INSTALL_PREFIX}/bin
    )
    #message("ADH_2D_3D = ${ADH_2D_3D_EXE}")
    if (EXISTS ${ADH_2D_3D_EXE})
      set(env_adh2d3d "ADH_2D_3D_EXE=${ADH_2D_3D_EXE}")
    else()
      message(WARNING "Unable to find adh_2d_3d executable")
    endif()

    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/SimBuilder/PythonExporters/AdHShallowWater.py
      DESTINATION ${CMB_TEST_DIR} )
    if(Q_WS_MAC)
      test_with_text_file_output(SimBuilderAdHExtrusion ${CMAKE_CURRENT_BINARY_DIR}/output.bcn
        ${CMB_TEST_DATA_ROOT}/Export/AdHExtrusionBaseline.bcn
        "${TEST_ENV}"
        ${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}.app/Contents/MacOS ${PROJECT_EXE} -dr "--test-directory=${CMB_TEST_DIR}" "--test-script=${TEST_XML}/AdHExtrusion.xml" --exit)
    else(Q_WS_MAC)
      test_with_text_file_output(SimBuilderAdHExtrusion ${CMAKE_CURRENT_BINARY_DIR}/output.bcn
        ${CMB_TEST_DATA_ROOT}/Export/AdHExtrusionBaseline.bcn
        "${TEST_ENV}"
        ${EXECUTABLE_OUTPUT_PATH} ${PROJECT_EXE} -dr "--test-directory=${CMB_TEST_DIR}" "--test-script=${TEST_XML}/AdHExtrusion.xml" --exit)
    endif(Q_WS_MAC)
    set_tests_properties(SimBuilderAdHExtrusion
      PROPERTIES
      ENVIRONMENT "${env_python};${env_adh2d3d}"
      FAIL_REGULAR_EXPRESSION "ImportError"
    )

  endif(BUILD_MEDIUM_TESTS)

  add_subdirectory(Testing/Cxx)

endif(BUILD_TESTING)

project (ModelBuilder)

find_package(Remus REQUIRED)

IF(NOT ParaView_BINARY_DIR)
  FIND_PACKAGE(ParaView REQUIRED)
  INCLUDE(${PARAVIEW_USE_FILE})
ENDIF(NOT ParaView_BINARY_DIR)

find_package(SMTK REQUIRED)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

# Add Qt UI files here
set(UI_FORMS
  pqCMBImportShapefile.ui
  pqCMBModelBuilderOptions.ui
  qtMesherSelector.ui
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
  SimBuilder/SimBuilderExportDialog.cxx
  SimBuilder/qtSimBuilderUIPanel.cxx
  SimBuilder/pqSimBuilderUIManager.cxx
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
if(NOT APPLE)
  #windows or unix
  install(DIRECTORY Icons DESTINATION "${VTK_INSTALL_RUNTIME_DIR}/Resources")
else()
  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/../Resources/qt.conf
    DESTINATION "${VTK_INSTALL_RUNTIME_DIR}/${PROJECT_NAME}.app/Contents/Resources")
  install(DIRECTORY Icons DESTINATION "${VTK_INSTALL_RUNTIME_DIR}/${PROJECT_NAME}.app/Contents/Resources")
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
  pqCMBModelBuilderMainWindow.cxx
  pqCMBModelBuilderMainWindowCore.cxx
  pqModelBuilderViewContextMenuBehavior.cxx
  pqModelBuilderViewContextMenuBehavior.h
  pqCMBContextMenuHelper.cxx
  pqCMBContextMenuHelper.h
  pqCMBModelBuilderOptions.cxx
  pqCMBModelBuilderOptions.h
  pqCMBImportShapefile.h
  pqCMBImportShapefile.cxx
  pqCMBModelManager.h
  pqCMBModelManager.cxx
  pqCheckableComboPopupEventPlayer.h
  pqCheckableComboPopupEventPlayer.cxx
  pqCheckableComboPopupEventTranslator.h
  pqCheckableComboPopupEventTranslator.cxx
  pqContextMenuEventTranslator.h
  pqContextMenuEventTranslator.cxx
  pqModelTreeViewEventTranslator.h
  pqModelTreeViewEventTranslator.cxx
  pqModelTreeViewEventPlayer.h
  pqModelTreeViewEventPlayer.cxx
  pqSMTKMeshInfo.h
  pqSMTKMeshInfo.cxx
  pqSMTKMeshPanel.h
  pqSMTKMeshPanel.cxx
  pqSMTKModelInfo.h
  pqSMTKModelInfo.cxx
  pqSMTKModelPanel.h
  pqSMTKModelPanel.cxx
  qtRemusMesherSelector.cxx
  qtRemusMesherSelector.h

  ${RCS_SOURCES}
  ${UI_BUILT_SOURCES}
  ${EXE_RES}
  ${apple_bundle_sources}
  ${SimBuilder_SOURCES}
  )
#------------------------------------------------------------------------------
set(CMB_ALL_PLUGINS
    CMB_Plugin
    ModelBridge_Plugin

    smtkCGMSessionPlugin
    smtkDiscreteSessionPlugin
    smtkExodusSessionPlugin
    smtkPolygonSessionPlugin
    smtkRemoteSessionPlugin
    smtkRemusMeshOperatorPlugin
    smtkVTKOperatorsPlugin
)

set(CMB_AUTOLOAD_PLUGINLIST)
foreach (plugin IN LISTS CMB_ALL_PLUGINS)
  if (TARGET "${plugin}")
    list(APPEND CMB_AUTOLOAD_PLUGINLIST
      "${plugin}")
  endif ()
endforeach ()

set(CMB_OPTIONAL_PLUGINS "" CACHE STRING
    "Semicolon separated optional plugin names.")
mark_as_advanced(CMB_OPTIONAL_PLUGINS)
list(APPEND CMB_OPTIONAL_PLUGINS ${CMB_AUTOLOAD_PLUGINLIST})

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
  OPTIONAL_PLUGINS
    ${CMB_OPTIONAL_PLUGINS}
  INSTALL_BIN_DIR "${VTK_INSTALL_RUNTIME_DIR}"
  INSTALL_LIB_DIR "${VTK_INSTALL_LIBRARY_DIR}"
)

#------------------------------------------------------------------------------
# Add #define for cumulus
target_compile_definitions(
  ModelBuilder PRIVATE "$<$<BOOL:${ENABLE_Cumulus}>:ENABLE_JOBS_PANEL>")

#let cmake do what qt4_wrap_cpp used to do automatically
set_target_properties(ModelBuilder PROPERTIES AUTOMOC TRUE)

#we need to explicitly state that you shouldn't build ModelBuilder
#before the help has been generated
add_dependencies(ModelBuilder CMBSuiteHelp)

target_link_libraries(ModelBuilder
LINK_PRIVATE
  cmbAppCommon
  smtkQtExt
  smtkPQComponentsExt
  smtkRemusExt
  RemusClient
  smtkCore
  ModelBridge_Plugin
  vtkSMTKSourceExt
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
  set(GrowTest_THRESHOLD 14)
  set(GrowTestCMB_THRESHOLD 14)
endif(BUILD_TESTING)

########################################################################
# write, copy and install the .plugin file for testing.
if(BUILD_TESTING)
  set (cmb_test_plugin_ini "<?xml version=\"1.0\"?>\n<Plugins>\n")
  foreach (cmb-plugin-name IN LISTS CMB_OPTIONAL_PLUGINS)
    set(found FALSE)
    foreach (plugin-path IN LISTS CMB_TEST_PLUGIN_PATHS)
      if(WIN32 AND NOT CYGWIN)
        set(full-plugin-path "${plugin-path}/${cmb-plugin-name}.dll")
      elseif(APPLE)
        set(full-plugin-path "${plugin-path}/lib${cmb-plugin-name}.dylib")
      elseif(UNIX)
        set(full-plugin-path "${plugin-path}/lib${cmb-plugin-name}.so")
      endif()

      if(EXISTS ${full-plugin-path})
        set(cmb_test_plugin_ini
          "${cmb_test_plugin_ini}  <Plugin name=\"${cmb-plugin-name}\" auto_load=\"1\" filename=\"${full-plugin-path}\" />\n")
        set(found TRUE)
        break()
      endif()
    endforeach()
    if (NOT found)
      # Assume ParaView will know how to find it.
      set(cmb_test_plugin_ini
        "${cmb_test_plugin_ini}  <Plugin name=\"${cmb-plugin-name}\" auto_load=\"1\" />\n")
    endif ()
  endforeach()

  set (cmb_test_plugin_ini "${cmb_test_plugin_ini}</Plugins>\n")
  file(WRITE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/.plugins" "${cmb_test_plugin_ini}")

  # Install the .plugins configuration file.
  if(APPLE)
    set(install_plugin_path "${VTK_INSTALL_RUNTIME_DIR}/${PROJECT_NAME}.app/Contents/Plugins")
  else()
    set(install_plugin_path "${VTK_INSTALL_RUNTIME_DIR}")
  endif()

  install(FILES "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/.plugins"
          DESTINATION "${install_plugin_path}"
          COMPONENT Runtime)
endif()
########################################################################


set(PROJECT_EXE ${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX})
if(BUILD_TESTING)
 set(TEST_XML ${ModelBuilder_SOURCE_DIR}/Testing/XML)
 set(TEST_SIMBUILDER_XML ${ModelBuilder_SOURCE_DIR}/SimBuilder/XML)
 set(TEST_PYTHON ${ModelBuilder_SOURCE_DIR}/Testing/Python)
 set(TEST_IMAGE ${CMB_TEST_DATA_ROOT}/baseline/cmb_application)

# Need path to python executable
find_package(PythonInterp)
if (PYTHONINTERP_FOUND)

  set(PYTHON_DEFINES
    CMAKE_PYTHON_EXECUTABLE="${PYTHON_EXECUTABLE}"
    CMAKE_PYTHON_VERSION="${PYTHON_VERSION_STRING}"
  )

  set_property(
    SOURCE export.cxx
    APPEND PROPERTY COMPILE_DEFINITIONS ${PYTHON_DEFINES}
  )
endif()

  #Tests that don't work on Apple

  set (APPLE_EXCLUDE_TESTS
    HelpDialog
    )
  set (APPLE_ONLY_TESTS
    )

  set (XML_TESTS_WITH_BASELINES
    AxesGrid
    CloseSession
    EdgeModification
    #ChesapeakeBayBathymetry
    OpenCMBFile
    OpenExodusFile
    OpenExodusFile2
    OpenHybridFile
    #OpenMapFile
    #OpenVTKFile
    Open2dmFile
    Open2dmFile2
    Open3dmFile
    Open3dmFile2
    FlipViewX
    FlipViewY
    FlipViewZ
    ShowCenter
    #Simple2dmBathymetryTest
    SelectObjectByBox
    Rotate2D
    Rotate3D
    RendererPoints
    RendererSurfaceWithEdges
    RendererWireframe
    RulerDialog
    OpacityLower
    OpenCMBFile2
    HideFace
    HideOtherFaces
    ManipulateCameraRoll
    ManipulateCameraElevation
    ManipulateCameraAzimuth
    #Model1meshedBathymetryTest
    ViewportZoom
    ObjectlessCenterRotation
    RotateAlongPickedCenter
    RotationButtons
    ViewportPan
    HideShowAllFaces
    OperationGrow
    OperationCreateEdges
    OperationSplitFace
    OperationMergeFace
    OperationGroupOps
    #OperationGroupOpCGM
    OperationBoundaryGroup2D
    OperationDomainGroup2D
    #OperationCombined
    WriteEdge
    Import2dmAndEdgeModify
    ImportExport2dm
    ModelAndMeshTransformLink
    ColorByAttribute
    ExtractImageContours
    AuxGeoVtpPts
    AuxGeoTif
    AuxGeoDEM
    DeleteAuxGeom
    ZoomToSelectionCMB
    ZoomToSelectionSTP
    )

  set (TESTS_WITHOUT_BASELINES
    AboutDialog
    HelpDialog
    AttributeOptionalItemUI
    LoadCRFWithoutModel
    )

  set (TEST_WITH_POSTPROCESS
    VerifyAttributeSave
    )

  set (TESTS_FOR_VERIFYSIMEXPORT
    ExportSimAdHShallow
    ExportSimAdHSurface
    ExportSimDakota
    ExportSimHydra
    ExportSimProteus
    )


  foreach(test ${TESTS_WITHOUT_BASELINES})
    #Run the test if it is on the correct platform
    #
    list(FIND APPLE_EXCLUDE_TESTS ${test} APPLE_FIND_RES)

    if(NOT APPLE OR NOT APPLE_FIND_RES GREATER -1)
      add_long_test(${PROJECT_NAME}${test}
    ModelBuilder -dr
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
    ModelBuilder -dr
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
      ModelBuilder -dr
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
  if (CMB_SUPERBUILD_DEVELOPER_ROOT)
    set(prefix "${CMB_SUPERBUILD_DEVELOPER_ROOT}")
  else ()
    set(prefix "${CMAKE_PREFIX_PATH}")
  endif ()

  set(paraview_dir "paraview-${PARAVIEW_VERSION_MAJOR}.${PARAVIEW_VERSION_MINOR}")
  #message("paraview_dir ${paraview_dir}")

  if (APPLE)
    set(PYTHON_PATH "${prefix}/Applications/paraview.app/Contents/Python:${prefix}/lib/python2.7/site-packages:${prefix}/lib/${paraview_dir}/site-packages")
    set(DYLD_FALLBACK_LIBRARY_PATH "${prefix}/lib/${paraview_dir}:${prefix}/lib")
    set(TEST_ENV "PYTHONPATH=${PYTHON_PATH};DYLD_FALLBACK_LIBRARY_PATH=${DYLD_FALLBACK_LIBRARY_PATH}")
  elseif(UNIX)
    set(PYTHON_PATH "${prefix}/lib/${paraview_dir}/site-packages:${prefix}/lib/python2.7/site-packages")
    set(LD_LIBRARY_PATH "${prefix}/lib/${paraview_dir}:${prefix}/lib")
    set(TEST_ENV "PYTHONPATH=${PYTHON_PATH};LD_LIBRARY_PATH=${LD_LIBRARY_PATH}")
  else() #windows
    # Use escaped-semicolon for PYTHONPATH, so that it can be part of a list (TEST_ENV)
    set(PYTHON_PATH "${prefix}/bin/Lib\;${prefix}/bin/Lib/site-packages")
    set(TEST_ENV "PYTHONPATH=${PYTHON_PATH}")
  endif()

  set(POSTPROCESS_RUNNER "${TEST_PYTHON}/RunModelBuilderAndVerify.py")
  if(CMB_TEST_DATA_ROOT)
    foreach(test ${TEST_WITH_POSTPROCESS})
      #Run the test if it is on the correct platform
      #
      list(FIND APPLE_EXCLUDE_TESTS ${test} APPLE_FIND_RES)

      if(NOT APPLE OR NOT APPLE_FIND_RES GREATER -1)
        add_medium_test(${PROJECT_NAME}${test}Py
          ${PYTHON_EXECUTABLE} ${POSTPROCESS_RUNNER}
          $<TARGET_FILE:ModelBuilder>
          ${CMB_TEST_DIR}
          ${TEST_XML}/${test}.xml
          ${PYTHON_EXECUTABLE}
          ${TEST_PYTHON}/${test}.py
          ${CMB_TEST_DATA_ROOT})
        set_tests_properties(${PROJECT_NAME}${test}Py
          PROPERTIES
            ENVIRONMENT "${TEST_ENV}"
        )
      endif()
    endforeach(test)

    # simexport tests
    foreach(test ${TESTS_FOR_VERIFYSIMEXPORT})
      #Run the test if it is on the correct platform
      #
      list(FIND APPLE_EXCLUDE_TESTS ${test} APPLE_FIND_RES)

      # VerifySimExport.py is hardcoded in here
      # The script parses the testscript path to find the testname
      if(NOT APPLE OR NOT APPLE_FIND_RES GREATER -1)
      add_medium_test(${PROJECT_NAME}${test}Py
        ${PYTHON_EXECUTABLE} ${POSTPROCESS_RUNNER}
        $<TARGET_FILE:ModelBuilder>
        ${CMB_TEST_DIR}
        ${TEST_XML}/${test}.xml
        ${PYTHON_EXECUTABLE}
        ${TEST_PYTHON}/VerifySimExport.py
        ${CMB_TEST_DATA_ROOT})
      set_tests_properties(${PROJECT_NAME}${test}Py
        PROPERTIES
          ENVIRONMENT "${TEST_ENV}"
      )
      endif()

    endforeach(test)
  endif(CMB_TEST_DATA_ROOT)

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

endif(BUILD_TESTING)

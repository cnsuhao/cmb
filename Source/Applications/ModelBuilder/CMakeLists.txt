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
  pqCMBModelBuilderMainWindow.cxx
  pqCMBModelBuilderMainWindowCore.cxx
  pqModelBuilderViewContextMenuBehavior.cxx
  pqModelBuilderViewContextMenuBehavior.h
  pqCMBModelBuilderOptions.cxx
  pqCMBModelBuilderOptions.h
  pqCMBImportShapefile.h
  pqCMBImportShapefile.cxx
  pqSMTKMeshPanel.h
  pqSMTKMeshPanel.cxx
  pqSMTKModelPanel.h
  pqSMTKModelPanel.cxx
  pqCMBModelManager.h
  pqCMBModelManager.cxx
  pqCheckableComboPopupEventTranslator.h
  pqCheckableComboPopupEventTranslator.cxx
  pqContextMenuEventTranslator.h
  pqContextMenuEventTranslator.cxx
  pqModelTreeViewEventTranslator.h
  pqModelTreeViewEventTranslator.cxx
  pqModelTreeViewEventPlayer.h
  pqModelTreeViewEventPlayer.cxx
  qtRemusMesherSelector.cxx
  qtRemusMesherSelector.h

  ${RCS_SOURCES}
  ${UI_BUILT_SOURCES}
  ${EXE_RES}
  ${apple_bundle_sources}
  ${SimBuilder_SOURCES}
  )
#------------------------------------------------------------------------------
set(  CMB_AUTOLOAD_PLUGINLIST
    smtkCGMSessionPlugin
    smtkDiscreteSessionPlugin
    smtkExodusSessionPlugin
    smtkRemoteSessionPlugin
    smtkRemusMeshOperatorPlugin
)

if(NOT CMB_OPTIONAL_PLUGINS)
  set(CMB_OPTIONAL_PLUGINS ${CMB_AUTOLOAD_PLUGINLIST} CACHE STRING
     "Semi-colon seperated optional plugin names.")
  mark_as_advanced(CMB_OPTIONAL_PLUGINS)
endif()

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

#let cmake do what qt4_wrap_cpp used to do automatically
set_target_properties(ModelBuilder PROPERTIES AUTOMOC TRUE)

#we need to explicitly state that you shouldn't build ModelBuilder
#before the help has been generated
add_dependencies(ModelBuilder CMBSuiteHelp)

target_link_libraries(ModelBuilder
LINK_PRIVATE
  cmbAppCommon
  smtkQtExt
  RemusClient
  smtkCore
  ModelBridge_Plugin
  vtkSMTKExt
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

########################################################################
# write, copy and install the .plugin file for testing.
if(BUILD_TESTING AND NOT ("${CMB_TEST_PLUGIN_PATHS}" STREQUAL ""))
  set (cmb_test_plugin_ini "<?xml version=\"1.0\"?>\n<Plugins>\n")
  foreach (cmb-plugin-name IN LISTS CMB_OPTIONAL_PLUGINS)
    foreach (plugin-path IN LISTS CMB_TEST_PLUGIN_PATHS)
      if(WIN32 AND NOT CYGWIN)
        set(full-plugin-path "${plugin-path}/${cmb-plugin-name}.dll")
      elseif(APPLE)
        set(full-plugin-path "${plugin-path}/lib${cmb-plugin-name}.dylib")
      elseif(UNIX)
        set(full-plugin-path "${plugin-path}/lib${cmb-plugin-name}.so")
      endif()

      # message(STATUS "full-plugin-path=${full-plugin-path}")
      if(EXISTS ${full-plugin-path})
          set(cmb_test_plugin_ini
            "${cmb_test_plugin_ini}  <Plugin name=\"${cmb-plugin-name}\" auto_load=\"1\" filename=\"${full-plugin-path}\" />\n")
        list(APPEND TEST_PLUGIN_LIBS ${full-plugin-path})
      endif()
    endforeach()
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

# message(STATUS "TEST_BINARY=${TEST_BINARY}")

  #Tests that don't work on Apple

  set (APPLE_EXCLUDE_TESTS
    HelpDialog
    )
  set (APPLE_ONLY_TESTS
    )

  set (XML_TESTS_WITH_BASELINES
    EdgeModification
    OpenCMBFile
    OpenExodusFile
    #OpenVTKFile
    FlipViewX
    FlipViewY
    FlipViewZ
    ShowCenter
    SelectObjectByBox
    Rotate2D
    Rotate3D
    RendererPoints
    RendererSurfaceWithEdges
    RendererWireframe
    OpacityLower
    ShowAxis
    OpenCMBFile2
    HideFace
    HideOtherFaces
    ManipulateCameraRoll
    ManipulateCameraElevation
    ManipulateCameraAzimuth
    ViewportZoom
    ObjectlessCenterRotation
    RotateAlongPickedCenter
    ViewportPan
    HideShowAllFaces

    )

  set (TESTS_WITHOUT_BASELINES
    HelpDialog
    AboutDialog
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

  add_subdirectory(Testing/Cxx)

endif(BUILD_TESTING)

project (GeologyBuilder)

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
   ../../CMBModel/Plugin
  ${vtkCmbDiscreteModel_INCLUDE_DIRS}
  ${cmbAppCommon_SOURCE_DIR}
  ${cmbAppCommon_BINARY_DIR}
  ${cmbAppCommon_SOURCE_DIR}/Scene
  ${vtkDiscreteModel_INCLUDE_DIRS}
  ${vtkDiscreteModel_SOURCE_DIR}/Model
  )

# Add Qt UI files here
set(UI_FORMS
  qtCMBGeologyBuilderPanel.ui
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
  set(EXE_ICON ${CMAKE_CURRENT_SOURCE_DIR}/cmbGeologyBuilder.rc)
  set(EXE_RES ${CMAKE_CURRENT_BINARY_DIR}/cmbGeologyBuilder.res)
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

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmbGeologyBuilderConfig.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/cmbGeologyBuilderConfig.h
  ESCAPE_QUOTES)

set(GeologyBuilder_SOURCE_FILES
  pqCMBGeologyBuilderMainWindowCore.cxx
  pqCMBGeologyBuilderMainWindow.cxx
  qtCMBGeologyBuilderPanelWidget.cxx
  qtCMBGeologyBuilderContextMenuBehavior.cxx
  qtCMBGeologyBuilderContextMenuBehavior.h

  ${MOC_BUILT_SOURCES}
  ${RCS_SOURCES}
  ${UI_BUILT_SOURCES}
  ${EXE_RES}
  ${apple_bundle_sources}
  )

#------------------------------------------------------------------------------
# Build the client
build_paraview_client(GeologyBuilder
  TITLE "Geology Builder ${CMB_VERSION_MAJOR}.${CMB_VERSION_MINOR}.${CMB_VERSION_PATCH}"
  ORGANIZATION  "Kitware"
  VERSION_MAJOR ${CMB_VERSION_MAJOR}
  VERSION_MINOR ${CMB_VERSION_MINOR}
  VERSION_PATCH ${CMB_VERSION_PATCH}
  SPLASH_IMAGE "${CMAKE_CURRENT_SOURCE_DIR}/../Resources/Icons/GeologyBuilderSplash.png"
  PVMAIN_WINDOW pqCMBGeologyBuilderMainWindow
  PVMAIN_WINDOW_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/pqCMBGeologyBuilderMainWindow.h
  BUNDLE_ICON   "${CMAKE_CURRENT_SOURCE_DIR}/MacIcon.icns"
  APPLICATION_ICON  "${CMAKE_CURRENT_SOURCE_DIR}/geologybuilderappico.ico"
  SOURCES ${GeologyBuilder_SOURCE_FILES}
  COMPRESSED_HELP_FILE "${CMAKE_CURRENT_BINARY_DIR}/../Help/cmbsuite.qch"
  INSTALL_BIN_DIR "${VTK_INSTALL_RUNTIME_DIR}"
  INSTALL_LIB_DIR "${VTK_INSTALL_LIBRARY_DIR}"
  )

#let cmake do what qt4_wrap_cpp used to do automatically
set_target_properties(GeologyBuilder PROPERTIES AUTOMOC TRUE)

#we need to explicitly state that you shouldn't build GeologyBuilder
#before the help has been generated
add_dependencies(GeologyBuilder CMBSuiteHelp)

target_link_libraries(GeologyBuilder
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
     \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${VTK_INSTALL_RUNTIME_DIR}/GeologyBuilder.app/Contents/MacOS/GeologyBuilder\"
     \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${VTK_INSTALL_RUNTIME_DIR}/GeologyBuilder.app\"
     \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${VTK_INSTALL_LIBRARY_DIR}\")

   # Place the App at the requested location.
   file(INSTALL DESTINATION \"${MACOSX_APP_INSTALL_PREFIX}\"
        TYPE DIRECTORY FILES
          \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${VTK_INSTALL_RUNTIME_DIR}/GeologyBuilder.app\"
        USE_SOURCE_PERMISSIONS)
   "
   COMPONENT Runtime)
endif()

if(BUILD_TESTING)
  set(TEST_XML ${GeologyBuilder_SOURCE_DIR}/Testing/XML)
  set(TEST_IMAGE ${CMB_TEST_DATA_ROOT}/Baseline)

  #Tests that don't work on Apple
  #
  set (APPLE_EXCLUDE_TESTS
    )

  set (XML_TESTS_WITH_BASELINES
  BorFileLoading
  MultiBorFileLoading
  BorObjectsColoring
  )

  set (TESTS_WITHOUT_BASELINES
    )

  foreach(test ${TESTS_WITHOUT_BASELINES})
    #Run the test if it is on the correct platform
    #
    list(FIND APPLE_EXCLUDE_TESTS ${test} APPLE_FIND_RES)

    if(NOT APPLE OR NOT APPLE_FIND_RES GREATER -1)
      add_medium_test(GeologyBuilder${test}
        GeologyBuilder
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
      add_medium_test(GeologyBuilder${test}
      GeologyBuilder
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
  #add_subdirectory(Testing)
endif(BUILD_TESTING)

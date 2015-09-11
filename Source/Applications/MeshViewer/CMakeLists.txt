project (MeshViewer)

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${vtkCmbDiscreteModel_INCLUDE_DIRS}
  ${cmbAppCommon_SOURCE_DIR}
  ${cmbAppCommon_BINARY_DIR}
  ${cmbAppCommon_SOURCE_DIR}/Scene
  )

# Add Qt UI files here
set(UI_FORMS
  qtMeshViewerPanel.ui
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
  set(EXE_ICON ${CMAKE_CURRENT_SOURCE_DIR}/MeshViewer.rc)
  set(EXE_RES ${CMAKE_CURRENT_BINARY_DIR}/MeshViewer.res)
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

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmbMeshViewerConfig.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/cmbMeshViewerConfig.h
  ESCAPE_QUOTES)

set(MeshViewer_SOURCE_FILES
  pqCMBMeshViewerMainWindowCore.cxx
  pqCMBMeshViewerMainWindow.cxx
  qtCMBMeshViewerPanelWidget.cxx

  ${RCS_SOURCES}
  ${UI_BUILT_SOURCES}
  ${EXE_RES}
  ${apple_bundle_sources}
  )

#------------------------------------------------------------------------------
# Build the client

include(ParaViewBranding)

build_paraview_client(MeshViewer
  TITLE "Mesh Viewer ${CMB_VERSION_MAJOR}.${CMB_VERSION_MINOR}.${CMB_VERSION_PATCH}"
  ORGANIZATION  "Kitware"
  VERSION_MAJOR ${CMB_VERSION_MAJOR}
  VERSION_MINOR ${CMB_VERSION_MINOR}
  VERSION_PATCH ${CMB_VERSION_PATCH}
  SPLASH_IMAGE "${CMAKE_CURRENT_SOURCE_DIR}/../Resources/Icons/MeshViewerSplash.png"
  APPLICATION_ICON  "${CMAKE_CURRENT_SOURCE_DIR}/meshviewerappico.ico"
  PVMAIN_WINDOW pqCMBMeshViewerMainWindow
  PVMAIN_WINDOW_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/pqCMBMeshViewerMainWindow.h
  BUNDLE_ICON   "${CMAKE_CURRENT_SOURCE_DIR}/MacIcon.icns"
  SOURCES ${MeshViewer_SOURCE_FILES}
  COMPRESSED_HELP_FILE "${CMAKE_CURRENT_BINARY_DIR}/../Help/cmbsuite.qch"
  INSTALL_BIN_DIR "${VTK_INSTALL_RUNTIME_DIR}"
  INSTALL_LIB_DIR "${VTK_INSTALL_LIBRARY_DIR}"
  )

#let cmake do what qt4_wrap_cpp used to do automatically
set_target_properties(MeshViewer PROPERTIES AUTOMOC TRUE)

#we need to explicitly state that you shouldn't build MeshViewer
#before the help has been generated
add_dependencies(MeshViewer CMBSuiteHelp)

target_link_libraries(MeshViewer
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
       \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${VTK_INSTALL_RUNTIME_DIR}/MeshViewer.app/Contents/MacOS/MeshViewer\"
       \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${VTK_INSTALL_RUNTIME_DIR}/MeshViewer.app\"
       \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${VTK_INSTALL_LIBRARY_DIR}\")

     # Place the App at the requested location.
     file(INSTALL DESTINATION \"${MACOSX_APP_INSTALL_PREFIX}\"
          TYPE DIRECTORY FILES
            \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${VTK_INSTALL_RUNTIME_DIR}/MeshViewer.app\"
          USE_SOURCE_PERMISSIONS)
     "
   COMPONENT Runtime)
endif()

if(BUILD_TESTING)
  if(Q_WS_MAC)
  set(TEST_BINARY ${EXECUTABLE_OUTPUT_PATH}/MeshViewer.app/Contents/MacOS/MeshViewer)
  else()
   set(TEST_BINARY ${EXECUTABLE_OUTPUT_PATH}/MeshViewer)
  endif()

  set(TEST_XML ${MeshViewer_SOURCE_DIR}/Testing/XML)
  set(TEST_IMAGE ${CMB_TEST_DATA_ROOT}/Baseline)

  #Tests that don't work on Apple
  #
  set (APPLE_EXCLUDE_TESTS
    )

  set (XML_TESTS_WITH_BASELINES
    MeshSelection
    MeshChangeMaterial
    # XXX: nodata(stone3.2dm): 2DMeshSculpting
    3DMeshSculpting
    InvertSelection
    # XXX: nodata(stone3.2dm): ContourSelection2dm
    ContourSelection3dm
    # XXX: nodata(stone3.2dm): MeshWriteRead2dm
    MeshWriteRead3dm
    # XXX: nodata(stone3.2dm): ContourPlane2dm
    # XXX: nodata(stone3.2dm): MeshSmoothing2dm
    MeshSmoothing3dm
    # XXX: nodata(test2dmesh_domainset.2dm): ConeSelection2dm
    # XXX: nodata(mesh_cyl_frame_2regions.3dm): ConeSelection3dm
  )

  set (TESTS_WITHOUT_BASELINES
  AboutTest
  HelpTest
  # XXX: nodata(stone3.2dm): MeshSelectionTools
    )

  foreach(test ${TESTS_WITHOUT_BASELINES})
    #Run the test if it is on the correct platform
    #
    list(FIND APPLE_EXCLUDE_TESTS ${test} APPLE_FIND_RES)

    if(NOT APPLE OR NOT APPLE_FIND_RES GREATER -1)
      add_medium_test(MeshViewer${test}
        ${TEST_BINARY}
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
      add_medium_test(MeshViewer${test}
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

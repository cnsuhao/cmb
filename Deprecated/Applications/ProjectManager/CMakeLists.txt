project (ProjectManager)

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${cmbAppCommon_SOURCE_DIR}
  ${cmbAppCommon_BINARY_DIR}
  ${ProjectManager_BINARY_DIR}
  ${ProjectManager_SOURCE_DIR}
  )

# Add Qt UI files here
set(UI_FORMS
  UI/qtProjectManager.ui
  UI/qtProjectDirectoryDialog.ui
  UI/qtProjectFileToLoadDialog.ui
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
  set(EXE_ICON ${CMAKE_CURRENT_SOURCE_DIR}/ProjectManager.rc)
  set(EXE_RES ${CMAKE_CURRENT_BINARY_DIR}/ProjectManager.res)
  add_custom_command(
    OUTPUT ${EXE_RES}
    COMMAND rc.exe
    ARGS /fo ${EXE_RES} ${EXE_ICON}
    )
endif(WIN32)

source_group("Resources" FILES
  ${UI_RESOURCES}
  ${UI_FORMS}
  )

source_group("Generated" FILES
  ${RCS_SOURCES}
  ${UI_BUILT_SOURCES}
  )

if(Q_WS_MAC)
  set(apple_bundle_sources "MacIcon.icns")
  set_source_files_properties(
    "MacIcon.icns"
    PROPERTIES
    MACOSX_PACKAGE_LOCATION Resources
    )
  set(MACOSX_BUNDLE_ICON_FILE MacIcon.icns)
endif(Q_WS_MAC)

IF(QT_MAC_USE_COCOA)
  GET_FILENAME_COMPONENT(qt_menu_nib
    "@QT_QTGUI_LIBRARY_RELEASE@/Resources/qt_menu.nib"
    REALPATH)

  set(qt_menu_nib_sources
    "${qt_menu_nib}/classes.nib"
    "${qt_menu_nib}/info.nib"
    "${qt_menu_nib}/keyedobjects.nib"
    )
  SET_SOURCE_FILES_PROPERTIES(
    ${qt_menu_nib_sources}
    PROPERTIES
    MACOSX_PACKAGE_LOCATION Resources/qt_menu.nib
    )
  ELSE(QT_MAC_USE_COCOA)
    set(qt_menu_nib_sources)
ENDIF(QT_MAC_USE_COCOA)


if(APPLE)
  set(MACOSX_BUNDLE_BUNDLE_NAME "ProjectManager")
  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/../Resources/qt.conf
    DESTINATION "${VTK_INSTALL_RUNTIME_DIR}/ProjectManager.app/Contents/Resources")
endif(APPLE)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmbProjectManagerConfig.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/cmbProjectManagerConfig.h
  ESCAPE_QUOTES)

set(ProjectManager_SOURCE_FILES
  main.cxx
  qtCMBProjectDirectoryDialog.cxx
  qtCMBProjectDirectoryDialog.h
  qtCMBProjectFileToLoadDialog.cxx
  qtCMBProjectFileToLoadDialog.h
  qtCMBProjectManager.cxx
  qtCMBProjectManager.h
  ${RCS_SOURCES}
  ${UI_BUILT_SOURCES}
  ${EXE_RES}
  ${apple_bundle_sources}
  ${qt_menu_nib_sources}
  )

#------------------------------------------------------------------------------
# Build the client
add_executable(ProjectManager WIN32 MACOSX_BUNDLE
  ${ProjectManager_SOURCE_FILES}
  )

#let cmake do what qt4_wrap_cpp used to do automatically
set_target_properties(ProjectManager PROPERTIES AUTOMOC TRUE)

target_link_libraries(ProjectManager
LINK_PRIVATE
 cmbAppCommon ${QT_QTMAIN_LIBRARY})

install(TARGETS ProjectManager
        DESTINATION ${VTK_INSTALL_RUNTIME_DIR}
        COMPONENT Runtime)

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
    \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${VTK_INSTALL_RUNTIME_DIR}/ProjectManager.app/Contents/MacOS/ModelBuilder\"
    \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${VTK_INSTALL_RUNTIME_DIR}/ProjectManager.app\"
    \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${VTK_INSTALL_LIBRARY_DIR}\")

  # Place the App at the requested location.
  file(INSTALL DESTINATION \"${MACOSX_APP_INSTALL_PREFIX}\"
      TYPE DIRECTORY FILES
        \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${VTK_INSTALL_RUNTIME_DIR}/ProjectManager.app\"
      USE_SOURCE_PERMISSIONS)
      "
    COMPONENT Runtime)
endif()

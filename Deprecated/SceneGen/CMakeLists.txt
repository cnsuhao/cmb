project(SceneGen)
find_package(Qt4 REQUIRED)
find_package(OpenGL REQUIRED)

set(QT_USE_QTMAIN TRUE)
set(QT_USE_QTOPENGL 1)
include(${QT_USE_FILE})
cmake_minimum_required(VERSION 2.4.5)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

if(APPLE)
 add_definitions(-DMAC_COMPILE=1)
endif(APPLE)


# Add files to be wrapped using Qt moc.
qt4_wrap_cpp(MOC_BUILT_SOURCES
  BaseMatIDDialog.h
  cOSDL.h
  cObjectGeometry.h
  dlgBaseMatID.h
  dlgCameraLocation.h
  dlgNew.h
  dlgObjectImport.h
  dlgScale.h
  SceneGenMainWnd.h
  ViewComb.h
  ViewWnd.h
  )

set(SceneGen_SOURCES
  aabb.h
  Accelerator.h
  cAOI.h
  cObjectDefinition.h
  cObjectGeometry.h
  cOmicron.h
  ControlSettings.h
  cOSDL.h
  cOutputs.h
  CrossPlatform.h
  cSurfaceObjectDefinition.h
  cVeg.h
  EulerAngles.h
  gm.h
  gmConst.h
  gmMat3.h
  gmMat4.h
  gmUtils.h
  gmVec2.h
  gmVec3.h
  gmVec4.h
  OGLCamera.h
  OGLMaterial.h
  OrthoControl.h
  PathFix.h
  PerspControl.h
  QuatTypes.h
  SceneGen.h
  triangle.h
  ViewMenu.h
  ViewWnd.h
  SceneGenMainWnd.h
  ViewComb.h
  cTWI.h
  dlgBaseMatID.h
  dlgCameraLocation.h
  dlgScale.h
  dlgObjectImport.h
  dlgNew.h
  main.cpp
  ViewWnd.cpp
  aabb.cpp
  Accelerator.cpp
  cAOI.cpp
  cObjectDefinition.cpp
  cObjectGeometry.cpp
  cOmicron.cpp
  cOSDL.cpp
  cOutputs.cpp
  cSurfaceObjectDefinition.cpp
  cVeg.cpp
  EulerAngles.cpp
  gmMat3.cpp
  gmMat4.cpp
  OGLCamera.cpp
  OGLMaterial.cpp
  OrthoControl.cpp
  PathFix.cpp
  PerspControl.cpp
  triangle.cpp
  ViewMenu.cpp
  SceneGenMainWnd.cpp
  ViewComb.cpp
  cTWI.cpp
  dlgBaseMatID.cpp
  dlgCameraLocation.cpp
  dlgScale.cpp
  dlgObjectImport.cpp
  dlgNew.cpp
  ${MOC_BUILT_SOURCES}
)

set(UI_RESOURCES SceneGen.qrc)

qt4_add_resources(RCS_SOURCES
  ${UI_RESOURCES}
  )

if(WIN32)
  set(EXE_TYPE WIN32)
endif(WIN32)

if(APPLE)
  set(EXE_TYPE MACOSX_BUNDLE)
  set(MACOSX_BUNDLE_BUNDLE_NAME "SceneGen")
endif(APPLE)

if(Q_WS_MAC)
  set(apple_bundle_sources "MacIcon.icns")
  set_source_files_properties(
    "MacIcon.icns"
    PROPERTIES
    MACOSX_PACKAGE_LOCATION Resources
    )
  set(MACOSX_BUNDLE_ICON_FILE MacIcon.icns)
endif(Q_WS_MAC)
add_executable(SceneGen ${EXE_TYPE}
  ${SceneGen_SOURCES}
  ${RCS_SOURCES}
  ${apple_bundle_sources})

target_link_libraries(SceneGen ${QT_LIBRARIES})
target_link_libraries(SceneGen ${OPENGL_gl_LIBRARY})
target_link_libraries(SceneGen ${OPENGL_glu_LIBRARY})

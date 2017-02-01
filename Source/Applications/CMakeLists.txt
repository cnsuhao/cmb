find_package(Qt4 REQUIRED COMPONENTS QtXml QtNetwork QtCore QtGui QtMain)
pv_find_package_qt(qt_targets REQUIRED
  QT4_COMPONENTS QtXml QtNetwork QtCore QtGui QtMain)

set(test_source_directories "${ParaView_SOURCE_DIR}/Qt/Testing")
set(test_build_directories "${ParaView_DIR}/Qt/Testing")

#even more common includes
include_directories(
  ${PARAVIEW_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}/../VTKExtensions/Client
  ../VTKExtensions/Client
  ${vtkCMBFiltering_INCLUDE_DIRS}
  ${vtkCMBGeneral_INCLUDE_DIRS}
  ${vtkCMBGraphics_INCLUDE_DIRS}
  ${vtkCMBIO_INCLUDE_DIRS}
  )

option(ENABLE_Cumulus "Build Cumulus/Jobs panel" OFF)
mark_as_advanced(ENABLE_Cumulus)

add_subdirectory(AppCommon)

#help needs to before any of the applications, as they depend on help
#having things configured into the build directory
add_subdirectory(Help)

function (add_application name required)
  if (required)
    set("ENABLE_${name}" ON)
  else ()
    option("ENABLE_${name}" "Enable the ${name} application" ON)
  endif ()
  if (ENABLE_${name})
    add_subdirectory("${name}")
  endif ()
endfunction ()

add_application(MeshViewer    NO)
add_application(ModelBuilder  YES)
add_application(PointsBuilder NO)
add_application(SceneBuilder  NO)
add_application(GrabCuts  NO)


#make sure we use the same qt as paraview
set(QT_QMAKE_EXECUTABLE ${PARAVIEW_QT_QMAKE_EXECUTABLE})
set(QT_USE_IMPORTED_TARGETS TRUE)
set(QT_USE_QTMAIN TRUE)
find_package(Qt4 REQUIRED COMPONENTS QtXml QtNetwork QtCore QtGui QtMain)

# This ensures that the Qt includes dir etc are set properly.
# We don't need to find the Qt4 package or include the use file
# in any of the SUBDIRS in this directory.
include(${QT_USE_FILE})


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

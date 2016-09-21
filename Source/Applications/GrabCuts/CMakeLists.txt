project (GrabCuts)

find_package(OpenCV REQUIRED)

set(grabcuts_libraries vtkCMBFiltering
                       vtkCMBGeneral
                       vtkCMBGraphics
                       vtkCMBIO
                       vtkCMBMeshing
                       vtkInteractionStyle
                       vtkInteractionWidgets
                       vtkIOGDAL
                       vtkRenderingCore
                       vtkRenderingFreeType
                       vtkRendering${VTK_RENDERING_BACKEND}
                       vtkTestingRendering
                       vtksys
                       vtkSMTKReaderExt)

include_directories(
  ${vtkCMBIO_INCLUDE_DIRS}
  ${vtkIOGDAL_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${cmbAppCommon_SOURCE_DIR}
  ${cmbAppCommon_SOURCE_DIR}/Scene
  ${cmbAppCommon_BINARY_DIR}
  ${test_source_directories}
  ${test_build_directories}
  )

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
QT4_WRAP_UI(UISrcs grabCuts.ui)
QT4_WRAP_CPP(MOCSrcs vtkCMBGrabCutUI.h)

#we include headers, so that MSVC on "Go To Header File" context menu  works properly
set(GrabCuts_SRCS vtkCMBGrabCutUI.cxx)

#------------------------------------------------------------------------------
# Build the client

add_executable(GrabCuts MACOSX_BUNDLE GrabCuts.cxx ${GrabCuts_SRCS} ${UISrcs} ${MOCSrcs})
target_link_libraries(GrabCuts ${grabcuts_libraries} ${VTK_LIBRARIES})
install(TARGETS GrabCuts BUNDLE DESTINATION ${VTK_INSTALL_RUNTIME_DIR})


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
     \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${VTK_INSTALL_RUNTIME_DIR}/GrabCuts.app/Contents/MacOS/GrabCuts\"
     \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${VTK_INSTALL_RUNTIME_DIR}/GrabCuts.app\"
     \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${VTK_INSTALL_LIBRARY_DIR}\")

   # Place the App at the requested location.
   file(INSTALL DESTINATION \"${MACOSX_APP_INSTALL_PREFIX}\"
        TYPE DIRECTORY FILES
          \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${VTK_INSTALL_RUNTIME_DIR}/GrabCuts.app\"
        USE_SOURCE_PERMISSIONS)
   "
   COMPONENT Runtime)
endif()


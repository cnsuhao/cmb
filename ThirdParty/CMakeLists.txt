option(USE_SYSTEM_SMTK "Use a system SMTK install" OFF)
mark_as_advanced(USE_SYSTEM_SMTK)
if (USE_SYSTEM_SMTK)
  find_package(SMTK REQUIRED)
else ()
  set(SMTK_ENABLE_TESTING ${BUILD_TESTING})
  set(smtk_options
    SMTK_ENABLE_PYTHON_WRAPPING
    SMTK_ENABLE_QT_SUPPORT
    SMTK_ENABLE_VTK_SUPPORT
    SMTK_ENABLE_DISCRETE_SESSION
    SMTK_ENABLE_EXODUS_SESSION
    SMTK_ENABLE_REMUS_SUPPORT
    SMTK_ENABLE_REMOTE_SESSION
    SMTK_ENABLE_PARAVIEW_SUPPORT)
  foreach (smtk_option IN LISTS smtk_options)
    set("${smtk_option}" ON)
  endforeach ()

  add_subdirectory(SMTK)
  set(SMTK_DIR "${CMAKE_CURRENT_BINARY_DIR}/SMTK"
    CACHE INTERNAL "Path to SMTKConfig.cmake")
  mark_as_advanced(SMTK_DIR)

  set_property(CACHE ${smtk_options}
    PROPERTY
      TYPE INTERNAL)
endif ()

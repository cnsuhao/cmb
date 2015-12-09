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
    # These must be set as INTERNAL cache variables so that SMTK cannot
    # override them other than with local variables. Setting a local variable
    # is not sufficient due to ${unfortunate_historical_reasons} in CMake.
    set("${smtk_option}" ON
      CACHE INTERNAL "Forced by CMB")
  endforeach ()

  add_subdirectory(SMTK)
  set(SMTK_DIR "${CMAKE_CURRENT_BINARY_DIR}/SMTK"
    CACHE INTERNAL "Path to SMTKConfig.cmake")
  mark_as_advanced(SMTK_DIR)
endif ()

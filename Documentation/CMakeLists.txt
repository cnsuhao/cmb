####### CMB Documentation


## End-user Documentation
#
# If we have rst2html, create the user's guide for ModelBuilder
# as an HTML document.

# Define a macro for processing reStructuredText files
# if docutils were found.
if (SPHINX_FOUND)
  function(cmb_add_doc sphinxTargetName)
    set(options)
    set(oneValueArgs DESTINATION SOURCE_DIR BUILD_DIR)
    set(multiValueArgs DEPENDS FIGURES)
    cmake_parse_arguments(sphinx "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )
    if (NOT sphinx_SOURCE_DIR)
      set(sphinx_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/") # Reasonable default
    endif()
    if (NOT sphinx_BUILD_DIR)
      set(sphinx_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}") # Reasonable default
    endif()
    # Generate HTML version of docs
    set(sphinx_HTML_TOP "${sphinx_BUILD_DIR}/html/index.html")
    add_custom_command(
      OUTPUT "${sphinx_HTML_TOP}"
      All
      DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/conf.py
        ${sphinx_DEPENDS}
        ${figureList}
        COMMAND ${SPHINX_EXECUTABLE}
      ARGS
        -b html
        "${sphinx_SOURCE_DIR}"
        "${sphinx_BUILD_DIR}/html"
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Generating HTML for ${sphinxTargetName}"
    )
    add_custom_target(doc-${sphinxTargetName} ALL DEPENDS "${sphinx_HTML_TOP}")

    if (sphinx_DESTINATION)
      message("In Sphinx DESTINATION")
      install(
        DIRECTORY "${sphinx_BUILD_DIR}/html/"
        DESTINATION "${sphinx_DESTINATION}"
        COMPONENT Development)
      install(
        FILES ${figureList}
        DESTINATION "${sphinx_DESTINATION}/images"
        COMPONENT Development)
    endif()
  endfunction()



  # at the moment all other pages are under index.rst
  set(CMB_DOCS
    index.rst
  )

  set(CMB_FIGS
    ModelBuilder/AssigningAttributes.png
    ModelBuilder/AttributeTabInstanced.png
    ModelBuilder/AttributeTabNewCopyDelete.png
    ModelBuilder/CameraManipulationStyle.png
    ModelBuilder/ColorBy.png
    ModelBuilder/ColormapBar.png
    ModelBuilder/DefaultScreen.png
    ModelBuilder/DisplayTab.png
    ModelBuilder/FileMenuSimulation.png
    ModelBuilder/GettingStartedStateLoadedAttributes.png
    ModelBuilder/ImportObjectsStep1.png
    ModelBuilder/LinkCenterAndFocal.png
    ModelBuilder/LoadPluginsStep1.png
    ModelBuilder/LoadPluginsStep2.png
    ModelBuilder/LoadPluginsStep3.png
    ModelBuilder/ModelBuilderIcon.png
    ModelBuilder/ModelFaceRightClick.png
    ModelBuilder/ModelTab.png
    ModelBuilder/pqEyeball.png
    ModelBuilder/pqOpen32.png
    ModelBuilder/pqPickCenter24.png
    ModelBuilder/pqResetCamera32.png
    ModelBuilder/pqResetCenter24.png
    ModelBuilder/pqScalarBar24.png
    ModelBuilder/pqSelect32.png
    ModelBuilder/pqShowCenterAxes24.png
    ModelBuilder/pqXMinus24.png
    ModelBuilder/pqXPlus24.png
    ModelBuilder/pqYMinus24.png
    ModelBuilder/pqYPlus24.png
    ModelBuilder/pqZMinus24.png
    ModelBuilder/pqZoomToBox24.png
    ModelBuilder/pqDataScale.png
    ModelBuilder/pqZPlus24.png
  )

  # add_subdirectory(ModelBuilder)

  # Add the top-level reStructuredText file.
  # All others are included by it.
  cmb_add_doc(userguide
    DEPENDS ${CMB_DOCS}
    FIGURES ${CMB_FIGS}
    BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/user
    DESTINATION share/doc/cmb/userguide
  )

endif()

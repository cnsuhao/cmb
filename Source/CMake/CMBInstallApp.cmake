# This is a helper module that can be used to package an APP for MacOSX. It is
# not designed to bring in all *external* dependencies. It's meant to package
# 'what is  built'.
if (NOT APPLE)
  return()
endif()

# Download PDF documentation. These are used in ModelBuilder's help menu.
macro(download_pdf_guides)
  set (CMB_PDF_DOC_DIR "${CMAKE_BINARY_DIR}/downloads/pdf-doc")
  file(DOWNLOAD "https://media.readthedocs.org/pdf/cmb/master/cmb.pdf"
     "${CMB_PDF_DOC_DIR}/CMBUsersGuide.pdf"
    INACTIVITY_TIMEOUT 10
    SHOW_PROGRESS)

  file(DOWNLOAD "https://media.readthedocs.org/pdf/smtk/latest/smtk.pdf"
     "${CMB_PDF_DOC_DIR}/SMTKUsersGuide.pdf"
    INACTIVITY_TIMEOUT 10
    SHOW_PROGRESS)
endmacro()

# cleanup_bundle is called to fillup the .app file with libraries and plugins
# built. This does not package any external libraries, only those that are
# at the specified locations. This will create a workable bundle that works on
# the machine where its built (since it relies on other shared frameworks and
# libraries) e.g. python
macro(cleanup_bundle app app_root libdir)
  # Install PDF documentation
  download_pdf_guides()
  file(GLOB guides "${CMB_PDF_DOC_DIR}/*")
  foreach(guide IN LISTS guides)
    if(EXISTS "${guide}" AND NOT IS_DIRECTORY "${guide}")
      file(INSTALL "${guide}"
        DESTINATION "${app_root}/Contents/doc"
        USE_SOURCE_PERMISSIONS)
    endif()
  endforeach()

  # take all libs from ${ARGN} and put it in the Libraries dir.
  file(GLOB_RECURSE dylibs ${libdir}/*.dylib)
  file(GLOB_RECURSE solibs ${libdir}/*.so)

  # third party libs are installed using the CMAKE_INSTALL_PREFIX. Let's include
  # them too.
  file(GLOB_RECURSE thirdparty_dylibs ${CMAKE_INSTALL_PREFIX}/lib/*.dylib)
  file(GLOB_RECURSE thirdparty_solibs ${CMAKE_INSTALL_PREFIX}/lib/*.so)

  list(APPEND dylibs ${thirdparty_dylibs})
  list(APPEND solibs ${thirdparty_solibs})

  list(REMOVE_DUPLICATES dylibs)
  list(REMOVE_DUPLICATES solibs)

  # We need to create the plugins directory before we symlink our plugins into
  # it.
  execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${app_root}/Contents/MacOS/plugins)

  # some of the .dylibs are actually plugins. These need to be installed in the
  # plugins directory so they are automatically found by the app. We also adjust
  # their id to reflect this change.
  foreach(lib IN LISTS dylibs)
    # We assume that all plugins match the pattern "###Plugin.dylib". Plugins
    # are put in the Libraries directory and symlinked to the plugins directory
    # so the plugin manager can find them.
    if (${lib} MATCHES "Plugin.dylib")
      file(INSTALL ${lib}
        DESTINATION ${app_root}/Contents/Libraries
        USE_SOURCE_PERMISSIONS)
      get_filename_component(libname ${lib} NAME)
      execute_process(COMMAND install_name_tool -id
        @executable_path/../Libraries/${libname}
        ${app_root}/Contents/Libraries/${libname})
      execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${app_root}/Contents/Libraries/${libname}
        ${app_root}/Contents/MacOS/plugins/${libname})
    else ()
      file(INSTALL ${lib}
        DESTINATION ${app_root}/Contents/Libraries
        USE_SOURCE_PERMISSIONS)
      endif ()
    endforeach ()

  file(INSTALL ${solibs}
       DESTINATION ${app_root}/Contents/Libraries
       USE_SOURCE_PERMISSIONS)

  file(GLOB pyfiles ${libdir}/*.py)
  file(GLOB pycfiles ${libdir}/*.pyc)
  if (pycfiles OR pyfiles)
    file(INSTALL ${pyfiles} ${pycfiles}
         DESTINATION ${app_root}/Contents/Python
         USE_SOURCE_PERMISSIONS)
  endif()

  # HACK: this refers to "paraview" directly. Not a good idea.
  set (python_packages_dir "${libdir}/site-packages/paraview")
  if (EXISTS "${python_packages_dir}")
    file(INSTALL DESTINATION ${app_root}/Contents/Python
         TYPE DIRECTORY FILES "${python_packages_dir}"
         USE_SOURCE_PERMISSIONS)
  endif()

  # package other executables such as pvserver.
  get_filename_component(bin_dir "${app_root}" PATH)
  file(GLOB executables "${bin_dir}/*")
  foreach(exe IN LISTS executables)
    if (EXISTS "${exe}" AND NOT IS_DIRECTORY "${exe}")
      file(INSTALL "${exe}"
           DESTINATION "${app_root}/Contents/bin"
           USE_SOURCE_PERMISSIONS)
    endif()
  endforeach()
endmacro()

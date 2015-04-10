#=========================================================================
#
#  This software is distributed WITHOUT ANY WARRANTY; without even
#  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#  PURPOSE.  See the above copyright notice for more information.
#
#=========================================================================

#setup a cmake variable that stores the location of this cmake file on disk
#that way when other cmake files call functions found in this file, we can
#properly determine the location on disk of where the .rw files are
set(CMB_MESHING_CMAKE_DIR "${CMAKE_CURRENT_LIST_DIR}")

#we create a text file that list this mesh worker and its type
function(Register_Mesh_Worker workerExecutableName)
  set(options )
  set(oneValueArgs INPUT_TYPE OUTPUT_TYPE FILE_TYPE FILE_PATH)
  set(multiValueArgs )
  cmake_parse_arguments(R "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

  #setup the files that this worker needs to be installed. By default it is
  #just the .rw file, but if the worker has file based requirements we need
  #to properly install that xml/json file too
  set(files_to_install "${EXECUTABLE_OUTPUT_PATH}/${workerExecutableName}.rw")

  #configure the correct file(s) based on if the worker has file based
  #requirements
  if(R_FILE_TYPE AND R_FILE_PATH)
    get_filename_component(R_FILE_NAME "${R_FILE_PATH}" NAME)

    configure_file(${CMB_MESHING_CMAKE_DIR}/RemusWorkerWithFile.rw.in
                   ${EXECUTABLE_OUTPUT_PATH}/${workerExecutableName}.rw
                   @ONLY)
    file(COPY ${R_FILE_PATH} DESTINATION ${EXECUTABLE_OUTPUT_PATH})

    #add the file that holds the requirements as an item that needs to be
    #installed
    list(APPEND files_to_install "${EXECUTABLE_OUTPUT_PATH}/${R_FILE_NAME}")
  else()
    configure_file(${CMB_MESHING_CMAKE_DIR}/RemusWorker.rw.in
                   ${EXECUTABLE_OUTPUT_PATH}/${workerExecutableName}.rw
                   @ONLY)
  endif()

  #setup the places to install this worker. On Windows and Linux that is just
  #bin, while on the mac it is inside the SceneBuilder and ModelBuilder apps
  set(destinations_to_install )
  if(WIN32)
    set(target_depends )
    foreach(f ${files_to_install})
      get_filename_component(fname "${f}" NAME)

      #copy the worker files to install to the Debug/Release/etc build dirs
      set(output_path "${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/${fname}")
      add_custom_command(OUTPUT "${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/${fname}"
        COMMAND ${CMAKE_COMMAND} -E copy "${f}" "${output_path}")

      #properly save all the files we copied so that we can state that the
      #custom target depends on them
      list(APPEND target_depends "${output_path}")
    endforeach()

    add_custom_target(Copy${workerExecutableName}WorkerRegFile
                      ALL DEPENDS "${target_depends}" )

    set(destinations_to_install "bin")

  elseif(APPLE)
    set(destinations_to_install
        "${VTK_INSTALL_RUNTIME_DIR}/SceneBuilder.app/Contents/bin"
        "${VTK_INSTALL_RUNTIME_DIR}/ModelBuilder.app/Contents/bin"
        )
  else()
    set(destinations_to_install "bin")
  endif()

  #install all the files needed for this worker in every of the bin directories
  #that need meshing workers
  foreach(dest ${destinations_to_install})
    foreach(f ${files_to_install})
      install (FILES "${f}" DESTINATION "${dest}")
    endforeach()
  endforeach()


  #now that we have installed the rw file, we move onto the executable, which needs
  #to be installed on the mac into the scene and model apps
  if(APPLE)
    install(TARGETS  ${workerExecutableName}
            DESTINATION ${VTK_INSTALL_RUNTIME_DIR}/SceneBuilder.app/Contents/bin)
    install(TARGETS  ${workerExecutableName}
            DESTINATION ${VTK_INSTALL_RUNTIME_DIR}/ModelBuilder.app/Contents/bin)
  else()
    install (TARGETS ${workerExecutableName} RUNTIME DESTINATION bin)
  endif()

endfunction()

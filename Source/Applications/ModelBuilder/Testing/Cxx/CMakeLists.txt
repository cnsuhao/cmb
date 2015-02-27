add_executable(CompareADHFiles CompareADHFiles.cxx)
target_link_libraries(CompareADHFiles vtksys)

set(REAL_EXECUTABLE_PATH ${EXECUTABLE_OUTPUT_PATH})
if(Q_WS_MAC)
  set(REAL_EXECUTABLE_PATH ${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}.app/Contents/MacOS)
endif(Q_WS_MAC)

# function(test_with_text_file_output testname tempfile baselinefile exepath exe)
set(PROJECT_EXE ${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX})
file(WRITE "${CMAKE_BINARY_DIR}/RunSimBuilderADHCompareOutput.cmake"
"set(fullexe \"${REAL_EXECUTABLE_PATH}/${PROJECT_EXE}\")
if(NOT EXISTS \${fullexe})
  set(fullexe \"${REAL_EXECUTABLE_PATH}/\${cfg}/${PROJECT_EXE}\")
endif()
FILE(REMOVE \"RayAndVeg.bc\" \"vegfiles/SandySilt.mtd\" \"vegfiles/Duricrust.mtd\"
            \"vegfiles/RayAndVeg.dat\" \"vegfiles/RayAndVeg.txt\")

set(ARGS \"-dr;--test-directory=${CMB_TEST_DIR};--test-script=${TEST_XML}/SBADHCompare.xml;--exit\")
execute_process(COMMAND \${fullexe} \${ARGS} RESULT_VARIABLE rv)
if(NOT rv EQUAL 0)
  message(FATAL_ERROR \"ModelBuilder return value was \${rv}\")
endif()

set(fullexe \"${REAL_EXECUTABLE_PATH}/CompareADHFiles\")
if(NOT EXISTS \${fullexe})
  set(fullexe \"${REAL_EXECUTABLE_PATH}/\${cfg}/CompareADHFiles.exe\")
endif()
if(NOT EXISTS \${fullexe})
  set(fullexe \"${EXECUTABLE_OUTPUT_PATH}/CompareADHFiles\")
endif()
if(NOT EXISTS \${fullexe})
  message(FATAL_ERROR \"'\${fullexe}' does not exist\")
endif()

set(ARGS \"${CMB_TEST_DATA_ROOT}/SimBuilderADHCompareTest;${CMB_TEST_DIR}/../../Applications/ModelBuilder/Testing/Cxx\")
execute_process(COMMAND \${fullexe} \${ARGS} RESULT_VARIABLE failed)
if(failed)
  message(FATAL_ERROR \"some of the files differ  failed = '\${failed}' \")
endif()" )

add_short_test(SimBuilderADHCompareOutput ${CMAKE_COMMAND} -Dcfg=$<CONFIGURATION> -P "${CMAKE_BINARY_DIR}/RunSimBuilderADHCompareOutput.cmake")

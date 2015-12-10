message("test_cmd " ${test_cmd})
message("output_base " ${output_base})
message("output_test " ${output_test})
FILE(REMOVE "${output_test}")
execute_process(
  COMMAND ${test_exe} -T ${test_dir}
   RESULT_VARIABLE test_result
  )

if(test_result)
  message(SEND_ERROR "${test_exe} did not run successfully.")
endif(test_result)

execute_process(
  COMMAND ${CMAKE_COMMAND} -E compare_files ${output_base} ${output_test}
  RESULT_VARIABLE comp_result
  ERROR_QUIET
  OUTPUT_QUIET
)

if(comp_result)
  message(SEND_ERROR "${output_test} does not match ${output_base}")
endif(comp_result)

########################################################################
macro(GET_IMAGE_THRESHOLD_ARG variable testname)
  # Macro used to obtain the command argument to set the image threshold.
  if (${testname}_THRESHOLD)
    set (${variable} --test-threshold=${${testname}_THRESHOLD})
  endif (${testname}_THRESHOLD)
endmacro(GET_IMAGE_THRESHOLD_ARG)


#a helper macro that properly finds and includes boost for us
#this is a macro so that it sets the local variables properly, otherwise
#if this was a function we would have to use PARENT_SCOPE to lift up
#all the variables that the Boost package and Threads package set
macro(cmb_find_boost )
  #When on windows we want to also support static builds of boost.
  if(NOT DEFINED Boost_USE_STATIC_LIBS)
    if(${BUILD_SHARED_LIBS})
      set(Boost_USE_STATIC_LIBS OFF)
    else()
      set(Boost_USE_STATIC_LIBS ON)
    endif()
  endif()

  if(WIN32)
    find_package(Boost 1.48.0
                 COMPONENTS thread chrono filesystem system date_time REQUIRED)
  else()
    find_package(Boost 1.45.0
                 COMPONENTS thread filesystem system date_time REQUIRED)
  endif()

  #on unix boost uses pthreads which we need to require so that we properly build.
  find_package(Threads REQUIRED)

endmacro()

# Custom functions for adding tests based on timeout.  Continuous builds may only want to
# enable short and medium tests, for example, and not want to build long tests.

set(SHORT_TIMEOUT 10)
set(MEDIUM_TIMEOUT 60)
set(LONG_TIMEOUT 250)

# If short tests are enabled, add test TESTNAME with a timeout of SHORT_TIMEOUT seconds.
function(add_short_test TESTNAME EXENAME)
  if(BUILD_SHORT_TESTS AND BUILD_TESTING)
    add_test(NAME ${TESTNAME} COMMAND ${EXENAME} ${ARGN})
    set_tests_properties(${TESTNAME} PROPERTIES TIMEOUT ${SHORT_TIMEOUT})
  endif()
endfunction()

# If medium tests are enabled, add test TESTNAME with a timeout of MEDIUM_TIMEOUT seconds.
function(add_medium_test TESTNAME EXENAME)
  if(BUILD_MEDIUM_TESTS AND BUILD_TESTING)
    add_test(NAME ${TESTNAME} COMMAND ${EXENAME} ${ARGN})
    set_tests_properties(${TESTNAME} PROPERTIES TIMEOUT ${MEDIUM_TIMEOUT})
  endif()
endfunction()

# If long tests are enabled, add test TESTNAME with a timeout of LONG_TIMEOUT seconds.
function(add_long_test TESTNAME EXENAME)
  if(BUILD_LONG_TESTS AND BUILD_TESTING)
    add_test(NAME ${TESTNAME} COMMAND ${EXENAME} ${ARGN})
    set_tests_properties(${TESTNAME} PROPERTIES TIMEOUT ${LONG_TIMEOUT})
  endif()
endfunction()

macro(set_short_timeout TESTNAME)
  set_tests_properties(${TESTNAME} PROPERTIES TIMEOUT 10)
  set(SHORT_TESTS ${SHORT_TESTS}, ${TESTNAME})
endmacro()

# Set a timeout of 60 seconds on test TESTNAME.
macro(set_medium_timeout TESTNAME)
  set_tests_properties(${TESTNAME} PROPERTIES TIMEOUT 60)
  set(MEDIUM_TESTS ${MEDIUM_TESTS}, ${TESTNAME})
endmacro()

# Set a timeout of 200 seconds on test TESTNAME.
macro(set_long_timeout TESTNAME)
  set_tests_properties(${TESTNAME} PROPERTIES TIMEOUT 200)
  set(LONG_TESTS ${LONG_TESTS}, ${TESTNAME})
endmacro()

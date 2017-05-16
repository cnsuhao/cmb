file(STRINGS ../version.txt version_string )

string(REGEX MATCH "([0-9]+)\\.([0-9]+)\\.([0-9]+)[-]*(.*)"
    version_matches "${version_string}")

set(CMB_VERSION_MAJOR ${CMAKE_MATCH_1})
set(CMB_VERSION_MINOR ${CMAKE_MATCH_2})
set(CMB_VERSION_PATCH "${CMAKE_MATCH_3}")
# Do we just have a patch version or are there extra stuff?
if (CMAKE_MATCH_4)
  set(CMB_VERSION_PATCH "${CMAKE_MATCH_3}-${CMAKE_MATCH_4}")
endif()
set(CMB_VERSION "${CMB_VERSION_MAJOR}.${CMB_VERSION_MINOR}")

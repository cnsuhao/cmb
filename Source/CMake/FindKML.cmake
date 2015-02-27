
# - Find KML
# Find the KML includes and library
# This module defines
#  KML_INCLUDE_DIRS, where to find kml and expat
#  KML_LIBRARIES, the libraries needed to use KML.
#  KML_FOUND, If false, do not try to use KML.

# You can set KML_DIR to specify where to search
# for KML's include and lib directories

find_path(KML_INCLUDE_DIR kml/dom.h
          ${KML_DIR}/include
          ${KML_DIR}/../KML/src
          $ENV{KML_DIR}/includefv
          NO_DEFAULT_PATH
)
find_library(KML_LIBRARY kml
             PATHS
             ${KML_DIR}/lib
             ${CMAKE_BINARY_DIR}/bin
             )

find_library(EXPAT_LIBRARY expat
             PATHS
             ${KML_DIR}/lib
             ${CMAKE_BINARY_DIR}/bin
             NO_DEFAULT_PATH
             )

# if not found try to use system expat
if(NOT EXPAT_LIBRARY)
  find_library(EXPAT_LIB expat)
endif()

set(KML_FOUND FALSE)
if(KML_INCLUDE_DIR AND KML_LIBRARY AND EXPAT_LIBRARY)
  set(KML_FOUND TRUE)
endif()

if(KML_FOUND)
  set(KML_LIBRARIES ${KML_LIBRARY} ${EXPAT_LIBRARY})
  set(KML_INCLUDE_DIRS
    ${KML_INCLUDE_DIR}
    ${KML_INCLUDE_DIR}/kml/third_party/boost_1_34_1
    ${KML_INCLUDE_DIR}/kml/third_party/expat.src
    )
endif()

mark_as_advanced(
  KML_LIBRARY
  KML_INCLUDE_DIR
  EXPAT_LIBRARY
  )

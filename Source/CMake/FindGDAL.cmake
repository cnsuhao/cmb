
# - Find GDAL
# Find the GDAL includes and library
# This module defines
#  GDAL_INCLUDE_DIR, where to find GDAL
#  GDAL_LIBRARY, the library needed to use GDAL.
#  GDAL_FOUND, If false, do not try to use GDAL.

# You can set GDAL_DIR to specify where to search
# for GDAL's include and lib directories

find_path(GDAL_INCLUDE_DIR gdal.h
  HINTS ${GDAL_DIR}
  PATH_SUFFIXES include
  )

find_library(GDAL_LIBRARY
  NAMES gdal gdal_i gdal1.5.0 gdal1.4.0 gdal1.3.2 GDAL
  PATHS
  ${GDAL_DIR}/lib
  ${EXECUTABLE_OUTPUT_PATH}
  NO_DEFAULT_PATH
)

set(GDAL_FOUND FALSE)
if(GDAL_INCLUDE_DIR AND GDAL_LIBRARY)
  set(GDAL_FOUND TRUE)
endif()

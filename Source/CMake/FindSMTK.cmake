# - Try to find SMTK headers and libraries
#
# Usage of this module as follows:
#
#     find_package(SMTK)
#
# Variables used by this module, they can change the default behaviour and need
# to be set before calling find_package:
#
#  SMTK_ROOT_DIR  Set this variable to the root installation of
#                            SMTK if the module has problems finding
#                            the proper installation path.
#
# Variables defined by this module:
#
#  SMTK_FOUND              System has SMTK libs/headers
#  SMTK_INCLUDE_DIR        The location of SMTK headers

find_path(SMTK_ROOT_DIR
    NAMES include/smtk/SMTKCoreExports.h
)

find_path(SMTK_INCLUDE_DIR
    NAMES smtk/SMTKCoreExports.h
    HINTS ${SMTK_ROOT_DIR}/include/
)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(SMTK DEFAULT_MSG
    SMTK_INCLUDE_DIR
)

#now we create fake targets to be used
if(SMTK_FOUND)
  include(${SMTK_ROOT_DIR}/lib/SMTK-targets.cmake)
endif()

set(SMTK_INCLUDE_DIRS ${SMTK_INCLUDE_DIR})

mark_as_advanced(
    SMTK_ROOT_DIR
    SMTK_INCLUDE_DIR
)

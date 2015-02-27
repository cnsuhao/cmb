# - Try to find SlctkAttribute headers and libraries
#
# Usage of this module as follows:
#
#     find_package(SlctkAttribute)
#
# Variables used by this module, they can change the default behaviour and need
# to be set before calling find_package:
#
#  SlctkAttribute_ROOT_DIR  Set this variable to the root installation of
#                            SlctkAttribute if the module has problems finding
#                            the proper installation path.
#
# Variables defined by this module:
#
#  SlctkAttribute_FOUND              System has SlctkAttribute libs/headers
#  SlctkAttribute_INCLUDE_DIR        The location of SlctkAttribute headers

    message(STATUS "SlctkAttribute_INCLUDE_DIR=${SlctkAttribute_INCLUDE_DIR}")
    message(STATUS "SlctkAttribute_CMAKE_DIR=${SlctkAttribute_CMAKE_DIR}")

FIND_PATH(SlctkAttribute_INCLUDE_DIR attribute/PublicPointerDefs.h
  /usr/local/include
  /usr/include
  )

FIND_PATH(SlctkAttribute_LIBRARY_DIR SlctkAttribute.lib
  /lib /usr/lib /usr/local/lib
  )

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(SlctkAttribute DEFAULT_MSG
    SlctkAttribute_INCLUDE_DIR
)

mark_as_advanced(
    SlctkAttribute_LIBRARY
    SlctkAttribute_INCLUDE_DIR
)

# Find the cetcd libraries
#
# The following variables are optionally searched for defaults
#  CETCD_ROOT_DIR: Base directory where all cetcd components are found
#  CETCD_INCLUDE_DIR: Directory where cetcd headers are found
#  CETCD_LIB_DIR: Directory where cetcd library is found
#
# The following are set after configuration is done:
#  CETCD_FOUND
#  CETCD_INCLUDE_DIRS
#  CETCD_LIBRARIES

find_path(CETCD_INCLUDE_DIRS
  NAMES cetcd.h
  HINTS
  ${CETCD_INCLUDE_DIR}
  ${CETCD_ROOT_DIR}
  ${CETCD_ROOT_DIR}/include
  PATH_SUFFIXES cetcd)

find_library(CETCD_LIBRARIES
  NAMES cetcd
  HINTS
  ${CETCD_LIB_DIR}
  ${CETCD_ROOT_DIR}
  ${CETCD_ROOT_DIR}/lib
  PATH_SUFFIXES cetcd)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(cetcd DEFAULT_MSG CETCD_INCLUDE_DIRS CETCD_LIBRARIES)
mark_as_advanced(CETCD_INCLUDE_DIRS CETCD_LIBRARIES)

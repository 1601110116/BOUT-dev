# Find SUNDIALS, the SUite of Nonlinear and DIfferential/ALgebraic equation Solvers
#
# Currently only actually looks for arkode, cvode and ida, as well as nvecparallel
#
# This module will define the following variables:
#   SUNDIALS_FOUND: true if SUNDIALS was found on the system
#   SUNDIALS_INCLUDE_DIRS: Location of the SUNDIALS includes
#   SUNDIALS_LIBRARIES: Required libraries
#   SUNDIALS_VERSION: Full version string

include(FindPackageHandleStandardArgs)

find_path(SUNDIALS_INCLUDE_DIR
  sundials_config.h
  HINTS ENV SUNDIALS_DIR
  PATH_SUFFIXES include include/sundials
  DOC "SUNDIALS Directory")

if (SUNDIALS_INCLUDE_DIR)
  message(STATUS "Found SUNDIALS include directory: ${SUNDIALS_INCLUDE_DIR}")
else()
  message(STATUS "Could not find SUNDIALS include directory")
endif()

set(SUNDIALS_INCLUDE_DIRS
  "${SUNDIALS_INCLUDE_DIR}"
  "${SUNDIALS_INCLUDE_DIR}/..")

find_library(SUNDIALS_nvecparallel_LIBRARY
  NAMES sundials_nvecparallel
  HINTS
    "${SUNDIALS_INCLUDE_DIR}/.."
    "${SUNDIALS_INCLUDE_DIR}/../.."
  PATH_SUFFIXES lib lib64
  )

if (SUNDIALS_nvecparallel_LIBRARY)
  list(APPEND SUNDIALS_LIBRARIES "${SUNDIALS_nvecparallel_LIBRARY}")
endif()
mark_as_advanced(SUNDIALS_nvecparallel_LIBRARY)

set(SUNDIALS_COMPONENTS arkode cvode ida)

foreach (LIB ${SUNDIALS_COMPONENTS})
  find_library(SUNDIALS_${LIB}_LIBRARY
    NAMES sundials_${LIB}
    HINTS
      "${SUNDIALS_INCLUDE_DIR}/.."
      "${SUNDIALS_INCLUDE_DIR}/../.."
    PATH_SUFFIXES lib lib64
    )

  if (SUNDIALS_${LIB}_LIBRARY)
    list(APPEND SUNDIALS_LIBRARIES "${SUNDIALS_${LIB}_LIBRARY}")
  endif()
  mark_as_advanced(SUNDIALS_${LIB}_LIBRARY)
endforeach()

if (SUNDIALS_INCLUDE_DIR)
  file(READ "${SUNDIALS_INCLUDE_DIR}/sundials_config.h" SUNDIALS_CONFIG_FILE)
  string(FIND "${SUNDIALS_CONFIG_FILE}" "SUNDIALS_PACKAGE_VERSION" index)
  if("${index}" LESS 0)
    # Version >3
    set(SUNDIALS_VERSION_REGEX_PATTERN
      ".*#define SUNDIALS_VERSION \"([0-9]+)\\.([0-9]+)\\.([0-9]+)\".*")
  else()
    # Version <3
    set(SUNDIALS_VERSION_REGEX_PATTERN
      ".*#define SUNDIALS_PACKAGE_VERSION \"([0-9]+)\\.([0-9]+)\\.([0-9]+)\".*")
  endif()
  string(REGEX MATCH ${SUNDIALS_VERSION_REGEX_PATTERN} _ "${SUNDIALS_CONFIG_FILE}")
  set(SUNDIALS_VERSION_MAJOR ${CMAKE_MATCH_1})
  set(SUNDIALS_VERSION_MINOR ${CMAKE_MATCH_2})
  set(SUNDIALS_VERSION_PATCH ${CMAKE_MATCH_3})
  set(SUNDIALS_VERSION "${SUNDIALS_VERSION_MAJOR}.${SUNDIALS_VERSION_MINOR}.${SUNDIALS_VERSION_PATCH}")
endif()

find_package_handle_standard_args(SUNDIALS
  REQUIRED_VARS SUNDIALS_LIBRARIES SUNDIALS_INCLUDE_DIR SUNDIALS_INCLUDE_DIRS
  VERSION_VAR SUNDIALS_VERSION
  )

mark_as_advanced(SUNDIALS_LIBRARIES SUNDIALS_INCLUDE_DIR SUNDIALS_INCLUDE_DIRS)

if (SUNDIALS_FOUND AND NOT TARGET SUNDIALS::SUNDIALS)
  add_library(SUNDIALS::NVecParallel UNKNOWN IMPORTED)
  set_target_properties(SUNDIALS::NVecParallel PROPERTIES
    IMPORTED_LOCATION "${SUNDIALS_nvecparallel_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${SUNDIALS_INCLUDE_DIRS}")

  foreach (LIB ${SUNDIALS_COMPONENTS})  
    add_library(SUNDIALS_${LIB}::${LIB} UNKNOWN IMPORTED)
    target_link_libraries(SUNDIALS_${LIB}::${LIB} INTERFACE SUNDIALS::NVecParallel)
    set_target_properties(SUNDIALS_${LIB}::${LIB} PROPERTIES
      IMPORTED_LOCATION "${SUNDIALS_${LIB}_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${SUNDIALS_INCLUDE_DIRS}")
  endforeach()
endif()

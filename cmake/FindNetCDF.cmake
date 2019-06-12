# Taken from https://github.com/conan-io/conan/issues/2125#issuecomment-351176653
function(add_cloned_imported_target dst src)
    add_library(${dst} INTERFACE IMPORTED)
    foreach(name INTERFACE_LINK_LIBRARIES INTERFACE_INCLUDE_DIRECTORIES INTERFACE_COMPILE_DEFINITIONS INTERFACE_COMPILE_OPTIONS)
        get_property(value TARGET ${src} PROPERTY ${name} )
        set_property(TARGET ${dst} PROPERTY ${name} ${value})
    endforeach()
endfunction()

find_package(netCDFCxx QUIET)
if (netCDFCxx_FOUND)
  if(NOT TARGET NetCDF::NetCDF_CXX)
    add_cloned_imported_target(NetCDF::NetCDF_CXX netCDF::netcdf-cxx4)
  endif()
  set(NetCDF_FOUND TRUE)
  return()
endif()

# A function to call nx-config with an argument, and append the resulting path to a list
# Taken from https://github.com/LiamBindle/geos-chem/blob/feature/CMake/CMakeScripts/FindNetCDF.cmake
function(inspect_netcdf_config VAR NX_CONFIG ARG)
    execute_process(
        COMMAND ${NX_CONFIG} ${ARG}
        OUTPUT_VARIABLE NX_CONFIG_OUTPUT
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(EXISTS "${NX_CONFIG_OUTPUT}")
        list(APPEND ${VAR} ${NX_CONFIG_OUTPUT})
        set(${VAR} ${${VAR}} PARENT_SCOPE)
    endif()
endfunction()

find_program(NC_CONFIG "nc-config"
  DOC "Path to NetCDF C config helper"
  )
get_filename_component(NC_CONFIG_TMP "${NC_CONFIG}" DIRECTORY)
get_filename_component(NC_CONFIG_LOCATION "${NC_CONFIG_TMP}" DIRECTORY)

find_program(NCXX4_CONFIG "ncxx4-config"
  DOC "Path to NetCDF C++ config helper"
  )
get_filename_component(NCXX4_CONFIG_TMP "${NCXX4_CONFIG}" DIRECTORY)
get_filename_component(NCXX4_CONFIG_LOCATION "${NCXX4_CONFIG_TMP}" DIRECTORY)

set(NC_HINTS "")
inspect_netcdf_config(NC_HINTS "${NC_CONFIG}" "--includedir")
inspect_netcdf_config(NC_HINTS "${NC_CONFIG}" "--prefix")

find_path(NetCDF_INCLUDE_DIR
  NAMES netcdf.h
  DOC "NetCDF C include directories"
  HINTS
    "${NC_HINTS}"
    "${NC_CONFIG_LOCATION}"
  PATH_SUFFIXES
    "include"
  )
message(${NetCDF_INCLUDE_DIR})
mark_as_advanced(NetCDF_INCLUDE_DIR)

find_library(NetCDF_LIBRARY
  NAMES netcdf
  DOC "NetCDF C library"
  HINTS
    "${NC_HINTS}"
    "${NC_CONFIG_LOCATION}"
  PATH_SUFFIXES
    "lib" "lib64"
 )
mark_as_advanced(NetCDF_LIBRARY)

set(NCXX4_HINTS "")
inspect_netcdf_config(NCXX4_HINTS "${NCXX4_CONFIG}" "--includedir")
inspect_netcdf_config(NCXX4_HINTS "${NCXX4_CONFIG}" "--prefix")

find_path(NetCDF_CXX_INCLUDE_DIR
  NAMES netcdf
  DOC "NetCDF C++ include directories"
  HINTS
    "${NCXX4_HINTS}"
    "${NCXX4_CONFIG_LOCATION}"
  PATH_SUFFIXES
    "include"
  )
mark_as_advanced(NetCDF_CXX_INCLUDE_DIR)

find_library(NetCDF_CXX_LIBRARY
  NAMES netcdf_c++4 netcdf-cxx4
  DOC "NetCDF C++ library"
  HINTS
    "${NCXX4_HINTS}"
    "${NCXX4_CONFIG_LOCATION}"
  PATH_SUFFIXES
    "lib" "lib64"
  )
mark_as_advanced(NetCDF_CXX_LIBRARY)

if (NetCDF_INCLUDE_DIR)
  file(STRINGS "${NetCDF_INCLUDE_DIR}/netcdf_meta.h" _netcdf_version_lines
    REGEX "#define[ \t]+NC_VERSION_(MAJOR|MINOR|PATCH|NOTE)")
  string(REGEX REPLACE ".*NC_VERSION_MAJOR *\([0-9]*\).*" "\\1" _netcdf_version_major "${_netcdf_version_lines}")
  string(REGEX REPLACE ".*NC_VERSION_MINOR *\([0-9]*\).*" "\\1" _netcdf_version_minor "${_netcdf_version_lines}")
  string(REGEX REPLACE ".*NC_VERSION_PATCH *\([0-9]*\).*" "\\1" _netcdf_version_patch "${_netcdf_version_lines}")
  string(REGEX REPLACE ".*NC_VERSION_NOTE *\"\([^\"]*\)\".*" "\\1" _netcdf_version_note "${_netcdf_version_lines}")
  set(NetCDF_VERSION "${_netcdf_version_major}.${_netcdf_version_minor}.${_netcdf_version_patch}${_netcdf_version_note}")
  unset(_netcdf_version_major)
  unset(_netcdf_version_minor)
  unset(_netcdf_version_patch)
  unset(_netcdf_version_note)
  unset(_netcdf_version_lines)
endif ()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(NetCDF
  REQUIRED_VARS NetCDF_LIBRARY NetCDF_INCLUDE_DIR NetCDF_CXX_LIBRARY NetCDF_CXX_INCLUDE_DIR
  VERSION_VAR NetCDF_VERSION)

if (NetCDF_FOUND)
  set(NetCDF_INCLUDE_DIRS "${NetCDF_CXX_INCLUDE_DIR}" "${NetCDF_INCLUDE_DIR}")
  set(NetCDF_LIBRARIES "${NetCDF_CXX_LIBRARY}" "${NetCDF_LIBRARY}")

  if (NOT TARGET NetCDF::NetCDF)
    add_library(NetCDF::NetCDF_C UNKNOWN IMPORTED)
    set_target_properties(NetCDF::NetCDF_C PROPERTIES
      IMPORTED_LOCATION "${NetCDF_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${NetCDF_INCLUDE_DIR}"
      )
    add_library(NetCDF::NetCDF_CXX UNKNOWN IMPORTED)
    set_target_properties(NetCDF::NetCDF_CXX PROPERTIES
      IMPORTED_LINK_INTERFACE_LIBRARIES NetCDF::NetCDF_C
      IMPORTED_LOCATION "${NetCDF_CXX_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${NetCDF_CXX_INCLUDE_DIR}")
  endif ()
endif ()

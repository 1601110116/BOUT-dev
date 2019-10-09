cmake_minimum_required(VERSION 3.9...3.12)

set(EXE_NAME 6field_cmod)
project(${EXE_NAME} LANGUAGES CXX)

set(CMAKE_PREFIX_PATH /home/ylang/local/BOUT)
find_package(MPI REQUIRED)
include_directories($MPI_INCLUDE_PATH)
find_package(bout++ REQUIRED)
add_executable(${EXE_NAME} elm_6f.cxx)
target_link_libraries(${EXE_NAME} PRIVATE bout++::bout++)
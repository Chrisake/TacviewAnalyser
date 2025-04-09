# CMakeList.txt : CMake project for TacViewerAnalyser, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.21)

project ("TacViewAnalyser")

set(PROJECT_MAJOR_VERSION 0)
set(PROJECT_MINOR_VERSION 1)

include("${CMAKE_SOURCE_DIR}/cmake/compiler_setup.cmake")

# Set Common Variables
set(PROJECT_ROOT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
set(TA_COM_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/TA_Com")
set(TA_LIB_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/TA_Lib")
set(TA_CLI_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/TA_Cli")
set(TA_SRV_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/TA_Srv")

add_subdirectory(${TA_LIB_DIRECTORY})
add_subdirectory(${TA_CLI_DIRECTORY})

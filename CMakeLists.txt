# CMakeList.txt : CMake project for TacViewerAnalyser, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

set(PROJECT_MAJOR_VERSION 0)
set(PROJECT_MINOR_VERSION 1)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("TacViewAnalyser")

find_package(boost_program_options CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(minizip CONFIG REQUIRED) 
find_package(indicators CONFIG REQUIRED)

# Add source to this project's executable.
add_executable (${PROJECT_NAME}   
		"src/main.cpp" 
		"src/cache.cpp" 
		"src/sim_object.cpp" 
		"src/tac_file.cpp" 
		"src/utils.cpp" 
		"src/tac_file_load.cpp" 
		"src/tac_file_load2.cpp")

target_include_directories(${PROJECT_NAME} PRIVATE "include")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE Boost::program_options)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(${PROJECT_NAME} PRIVATE minizip::minizip)
target_link_libraries(${PROJECT_NAME} PRIVATE indicators::indicators)

include(InstallRequiredSystemLibraries)
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "TacviewAnalyser")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/License")
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_MAJOR_VERSION}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_MINOR_VERSION}")
set(CPACK_GENERATOR "NSIS")
set(CPACK_SOURCE_GENERATOR "NSIS")

# Specify the installer script
set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
    !include ${CMAKE_SOURCE_DIR}\\NSIS_inst_registry.nsh
")

set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "
	!include ${CMAKE_SOURCE_DIR}\\NSIS_uninst_registry.nsh
")

include(CPack)

# Install Boost::program_options, minizip, and zlib DLLs
install(TARGETS ${PROJECT_NAME})
install(FILES $<TARGET_FILE:Boost::program_options> DESTINATION bin)
install(FILES "${CMAKE_BINARY_DIR}/minizip.dll" DESTINATION bin)
install(FILES "License" DESTINATION .)


# CMakeList.txt : CMake project for TacViewerAnalyser, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.21)

project ("TacViewAnalyser")

set(PROJECT_MAJOR_VERSION 0)
set(PROJECT_MINOR_VERSION 1)

include("${CMAKE_SOURCE_DIR}/cmake/compiler_setup.cmake")


find_package(boost_program_options CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(unofficial-minizip CONFIG REQUIRED) 
find_package(indicators CONFIG REQUIRED)

# Add source to this project's executable.
add_executable (${PROJECT_NAME}   
		"src/main.cpp" 
		"src/cache.cpp" 
		"src/sim_object.cpp" 
		"src/tac_file.cpp" 
		"src/utils.cpp" 
		"src/tac_file_load.cpp" 
		app.rc
)

target_include_directories(${PROJECT_NAME} PRIVATE "include")

target_link_libraries(${PROJECT_NAME} PRIVATE Boost::program_options)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(${PROJECT_NAME} PRIVATE unofficial::minizip::minizip)
target_link_libraries(${PROJECT_NAME} PRIVATE indicators::indicators)

include(InstallRequiredSystemLibraries)
set(CPACK_PACKAGE_DIRECTORY ${CMAKE_BINARY_DIR}/package)
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "TacviewAnalyser")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/License")
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_MAJOR_VERSION}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_MINOR_VERSION}")
set(CPACK_GENERATOR "NSIS")
set(CPACK_SOURCE_GENERATOR "NSIS")

# Specify the installer script
set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "!include ${CMAKE_SOURCE_DIR}\\\\NSIS_inst_registry.nsh")
set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "!include ${CMAKE_SOURCE_DIR}\\\\NSIS_uninst_registry.nsh")

include(CPack)

# Install Boost::program_options, minizip, and zlib DLLs
install(TARGETS ${PROJECT_NAME})
install(FILES $<TARGET_FILE:Boost::program_options> DESTINATION bin)
install(FILES "${CMAKE_BINARY_DIR}/minizip.dll" DESTINATION bin)
install(FILES "License" DESTINATION .)


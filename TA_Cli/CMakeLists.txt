
project("TacViewAnalyser_cli")

find_package(argparse CONFIG REQUIRED)
find_package(indicators CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

# Add source to this project's executable.
add_executable (${PROJECT_NAME}   
		"${CMAKE_CURRENT_SOURCE_DIR}/source/main.cpp"
		"${PROJECT_ROOT_DIRECTORY}/app.rc"
)

target_include_directories(${PROJECT_NAME} PRIVATE 
		"${CMAKE_CURRENT_SOURCE_DIR}/include"
		"${TA_COM_DIRECTORY}/include"
		"${TA_LIB_DIRECTORY}/include"
)

target_link_libraries(${PROJECT_NAME} PRIVATE 
						TacViewAnalyser_lib_sta
						argparse::argparse
						indicators::indicators
						spdlog::spdlog
)

include(InstallRequiredSystemLibraries)
set(CPACK_PACKAGE_DIRECTORY ${CMAKE_BINARY_DIR}/package)
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VENDOR "CSakellaris Solutions")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "TacviewAnalyser")
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_ROOT_DIRECTORY}/License")
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_MAJOR_VERSION}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_MINOR_VERSION}")
set(CPACK_NSIS_MUI_ICON "${PROJECT_ROOT_DIRECTORY}\\\\res\\\\icon\\\\TacviewAnalyser_128p.ico")
set(CPACK_NSIS_MUI_UNIICON "${PROJECT_ROOT_DIRECTORY}\\\\res\\\\icon\\\\Uninstall.ico")
set(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\TacviewAnalyser.exe")
# set(CPACK_NSIS_MUI_HEADERIMAGE "${CMAKE_SOURCE_DIR}\\\\res\\\\icon\\\\TacviewAnalyser.png")
set(CPACK_GENERATOR "NSIS")
set(CPACK_SOURCE_GENERATOR "NSIS")

# Specify the installer script
set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "!include ${CMAKE_CURRENT_SOURCE_DIR}\\\\NSIS_inst_registry.nsh")
set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "!include ${CMAKE_CURRENT_SOURCE_DIR}\\\\NSIS_uninst_registry.nsh")

include(CPack)

# Install Boost::program_options, minizip, and zlib DLLs
install(TARGETS ${PROJECT_NAME})
install(FILES "${CMAKE_BINARY_DIR}/minizip.dll" DESTINATION bin)
install(FILES "License" DESTINATION .)
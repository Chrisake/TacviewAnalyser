project("TacViewAnalyser_lib")

set(PROJECT_MAJOR_VERSION 0)
set(PROJECT_MINOR_VERSION 1)

add_library(${PROJECT_NAME} STATIC
	"${CMAKE_CURRENT_SOURCE_DIR}/source/tac_file.cpp" 
	"${CMAKE_CURRENT_SOURCE_DIR}/source/sim_object.cpp"
)

target_include_directories(${PROJECT_NAME} PRIVATE 
	"${CMAKE_CURRENT_SOURCE_DIR}/include"
	"${TA_COM_DIRECTORY}/include"
)

find_package(argparse CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(indicators CONFIG REQUIRED)
find_package(minizip CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE 
						argparse::argparse
						indicators::indicators
						minizip::minizip
						spdlog::spdlog
						TacViewAnalyser_com_sta
)

add_library(${PROJECT_NAME}_sta ALIAS ${PROJECT_NAME})
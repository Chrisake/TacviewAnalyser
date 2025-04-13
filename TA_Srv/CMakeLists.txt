project("TacViewAnalyser_Server")

set(PROJECT_MAJOR_VERSION 0)
set(PROJECT_MINOR_VERSION 1)

add_executable (${PROJECT_NAME}   
    "${CMAKE_CURRENT_SOURCE_DIR}/source/main.cpp"
)

target_include_directories(${PROJECT_NAME} PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

find_package(argparse CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE 
						argparse::argparse
						spdlog::spdlog
						TacViewAnalyser_com_sta
)
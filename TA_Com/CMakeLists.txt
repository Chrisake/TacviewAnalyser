project("TacViewAnalyser_com")

find_package(spdlog CONFIG REQUIRED)
find_package(minizip CONFIG REQUIRED)

add_library(${PROJECT_NAME} STATIC
	"${CMAKE_CURRENT_SOURCE_DIR}/src/utils.cpp"
  "src/logging.cpp")

add_library(${PROJECT_NAME}_sta ALIAS ${PROJECT_NAME})


target_include_directories(${PROJECT_NAME} PRIVATE 
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(${PROJECT_NAME} PRIVATE 
  spdlog::spdlog
	minizip::minizip
)
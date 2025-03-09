# CMakeList.txt : CMake project for TacViewerAnalyser, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("TacViewerAnalyser")

find_package(boost_program_options REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(minizip REQUIRED) 
find_package(indicators REQUIRED)

# Add source to this project's executable.
add_executable (${PROJECT_NAME}   "src/main.cpp" "src/cache.cpp" "src/sim_object.cpp" "src/tac_file.cpp" "src/utils.cpp" "src/tac_file_load.cpp" "src/tac_file_load2.cpp")

target_include_directories(${PROJECT_NAME} PRIVATE "include")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE Boost::program_options)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(${PROJECT_NAME} PRIVATE minizip::minizip)
target_link_libraries(${PROJECT_NAME} PRIVATE indicators::indicators)

# TODO: Add tests and install targets if needed.

include("${CMAKE_SOURCE_DIR}/cmake/utils.cmake")

set(CMAKE_CXX_STANDARD			23 )
set(CMAKE_CXX_STANDARD_REQUIRED ON )
set(CMAKE_CXX_EXTENSIONS		OFF)

message(STATUS "Compiler C   ID: ${CMAKE_C_COMPILER_ID}")
message(STATUS "Compiler CXX ID: ${CMAKE_CXX_COMPILER_ID}")

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  message(STATUS "Clang compiler detected.")
  set(CLANG_CMPL true)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  message(STATUS "GCC compiler detected.")
  set(GCC_CMPL true)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  message(STATUS "MSVC compiler detected.")
  set(MSVC_CMPL true)
endif()

# Enable Hot Reload for MSVC compilers if supported.
if (MSVC AND POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# Disable Clatter vcpkg logs
set(VCPKG_INSTALL_OPTIONS "--no-print-usage")

# Enable All Warnings and treat them as errors.
if (CLANG_CMPL)
  add_compile_options(-Wall -Wextra -pedantic -Werror)
elseif (GCC_CMPL)
  add_compile_options(-Wall -Wextra -pedantic -Werror)
elseif (MSVC_CMPL)
  add_compile_options(/W4 /WX)
endif()



# Compiler Flags Setup
if (CLANG_CMPL)
elseif (GCC_CMPL)
elseif (MSVC_CMPL)
  remove_flag(CMAKE_CXX_FLAGS "/W3")
  remove_flag(CMAKE_CXX_FLAGS_RELEASE "/Ob2")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Ob3")
endif()

message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS_DEBUG: ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CMAKE_CXX_FLAGS_RELEASE: ${CMAKE_CXX_FLAGS_RELEASE}")
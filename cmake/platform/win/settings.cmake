# Platform-specfic options

# check the CMake preload parameters (commented out by default)
add_definitions(-DWIN32_LEAN_AND_MEAN)
add_definitions(-DNOMINMAX)

#WorkAround CMAKE BUG after 3.3
if (WIN32 AND CMAKE_SYSTEM_VERSION MATCHES 10)
  SET(CMAKE_SYSTEM_VERSION "A.0")
endif()

set(BIN_DIR    "${CMAKE_BINARY_DIR}/bin")
set(LIBS_DIR   "${CMAKE_BINARY_DIR}/lib/${PLATFORM_NAME}_${CMAKE_BUILD_TYPE}") 

if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
  include(${CMAKE_SOURCE_DIR}/cmake/compiler/msvc/settings.cmake)
elseif (${CMAKE_CXX_PLATFORM_ID} MATCHES MinGW)
  include(${CMAKE_SOURCE_DIR}/cmake/compiler/mingw/settings.cmake)
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL Clang)
  include(${CMAKE_SOURCE_DIR}/cmake/compiler/clang/settings.cmake)
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL Intel)
  include(${CMAKE_SOURCE_DIR}/cmake/compiler/intel/settings.cmake)
ELSE()
    MESSAGE(FATAL_ERROR "Unable detect compiler ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_PLATFORM_ID}")
endif()

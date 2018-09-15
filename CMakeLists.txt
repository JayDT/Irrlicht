cmake_minimum_required(VERSION 3.6 FATAL_ERROR)
set(CMAKE_CXX_STANDARD 14)
project(IrrlichtDV VERSION 0.0.1 LANGUAGES CXX C)

message("")
message("Configuration:")
message("")

configure_file(buildin_data.h.in.cmake buildin_data.h)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake")
set( CMAKE_VERBOSE_MAKEFILE on )

set(CMAKE_MODULE_PATH
  ${CMAKE_MODULE_PATH}
  ${CMAKE_SOURCE_DIR}/cmake/macros
  ${CMAKE_SOURCE_DIR}/cmake/extensions
)
if (UNIX)
  # CMake will try to find -lpthreads and fail, these two lines fix it
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-narrowing -pthread")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-narrowing -pthread")
endif(UNIX)

# set default buildoptions and print them
include(cmake/defines.cmake)
include(cmake/options.cmake)
include(Macros)
include(CheckBuildDir)
include(CheckCXXSourceRuns)
include(CheckIncludeFiles)
include(CheckPlatform)
include(HlslUtils)
# Find revision ID and hash of the sourcetree
include(cmake/genrev.cmake)

if (NOT DEBUG AND INSTEX)
    include(CheckSSEFeatureSet)
    message(STATUS "Compiler Flags ${CMAKE_CXX_FLAGS}")
endif()

message("")
message("Information:")
message("------------------------------------------------------------------------------------------------------------------")
message("${PROJECT_NAME} revision : ${rev_hash}") # ${GIT_REVISION}
message("Core rev. date           : ${rev_date}")
message("Install prefix           : ${CMAKE_INSTALL_PREFIX}")
message("Install server to        : ${BIN_DIR}")
message("Install libaries to      : ${LIBS_DIR}")
message("Source Dir               : ${CMAKE_SOURCE_DIR}") 
message("------------------------------------------------------------------------------------------------------------------")
message("")
message("")
 
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH ${LIBS_DIR})
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

#######################################################################
# Find all necessary
#######################################################################

message(STATUS "Prepare Dependencies:")

IF (BUILD_EXAMPLES)
    include(ExampleBuild REQUIRED)
ENDIF()
include(FindPkgMacros REQUIRED)
include(Dependencies REQUIRED)

message(STATUS "Complete Dependencies")
message("")
message("")

enable_testing()

message(STATUS "Prepare Source:")
ADD_SUBDIRECTORY(dep)
ADD_SUBDIRECTORY(Irrlicht)
ADD_SUBDIRECTORY(examples)
message(STATUS "Complete Source")
message("")
message("")

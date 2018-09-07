add_subdirectory(glm)
add_subdirectory(jpeglib)
add_subdirectory(libpng)
add_subdirectory(zlib)

IF (BUILD_IN_USE_GLSLANG)

# Set Glslang settings and connections
SET(SPIRV-Headers_SOURCE_DIR ${CMAKE_SOURCE_DIR}/dep/SPIRV/SPIRV-Headers)

add_subdirectory(SPIRV/SPIRV-Tools)
add_subdirectory(SPIRV/Glslang)

ENDIF(BUILD_IN_USE_GLSLANG)
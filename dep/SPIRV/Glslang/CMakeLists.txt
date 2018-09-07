option(ENABLE_SPVREMAPPER "Enables building of SPVRemapper" ON)
option(ENABLE_AMD_EXTENSIONS "Enables support of AMD-specific extensions" ON)
option(ENABLE_GLSLANG_BINARIES "Builds glslangValidator and spirv-remap" OFF)
option(ENABLE_NV_EXTENSIONS "Enables support of Nvidia-specific extensions" ON)
option(ENABLE_HLSL "Enables HLSL input support" ON)
option(ENABLE_OPT "Enables spirv-opt capability if present" OFF) 
    
findpkg_begin(Glslang)

set(Glslang_INCLUDE_DIR 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/glslang/Public
    ${CMAKE_CURRENT_SOURCE_DIR}/SPIRV
)

set(Glslang_LIBRARY glslang SPIRV SPVRemapper HLSL OSDependent SPIRV-Tools)

findpkg_finish(Glslang)

if (NOT Glslang_FOUND)
    message(FATAL_ERROR "Missing Glslang")
endif(NOT Glslang_FOUND)

if(ENABLE_AMD_EXTENSIONS)
    add_definitions(-DAMD_EXTENSIONS)
endif(ENABLE_AMD_EXTENSIONS)
    
if(ENABLE_NV_EXTENSIONS)
    add_definitions(-DNV_EXTENSIONS)
endif(ENABLE_NV_EXTENSIONS)
    
if(ENABLE_HLSL)
    add_definitions(-DENABLE_HLSL)
endif(ENABLE_HLSL)

add_definitions(-DENABLE_OPT)

set(spirv-tools_SOURCE_DIR "${CMAKE_SOURCE_DIR}/dep/SPIRV/SPIRV-Tools")
set(ENABLE_OPT TRUE)
    

#ADD_DEFINITIONS(-DENABLE_GLSLANG_INSTALL=ON)
add_subdirectory(glslang)
add_subdirectory(OGLCompilersDLL) 
add_subdirectory(SPIRV)
if(ENABLE_HLSL)
    add_subdirectory(hlsl)
endif(ENABLE_HLSL)

AddInterfaceLibrary(Glslang)
include(HlslUtils)

set(_IRR_DYNAMIC_ false)

if(_IRR_DYNAMIC_)
  set(LIBTYPE SHARED)
else(_IRR_DYNAMIC_)
 set(LIBTYPE STATIC)
endif(_IRR_DYNAMIC_)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${DirectX11_INCLUDE}
)

file(GLOB_RECURSE Direct3D11_Source RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp *.c *.h *.hpp *.hlsl)
setup_hlsl(${Direct3D11_Source})
file(GLOB_RECURSE RESOURCE_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.hlsl)

if (BUILD_HAS_RESOURCE_SYSTEM)
    resource_prebuild(
        TARGET "Direct3D11"
        GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/.Resource"
        SOURCE_ROOT "${CMAKE_CURRENT_LIST_DIR}"
        RESOURCE_FILES ${RESOURCE_SRCS}
        OUT_MODULE_SOURCE RESOURCE_MODULE_SOURCE
        OUT_GENERATED_FILES RESOURCE_GENERATED_FILES
        OUT_SRC RESOURCE_GENERATED_SOURCES
    )
    
    LIST(APPEND Direct3D11_Source ${RESOURCE_GENERATED_FILES})
endif (BUILD_HAS_RESOURCE_SYSTEM)

CreateLibrary("Direct3D11" "${Direct3D11_Source}" "${LIBTYPE}" "" "")
add_dependencies(Direct3D11 ZLIB)

target_link_libraries (Direct3D11 
    ${DirectX11_LIBRARY}
    Winmm 
    ZLIB
)

if (BUILD_HAS_RESOURCE_SYSTEM)
    target_link_libraries (Direct3D11
        CFramework)

    resource_build(
        TARGET "Direct3D11"
        GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/.Resource"
        SOURCE_ROOT "${CMAKE_CURRENT_LIST_DIR}"
        SOURCE_FILE "${CMAKE_CURRENT_BINARY_DIR}/.Resource/Resource_Direct3D11.dsc"
        MODULE_SOURCE_FILE ${RESOURCE_MODULE_SOURCE}
        RESOURCE_FILES ${RESOURCE_SRCS}
        GENERATED_FILES ${RESOURCE_GENERATED_FILES}
        RESOURCE_EXECUTABLE ${ResourceCompilerExecutable}
    )
endif (BUILD_HAS_RESOURCE_SYSTEM)

if(_IRR_DYNAMIC_)
    target_link_libraries (Direct3D11 
        irrlicht
    )
    install(TARGETS Direct3D11 DESTINATION ${CMAKE_INSTALL_PREFIX})
else()
    add_definitions(-D_LIB_SOURCE_ROOT="${CMAKE_CURRENT_SOURCE_DIR}")
    add_definitions(-D_IRR_COMPILE_WITH_DIRECT3D_11_)
    add_definitions(-DIRRLICHT_EXPORTS)

    target_link_libraries (Direct3D11 LINK_INTERFACE_LIBRARIES
        irrlicht
    )
endif(_IRR_DYNAMIC_)

set_target_properties(Direct3D11
    PROPERTIES
      FOLDER
        "RenderEngine/IrrLicht")
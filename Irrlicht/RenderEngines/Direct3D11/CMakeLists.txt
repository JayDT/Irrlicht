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

file(GLOB_RECURSE Direct3D11_Source RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp *.c *.h *.hpp)
setup_hlsl(${Direct3D11_Source})
file(GLOB_RECURSE RESOURCE_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.hlsl)

CreateLibrary("Direct3D11" "${Direct3D11_Source}" "${LIBTYPE}" "" "")
add_dependencies(Direct3D11 ZLIB)

target_link_libraries (Direct3D11 
	CFramework
    ${DirectX11_LIBRARY}
    Winmm 
    ZLIB)

resource_build(
    TARGET "Direct3D11"
    GENERATED_DIR "${CMAKE_BINARY_DIR}"
    SOURCE_ROOT "${CMAKE_CURRENT_LIST_DIR}"
    SOURCE_FILE "${CMAKE_CURRENT_BINARY_DIR}/.Resource/Resource_Direct3D11.dsc"
    MODULE_SOURCE_FILE "Direct3D11"
    RESOURCE_FILES ${RESOURCE_SRCS}
    GENERATED_FILES ${RESOURCE_GENERATED_FILES}
    RESOURCE_EXECUTABLE ${ResourceCompilerExecutable}
)

SET_PROPERTY(GLOBAL APPEND PROPERTY ResxDataFiles "Direct3D11.resx")

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
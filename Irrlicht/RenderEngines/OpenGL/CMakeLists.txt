set(_IRR_DYNAMIC_ false)

if(_IRR_DYNAMIC_)
  set(LIBTYPE SHARED)
else(_IRR_DYNAMIC_)
 set(LIBTYPE STATIC)
endif(_IRR_DYNAMIC_)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    #${OPENGL_INCLUDE_DIRS}
)

file(GLOB_RECURSE OpenGL_Source RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp *.c *.h *.hpp *.vert *.frag)
file(GLOB_RECURSE RESOURCE_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.vert *.frag)

CreateLibrary(IrrOpenGL "${OpenGL_Source}" "${LIBTYPE}" "" "")
add_dependencies(IrrOpenGL ZLIB)
target_link_libraries (IrrOpenGL
    OpenGL::GL
    ZLIB
    Glslang)

target_link_libraries (IrrOpenGL
    CFramework)

resource_build(
    TARGET "IrrOpenGL"
    GENERATED_DIR "${CMAKE_BINARY_DIR}"
    SOURCE_ROOT "${CMAKE_CURRENT_LIST_DIR}"
    SOURCE_FILE "${CMAKE_CURRENT_BINARY_DIR}/.Resource/Resource_OpenGL.dsc"
    MODULE_SOURCE_FILE IrrOpenGL
    RESOURCE_FILES ${RESOURCE_SRCS}
    GENERATED_FILES ${RESOURCE_GENERATED_FILES}
    RESOURCE_EXECUTABLE ${ResourceCompilerExecutable}
)

SET_PROPERTY(GLOBAL APPEND PROPERTY ResxDataFiles "IrrOpenGL.resx")

if(_IRR_DYNAMIC_)
    target_link_libraries (IrrOpenGL
        irrlicht)
    IF (WIN32)
        target_link_libraries (IrrOpenGL winmm)
    ENDIF(WIN32)
    install(TARGETS IrrOpenGL DESTINATION ${CMAKE_INSTALL_PREFIX})
else()
    add_definitions(-D_LIB_SOURCE_ROOT="${CMAKE_CURRENT_SOURCE_DIR}")
    add_definitions(-D_IRR_COMPILE_WITH_OPENGL_)
    add_definitions(-DIRRLICHT_EXPORTS)
    
    target_link_libraries (IrrOpenGL LINK_INTERFACE_LIBRARIES
        irrlicht
    )
endif(_IRR_DYNAMIC_)

set_target_properties(IrrOpenGL
    PROPERTIES
      FOLDER
        "RenderEngine/IrrLicht")

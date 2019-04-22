set(EXECUTABLE_NAME "33.NoesisGUIWindow")
SET(EXECUTABLE_LINK_LIBS "")
SET(EXECUTABLE_LINK_FLAGS ${CMAKE_EXE_LINKER_FLAGS_DEBUG})
SET(MODUL_NAME ${EXECUTABLE_NAME})

list(APPEND EXECUTABLE_LINK_LIBS
PRIVATE
	CFramework
    irrlicht
    NoesisApp
 ) 
IF(WITHJEM GREATER 0)
    list(APPEND EXECUTABLE_LINK_LIBS jemalloc)
ENDIF(WITHJEM GREATER 0)

file(GLOB_RECURSE EXECUTABLE_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp *.h *.xaml *.rc *.json)
file(GLOB_RECURSE RESOURCE_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.xaml *.ttf *.json)

group_source(${EXECUTABLE_SRCS})
#AppendDefaultIncludeDir()

CreateExecutable("${EXECUTABLE_NAME}" "${EXECUTABLE_SRCS}" "${EXECUTABLE_LINK_LIBS}" "${EXECUTABLE_LINK_FLAGS}")
CreateCxxPch("${EXECUTABLE_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/StdAfx.h" "${CMAKE_CURRENT_SOURCE_DIR}/StdAfx.cpp")
add_dependencies(${EXECUTABLE_NAME} noesisgui)

target_include_directories(${EXECUTABLE_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${PROTOBUF_INCLUDE_DIRS}
        ${CMAKE_BINARY_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}/.ProtoStructs
)

resource_build(
    TARGET ${MODUL_NAME}
    GENERATED_DIR "${CMAKE_BINARY_DIR}"
    SOURCE_ROOT "${CMAKE_CURRENT_LIST_DIR}"
    SOURCE_FILE "${CMAKE_CURRENT_BINARY_DIR}/.Resource/Resource_${MODUL_NAME}.dsc"
    MODULE_SOURCE_FILE ${EXECUTABLE_NAME}
    RESOURCE_FILES ${RESOURCE_SRCS}
    RESOURCE_EXECUTABLE ${ResourceCompilerExecutable}
)

SET_PROPERTY(GLOBAL APPEND PROPERTY ResxDataFiles "${EXECUTABLE_NAME}.resx")

IF (NOT EXISTS ${BIN_DIR}/debug/Data)
    file(MAKE_DIRECTORY ${BIN_DIR}/debug/Data)
ENDIF()

IF (NOT EXISTS ${BIN_DIR}/release/Data)
    file(MAKE_DIRECTORY ${BIN_DIR}/release/Data)
ENDIF()

add_custom_command(TARGET ${MODUL_NAME} POST_BUILD
	                COMMAND ${CMAKE_COMMAND} -E copy_if_different
	                    "${CMAKE_BINARY_DIR}/${EXECUTABLE_NAME}.resx"
	                    "$<TARGET_FILE_DIR:${MODUL_NAME}>/Data/${EXECUTABLE_NAME}.resx"
	                DEPENDS "${CMAKE_BINARY_DIR}/${EXECUTABLE_NAME}.resx")

# Fix me
get_target_property(NoesisGuiBinary noesisgui IMPORTED_LOCATION)
add_custom_command(TARGET ${MODUL_NAME} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${NoesisGuiBinary}"
                        "$<TARGET_FILE_DIR:${MODUL_NAME}>/Noesis.dll"
                    DEPENDS "${NoesisGuiBinary}")

set_target_properties(${MODUL_NAME}
    PROPERTIES
      FOLDER
        "Example")
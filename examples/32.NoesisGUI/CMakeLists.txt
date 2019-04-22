set(EXECUTABLE_NAME "32.NoesisGUI")
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

file(GLOB_RECURSE EXECUTABLE_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp *.h *.xaml)

group_source(${EXECUTABLE_SRCS})
#AppendDefaultIncludeDir()

file(GLOB_RECURSE RESOURCE_SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.xaml *.ttf *.png *.jpg *.hlsl)

LIST(APPEND EXECUTABLE_SRCS ${RESOURCE_GENERATED_FILES})

CreateExecutable("${EXECUTABLE_NAME}" "${EXECUTABLE_SRCS}" "${EXECUTABLE_LINK_LIBS}" "${EXECUTABLE_LINK_FLAGS}")
CreateCxxPch("${EXECUTABLE_NAME}" "${CMAKE_CURRENT_SOURCE_DIR}/StdAfx.h" "${CMAKE_CURRENT_SOURCE_DIR}/StdAfx.cpp")

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

# hack
GET_PROPERTY(ResxList GLOBAL PROPERTY ResxDataFiles)
foreach(item IN LISTS ResxList)

	add_custom_command(TARGET ${MODUL_NAME} POST_BUILD
	                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
	                        "${CMAKE_BINARY_DIR}/${item}"
	                        "$<TARGET_FILE_DIR:${MODUL_NAME}>/Data/${item}"
	                    DEPENDS "${CMAKE_BINARY_DIR}/${item}")
endforeach()

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